#include <stdio.h>  // 标准输入输出
#include <unistd.h> // POSIX标准接口

#include "ohos_init.h" // 用于初始化服务(services)和功能(features)
#include "cmsis_os2.h" // CMSIS-RTOS API V2

#include "iot_gpio.h" // OpenHarmony HAL：IoT硬件设备操作接口-GPIO
#include "hi_io.h"    // 海思 Pegasus SDK：IoT硬件设备操作接口-IO
#include "hi_adc.h"   // 海思 Pegasus SDK：IoT硬件设备操作接口-ADC

// 定义一个宏，用于标识ADC3通道
#define HUMAN_SENSOR_CHAN_NAME HI_ADC_CHANNEL_3

// 主线程函数
static void ADCHumanTask(void *arg)
{
    (void)arg;

    // 工作循环，每隔100ms获取一次ADC3通道的值
    while (1)
    {
        // 保存ADC3通道的值
        unsigned short data = 0;
        // 获取ADC3通道的值
        // 读数速率较慢，请避免在中断中使用
        if (hi_adc_read(HUMAN_SENSOR_CHAN_NAME,  // ADC3通道
                        &data,                   // 读取到的ADC3通道的值
                        HI_ADC_EQU_MODEL_4,      // 采样数据平均滤波处理，平均次数4次
                        HI_ADC_CUR_BAIS_DEFAULT, // 模数转换采用默认电压基准
                        0)                       // 从配置采样到启动采样的延时时间计数为0
            == HI_ERR_SUCCESS)                   // 读取成功
        {
            // 打印ADC3通道的值
            printf("ADC_VALUE = %d\n", (unsigned int)data);
        }
        else
        {
            // 打印读取失败
            printf("ADC_READ_FAIL\n");
        }
        // 等待100ms
        osDelay(10);
    }
}

// 入口函数
static void ADCHumanDemo(void)
{
    // 定义线程属性
    osThreadAttr_t attr;
    attr.name = "ADCHumanTask";
    attr.attr_bits = 0U;
    attr.cb_mem = NULL;
    attr.cb_size = 0U;
    attr.stack_mem = NULL;
    attr.stack_size = 4096;
    attr.priority = osPriorityNormal;

    // 创建线程
    if (osThreadNew(ADCHumanTask, NULL, &attr) == NULL)
    {
        printf("[ADCHumanDemo] Falied to create ADCHumanTask!\n");
    }
}

// 运行入口函数
APP_FEATURE_INIT(ADCHumanDemo);
