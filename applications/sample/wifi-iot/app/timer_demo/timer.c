#include <stdio.h>      // 标准输入输出
#include <unistd.h>     // POSIX标准接口

#include "ohos_init.h"  // 用于初始化服务(services)和功能(features)
#include "cmsis_os2.h"  // CMSIS-RTOS API V2

// 全局变量，用于计数
static int times = 0;

// 定时器的回调函数
void cb_timeout_periodic(void *arg)
{
    (void)arg;
    times++;
    printf("[callback] timeout! times: %d\r\n", times);
}

// 主线程函数
// 使用osTimerNew创建一个定时器，每100个ticks调用一次回调函数cb_timeout_periodic
// 每100个ticks检查一下全局变量times是否小于3，若不小于3则停止定时器
void timer_periodic(void)
{
    // Creates and initializes a timer.
    // This function creates a timer associated with the arguments callback function.
    // The timer stays in the stopped state until OSTimerStart is used to start the timer. 
    // osTimerOnce：一次性的
    // osTimerPeriodic：周期性的
    // Returns timer ID for reference by other functions or NULL in case of error.
    osTimerId_t periodic_tid = osTimerNew(cb_timeout_periodic, osTimerPeriodic, NULL, NULL);
    if (periodic_tid == NULL)
    {
        printf("[Timer Test] osTimerNew(periodic timer) failed.\r\n");
        return;
    }
    else
    {
        printf("[Timer Test] osTimerNew(periodic timer) success, tid: %p.\r\n", periodic_tid);
    }

    // starts or restarts a timer specified by the parameter timer_id.
    // This function cannot be called from Interrupt Service Routines.
    // 1秒钟调用一次回调函数
    osStatus_t status = osTimerStart(periodic_tid, 100);
    if (status != osOK)
    {
        printf("[Timer Test] osTimerStart(periodic timer) failed.\r\n");
        return;
    }
    else
    {
        printf("[Timer Test] osTimerStart(periodic timer) success, wait a while and stop.\r\n");
    }

    // 等待3秒
    while (times < 3)
    {
        // printf("[Timer Test] times:%d.\r\n", times);
        osDelay(100);
    }

    // stops a timer specified by the parameter timer_id.
    // 停止定时器
    status = osTimerStop(periodic_tid);
    printf("[Timer Test] stop periodic timer, status :%d.\r\n", status);
    
    // 删除定时器
    status = osTimerDelete(periodic_tid);
    printf("[Timer Test] kill periodic timer, status :%d.\r\n", status);
}

// 入口函数
static void TimerTestTask(void)
{
    // 线程属性
    osThreadAttr_t attr;
    attr.name = "timer_periodic";
    attr.attr_bits = 0U;
    attr.cb_mem = NULL;
    attr.cb_size = 0U;
    attr.stack_mem = NULL;
    attr.stack_size = 1024;
    attr.priority = osPriorityNormal;

    // 创建一个线程，并将其加入活跃线程组中
    if (osThreadNew((osThreadFunc_t)timer_periodic, NULL, &attr) == NULL)
    {
        printf("[TimerTestTask] Falied to create timer_periodic!\n");
    }
}

// 运行入口函数
APP_FEATURE_INIT(TimerTestTask);