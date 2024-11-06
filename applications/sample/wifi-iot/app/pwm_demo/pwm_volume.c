#include <stdio.h>      // 标准输入输出

#include "ohos_init.h"  // 用于初始化服务(services)和功能(features)
#include "cmsis_os2.h"  // CMSIS-RTOS API V2

#include "iot_gpio.h"   // OpenHarmony HAL：IoT硬件设备操作接口-GPIO
#include "hi_io.h"      // 海思 Pegasus SDK：IoT硬件设备操作接口-IO
#include "hi_pwm.h"     // 海思 Pegasus SDK：IoT硬件设备操作接口-PWM

// 主线程函数
static void PwmVolumeTask(void *arg)
{
    (void)arg;

    // 初始化GPIO模块
    IoTGpioInit(HI_IO_NAME_GPIO_9);
    // 设置GPIO9为PWM0输出
    hi_io_set_func(HI_IO_NAME_GPIO_9, HI_IO_FUNC_GPIO_9_PWM0_OUT);
    // 初始化PWM模块
    IoTPwmInit(HI_PWM_PORT_PWM0);
    // 设置时钟源为外部晶体时钟（40MHz，默认时钟源160MHz）
    hi_pwm_set_clock(PWM_CLK_XTAL);

    // 工作循环
    while (1)
    {
        // 定义音量级别
        const int numLevels = 100;
        // 音量逐渐变大
        for (int i = numLevels; i >0; i--)
        {
            // 确定海思SDK接口的freq和duty
            // 实际频率使用611.6Hz
            // freq = 时钟源频率/实际频率 = 40000000/611.6 = 65402，取65400
            // 实际占空比使用i/numLevels
            // duty = freq*实际占空比 = 65400/numLevels * i
            // 开始输出PWM信号
            hi_pwm_start(HI_PWM_PORT_PWM0, 65400 / numLevels * i, 65400);
            // 等待20ms（让蜂鸣器鸣叫20ms）
            osDelay(2);
            // 停止输出PWM信号
            hi_pwm_stop(HI_PWM_PORT_PWM0);
        }
        // 音量逐渐减小
        for (int i = 1; i <= numLevels; i++)
        {
            // 开始输出PWM信号
            hi_pwm_start(HI_PWM_PORT_PWM0, 65400 / numLevels * i, 65400);
            // 等待20ms（让蜂鸣器鸣叫20ms）
            osDelay(2);
            // 停止输出PWM信号
            hi_pwm_stop(HI_PWM_PORT_PWM0);
        }
    }
}

// 入口函数
static void PwmVolumeEntry(void)
{
    // 定义线程属性
    osThreadAttr_t attr = {0};
    attr.name = "PwmVolumeTask";
    attr.stack_size = 4096;
    attr.priority = osPriorityNormal;

    // 创建线程
    if (osThreadNew(PwmVolumeTask, NULL, &attr) == NULL)
    {
        printf("[PwmVolumeExample] Falied to create PwmVolumeTask!\n");
    }
}

// 运行入口函数
SYS_RUN(PwmVolumeEntry);