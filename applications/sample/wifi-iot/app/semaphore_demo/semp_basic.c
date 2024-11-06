// 互斥：某一资源在某一时刻只允许一个访问者对其进行访问，具有唯一性和排它性。但互斥无法限制访问者对资源的访问顺序，即访问是无序的。
// 同步：在互斥的基础上（在大多数情况下），通过其它机制实现访问者对资源的有序访问。
//
// 互斥锁与信号量的区别：
// 互斥锁用于线程的互斥，信号量用于线程的同步。这是互斥锁和信号量的根本区别，也是互斥和同步之间的区别。
// 互斥锁：控制多个线程对一个共享资源的访问，某一时刻只允许一个线程访问同一个共享资源。
// 信号量：控制多个线程对多个共享资源的访问，某一时刻允许固定数量的线程访问共享资源池。
//
// 互斥锁与信号量的使用场合：
// 互斥锁：保护一个线程在关键区的操作不被其它线程打断。（对同一个资源的连续操作不被打断）
// 信号量：限制活跃线程的数量（控制一个过程的并发度），比如限流、限制一个房间的玩家在线数…
//
// 信号量机制：
// 信号量代表了一组有限资源的使用许可（tokens），多个线程通过得到这些许可来同时使用这一组资源，即“持证上岗”。
// tokens的数量，和资源的数量，保持一致。
// token <---> resource
// 信号量管理和维护这些tokens，并且提供获取和释放tokens的接口。
// acquire方法导致阻塞，直到有一个token可以获得，然后拿走一个token；
// release方法增加一个token，这可能会释放一个阻塞的acquire方法。
// 即：拿到token的线程就可以访问资源，如果tokens被拿完了，线程就会被放入等待队列。
// 然而，其实并没有实际的token这个对象，信号量只是维持了一个可获得tokens的数量。

// 例：
// 一个工厂有 4 台机器，但是有 7 个工人，一台机器某一时刻只能被一个工人使用，只有使用完了，其他工人才能继续使用。
// 也就是同时最多只能有4个人在干活（最大并发度为4）。可以通过信号量来实现。

#include <stdio.h>      // 标准输入输出
#include <unistd.h>     // POSIX标准接口
#include <string.h>     // 字符串处理(操作字符数组)
#include <malloc.h>     // 内存分配

#include "ohos_init.h"  // 用于初始化服务(services)和功能(features)
#include "cmsis_os2.h"  // CMSIS-RTOS API V2

// 工人的数量
#define WORKER_NUMBER 7     //3861实测最多开8个线程，主线程占用1个，剩余7个线程
// 机器的数量
#define MACHINE_NUMBER 4
// 信号量：机器
osSemaphoreId_t semMachine;

// 工人线程函数
void worker_thread(void *arg)
{
    (void)arg;
    while (1)
    {
        // 获取指定信号量的一个令牌（token），若获取失败（获取不到），则等待
        // osWaitForever：永远等待，不会超时
        // 工人线程先获取semMachine的一个令牌，来确认是否有空闲的机器供使用
        // 如果没有空闲的机器，则工人线程进入等待状态，直到有空闲的机器
        osSemaphoreAcquire(semMachine, osWaitForever);
        // 上面的函数是阻塞式的，如果执行完毕，说明有空闲机器了，工人开始使用机器
        // 输出日志
        printf("%s GOT a machine!\r\n", osThreadGetName(osThreadGetId()));
        // 模拟使用机器的时间
        osDelay(100);
        // 输出日志
        printf("%s RETURN a machine!\r\n", osThreadGetName(osThreadGetId()));
        // 释放（归还）指定信号量的一个令牌，这样其它工人线程就可以继续使用机器了
        // 令牌的数量，不超过信号量可以容纳的令牌的最大数量
        osSemaphoreRelease(semMachine);
    }
}

// 创建线程，返回线程ID。封装成一个函数，便于调用
// name：线程名称
// func：线程函数
// arg：线程函数的参数
osThreadId_t newThread(char *name, osThreadFunc_t func, void *arg)
{
    //把name参数在内存中建立一个拷贝，供新创建的线程使用
    char *threadName = (char *)malloc(strlen(name) + 1);
    strncpy(threadName, name, strlen(name) + 1);

    osThreadAttr_t attr = {
        threadName, 0, NULL, 0, NULL, 1024 * 2, osPriorityNormal, 0, 0};
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

    // 延迟1秒，避免跟系统输出混淆在一起
    osDelay(100);

    // 创建并且初始化一个信号量semMachine，最多5个令牌，初始5个令牌
    // 令牌的数量，可以理解为空闲的机器的数量。线程拿到令牌，使用机器，使用完了，线程归还令牌
    // max_count：信号量可以容纳的令牌的最大数量
    // initial_count：信号量容纳的令牌的初始数量
    // attr：信号量属性，目前没有用到
    semMachine = osSemaphoreNew(MACHINE_NUMBER, MACHINE_NUMBER, NULL);

    // 创建工人线程（方式1）
    for (int i = 0; i < WORKER_NUMBER; i++)
    {
        char tname[64] = "";
        snprintf(tname, sizeof(tname), "worker%d", i);
        newThread(tname, worker_thread, NULL);
        osDelay(50);
    }
    // 创建工人线程（方式2）
    // newThread("worker1", worker_thread, NULL);
    // newThread("worker2", worker_thread, NULL);
    // newThread("worker3", worker_thread, NULL);
    // newThread("worker4", worker_thread, NULL);
    // newThread("worker5", worker_thread, NULL);
    // newThread("worker6", worker_thread, NULL);
    // newThread("worker7", worker_thread, NULL);

    // osDelay(50);
    // osSemaphoreDelete(empty_id);
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
