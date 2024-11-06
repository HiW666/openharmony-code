#include <stdio.h>      // 标准输入输出
#include "ohos_init.h"  // 用于初始化服务(services)和功能(features)
#include "cmsis_os2.h"  // CMSIS-RTOS API V2

#include "iot_gpio.h"   // OpenHarmony HAL API：IoT硬件设备操作接口-GPIO
#include "hi_io.h"      // 海思Pegasus SDK API：IoT硬件设备操作接口-IO

// 定义GPIO引脚
#define LED_GPIO 9
#define BUTTON_GPIO 5

// 此全局变量用于控制LED灯的状态。
// 取值：0低电平，点亮LED；1高电平，熄灭LED。
// 初始值决定了LED灯的初始状态。
static volatile IotGpioValue g_ledPinValue = IOT_GPIO_VALUE0;

// GPIO5的中断处理函数。
static void OnButtonPressed(char *arg)
{
    (void)arg;
    printf("Button pressed\n");
    g_ledPinValue = !g_ledPinValue;
}

// 主线程函数
static void GpioTask(void *arg)
{
    (void)arg;

    // 初始化GPIO模块
    IoTGpioInit(LED_GPIO);
    IoTGpioInit(BUTTON_GPIO);

    // Hi3861芯片外设接口多，引脚数量少，部分引脚有多个功能，需要设置引脚功能。
    // 设置GPIO9为GPIO功能。
    // Hi3861引脚功能复用表参见本节的配套资源（“Hi3861引脚功能复用表.docx”）。
    hi_io_set_func(LED_GPIO, HI_IO_FUNC_GPIO_9_GPIO);

    // 设置GPIO9为输出模式（引脚方向为输出）。
    IoTGpioSetDir(LED_GPIO, IOT_GPIO_DIR_OUT);

    // 设置GPIO5为GPIO功能。
    hi_io_set_func(BUTTON_GPIO, HI_IO_FUNC_GPIO_5_GPIO);

    // 设置GPIO5为输入模式（引脚方向为输入）。
    IoTGpioSetDir(BUTTON_GPIO, IOT_GPIO_DIR_IN);

    // 设置GPIO5为上拉模式（引脚上拉）。
    // 引脚上拉后，在按键没有被按下时，读取到的值为1高电平，在按键按下时，读取到的值为0低电平。
    // 注意：本例不获取GPIO5引脚的输入电平。
    hi_io_set_pull(BUTTON_GPIO, HI_IO_PULL_UP);

    // 注册GPIO5中断处理函数。
    IoTGpioRegisterIsrFunc(BUTTON_GPIO,                 //GPIO-05引脚
                        IOT_INT_TYPE_EDGE,              //边沿触发
                        IOT_GPIO_EDGE_FALL_LEVEL_LOW,   //下降沿触发
                        OnButtonPressed,                //中断处理函数
                        NULL);                          //中断处理函数的参数

    // 工作循环
    while (1)
    {
        // 设置GPIO9引脚的输出电平。
        // 输出低电平，点亮LED。输出高电平，熄灭LED。
        IoTGpioSetOutputVal(LED_GPIO, g_ledPinValue);

        // 每隔0.1秒设置一次输出电平，单位是10ms。
        // 注意，如果不加osDelay，则会导致程序出现0x80000021错误。
        osDelay(10);
    }
}

// 入口函数
static void GpioEntry(void)
{
    // 定义线程属性
    osThreadAttr_t attr;
    // 线程属性设置：设置线程名称
    attr.name = "GpioTask";
    // 线程属性设置：设置线程栈大小
    attr.stack_size = 4096;
    // 线程属性设置：设置线程优先级
    attr.priority = osPriorityNormal;

    // 创建线程
    if (osThreadNew(GpioTask, NULL, &attr) == NULL)
    {
        printf("[GpioDemo] Create GpioTask failed!\n");
    }
}

// 运行入口函数
// ohos_init.h 中定义的宏，让函数在系统启动时执行
SYS_RUN(GpioEntry);
