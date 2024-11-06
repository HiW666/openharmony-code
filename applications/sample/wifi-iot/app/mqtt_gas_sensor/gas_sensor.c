// 环境监测板MQ-2可燃气体传感器接口文件。

#include <stdio.h>      // 标准输入输出
#include <unistd.h>     // POSIX标准接口

#include "ohos_init.h"  // 用于初始化服务(services)和功能(features)
#include "cmsis_os2.h"  // CMSIS-RTOS API V2

#include "iot_gpio.h"   // OpenHarmony HAL：IoT硬件设备操作接口-GPIO
#include "hi_io.h"      // 海思 Pegasus SDK：IoT硬件设备操作接口-IO
#include "hi_adc.h"     // 海思 Pegasus SDK：IoT硬件设备操作接口-ADC

// 定义一个宏，用于标识ADC5通道（MQ-2可燃气体传感器）
#define GAS_SENSOR_CHAN_NAME HI_ADC_CHANNEL_5

// 将ADC值转换为电压值
static float ConvertToVoltage(unsigned short data)
{
    return (float)data * 1.8 * 4 / 4096;
}

// 获取可燃气体浓度
float GetGasLevel()
{
    // 用于接收MQ-2可燃气体传感器的值
    unsigned short data = 0;

    // 读取ADC5通道的值
    if (hi_adc_read(GAS_SENSOR_CHAN_NAME, &data, HI_ADC_EQU_MODEL_4,
                    HI_ADC_CUR_BAIS_DEFAULT, 0) == HI_ERR_SUCCESS)
    {
        // 空气中可燃气体（或烟雾）浓度增加 -> 燃气传感器的电阻值降低 -> ADC通道的电压增大
        // 转换为电压值
        float Vx = ConvertToVoltage(data);

        // 计算燃气传感器的电阻值
        // Vcc            ADC            GND
        //  |    ______   |     ______   |
        //  +---| MG-2 |---+---| 1kom |---+
        //       ------         ------
        // 查阅原理图，ADC 引脚位于 1K 电阻和燃气传感器之间，燃气传感器另一端接在 5V 电源正极上
        // 串联电路电压和阻值成正比：
        // Vx / 5 == 1kom / (1kom + Rx)
        //   => Rx + 1 == 5/Vx
        //   =>  Rx = 5/Vx - 1
        float gasSensorResistance = 5 / Vx - 1;

        // 日志输出ADC值、电阻值
        printf("ADC_VALUE = %d, gasSensorResistance = %f\n", data, gasSensorResistance);

        // 返回燃气传感器的电阻值
        return gasSensorResistance;
    }

    // 读取失败返回0
    return 0;
}
