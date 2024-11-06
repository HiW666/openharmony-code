// 炫彩灯板光敏电阻接口文件。

#include <stdio.h>      // 标准输入输出
#include <unistd.h>     // POSIX标准接口

#include "ohos_init.h"  // 用于初始化服务(services)和功能(features)
#include "cmsis_os2.h"  // CMSIS-RTOS API V2

#include "iot_gpio.h"   // OpenHarmony API：IoT硬件设备操作接口-GPIO
#include "hi_io.h"      // 海思 Pegasus SDK：IoT硬件设备操作接口-IO
#include "hi_adc.h"     // 海思 Pegasus SDK：IoT硬件设备操作接口-ADC

// 定义一个宏，用于标识ADC4通道
#define LIGHT_SENSOR_CHAN_NAME HI_ADC_CHANNEL_4

// 获取环境光强度
unsigned short GetLightLevel()
{
    // 保存ADC4通道的值
    unsigned short data = 0;

    // 获取ADC4通道的值
    if (hi_adc_read(LIGHT_SENSOR_CHAN_NAME, &data, 
                    HI_ADC_EQU_MODEL_4, HI_ADC_CUR_BAIS_DEFAULT, 0) == HI_ERR_SUCCESS)
    {
        // 获取成功
        printf("ADC_VALUE = %d\n", (unsigned int)data);     // 输出ADC4通道的值
        return data;                                        // 返回ADC4通道的值
    }

    // 获取失败返回0
    return 0;
}
