// Pegasus交通灯板三色灯与主控芯片引脚的对应关系
// GPIO_10 连接红色LED，输出高电平点亮
// GPIO_11 连接绿色LED，输出高电平点亮
// GPIO_12 连接黄色LED，输出高电平点亮

#include <stdio.h>      // 标准输入输出
#include <unistd.h>     // POSIX标准接口

#include "ohos_init.h"  // 用于初始化服务(services)和功能(features)
#include "cmsis_os2.h"  // CMSIS-RTOS API V2

#include "iot_gpio.h"   // OpenHarmony HAL：IoT硬件设备操作接口-GPIO
#include "hi_io.h"      // 海思 Pegasus SDK：IoT硬件设备操作接口-IO
#include "hi_pwm.h"     // 海思 Pegasus SDK：IoT硬件设备操作接口-PWM

// 定义GPIO引脚，尽量避免直接用数值
#define BUTTON_GPIO 8   // 按钮
#define RED_GPIO 10     // 红色LED
#define GREEN_GPIO 11   // 绿色LED
#define YELLOW_GPIO 12  // 黄色LED

// 定义三色led灯的状态
static int g_ledStates[3] = {0, 0, 0};
// 当前点亮的灯
static int g_currentBright = 0;
// 蜂鸣器是否鸣叫
static int g_beepState = 0;

// 主线程函数
static void *TrafficLightTask(const char *arg)
{
    (void)arg;

    // 输出提示信息
    printf("TrafficLightTask start!\r\n");

    // 将三色灯的GPIO引脚放入数组，以便使用循环进行控制
    unsigned int pins[] = {RED_GPIO, GREEN_GPIO, YELLOW_GPIO};

    // 开始输出PWM信号
    hi_pwm_start(HI_PWM_PORT_PWM0, 20 * 1000, 40 * 1000);
    // 等待1s（让蜂鸣器鸣叫1s）
    usleep(1000 * 1000);
    // 停止输出PWM信号
    hi_pwm_stop(HI_PWM_PORT_PWM0);

    // 让三色灯循环亮灭4轮
    for (int i = 0; i < 4; i++)
    {
        // 每一轮按顺序亮灭三色灯
        for (unsigned int j = 0; j < 3; j++)
        {
            // 点亮
            IoTGpioSetOutputVal(pins[j], IOT_GPIO_VALUE1);
            // 等待200ms
            usleep(200 * 1000);

            // 熄灭
            IoTGpioSetOutputVal(pins[j], IOT_GPIO_VALUE0);
            // 等待100ms
            usleep(100 * 1000);
        }
    }

    // 工作循环
    while (1)
    {
        // 设置每个灯的点亮或熄灭状态
        for (unsigned int j = 0; j < 3; j++)
        {
            IoTGpioSetOutputVal(pins[j], g_ledStates[j]);
        }
        // 如果允许，让蜂鸣器鸣叫100ms，然后禁止蜂鸣器鸣叫
        if (g_beepState)
        {
            // 开始输出PWM信号
            hi_pwm_start(HI_PWM_PORT_PWM0, 20 * 1000, 40 * 1000);
            // 等待100ms（让蜂鸣器鸣叫100ms）
            usleep(100 * 1000);
            // 停止输出PWM信号
            hi_pwm_stop(HI_PWM_PORT_PWM0);
            // 禁止蜂鸣器鸣叫
            g_beepState = 0;
        }
        // 等待100ms
        usleep(100 * 1000);
    }

    return NULL;
}

// 按钮(GPIO8)的中断处理函数。
static void OnButtonPressed(char *arg)
{
    (void)arg;

    // 设置三色led灯的状态
    for (int i = 0; i < 3; i++)
    {
        // 根据g_currentBright设置当前点亮的灯
        if (i == g_currentBright)
        {
            g_ledStates[i] = 1;
        }
        // 其他灯熄灭
        else
        {
            g_ledStates[i] = 0;
        }
    }
    // 实现按键切灯
    g_currentBright++;
    // 循环点亮三色灯
    if (g_currentBright == 3)
        g_currentBright = 0;

    // 让蜂鸣器鸣叫
    g_beepState = 1;
}

// 入口函数
static void StartTrafficLightTask(void)
{
    // 定义线程属性
    osThreadAttr_t attr;
    attr.name = "TrafficLightTask";
    attr.attr_bits = 0U;
    attr.cb_mem = NULL;
    attr.cb_size = 0U;
    attr.stack_mem = NULL;
    attr.stack_size = 1024;
    attr.priority = osPriorityNormal;

    // 初始化GPIO模块
    IoTGpioInit(RED_GPIO);          // 红色灯
    IoTGpioInit(GREEN_GPIO);        // 绿色灯
    IoTGpioInit(YELLOW_GPIO);       // 黄色灯
    IoTGpioInit(BUTTON_GPIO);       // 按钮
    IoTGpioInit(HI_IO_NAME_GPIO_9); // 蜂鸣器

    // 设置GPIO10为GPIO功能。
    hi_io_set_func(RED_GPIO, HI_IO_FUNC_GPIO_10_GPIO);
    // 设置GPIO10为输出模式（引脚方向为输出）。
    IoTGpioSetDir(RED_GPIO, IOT_GPIO_DIR_OUT);

    // 设置GPIO11为GPIO功能。
    hi_io_set_func(GREEN_GPIO, HI_IO_FUNC_GPIO_11_GPIO);
    // 设置GPIO11为输出模式（引脚方向为输出）。
    IoTGpioSetDir(GREEN_GPIO, IOT_GPIO_DIR_OUT);

    // 设置GPIO12为GPIO功能。
    hi_io_set_func(YELLOW_GPIO, HI_IO_FUNC_GPIO_12_GPIO);
    // 设置GPIO12为输出模式（引脚方向为输出）。
    IoTGpioSetDir(YELLOW_GPIO, IOT_GPIO_DIR_OUT);

    // 设置GPIO8为GPIO功能。
    hi_io_set_func(BUTTON_GPIO, HI_IO_FUNC_GPIO_8_GPIO);
    // 设置GPIO8为输入模式（引脚方向为输入）。
    IoTGpioSetDir(BUTTON_GPIO, IOT_GPIO_DIR_IN);
    // 设置GPIO8为上拉模式（引脚上拉）。
    hi_io_set_pull(BUTTON_GPIO, HI_IO_PULL_UP);
    // 注册GPIO8中断处理函数。
    IoTGpioRegisterIsrFunc(BUTTON_GPIO,                 //GPIO-08引脚
                        IOT_INT_TYPE_EDGE,              //边沿触发
                        IOT_GPIO_EDGE_FALL_LEVEL_LOW,   //下降沿触发
                        OnButtonPressed,                //中断处理函数
                        NULL);                          //中断处理函数的参数

    // 设置GPIO9为PWM0输出
    hi_io_set_func(HI_IO_NAME_GPIO_9, HI_IO_FUNC_GPIO_9_PWM0_OUT);
    // 设置GPIO9为输出模式（引脚方向为输出）。
    IoTGpioSetDir(HI_IO_NAME_GPIO_9, IOT_GPIO_DIR_OUT);
    // 初始化PWM模块
    IoTPwmInit(HI_PWM_PORT_PWM0);

    // 创建线程
    if (osThreadNew((osThreadFunc_t)TrafficLightTask, NULL, &attr) == NULL)
    {
        printf("Falied to create TrafficLightTask!\n");
    }
}

// 运行入口函数
APP_FEATURE_INIT(StartTrafficLightTask);
