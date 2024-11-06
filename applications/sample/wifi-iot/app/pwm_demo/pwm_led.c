// Pegasus炫彩灯板三色灯与主控芯片引脚的对应关系
// **红色：**GPIO10/PWM1/低电平点亮
// **绿色：**GPIO11/PWM2/低电平点亮
// **蓝色：**GPIO12/PWM3/低电平点亮

#include <stdio.h>  // 标准输入输出
#include <stdlib.h> // 标准函数库

#include "ohos_init.h" // 用于初始化服务(services)和功能(features)
#include "cmsis_os2.h" // CMSIS-RTOS API V2

#include "iot_gpio.h" // OpenHarmony HAL API：IoT硬件设备操作接口-GPIO
#include "iot_pwm.h"  // OpenHarmony HAL API：IoT硬件设备操作接口-PWM
#include "hi_io.h"    // 海思SDK API：IoT硬件设备操作接口-IO
#include "hi_pwm.h"   // 海思SDK API：IoT硬件设备操作接口-PWM

/**
 * @brief demo1，使用海思SDK API接口控制红色灯亮度。
 * 因为设置的占空比较小（4096MAX/64000），所以红色灯总体亮度较低，相对护眼更安全。
 *
 * @return 无
 */
void demo1()
{
    // 红色灯越来越亮
    for (int i = 1; i <= 4096; i *= 2)
    {
        // 开始输出PWM信号，实际占空比i/64000，实际频率2500Hz
        // 时钟源频率：160MHz=160000000Hz
        // freq = 时钟源频率/实际频率 = 160000000/2500 = 64000
        // duty = freq*实际占空比 = 64000*i/64000 = i
        hi_pwm_start(HI_PWM_PORT_PWM1, i, 64000);
        // 等待50ms
        usleep(50 * 1000);
        // 停止输出PWM信号
        hi_pwm_stop(HI_PWM_PORT_PWM1);
    }
    // 红色灯越来越暗
    for (int i = 4096; i >= 1; i /= 2)
    {
        // 开始输出PWM信号，实际占空比i/64000，实际频率2500Hz
        hi_pwm_start(HI_PWM_PORT_PWM1, i, 64000);
        // 等待50ms，相当于画面刷新率的20fps，基本上是满足视觉暂留的最低下限
        usleep(50 * 1000);
        // 停止输出PWM信号
        hi_pwm_stop(HI_PWM_PORT_PWM1);
    }
}

/**
 * @brief demo2，使用OpenHarmony HAL API接口控制红色灯亮度。
 * 因为设置的占空比较大，所以红色灯总体亮度较高，注意保护眼睛。
 *
 * @return 无
 */
void demo2()
{
    //红色灯越来越亮
    for (int i = 1; i < 100; i++)
    {
        // 开始输出PWM信号，实际占空比i，实际频率4000Hz
        IoTPwmStart(HI_PWM_PORT_PWM1, i, 4000);
        // 等待20ms，相当于画面刷新率的50fps，人眼看起来会更平滑
        usleep(20 * 1000);
        // 停止输出PWM信号
        IoTPwmStop(HI_PWM_PORT_PWM1);
    }
    // 红色灯越来越暗
    for (int i = 99; i >= 1; i--)
    {
        // 开始输出PWM信号，实际占空比i，实际频率4000Hz
        IoTPwmStart(HI_PWM_PORT_PWM1, i, 4000);
        // 等待20ms
        usleep(20 * 1000);
        // 停止输出PWM信号
        IoTPwmStop(HI_PWM_PORT_PWM1);
    }
}

/**
 * @brief demo3，使用OpenHarmony HAL API接口控制三色灯变色，实现炫彩灯光效果。
 * 顺序：绿-->蓝-->红-->绿。
 * 因为设置的占空比较大，所以三色灯总体亮度较高，注意保护眼睛。
 *
 * @return 无
 */
