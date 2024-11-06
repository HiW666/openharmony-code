// 环境监测板AHT20数字温湿度传感器接口文件。

#include <stdio.h>      // 标准输入输出
#include <unistd.h>     // POSIX标准接口

#include "ohos_init.h"  // 用于初始化服务(services)和功能(features)
#include "cmsis_os2.h"  // CMSIS-RTOS API V2

#include "iot_gpio.h"   // OpenHarmony HAL：IoT硬件设备操作接口-GPIO
#include "iot_i2c.h"    // OpenHarmony HAL：IoT硬件设备操作接口-I2C
#include "iot_errno.h"  // OpenHarmony HAL：IoT硬件设备操作接口-错误代码定义
#include "hi_io.h"      // 海思 Pegasus SDK：IoT硬件设备操作接口-IO
#include "hi_adc.h"     // 海思 Pegasus SDK：IoT硬件设备操作接口-ADC

#include "aht20.h"      // AHT20数字温湿度传感器驱动接口

// 定义一个宏，用于标识AHT20的波特率（传输速率）
#define AHT20_BAUDRATE 400 * 1000

// 定义一个宏，用于标识要使用的I2C总线编号是I2C0
#define AHT20_I2C_IDX 0

/// @brief 初始化AHT20数字温湿度传感器
void InitTempHumiSensor()
{
    // 初始化GPIO
    IoTGpioInit(HI_IO_NAME_GPIO_13);
    IoTGpioInit(HI_IO_NAME_GPIO_14);

    // 设置GPIO-13引脚功能为I2C0_SDA
    hi_io_set_func(HI_IO_NAME_GPIO_13, HI_IO_FUNC_GPIO_13_I2C0_SDA);

    // 设置GPIO-14引脚功能为I2C0_SCL
    hi_io_set_func(HI_IO_NAME_GPIO_14, HI_IO_FUNC_GPIO_14_I2C0_SCL);

    // 用指定的波特速率初始化I2C0
    IoTI2cInit(AHT20_I2C_IDX, AHT20_BAUDRATE);

    // 校准AHT20，如果校准失败，等待100ms后重试
    while (IOT_SUCCESS != AHT20_Calibrate())
    {
        printf("AHT20 sensor init failed!\r\n");
        usleep(100 * 1000);
    }
}

/// @brief 获取温湿度值
/// @param[out] temp 温度值
/// @param[out] humi 湿度值
/// @return 成功返回0，失败返回-1
int GetTempHumiLevel(float *temp, float *humi)
{
    // 接收接口的返回值
    uint32_t retval = 0;

    // 启动测量
    retval = AHT20_StartMeasure();
    if (retval != IOT_SUCCESS)
    {
        printf("trigger measure failed!\r\n");
        return -1;
    }

    // 接收测量结果
    retval = AHT20_GetMeasureResult(temp, humi);
    if (retval != IOT_SUCCESS)
    {
        printf("get data failed!\r\n");
        return -1;
    }

    // 输出测量结果
    printf("temperature: %.2f, humidity: %.2f\r\n", *temp, *humi);

    // 返回成功
    return 0;
}
