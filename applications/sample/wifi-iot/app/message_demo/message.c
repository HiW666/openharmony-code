// 消息（Message）是一个自定义的数据或数据结构，可以是一个整数，一个字符串，一个指针，一个结构体等等。
// 消息队列（MessageQueue）提供了一个先进先出（First In First Out，FIFO）的消息传递机制，可以用来实现线程间的通信。
// 消息队列的使用者可以在消息队列中放入一个或多个消息，也可以从消息队列中取出一个或多个消息。
// 消息队列的使用者可以通过消息队列的接口来控制消息队列的大小，以及消息队列的消息的最大长度。
// 消息的获取者总是从消息队列的队首取出消息，而消息的放置者总是把消息放置在消息队列的队尾。
//

#include <stdio.h>      // 标准输入输出
#include <unistd.h>     // POSIX标准接口

#include "ohos_init.h"  // 用于初始化服务(services)和功能(features)
#include "cmsis_os2.h"  // CMSIS-RTOS API V2

// 消息队列的大小
#define QUEUE_SIZE 3

// 定义消息的数据结构
typedef struct
{
    osThreadId_t tid;
    int count;
} message_entry;

// 保存消息队列的ID
osMessageQueueId_t qid;

// 消息发送者线程函数
void sender_thread(void *arg)
{
    (void)arg;
    // 一个公共计数器
    static int count = 0;
    // 定义一个消息
    message_entry sentry;
    // 开始工作循环
    while (1)
    {
        // 将当前线程的ID放入消息中
        sentry.tid = osThreadGetId();

        // 将公共计数器的值放入消息中
        sentry.count = count;

        // 输出日志
        printf("[Message Test] %s send %d to message queue.\r\n", osThreadGetName(osThreadGetId()), count);

        // 将消息放入消息队列中
        // 如果消息队列已满，则会等待消息队列的空位，直到消息队列有空位，才会放入消息
        //
        // 参数:
        // mq_id：通过osMessageQueueNew拿到的消息队列ID
        // msg_ptr：要放入消息队列的消息的指针
        // msg_prio：要放入消息队列的消息的优先级，当前未使用
        // timeout：超时时间
        // 返回:
        // CMSIS-RTOS运行结果，参考kernel\liteos_m\kal\cmsis\cmsis_os2.h
        //
        // 注意：
        // 第二个参数msg_ptr是要放入消息队列的消息的指针，在消息进队列的时候，是复制一份进去的
        // 可以跟一下源码：osMessageQueuePut->LOS_QueueWriteCopy->OsQueueOperate->OsQueueBufferOperate->memcpy_s()
        // 所以循环使用sentry是不会有问题的
        //
        // 可以不去拿返回值，但是这样做就无法确保消息发出去了
        osMessageQueuePut(qid, (const void *)&sentry, 0, osWaitForever);

        // 每发送一次消息，计数器加1
        count++;
        osDelay(5);
    }
}

// 消息接收者线程函数
void receiver_thread(void *arg)
{
    (void)arg;
    // 定义一个消息
    message_entry rentry;
    // 开始工作循环
    while (1)
    {
        // 从消息队列中取出消息
        // 如果消息队列为空，则会等待消息队列的消息，直到消息队列有消息，才会取出消息
        // 
        // 参数:
        // mq_id：通过osMessageQueueNew拿到的消息队列ID
        // msg_ptr：一个消息的指针，用于接收从消息队列中取出的消息
        // msg_prio：一个消息优先级的指针，用于接收从消息队列中取出的消息优先级，当前未使用，传入NULL即可
        // timeout：超时时间
        // 返回:
        // CMSIS-RTOS运行结果，参考kernel\liteos_m\kal\cmsis\cmsis_os2.h
        osMessageQueueGet(qid, (void *)&rentry, NULL, osWaitForever);

        // 输出日志
        printf("[Message Test] %s get %d from %s by message queue.\r\n", osThreadGetName(osThreadGetId()), rentry.count, osThreadGetName(rentry.tid));
        osDelay(3);
    }
}

// 创建线程，返回线程ID。封装成一个函数，便于调用
osThreadId_t newThread(char *name, osThreadFunc_t func, void *arg)
{
    osThreadAttr_t attr = {
        name, 0, NULL, 0, NULL, 1024 * 2, osPriorityNormal, 0, 0};
    osThreadId_t tid = osThreadNew(func, arg, &attr);
    if (tid == NULL)
    {
        printf("[Message Test] osThreadNew(%s) failed.\r\n", name);
    }
    else
    {
        printf("[Message Test] osThreadNew(%s) success, thread id: %d.\r\n", name, tid);
    }
    return tid;
}

// 主线程函数
void rtosv2_msgq_main(void *arg)
{
    (void)arg;

    // 创建并且初始化一个消息队列
    // 参数:
    // msg_count：消息队列中可以容纳的消息的最大数量
    // msg_size：消息队列中每条消息的最大长度
    // attr：消息队列属性。当前未使用，可以传入NULL。
    // 返回:
    // 成功返回消息队列的ID，失败返回NULL。
    qid = osMessageQueueNew(QUEUE_SIZE, sizeof(message_entry), NULL);

    // 创建两个消息接收者线程，三个消息发送者线程
    osThreadId_t ctid1 = newThread("recevier1", receiver_thread, NULL);
    osThreadId_t ctid2 = newThread("recevier2", receiver_thread, NULL);
    osThreadId_t ptid1 = newThread("sender1", sender_thread, NULL);
    osThreadId_t ptid2 = newThread("sender2", sender_thread, NULL);
    osThreadId_t ptid3 = newThread("sender3", sender_thread, NULL);

    // 等待一段时间（让消息发送者线程和消息接收者线程都运行一段时间）
    osDelay(100);

    // 获取消息队列中可以容纳的消息的最大数量
    uint32_t cap = osMessageQueueGetCapacity(qid);
    printf("[Message Test] osMessageQueueGetCapacity, capacity: %d.\r\n", cap);
    // 获取消息队列中每条消息的最大长度
    uint32_t msg_size = osMessageQueueGetMsgSize(qid);
    printf("[Message Test] osMessageQueueGetMsgSize, size: %d.\r\n", msg_size);
    // 获取消息队列中当前的消息数量
    uint32_t count = osMessageQueueGetCount(qid);
    printf("[Message Test] osMessageQueueGetCount, count: %d.\r\n", count);
    // 获取消息队列中当前还可以放置的消息数量
    uint32_t space = osMessageQueueGetSpace(qid);
    printf("[Message Test] osMessageQueueGetSpace, space: %d.\r\n", space);

    osDelay(80);

    // 终止线程
    osThreadTerminate(ctid1);
    osThreadTerminate(ctid2);
    osThreadTerminate(ptid1);
    osThreadTerminate(ptid2);
    osThreadTerminate(ptid3);

    // 删除消息队列
    osMessageQueueDelete(qid);
}

// 入口函数
static void MessageTestTask(void)
{
    osThreadAttr_t attr;

    attr.name = "rtosv2_msgq_main";
    attr.attr_bits = 0U;
    attr.cb_mem = NULL;
    attr.cb_size = 0U;
    attr.stack_mem = NULL;
    attr.stack_size = 1024;
    attr.priority = osPriorityNormal;

    if (osThreadNew((osThreadFunc_t)rtosv2_msgq_main, NULL, &attr) == NULL)
    {
        printf("[MessageTestTask] Falied to create rtosv2_msgq_main!\n");
    }
}

// 运行入口函数
APP_FEATURE_INIT(MessageTestTask);
