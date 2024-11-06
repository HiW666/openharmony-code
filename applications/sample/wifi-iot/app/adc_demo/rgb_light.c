#include <stdio.h>      // 标准输入输出
#include <unistd.h>     // POSIX标准接口

#include "ohos_init.h"  // 用于初始化服务(services)和功能(features)
#include "cmsis_os2.h"  // CMSIS-RTOS API V2

#include "iot_gpio.h"   // OpenHarmony API：IoT硬件设备操作接口-GPIO
#include "hi_io.h"      // 海思 Pegasus SDK：IoT硬件设备操作接口-IO
#include "hi_pwm.h"     // 海思 Pegasus SDK：IoT硬件设备操作接口-PWM
#include "hi_adc.h"     // 海思 Pegasus SDK：IoT硬件设备操作接口-ADC

#define HUMAN_SENSOR_CHAN_NAME HI_ADC_CHANNEL_3
#define LIGHT_SENSOR_CHAN_NAME HI_ADC_CHANNEL_4

#define RED_LED_PIN_NAME HI_IO_NAME_GPIO_10
#define RED_LED_PIN_FUNCTION HI_IO_FUNC_GPIO_10_GPIO

#define GREEN_LED_PIN_NAME HI_IO_NAME_GPIO_11
#define GREEN_LED_PIN_FUNCTION HI_IO_FUNC_GPIO_11_GPIO

#define BLUE_LED_PIN_NAME HI_IO_NAME_GPIO_12
#define BLUE_LED_PIN_FUNCTION HI_IO_FUNC_GPIO_12_GPIO

#define LED_DELAY_TIME_US 300000
#define LED_BRIGHT IOT_GPIO_VALUE1
#define LED_DARK IOT_GPIO_VALUE0

#define NUM_LEDS 3
#define NUM_BLINKS 2
#define NUM_SENSORS 2

#define ADC_RESOLUTION 4096
#define PWM_FREQ_DIVITION 64000

static void CorlorfulLightTask(void *arg)
{
    (void)arg;
    unsigned int pins[] = {RED_LED_PIN_NAME, GREEN_LED_PIN_NAME, BLUE_LED_PIN_NAME};

    for (int i = 0; i < NUM_BLINKS; i++) {
        for (unsigned j = 0; j < sizeof(pins)/sizeof(pins[0]); j++) {
            IoTGpioSetOutputVal(pins[j], LED_BRIGHT);
            usleep(LED_DELAY_TIME_US);
            IoTGpioSetOutputVal(pins[j], LED_DARK);
            usleep(LED_DELAY_TIME_US);
        }
    }

    // GpioDeinit(); 
    hi_io_set_func(RED_LED_PIN_NAME, HI_IO_FUNC_GPIO_10_PWM1_OUT);
    hi_io_set_func(GREEN_LED_PIN_NAME, HI_IO_FUNC_GPIO_11_PWM2_OUT);
    hi_io_set_func(BLUE_LED_PIN_NAME, HI_IO_FUNC_GPIO_12_PWM3_OUT);

    IoTPwmInit(HI_PWM_PORT_PWM1); // R
    IoTPwmInit(HI_PWM_PORT_PWM2); // G
    IoTPwmInit(HI_PWM_PORT_PWM3); // B

    // use PWM control BLUE LED brightness
    for (int i = 1; i <= ADC_RESOLUTION; i *= 2) {
        hi_pwm_start(HI_PWM_PORT_PWM3, i, PWM_FREQ_DIVITION);
        usleep(100*1000);
        hi_pwm_stop(HI_PWM_PORT_PWM3);
    }

    while (1) {
        unsigned short duty[NUM_SENSORS] = {0, 0};
        unsigned short data[NUM_SENSORS] = {0, 0};
        static const hi_adc_channel_index chan[] = {HUMAN_SENSOR_CHAN_NAME, LIGHT_SENSOR_CHAN_NAME};
        static const hi_pwm_port port[] = {HI_PWM_PORT_PWM1, HI_PWM_PORT_PWM2};

        for (size_t i = 0; i < sizeof(chan)/sizeof(chan[0]); i++) { 
            if (hi_adc_read(chan[i], &data[i], HI_ADC_EQU_MODEL_4, HI_ADC_CUR_BAIS_DEFAULT, 0)
                == HI_ERR_SUCCESS) {
                duty[i] = PWM_FREQ_DIVITION * (unsigned int)data[i] / ADC_RESOLUTION;
            }
            hi_pwm_start(port[i], duty[i], PWM_FREQ_DIVITION);
            usleep(10000);
            hi_pwm_stop(port[i]);
        }
    }
}

static void ColorfulLightDemo(void)
{
    osThreadAttr_t attr;

    IoTGpioInit(RED_LED_PIN_NAME);
    IoTGpioInit(GREEN_LED_PIN_NAME);
    IoTGpioInit(BLUE_LED_PIN_NAME);

    // set Red/Green/Blue LED pin to GPIO function
    hi_io_set_func(RED_LED_PIN_NAME, RED_LED_PIN_FUNCTION);
    hi_io_set_func(GREEN_LED_PIN_NAME, GREEN_LED_PIN_FUNCTION);
    hi_io_set_func(BLUE_LED_PIN_NAME, BLUE_LED_PIN_FUNCTION);

    // set Red/Green/Blue LED pin as output
    IoTGpioSetDir(RED_LED_PIN_NAME, IOT_GPIO_DIR_OUT);
    IoTGpioSetDir(GREEN_LED_PIN_NAME, IOT_GPIO_DIR_OUT);
    IoTGpioSetDir(BLUE_LED_PIN_NAME, IOT_GPIO_DIR_OUT);

    attr.name = "CorlorfulLightTask";
    attr.attr_bits = 0U;
    attr.cb_mem = NULL;
    attr.cb_size = 0U;
    attr.stack_mem = NULL;
    attr.stack_size = 4096;
    attr.priority = osPriorityNormal;

    if (osThreadNew(CorlorfulLightTask, NULL, &attr) == NULL) {
        printf("[ColorfulLightDemo] Falied to create CorlorfulLightTask!\n");
    }
}

APP_FEATURE_INIT(ColorfulLightDemo);
