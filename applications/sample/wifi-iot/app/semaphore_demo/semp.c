// 经典的生产者与消费者问题
// 多个生产者，多个消费者
// 生产者生产共享资源（产品），消费者消费共享资源（产品）
// 需要确保：
// 1.当产品仓库满时，让继续生产共享资源（产品）的生产者线程进入等待状态
// 2.当共享资源（产品）在被消费者线程消费完之后，让消费新产品的消费者线程进入等待状态
// 可以通过定义一对信号量，来解决这个问题

#include <stdio.h>
#include <unistd.h>

#include "ohos_init.h"
#include "cmsis_os2.h"

// 产品仓库最大容量
#define BUFFER_SIZE 5U
// 产品数量，初始的时候，仓库中没有产品
static int product_number = 0;
// 信号量1：仓库中当前空闲的产品位置的数量
osSemaphoreId_t empty_id;
// 信号量2：当前已经生产好的产品的数量
osSemaphoreId_t filled_id;

// 生产者线程函数
void producer_thread(void *arg)
{
    (void)arg;
    // empty_id = osSemaphoreNew(BUFFER_SIZE, BUFFER_SIZE, NULL);
    // filled_id = osSemaphoreNew(BUFFER_SIZE, 0U, NULL);
    while (1)
    {
        // 获取指定信号量的一个令牌，若获取失败（获取不到），则等待
        // osWaitForever：永远等待，不会超时
        // 生产者先获取empty_id，来确认是否有空闲的产品位置，来存放新生产的产品
        // 如果没有空闲的位置，则生产者线程进入等待状态，直到有空闲的位置
        osSemaphoreAcquire(empty_id, osWaitForever);
        // 上面的函数是阻塞式的，如果执行完毕，说明有空闲位置了，生产者生产一个产品
        product_number++;
        printf("[Semp Test]%s produces a product, now product number: %d.\r\n", osThreadGetName(osThreadGetId()), product_number);
        osDelay(4);
        // 释放指定信号量的一个令牌（将生产好的产品放入）
        // 令牌的数量，不超过信号量可以容纳的令牌的最大数量
        osSemaphoreRelease(filled_id);
    }
}

// 消费者线程函数
void consumer_thread(void *arg)
{
    (void)arg;
    while (1)
    {
        // 获取指定信号量的一个令牌，若获取失败，则等待
        // 消费者先获取filled_id，来确认是否有产品供消费
        // 如果没有产品供消费，则消费者线程进入等待状态，直到有产品供消费
        osSemaphoreAcquire(filled_id, osWaitForever);
        // 上面的函数是阻塞式的，如果执行完毕，说明有产品供消费了，消费者消费一个产品
        product_number--;
        printf("[Semp Test]%s consumes a product, now product number: %d.\r\n", osThreadGetName(osThreadGetId()), product_number);
        osDelay(3);
        // 释放指定信号量的一个令牌（将产品位置空出一个）
        osSemaphoreRelease(empty_id);
    }
}

// 创建线程，返回线程ID。封装成一个函数，便于调用
// name：线程名称
// func：线程函数
// arg：线程函数的参数
osThreadId_t newThread(char *name, osThreadFunc_t func, void *arg)
{
    osThreadAttr_t attr = {
        name, 0, NULL, 0, NULL, 1024 * 2, osPriorityNormal, 0, 0};
    osThreadId_t tid = osThreadNew(func, arg, &attr);
    if (tid == NULL)
    {
        printf("[Semp Test]osThreadNew(%s) failed.\r\n", name);
    }
    else
    {
        printf("[Semp Test]osThreadNew(%s) success, thread id: %d.\r\n", name, tid);
    }
    return tid;
}

// 主线程函数
void rtosv2_semp_main(void *arg)
{
    (void)arg;

    // 创建并且初始化一个信号量empty_id，最多5个令牌，初始5个令牌
    // 令牌的数量，可以理解为空闲的产品位置的数量。线程拿到令牌，生产产品
    // max_count：信号量可以容纳的令牌的最大数量
    // initial_count：信号量容纳的令牌的初始数量
    // attr：信号量属性，目前没有用到
    empty_id = osSemaphoreNew(BUFFER_SIZE, BUFFER_SIZE, NULL);

    // 创建并且初始化一个信号量filled_id，最多5个令牌，初始0个令牌
    // 令牌的数量，可以理解为已经生产好的产品的数量。线程拿到令牌，消费产品
    filled_id = osSemaphoreNew(BUFFER_SIZE, 0U, NULL);

    // 创建3个生产者线程，2个消费者线程
    osThreadId_t ptid1 = newThread("producer1", producer_thread, NULL);
    osThreadId_t ptid2 = newThread("producer2", producer_thread, NULL);
    osThreadId_t ptid3 = newThread("producer3", producer_thread, NULL);
    osThreadId_t ctid1 = newThread("consumer1", consumer_thread, NULL);
    osThreadId_t ctid2 = newThread("consumer2", consumer_thread, NULL);

    // 等待一段时间（让生产者线程和消费者线程都运行一段时间）
    osDelay(500);

    // 终止所有线程
    osThreadTerminate(ptid1);
    osThreadTerminate(ptid2);
    osThreadTerminate(ptid3);
    osThreadTerminate(ctid1);
    osThreadTerminate(ctid2);

    // 删除信号量
    osSemaphoreDelete(empty_id);
    osSemaphoreDelete(filled_id);
}

// 入口函数
static void SempTestTask(void)
{
    // 线程属性
    osThreadAttr_t attr;
    attr.name = "rtosv2_semp_main";
    attr.attr_bits = 0U;
    attr.cb_mem = NULL;
    attr.cb_size = 0U;
    attr.stack_mem = NULL;
    attr.stack_size = 1024;
    attr.priority = osPriorityNormal;

    // 创建一个线程，并将其加入活跃线程组中
    if (osThreadNew((osThreadFunc_t)rtosv2_semp_main, NULL, &attr) == NULL)
    {
        printf("[SempTestTask] Falied to create rtosv2_semp_main!\n");
    }
}

// 运行入口函数
APP_FEATURE_INIT(SempTestTask);