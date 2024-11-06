#include <stdio.h>      // 标准输入输出
#include <unistd.h>     // POSIX标准接口

#include "ohos_init.h"  // 用于初始化服务(services)和功能(features)
#include "cmsis_os2.h"  // CMSIS-RTOS API V2

#include "iot_gpio.h"   // OpenHarmony HAL：IoT硬件设备操作接口-GPIO
#include "hi_io.h"      // 海思 Pegasus SDK：IoT硬件设备操作接口-IO
#include "hi_adc.h"     // 海思 Pegasus SDK：IoT硬件设备操作接口-ADC

// OLED显示屏简化版驱动接口
#include "oled_ssd1306.h"

// 定义一个宏，用于标识ADC2通道
#define ANALOG_KEY_CHAN_NAME HI_ADC_CHANNEL_2

// 将ADC值转换为电压值
static float ConvertToVoltage(unsigned short data)
{
    return (float)data * 1.8 * 4 / 4096;
}

// 主线程函数
static void OledTask(void *arg)
{
    (void)arg;

    // 初始化SSD1306显示屏驱动芯片
    OledInit();

    // 全屏填充黑色
    OledFillScreen(0x00);

    // 显示字符串OpenHarmony
    OledShowString(20, 3, "OpenHarmony", FONT8x16);   //居中

    // 等待3秒
    sleep(3);

    // 依次显示3屏内容
    for (int i = 0; i < 3; i++) {
        // 全屏填充黑色
        OledFillScreen(0x00);
        // 显示8行ABCDEFGHIJKLMNOP
        for (int y = 0; y < 8; y++) {
            static const char text[] = "ABCDEFGHIJKLMNOP"; // QRSTUVWXYZ
            OledShowString(0, y, text, FONT6x8);
        }
        // 等待1秒
        sleep(1);
    }

    // 全屏填充黑色
    OledFillScreen(0x00);

    // 工作循环
    while (1) {
        // 要显示的字符串
        static char text[128] = {0};
        // 用于存放ADC2通道的值
        unsigned short data = 0;
        // 读取ADC2通道的值
        hi_adc_read(ANALOG_KEY_CHAN_NAME, &data, HI_ADC_EQU_MODEL_4, HI_ADC_CUR_BAIS_DEFAULT, 0);
        // 转换为电压值
        float voltage = ConvertToVoltage(data);
        // 格式化字符串
        snprintf(text, sizeof(text), "voltage: %.3f!", voltage);
        // 显示字符串
        OledShowString(0, 1, text, FONT6x8);
        // 等待30ms
        usleep(30*1000);
    }
}

// 入口函数
static void OledDemo(void)
{
    // 定义线程属性
    osThreadAttr_t attr;
    attr.name = "OledTask";
    attr.attr_bits = 0U;
    attr.cb_mem = NULL;
    attr.cb_size = 0U;
    attr.stack_mem = NULL;
    attr.stack_size = 4096;
    attr.priority = osPriorityNormal;

    // 创建线程
    if (osThreadNew(OledTask, NULL, &attr) == NULL) {
        printf("[OledDemo] Falied to create OledTask!\n");
    }
}

// 运行入口函数
APP_FEATURE_INIT(OledDemo);
