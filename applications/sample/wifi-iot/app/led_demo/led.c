#include <stdio.h>      // 标准输入输出
#include "ohos_init.h"  // 用于初始化服务(services)和功能(features)
#include "cmsis_os2.h"  // CMSIS-RTOS API V2

// HAL(硬件抽象层) 接口：IoT硬件设备操作接口-GPIO
// base/iot_hardware/peripheral/interfaces/kits提供了一系列IoT（The Internet of Things）
// 硬件设备操作的接口，包括FLASH,GPIO,I2C,PWM,UART,WATCHDOG等。
#include "iot_gpio.h"

// 海思SDK接口：IoT硬件设备操作接口-IO
// OpenHarmony1.0之后，1.0.1是一个比较完善的版本。
// OpenHarmony从1.1.0版本开始，到1.1.3版本，IoT接口的变动比较大，功能上比较残缺。
// 有些接口被移除，只能直接调用海思SDK的接口。
// 海思SDK的接口，以hi_开头，如hi_io_set_func(...)。
// 目前来说，OpenHarmony在轻量级设备这块，尤其是外设接口，支持的不是很好。
// 官方以后主推HDF接口，形式上和1.0的接口比较接近。
// 在2.x和3.0阶段，官方对轻量级设备投入的精力比较少。
// 到目前(2021-12-31)为止，HDF还没有一个轻量级芯片支持，包括Hi3861。
#include "hi_io.h"

#define LED_TASK_GPIO 9

static void LedTask(void *arg)
{
    (void)arg;

    // 初始化GPIO模块
    IoTGpioInit(LED_TASK_GPIO);

    // Hi3861芯片外设接口多，引脚数量少，部分引脚有多个功能，需要设置引脚功能。
    // 设置GPIO9为GPIO功能。
    // Hi3861引脚功能复用表参见教材附录E。
    // 在OpenHarmony 1.0 时期，使用：
    // IoSetFunc(LED_TASK_GPIO, 0);
    hi_io_set_func(LED_TASK_GPIO, HI_IO_FUNC_GPIO_9_GPIO);
 
    // 设置GPIO9为输出模式（引脚方向为输出）。
    IoTGpioSetDir(LED_TASK_GPIO,IOT_GPIO_DIR_OUT);

    while (1)
    {
        // 设置GPIO引脚的输出状态（1高电平或0低电平）。
        // 输出低电平，点亮LED。
        // 原理参考 https://www.bilibili.com/video/BV1Xi4y1b7Nj
        IoTGpioSetOutputVal(LED_TASK_GPIO,IOT_GPIO_VALUE0);
        
        // 等待0.5秒。单位是10ms。
        osDelay(50);

        // 输出高电平，熄灭LED。
        IoTGpioSetOutputVal(LED_TASK_GPIO,IOT_GPIO_VALUE1);

        osDelay(100);
    }
}

static void LedEntry(void)
{
    osThreadAttr_t attr;

    attr.name = "LedTask";
    attr.stack_size = 4096;
    attr.priority = osPriorityNormal;

    // 创建线程（到内核API详述）
    // 1.0 CMSIS-RTOS2 API：third_party\cmsis\CMSIS\RTOS2\Include\cmsis_os2.h
    // 1.1.3 kernel\liteos_m\kal\cmsis\cmsis_liteos2.c
    // RTOS: real-time operating system
    // CMSIS: Cortex Microcontroller Software Interface Standard
    if (osThreadNew(LedTask, NULL, &attr) == NULL)
    {
        printf("[LedDemo] Create LedTask failed!\n");
    }
}

// ohos_init.h 中定义的宏，让函数在系统启动时执行
SYS_RUN(LedEntry);
