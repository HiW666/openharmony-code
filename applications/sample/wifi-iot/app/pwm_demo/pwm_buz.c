#include <stdio.h>      // 标准输入输出

#include "ohos_init.h"  // 用于初始化服务(services)和功能(features)
#include "cmsis_os2.h"  // CMSIS-RTOS API V2

#include "iot_gpio.h"   // OpenHarmony HAL API：IoT硬件设备操作接口-GPIO
#include "hi_io.h"      // 海思 Pegasus SDK：IoT硬件设备操作接口-IO
#include "iot_pwm.h"    // OpenHarmony HAL API：IoT硬件设备操作接口-PWM

// 海思 Pegasus SDK：IoT硬件设备操作接口-PWM
// OpenHarmony1.0之后，1.0.1是一个比较完善的版本。
// OpenHarmony从1.1.0版本开始，到1.1.3版本，IoT接口的变动比较大，功能上比较残缺。
// 有些接口被移除，只能直接调用海思Pegasus SDK的接口。
// 海思Pegasus SDK的接口，以hi_开头，如hi_pwm_set_clock(...)。
// 目前来说，OpenHarmony在轻量级设备这块，尤其是外设接口，支持的不是很好。
// 官方以后主推HDF接口，形式上和1.0的接口比较接近。
// 在2.x和3.0阶段，官方对轻量级设备投入的精力比较少。
// 到目前(2021-12-31)为止，HDF还没有一个轻量级芯片支持，包括Hi3861。
#include "hi_pwm.h"

// 主线程函数
static void PwmBuzTask(void *arg)
{
    (void)arg;

    // 初始化GPIO模块
    IoTGpioInit(HI_IO_NAME_GPIO_9);

    // 设置GPIO9为PWM0输出
    hi_io_set_func(HI_IO_NAME_GPIO_9, HI_IO_FUNC_GPIO_9_PWM0_OUT);

    // 设置GPIO9为输出模式（引脚方向为输出）。
    IoTGpioSetDir(HI_IO_NAME_GPIO_9, IOT_GPIO_DIR_OUT);

    // 初始化PWM模块
    IoTPwmInit(HI_PWM_PORT_PWM0);

    // 开始输出PWM信号（蜂鸣器开始鸣叫），占空比50%，频率4000Hz
    // OpenHarmony的HAL接口
    IoTPwmStart(HI_PWM_PORT_PWM0, 50, 4000);
    // 海思SDK接口
    // 内部时钟(默认)160MHz，外部时钟40MHz
    // 时钟源频率：160MHz=160000000Hz
    // freq = 时钟源频率/实际频率 = 160000000/4000 = 40000
    // duty = freq*实际占空比 = 40000*50% = 20000
    // hi_pwm_start(HI_PWM_PORT_PWM0, 20*1000, 40*1000);

    // 等待1秒（让蜂鸣器鸣叫1秒）
    osDelay(100);

    // 停止输出PWM信号（蜂鸣器停止鸣叫）
    // OpenHarmony的HAL接口
    IoTPwmStop(HI_PWM_PORT_PWM0);
    // 海思SDK接口
    // hi_pwm_stop(HI_PWM_PORT_PWM0);

    // 停止鸣叫后，熄灭LED
    // 设置GPIO9为GPIO功能。
    hi_io_set_func(HI_IO_NAME_GPIO_9, HI_IO_FUNC_GPIO_9_GPIO);
    // 设置GPIO9为输出模式（引脚方向为输出）。
    IoTGpioSetDir(HI_IO_NAME_GPIO_9, IOT_GPIO_DIR_OUT);
    // 设置GPIO引脚的输出状态（输出高电平，熄灭LED）。
    IoTGpioSetOutputVal(HI_IO_NAME_GPIO_9, IOT_GPIO_VALUE1);
}

// 入口函数
static void PwmBuzEntry(void)
{
    // 定义线程属性
    osThreadAttr_t attr;
    attr.name = "PwmBuzTask";
    attr.stack_size = 4096;
    attr.priority = osPriorityNormal;

    // 创建线程
    if (osThreadNew(PwmBuzTask, NULL, &attr) == NULL)
    {
        printf("[PwmBuzExample] Falied to create PwmBuzTask!\n");
    }
}

// 运行入口函数
SYS_RUN(PwmBuzEntry);