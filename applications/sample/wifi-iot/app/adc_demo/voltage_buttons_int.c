// Hi3861芯片的相应引脚在作为ADC通道使用时，GPIO中断依然能够触发。

#include <stdio.h>      // 标准输入输出
#include <unistd.h>     // POSIX标准接口

#include "ohos_init.h"  // 用于初始化服务(services)和功能(features)
#include "cmsis_os2.h"  // CMSIS-RTOS API V2

#include "iot_gpio.h"   // OpenHarmony HAL：IoT硬件设备操作接口-GPIO
#include "hi_io.h"      // 海思 Pegasus SDK：IoT硬件设备操作接口-IO
#include "hi_adc.h"     // 海思 Pegasus SDK：IoT硬件设备操作接口-ADC

// 定义一个宏，用于标识ADC2通道
#define BUTTONS_CHAN_NAME HI_ADC_CHANNEL_2

// GPIO5的中断处理函数。
// ADC读数速率较慢，请尽量避免在中断中使用。本例仅供对比展示
static void OnButtonPressed(char *arg)
{
    (void)arg;

    // 用于存放ADC2通道的值
    unsigned short data = 0;

    // 读取ADC2通道的值
    if (hi_adc_read(BUTTONS_CHAN_NAME, &data,
                    HI_ADC_EQU_MODEL_4, HI_ADC_CUR_BAIS_DEFAULT, 0) == HI_ERR_SUCCESS)
    {
        // 如果ADC2通道的值大于等于5，小于228，则输出“USER Button!”
        if (data >= 5 && data < 228)
        {
            printf("USER Button! ADC_VALUE = %d\n", data);
        }
        // 如果ADC2通道的值大于等于228，小于455，则输出“S1 Button!”
        else if (data >= 228 && data < 455)
        {
            printf("S1 Button! ADC_VALUE = %d\n", data);
        }
        // 如果ADC2通道的值大于等于455，小于682，则输出“S2 Button!”
        else if (data >= 455 && data < 682)
        {
            printf("S2 Button! ADC_VALUE = %d\n", data);
        }
    }
}

// 入口函数
static void VoltageButtonDemo(void)
{
    // 初始化GPIO模块
    IoTGpioInit(HI_IO_NAME_GPIO_5);

    // 设置GPIO5为GPIO功能。
    hi_io_set_func(HI_IO_NAME_GPIO_5, HI_IO_FUNC_GPIO_5_GPIO);

    // 设置GPIO5为输入模式（引脚方向为输入）。
    IoTGpioSetDir(HI_IO_NAME_GPIO_5, IOT_GPIO_DIR_IN);

    // 设置GPIO5为上拉模式（引脚上拉）。
    // 引脚上拉后，在按键没有被按下时，读取到的值为1高电平，在按键按下时，读取到的值为0低电平。
    hi_io_set_pull(HI_IO_NAME_GPIO_5, HI_IO_PULL_UP);

    // 注册GPIO5中断处理函数。
    IoTGpioRegisterIsrFunc(HI_IO_NAME_GPIO_5,           //GPIO-05引脚
                        IOT_INT_TYPE_EDGE,              //边沿触发
                        IOT_GPIO_EDGE_FALL_LEVEL_LOW,   //下降沿触发
                        OnButtonPressed,                //中断处理函数
                        NULL);                          //中断处理函数的参数
}

// 运行入口函数
APP_FEATURE_INIT(VoltageButtonDemo);
