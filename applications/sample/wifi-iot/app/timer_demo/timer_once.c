#include <stdio.h>      // 标准输入输出
#include <unistd.h>     // POSIX标准接口

#include "ohos_init.h"  // 用于初始化服务(services)和功能(features)
#include "cmsis_os2.h"  // CMSIS-RTOS API V2

// 定时器的回调函数
void cb_timeout(void *arg)
{
    (void)arg;
    printf("timeout!\n");
}

// 主线程函数
// 使用osTimerNew创建一个定时器，200个ticks后调用一次回调函数cb_timeout
// 但是100个ticks后就停止了定时器
void timer_once(void)
{

    // 延迟1秒，避免跟系统输出混淆在一起
    osDelay(100);

    // Creates and initializes a timer.
    // osTimerOnce：一次性的
    // osTimerPeriodic：周期性的
    osTimerId_t once_tid = osTimerNew(cb_timeout, osTimerOnce, NULL, NULL);
    if (once_tid == NULL)
    {
        printf("[Timer Test] osTimerNew(once timer) failed.\r\n");
        return;
    }
    else
    {
        printf("[Timer Test] osTimerNew(once timer) success, tid: %p.\r\n", once_tid);
    }

    // starts or restarts a timer specified by the parameter timer_id.
    // 200个ticks后调用一次回调函数。
    osStatus_t status = osTimerStart(once_tid, 200);
    if (status != osOK)
    {
        printf("[Timer Test] osTimerStart(once timer) failed.\r\n");
        return;
    }
    else
    {
        printf("[Timer Test] osTimerStart(once timer) success, wait a while and stop.\r\n");
    }

    // 等待100个ticks
    // 每10个ticks，使用osTimerIsRunning检查一下定时器是否还在运行
    for (int i = 0; i < 10; i++)
    {
        osDelay(10);
        printf("timer is running: %s\n", osTimerIsRunning(once_tid) == 1 ? "yes" : "no");
    }

    // stops a timer specified by the parameter timer_id.
    // 停止定时器
    status = osTimerStop(once_tid);
    printf("[Timer Test] stop once timer, status :%d.\r\n", status);

    // 检查定时器是否还在运行
    printf("timer is running: %s\n", osTimerIsRunning(once_tid) == 1 ? "yes" : "no");

    // 删除定时器
    status = osTimerDelete(once_tid);
    printf("[Timer Test] kill once timer, status :%d.\r\n", status);
}

// 入口函数
static void TimerTestTask(void)
{
    // 线程属性
    osThreadAttr_t attr;
    attr.name = "timer_once";
    attr.attr_bits = 0U;
    attr.cb_mem = NULL;
    attr.cb_size = 0U;
    attr.stack_mem = NULL;
    attr.stack_size = 1024;
    attr.priority = osPriorityNormal;

    // 创建一个线程，并将其加入活跃线程组中
    if (osThreadNew((osThreadFunc_t)timer_once, NULL, &attr) == NULL)
    {
        printf("[TimerTestTask] Falied to create timer_once!\n");
    }
}

// 运行入口函数
APP_FEATURE_INIT(TimerTestTask);