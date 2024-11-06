#include <stdio.h>      // 标准输入输出
#include <unistd.h>     // POSIX标准接口
#include "ohos_init.h"  // 用于初始化服务(services)和功能(features)
#include "cmsis_os2.h"  // CMSIS-RTOS API V2

// 全局变量g_test_value会同时被多个线程访问，当这些线程访问这个全局变量时，会将其加1，然后判断其奇偶性，并输出日志。
// 如果没有互斥锁的保护，在多线程的情况下，加1操作、判断奇偶性操作、输出日志操作之间可能会被其它线程中断，导致错误。
// 所以需要创建一个互斥锁，来保护这个多线程共享区域。
static int g_test_value = 0;

// 操作多线程共享区域的线程函数
// 将全局变量g_test_value加1，然后判断其奇偶性，并输出日志
void number_thread(void *arg)
{
    // 得到互斥锁ID
    osMutexId_t *mid = (osMutexId_t *)arg;

    while (1)
    {
        // 加锁（获取互斥锁/占用互斥锁）
        if (osMutexAcquire(*mid, 100) == osOK)
        {
            g_test_value++;
            if (g_test_value % 2 == 0)  //偶数
            {
                printf("[Mutex Test]%s gets an even value %d.\r\n", osThreadGetName(osThreadGetId()), g_test_value);
            }
            else
            {
                printf("[Mutex Test]%s gets an odd value %d.\r\n", osThreadGetName(osThreadGetId()), g_test_value);
            }
            // 解锁（释放互斥锁）
            osMutexRelease(*mid);

            // 处理工作告一段落，放弃对CPU的占用
            osDelay(5); //50ms
        }
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
        printf("[Mutex Test]osThreadNew(%s) failed.\r\n", name);
    }
    else
    {
        printf("[Mutex Test]osThreadNew(%s) success, thread id: %d.\r\n", name, tid);
    }
    return tid;
}

// 主线程函数
// 创建三个运行number_thread函数的线程，访问全局变量g_test_value，同时创建一个互斥锁供所有线程使用
void rtosv2_mutex_main(void *arg)
{
    (void)arg;

    // 互斥锁属性
    // const char *name：     互斥锁名称
    // uint32_t attr_bits：   保留的属性位，必须为0
    // void *cb_mem：         互斥锁控制块的内存初始地址，默认为系统自动分配
    // uint32_t cb_size：     互斥锁控制块的内存大小
    osMutexAttr_t attr = {0};

    // 创建互斥锁，拿到互斥锁的ID
    // attr参数目前没有用到
    // Returns the mutex ID; returns NULL in the case of an error.
    osMutexId_t mid = osMutexNew(&attr);
    if (mid == NULL)
    {
        printf("[Mutex Test]osMutexNew, create mutex failed.\r\n");
    }
    else
    {
        printf("[Mutex Test]osMutexNew, create mutex success.\r\n");
    }

    // 创建三个运行number_thread函数的线程，访问全局变量g_test_value
    // 把互斥锁ID传给number_thread函数
    // 保存线程ID，用于后续线程操作
    osThreadId_t tid1 = newThread("Thread_1", number_thread, &mid);
    osThreadId_t tid2 = newThread("Thread_2", number_thread, &mid);
    osThreadId_t tid3 = newThread("Thread_3", number_thread, &mid);

    osDelay(13);
    // 获得当前占用互斥锁的线程ID
    osThreadId_t tid = osMutexGetOwner(mid);
    // 打印当前占用互斥锁的线程ID和线程名称
    printf("[Mutex Test]osMutexGetOwner, thread id: %p, thread name: %s.\r\n", tid, osThreadGetName(tid));
    osDelay(17);

    // 终止线程
    osThreadTerminate(tid1);
    osThreadTerminate(tid2);
    osThreadTerminate(tid3);

    // 删除互斥锁
    osMutexDelete(mid);
}

// 入口函数
static void MutexTestTask(void)
{
    // 线程属性
    osThreadAttr_t attr;
    attr.name = "rtosv2_mutex_main";
    attr.attr_bits = 0U;
    attr.cb_mem = NULL;
    attr.cb_size = 0U;
    attr.stack_mem = NULL;
    attr.stack_size = 1024;
    attr.priority = osPriorityNormal;

    // 创建一个线程，并将其加入活跃线程组中
    if (osThreadNew((osThreadFunc_t)rtosv2_mutex_main, NULL, &attr) == NULL)
    {
        printf("[MutexTestTask] Falied to create rtosv2_mutex_main!\n");
    }
}

// 运行入口函数
APP_FEATURE_INIT(MutexTestTask);