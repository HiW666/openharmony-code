/*
 * Copyright (c) 2022, Dragon. HeBei University. email:dragon@hbu.edu.cn
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its
 *    contributors may be used to endorse or promote products derived from
 *    this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

// 标准输入输出
#include <stdio.h>

// POSIX标准接口
#include <unistd.h>

// 用于初始化服务(services)和功能(features)
#include "ohos_init.h"

// CMSIS-RTOS API V2
// 是OpenHarmony的LiteOS_m内核与应用程序之间的抽象层。（LiteOS_m内核基于Cortex M系列芯片）
// 提供标准接口，便于应用程序移植来OpenHarmony，或者移植到其他支持CMSIS-RTOS API V2的系统。
// 头文件：kernel/liteos_m/kal/cmsis/cmsis_os2.h
// 源文件：kernel/liteos_m/kal/cmsis/cmsis_liteos2.c
#include "cmsis_os2.h"

// 线程要运行的函数
void rtosv2_thread_main(void *arg)
{
    // 延迟1秒，避免跟系统输出混淆在一起
    osDelay(100);

    // 打印当前线程ID
    osThreadId_t tid = osThreadGetId();
    printf("thread id: %p\r\n", tid);

    // 打印参数
    printf("args: %s\r\n", (char *)arg);

    // 输出计数
    static int count = 0;
    while (1)
    {
        count++;
        printf("count: %d\r", count);
        osDelay(10);
    }
}

// 此demo的入口函数
// 入口函数不能做一些耗时的操作，必须快速返回，否则会妨碍其它应用程序的运行。
// 因此，在入口函数中创建专用的任务（线程）是一种“标准”操作。
static void ThreadTestTask(void)
{
    // 定义线程属性
    osThreadAttr_t attr;
    // 线程名
    attr.name = "rtosv2_thread_main";
    // 线程属性位
    attr.attr_bits = 0U;
    // 线程控制块的内存初始地址，默认为系统自动分配
    attr.cb_mem = NULL;
    // 线程控制块的内存大小
    attr.cb_size = 0U;
    // 线程栈的内存初始地址，默认为系统自动分配
    attr.stack_mem = NULL;
    // 线程栈的内存大小
    attr.stack_size = 1024;
    // 线程优先级，9(highest) - 38(lowest priority)，默认为osPriorityNormal
    // 位置：kernel\liteos_m\kal\cmsis\cmsis_os2.h
    attr.priority = osPriorityNormal;

    // 创建一个线程，并将其加入活跃线程组中
    // osThreadId_t osThreadNew(osThreadFunc_t func, void *argument, const osThreadAttr_t *attr)
    // func	线程要运行的函数
    // argument	指针，指向传递给线程函数的参数
    // attr	线程属性
    // 注意：不能在中断服务调用该函数
    if (osThreadNew((osThreadFunc_t)rtosv2_thread_main, "This is a test thread.", &attr) == NULL)
    {
        printf("[ThreadTestTask] Falied to create rtosv2_thread_main!\n");
    }
}

// 让ThreadTestTask函数以“优先级2”在系统启动过程的“阶段8. application-layer feature”阶段执行。
APP_FEATURE_INIT(ThreadTestTask);
