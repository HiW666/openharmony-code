// 时间管理以系统时钟为基础
// 系统时钟（Tick|时标）：是由定时器/计数器产生的输出脉冲触发中断产生的，一般定义为整数或长整数。
// 时钟周期：输出脉冲的周期，即两个Tick间隔的时间长度。
// 注意：Tick不等于时钟周期。Tick是输出脉冲触发中断，在特定时间点上产生的标记；而时钟周期是ticks间隔的时间长度。

#include <stdio.h>      // 标准输入输出
#include <unistd.h>     // POSIX标准接口

#include "ohos_init.h"  // 用于初始化服务(services)和功能(features)
#include "cmsis_os2.h"  // CMSIS-RTOS API V2

void rtosv2_delay_main(void *arg)
{
    (void)arg;

    // 内核每秒的ticks数
    // 在3861上面，每秒100个ticks，每个tick的时钟周期为10ms
    printf("[Delay Test] number of kernel ticks per second: %d.\r\n", osKernelGetTickFreq());

    // 系统的当前ticks计数
    printf("[Delay Test] Current system tick: %d.\r\n", osKernelGetTickCount());

    // 等待100个ticks，即大约延时1秒(100*10ms=1000ms=1s)
    // 注意：等待指定的ticks，不精确等于等待指定的时钟周期，误差最多会少一个时钟周期(10ms)。
    // The function osDelay waits for a time period specified in kernel ticks.
    // For a value of 1 the system waits until the next timer tick occurs.
    // The actual time delay may be up to one timer tick less than specified,
    // i.e. calling osDelay(1) right before the next system tick occurs the thread is rescheduled immediately.
    // The delayed thread is put into the BLOCKED state and a context switch occurs immediately.
    // The thread is automatically put back to the READY state after the given amount of ticks has elapsed.
    // If the thread will have the highest priority in READY state it will be scheduled immediately.
    // 用户以秒、毫秒为单位计时，而操作系统以Tick为单位计时。今后，在多处场合下，会认为osDelay(n)就是近似等待n*10ms。
    osStatus_t status = osDelay(100);
    printf("[Delay Test] osDelay, status: %d.\r\n", status);

    // 系统的当前ticks计数
    printf("[Delay Test] Current system tick: %d.\r\n", osKernelGetTickCount());

    // 等待到指定的ticks
    uint32_t tick = osKernelGetTickCount();
    tick += 100;
    // Waits until a specified time arrives. This function handles the overflow of the system timer.
    // Note that the maximum value of this parameter is (2^31 - 1) ticks.
    status = osDelayUntil(tick);
    printf("[Delay Test] osDelayUntil, status: %d.\r\n", status);
    printf("[Delay Test] Current system tick: %d.\r\n", osKernelGetTickCount());
}

static void DelayTestTask(void)
{
    osThreadAttr_t attr;

    attr.name = "rtosv2_delay_main";
    attr.attr_bits = 0U;
    attr.cb_mem = NULL;
    attr.cb_size = 0U;
    attr.stack_mem = NULL;
    attr.stack_size = 1024;
    attr.priority = osPriorityNormal;

    if (osThreadNew((osThreadFunc_t)rtosv2_delay_main, NULL, &attr) == NULL)
    {
        printf("[DelayTestTask] Falied to create rtosv2_delay_main!\n");
    }
}

APP_FEATURE_INIT(DelayTestTask);