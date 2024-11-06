#include <stdio.h>      // 标准输入输出
#include <unistd.h>     // POSIX标准接口

#include "ohos_init.h"  // 用于初始化服务(services)和功能(features)
#include "cmsis_os2.h"  // CMSIS-RTOS API V2

#include "iot_gpio.h"   // OpenHarmony HAL：IoT硬件设备操作接口-GPIO
#include "hi_io.h"      // 海思 Pegasus SDK：IoT硬件设备操作接口-IO

// 海思 Pegasus SDK：IoT硬件设备操作接口-ADC
// Analog-to-digital conversion (ADC)
// 提供8个ADC通道，通道7为参考电压，不能adc转换
#include "hi_adc.h"

// 定义一个宏，用于标识ADC4通道
#define LIGHT_SENSOR_CHAN_NAME HI_ADC_CHANNEL_4

// 主线程函数
static void ADCLightTask(void *arg)
{
    (void)arg;
    
    // 工作循环，每隔100ms获取一次ADC4通道的值
    while (1) {
        // 保存ADC4通道的值
        unsigned short data = 0;
        // 获取ADC4通道的值
        // 读数速率较慢，请避免在中断中使用
        if (hi_adc_read(LIGHT_SENSOR_CHAN_NAME, &data, HI_ADC_EQU_MODEL_4, HI_ADC_CUR_BAIS_DEFAULT, 0)
            == HI_ERR_SUCCESS) {
            // LIGHT_SENSOR_CHAN_NAME表示ADC4通道
            // HI_ADC_EQU_MODEL_4表示采样数据平均滤波处理，平均次数4次
            // HI_ADC_CUR_BAIS_DEFAULT表示模数转换采用默认电压基准
            // 0表示从配置采样到启动采样的延时时间计数为0
            // 返回值HI_ERR_SUCCESS表示成功

            // 打印ADC4通道的值
            printf("ADC_VALUE = %d\n", (unsigned int)data);
        }
        // 等待100ms
        osDelay(10);
    }
}

// 入口函数
static void ADCLightDemo(void)
{
    // 定义线程属性
    osThreadAttr_t attr;
    attr.name = "ADCLightTask";
    attr.attr_bits = 0U;
    attr.cb_mem = NULL;
    attr.cb_size = 0U;
    attr.stack_mem = NULL;
    attr.stack_size = 4096;
    attr.priority = osPriorityNormal;

    // 创建线程
    if (osThreadNew(ADCLightTask, NULL, &attr) == NULL) {
        printf("[ADCLightDemo] Falied to create ADCLightTask!\n");
    }
}

// 运行入口函数
APP_FEATURE_INIT(ADCLightDemo);
