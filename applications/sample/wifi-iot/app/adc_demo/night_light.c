#include <stdio.h>  // 标准输入输出
#include <unistd.h> // POSIX标准接口

#include "ohos_init.h" // 用于初始化服务(services)和功能(features)
#include "cmsis_os2.h" // CMSIS-RTOS API V2

#include "iot_gpio.h" // OpenHarmony HAL：IoT硬件设备操作接口-GPIO
#include "iot_pwm.h"  // OpenHarmony HAL：IoT硬件设备操作接口-PWM
#include "hi_io.h"    // 海思 Pegasus SDK：IoT硬件设备操作接口-IO
#include "hi_pwm.h"   // 海思 Pegasus SDK：IoT硬件设备操作接口-PWM
#include "hi_adc.h"   // 海思 Pegasus SDK：IoT硬件设备操作接口-ADC

// 定义一个宏，用于标识ADC3通道
#define HUMAN_SENSOR_CHAN_NAME HI_ADC_CHANNEL_3
// 定义一个宏，用于标识ADC4通道
#define LIGHT_SENSOR_CHAN_NAME HI_ADC_CHANNEL_4

// 定义一个宏，用于标识GPIO-10
#define RED_LED_PIN_NAME HI_IO_NAME_GPIO_10
// 定义一个宏，用于标识红色LED灯引脚功能为PWM1输出
#define RED_LED_PIN_FUNCTION HI_IO_FUNC_GPIO_10_PWM1_OUT

// 定义一个宏，用于标识GPIO-11
#define GREEN_LED_PIN_NAME HI_IO_NAME_GPIO_11
// 定义一个宏，用于标识绿色LED灯引脚功能为PWM2输出
#define GREEN_LED_PIN_FUNCTION HI_IO_FUNC_GPIO_11_PWM2_OUT

// 定义一个宏，用于标识GPIO-12
#define BLUE_LED_PIN_NAME HI_IO_NAME_GPIO_12
// 定义一个宏，用于标识蓝色LED灯引脚功能为PWM3输出
#define BLUE_LED_PIN_FUNCTION HI_IO_FUNC_GPIO_12_PWM3_OUT

// 主线程函数 
static void NightLightTask(void *arg)
{
    (void)arg;

    // 初始化GPIO
    IoTGpioInit(RED_LED_PIN_NAME);   // 红色灯引脚
    IoTGpioInit(GREEN_LED_PIN_NAME); // 绿色灯引脚
    IoTGpioInit(BLUE_LED_PIN_NAME);  // 蓝色灯引脚

    // 设置引脚功能为PWM输出
    hi_io_set_func(RED_LED_PIN_NAME, RED_LED_PIN_FUNCTION);     // 红色灯引脚
    hi_io_set_func(GREEN_LED_PIN_NAME, GREEN_LED_PIN_FUNCTION); // 绿色灯引脚
    hi_io_set_func(BLUE_LED_PIN_NAME, BLUE_LED_PIN_FUNCTION);   // 蓝色灯引脚

    // 设置引脚方向为输出
    IoTGpioSetDir(RED_LED_PIN_NAME, IOT_GPIO_DIR_OUT);   // 红色灯引脚
    IoTGpioSetDir(GREEN_LED_PIN_NAME, IOT_GPIO_DIR_OUT); // 绿色灯引脚
    IoTGpioSetDir(BLUE_LED_PIN_NAME, IOT_GPIO_DIR_OUT);  // 蓝色灯引脚

    // 初始化PWM
    IoTPwmInit(HI_PWM_PORT_PWM1); // 红色灯PWM1
    IoTPwmInit(HI_PWM_PORT_PWM2); // 绿色灯PWM2
    IoTPwmInit(HI_PWM_PORT_PWM3); // 蓝色灯PWM3

    // 工作循环，每隔100ms获取一次人体红外传感器和光敏电阻传感器的值
    while (1)
    {
        // 读取人体红外传感器的值
        unsigned short dataHuman = 0;   // 用于接收人体红外传感器的值
        // 读取ADC3通道的值
        if (hi_adc_read(HUMAN_SENSOR_CHAN_NAME, &dataHuman, HI_ADC_EQU_MODEL_4, HI_ADC_CUR_BAIS_DEFAULT, 0) == HI_ERR_SUCCESS)
        {
            // 成功，输出人体红外传感器的值
            printf("dataHuman = %d\n", (unsigned int)dataHuman);
        }
        else
        {
            // 失败，本轮不执行任何操作
            continue;
        }

        // 读取光敏电阻传感器的值
        unsigned short dataLight = 0;   // 用于接收光敏电阻传感器的值
        // 读取ADC4通道的值
        if (hi_adc_read(LIGHT_SENSOR_CHAN_NAME, &dataLight, HI_ADC_EQU_MODEL_4, HI_ADC_CUR_BAIS_DEFAULT, 0) == HI_ERR_SUCCESS)
        {
            // 成功，输出光敏电阻传感器的值
            printf("dataLight = %d\n", (unsigned int)dataLight);
        }
        else
        {
            // 失败，本轮不执行任何操作
            continue;
        }

        // 智能夜灯：白天不亮。夜间，无人靠近不亮，人员靠近亮起。
        if (dataHuman > 1200 && dataLight > 1200)   // 夜间有人靠近，亮灯
        {
            // 亮度系数，负责整体亮度调节
            float brightness = 1.0;

            // 发色，亮度系数影响占空比
            IoTPwmStart(HI_PWM_PORT_PWM1, (unsigned short)(65 * brightness), 4000);
            IoTPwmStart(HI_PWM_PORT_PWM2, (unsigned short)(39 * brightness), 4000);
            IoTPwmStart(HI_PWM_PORT_PWM3, (unsigned short)(8 * brightness), 4000);
        }
        else    // 白天，和夜间无人靠近，灭灯
        {
            IoTPwmStop(HI_PWM_PORT_PWM1);
            IoTPwmStop(HI_PWM_PORT_PWM2);
            IoTPwmStop(HI_PWM_PORT_PWM3);
        }
        
        // 等待100ms
        osDelay(10);
    }
}

// 入口函数
static void NightLightDemo(void)
{
    // 定义线程属性
    osThreadAttr_t attr;
    attr.name = "NightLightTask";
    attr.attr_bits = 0U;
    attr.cb_mem = NULL;
    attr.cb_size = 0U;
    attr.stack_mem = NULL;
    attr.stack_size = 4096;
    attr.priority = osPriorityNormal;

    // 创建线程
    if (osThreadNew(NightLightTask, NULL, &attr) == NULL)
    {
        printf("[NightLightDemo] Falied to create NightLightTask!\n");
    }
}

// 运行入口函数
APP_FEATURE_INIT(NightLightDemo);
