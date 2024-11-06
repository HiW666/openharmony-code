#include <stdio.h>      // 标准输入输出
#include <unistd.h>     // POSIX标准接口

#include "ohos_init.h"  // 用于初始化服务(services)和功能(features)
#include "cmsis_os2.h"  // CMSIS-RTOS API V2

#include "iot_gpio.h"   // OpenHarmony HAL：IoT硬件设备操作接口-GPIO
#include "hi_io.h"      // 海思 Pegasus SDK：IoT硬件设备操作接口-IO
#include "hi_adc.h"     // 海思 Pegasus SDK：IoT硬件设备操作接口-ADC

// 定义一个宏，用于标识ADC2通道
#define BUTTONS_CHAN_NAME HI_ADC_CHANNEL_2

// 主线程函数
static void VoltageButtonTask(void *arg)
{
    (void)arg;

    // 工作循环，每隔100ms获取一次ADC2通道的值
    while (1)
    {
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
            // else
            // {
            //     printf("ADC_VALUE = %d\n", data);
            // }
        }

        //一定要注意加等待，否则其他任务不能得到调度。
        //等待时间就是按键的采集周期，这里是100ms，可以根据产品对按键的响应灵敏度要求来设置。
        osDelay(10);
    }
}

// 入口函数
static void VoltageButtonDemo(void)
{
    // 定义线程属性
    osThreadAttr_t attr = {0};
    attr.name = "VoltageButtonTask";
    attr.stack_size = 4096;
    attr.priority = osPriorityNormal;

    // 创建线程
    if (osThreadNew(VoltageButtonTask, NULL, &attr) == NULL)
    {
        printf("[VoltageButtonDemo] Create VoltageButtonTask failed!\n");
    }
}

// 运行入口函数
APP_FEATURE_INIT(VoltageButtonDemo);
