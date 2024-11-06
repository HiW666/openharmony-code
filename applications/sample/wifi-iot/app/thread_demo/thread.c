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

#include <stdio.h>      // 标准输入输出
#include <unistd.h>     // POSIX标准接口

#include "ohos_init.h"  // 用于初始化服务(services)和功能(features)
#include "cmsis_os2.h"  // CMSIS-RTOS API V2

// 创建线程，返回线程ID。封装成一个函数，便于调用
osThreadId_t newThread(char *name, osThreadFunc_t func, void *arg)
{
    // 定义线程属性
    osThreadAttr_t attr = {
        name, 0, NULL, 0, NULL, 1024 * 2, osPriorityNormal, 0, 0};
    // 创建线程，拿到线程ID
    osThreadId_t tid = osThreadNew(func, arg, &attr);
    // 得到当前线程的名字
    const char *c_name = osThreadGetName(osThreadGetId());
    if (tid == NULL)
    {
        printf("[%s] osThreadNew(%s) failed.\r\n", c_name, name);
    }
    else
    {
        printf("[%s] osThreadNew(%s) success, thread id: %d.\r\n", c_name, name, tid);
    }
    return tid;
}

// 测试线程函数：先输出自己的参数，然后对全局变量count进行循环+1操作，之后会打印count的值
void threadTest(void *arg)
{
    static int count = 0;
    // 得到当前线程ID和名字
    osThreadId_t tid = osThreadGetId();
    const char *c_name = osThreadGetName(tid);
    // 输出参数
    printf("[%s] %s\r\n", c_name, (char *)arg);
    // 输出当前线程名字和ID
    printf("[%s] osThreadGetId, thread id:%p\r\n", c_name, tid);
    // 输出计数
    while (1)
    {
        count++;
        printf("[%s] count: %d.\r\n", c_name, count);
        osDelay(20);
    }
}

// 主线程函数：创建线程并运行，并使用线程API进行相关操作，最后终止所创建的线程。
void threadMain(void *arg)
{
    (void)arg;

    // 延迟1秒，避免跟系统输出混淆在一起
    osDelay(100);

    // 创建线程
    osThreadId_t tid = newThread("threadTest", threadTest, "This is a test thread.");

    // 得到创建的线程的名字
    const char *t_name = osThreadGetName(tid);
    // 得到当前线程的名字
    const char *c_name = osThreadGetName(osThreadGetId());
    // 输出当前线程的名字
    printf("[%s] osThreadGetName, thread name: %s.\r\n", c_name, t_name);

    // 得到线程的状态 
    osThreadState_t state = osThreadGetState(tid);
    // 输出线程的状态 
    printf("[%s] osThreadGetState, state :%d.\r\n", c_name, state);

    // 设置线程的优先级
    osStatus_t status = osThreadSetPriority(tid, osPriorityNormal4);
    // 输出设置结果
    printf("[%s] osThreadSetPriority, status: %d.\r\n", c_name, status);

    // 得到线程的优先级
    osPriority_t pri = osThreadGetPriority(tid);
    // 输出线程的优先级
    printf("[%s] osThreadGetPriority, priority: %d.\r\n", c_name, pri);

    // 挂起线程
    status = osThreadSuspend(tid);
    // 输出挂起结果
    printf("[%s] osThreadSuspend, status: %d.\r\n", c_name, status);

    // 恢复线程
    status = osThreadResume(tid);
    // 输出恢复结果
    printf("[%s] osThreadResume, status: %d.\r\n", c_name, status);

    // 得到线程的栈空间大小
    uint32_t stacksize = osThreadGetStackSize(tid);
    // 输出线程的栈空间大小
    printf("[%s] osThreadGetStackSize, stacksize: %d.\r\n", c_name, stacksize);

    // 得到线程的未使用的栈空间大小
    uint32_t stackspace = osThreadGetStackSpace(tid);
    // 输出线程的未使用的栈空间大小
    printf("[%s] osThreadGetStackSpace, stackspace: %d.\r\n", c_name, stackspace);

    // 获取活跃线程数
    uint32_t t_count = osThreadGetCount();
    // 输出活跃线程数
    printf("[%s] osThreadGetCount, count: %d.\r\n", c_name, t_count);

    osDelay(100);
    // 终止线程
    status = osThreadTerminate(tid);
    // 输出终止结果
    printf("[%s] osThreadTerminate, status: %d.\r\n", c_name, status);
}

// 此demo的入口函数
static void ThreadTestTask(void)
{
    // 定义线程属性
    osThreadAttr_t attr;
    attr.name = "threadMain";
    attr.attr_bits = 0U;
    attr.cb_mem = NULL;
    attr.cb_size = 0U;
    attr.stack_mem = NULL;
    attr.stack_size = 1024;
    attr.priority = osPriorityNormal;

    // 创建主线程
    if (osThreadNew((osThreadFunc_t)threadMain, NULL, &attr) == NULL)
    {
        printf("[%s] Falied to create threadMain!\n", __func__);
    }
}

// 让ThreadTestTask函数以“优先级2”在系统启动过程的“阶段8. application-layer feature”阶段执行。
APP_FEATURE_INIT(ThreadTestTask);
