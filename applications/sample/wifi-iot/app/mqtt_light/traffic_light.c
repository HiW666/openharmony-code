// 交通灯板三色灯接口文件。
// Pegasus交通灯板三色灯与主控芯片引脚的对应关系
// GPIO_10 连接红色LED，输出高电平点亮
// GPIO_11 连接绿色LED，输出高电平点亮
// GPIO_12 连接黄色LED，输出高电平点亮

#include <stdio.h>      // 标准输入输出
#include <unistd.h>     // POSIX标准接口

#include "ohos_init.h"  // 用于初始化服务(services)和功能(features)
#include "cmsis_os2.h"  // CMSIS-RTOS API V2

#include "iot_gpio.h"   // OpenHarmony API：IoT硬件设备操作接口-GPIO
#include "hi_io.h"      // 海思 Pegasus SDK：IoT硬件设备操作接口-IO
#include "hi_pwm.h"     // 海思 Pegasus SDK：IoT硬件设备操作接口-PWM

// 定义GPIO引脚，尽量避免直接用数值
#define RED_GPIO 10     // 红色LED
#define GREEN_GPIO 11   // 绿色LED
#define YELLOW_GPIO 12  // 黄色LED

/// 初始化三色灯设备
void InitTrafficLight(void)
{
    // 初始化GPIO模块
    IoTGpioInit(RED_GPIO);      // 初始化红色LED
    IoTGpioInit(GREEN_GPIO);    // 初始化绿色LED
    IoTGpioInit(YELLOW_GPIO);   // 初始化黄色LED

    // 设置GPIO10为GPIO功能
    hi_io_set_func(RED_GPIO, HI_IO_FUNC_GPIO_10_GPIO);
    // 设置GPIO10为输出模式（引脚方向为输出）
    IoTGpioSetDir(RED_GPIO, IOT_GPIO_DIR_OUT);

    // 设置GPIO11为GPIO功能
    hi_io_set_func(GREEN_GPIO, HI_IO_FUNC_GPIO_11_GPIO);
    // 设置GPIO11为输出模式（引脚方向为输出）
    IoTGpioSetDir(GREEN_GPIO, IOT_GPIO_DIR_OUT);

    // 设置GPIO12为GPIO功能
    hi_io_set_func(YELLOW_GPIO, HI_IO_FUNC_GPIO_12_GPIO);
    // 设置GPIO12为输出模式（引脚方向为输出）
    IoTGpioSetDir(YELLOW_GPIO, IOT_GPIO_DIR_OUT);
}

/// 设置三色灯的状态
/// @param color 指定灯的颜色。1：红色，2：绿色，3：黄色
/// @param state 指定灯的状态。1：关闭，2：打开
void SetTrafficLight(int color, int state)
{
    // 设置红色灯
    if (color == 1)
    {
        IoTGpioSetOutputVal(RED_GPIO, state == 1 ? IOT_GPIO_VALUE0 : IOT_GPIO_VALUE1);
    }

    // 设置绿色灯
    else if (color == 2)
    {
        IoTGpioSetOutputVal(GREEN_GPIO, state == 1 ? IOT_GPIO_VALUE0 : IOT_GPIO_VALUE1);
    }

    // 设置黄色灯
    else if (color == 3)
    {
        IoTGpioSetOutputVal(YELLOW_GPIO, state == 1 ? IOT_GPIO_VALUE0 : IOT_GPIO_VALUE1);
    }
}