void demo3()
{
    // 绿-->蓝
    for (int i = 1; i < 100; i++)
    {
        // 开始输出PWM信号，实际频率4000Hz
        IoTPwmStart(HI_PWM_PORT_PWM1, 1, 4000);         //红色，亮度保持最低，实际占空比1
        IoTPwmStart(HI_PWM_PORT_PWM2, 100 - i, 4000);   //绿色，亮度从最高到最低，实际占空比99~1
        IoTPwmStart(HI_PWM_PORT_PWM3, i, 4000);         //蓝色，亮度从最低到最高，实际占空比1~99
        usleep(20 * 1000);                              // 等待20ms
        // 停止输出PWM信号
        IoTPwmStop(HI_PWM_PORT_PWM1);
        IoTPwmStop(HI_PWM_PORT_PWM2);
        IoTPwmStop(HI_PWM_PORT_PWM3);
    }
    // 蓝-->红
    for (int i = 1; i < 100; i++)
    {
        // 开始输出PWM信号，实际频率4000Hz
        IoTPwmStart(HI_PWM_PORT_PWM1, i, 4000);         //红色，亮度从最低到最高，实际占空比1~99
        IoTPwmStart(HI_PWM_PORT_PWM2, 1, 4000);         //绿色，亮度保持最低，实际占空比1
        IoTPwmStart(HI_PWM_PORT_PWM3, 100 - i, 4000);   //蓝色，亮度从最高到最低，实际占空比99~1
        usleep(20 * 1000);                              // 等待20ms
        // 停止输出PWM信号
        IoTPwmStop(HI_PWM_PORT_PWM1);
        IoTPwmStop(HI_PWM_PORT_PWM2);
        IoTPwmStop(HI_PWM_PORT_PWM3);
    }
    // 红-->绿
    for (int i = 1; i < 100; i++)
    {
        // 开始输出PWM信号，实际频率4000Hz
        IoTPwmStart(HI_PWM_PORT_PWM1, 100 - i, 4000);   //红色，亮度从最高到最低，实际占空比99~1
        IoTPwmStart(HI_PWM_PORT_PWM2, i, 4000);         //绿色，亮度从最低到最高，实际占空比1~99
        IoTPwmStart(HI_PWM_PORT_PWM3, 1, 4000);         //蓝色，亮度保持最低，实际占空比1
        usleep(20 * 1000);                              // 等待20ms
        // 停止输出PWM信号
        IoTPwmStop(HI_PWM_PORT_PWM1);
        IoTPwmStop(HI_PWM_PORT_PWM2);
        IoTPwmStop(HI_PWM_PORT_PWM3);
    }
}

// 主线程函数
static void PwmLedTask(void *arg)
{
    (void)arg;

    // 初始化GPIO
    IoTGpioInit(HI_IO_NAME_GPIO_10); // 红色灯引脚，对应GPIO10
    IoTGpioInit(HI_IO_NAME_GPIO_11); // 绿色灯引脚，对应GPIO11
    IoTGpioInit(HI_IO_NAME_GPIO_12); // 蓝色灯引脚，对应GPIO12

    // 设置引脚功能为PWM输出
    hi_io_set_func(HI_IO_NAME_GPIO_10, HI_IO_FUNC_GPIO_10_PWM1_OUT); // 红色灯引脚，对应PWM1
    hi_io_set_func(HI_IO_NAME_GPIO_11, HI_IO_FUNC_GPIO_11_PWM2_OUT); // 绿色灯引脚，对应PWM2
    hi_io_set_func(HI_IO_NAME_GPIO_12, HI_IO_FUNC_GPIO_12_PWM3_OUT); // 蓝色灯引脚，对应PWM3

    // 设置引脚方向为输出
    IoTGpioSetDir(HI_IO_NAME_GPIO_10, IOT_GPIO_DIR_OUT); // 红色灯引脚
    IoTGpioSetDir(HI_IO_NAME_GPIO_11, IOT_GPIO_DIR_OUT); // 绿色灯引脚
    IoTGpioSetDir(HI_IO_NAME_GPIO_12, IOT_GPIO_DIR_OUT); // 蓝色灯引脚

    // 初始化PWM
    IoTPwmInit(HI_PWM_PORT_PWM1); // 红色灯PWM
    IoTPwmInit(HI_PWM_PORT_PWM2); // 绿色灯PWM
    IoTPwmInit(HI_PWM_PORT_PWM3); // 蓝色灯PWM

    // 工作循环
    while (1)
    {
        // demo1：使用海思SDK API接口控制红色灯亮度
        demo1();
        // demo2：使用OpenHarmony HAL API接口控制红色灯亮度
        // demo2();
        // demo3：使用OpenHarmony HAL API接口控制三色灯变色，实现炫彩灯光效果
        // demo3();
    }
}

// 入口函数
static void PwmLedEntry(void)
{
    // 定义线程属性
    osThreadAttr_t attr;
    attr.name = "PwmLedTask";
    attr.stack_size = 4096;
    attr.priority = osPriorityNormal;

    // 创建线程
    if (osThreadNew(PwmLedTask, NULL, &attr) == NULL)
    {
        printf("[PwmLedExample] Falied to create PwmLedTask!\n");
    }
}

// 运行入口函数
SYS_RUN(PwmLedEntry);
