// 中断+消息队列方式
// Hi3861芯片的相应引脚在作为ADC通道使用时，GPIO中断依然能够触发。

#include <stdio.h>      // 标准输入输出
#include <unistd.h>     // POSIX标准接口

#include "ohos_init.h"  // 用于初始化服务(services)和功能(features)
#include "cmsis_os2.h"  // CMSIS-RTOS API V2

#include "iot_gpio.h"   // OpenHarmony HAL：IoT硬件设备操作接口-GPIO
#include "hi_io.h"      // 海思 Pegasus SDK：IoT硬件设备操作接口-IO
#include "hi_adc.h"     // 海思 Pegasus SDK：IoT硬件设备操作接口-ADC

// 定义一个宏，用于标识ADC2通道
#define BUTTONS_CHAN_NAME HI_ADC_CHANNEL_2

// 定义一个消息队列
static osMessageQueueId_t buttons_queue;

// GPIO5的中断处理函数
static void OnButtonPressed(char *arg)
{
    (void)arg;

    // 定义消息内容，本例不关心消息内容，值随意。
    int msg = 1;

    // 一般来说，中断处理分为中断上半部和中断下半部。
    // 中断上半部为中断服务函数，在这个函数中不建议处理具体业务，只需要通过系统同步机制（如消息队列）通知“业务处理线程”即可。
    // 发送到消息队列
    osMessageQueuePut(buttons_queue, &msg, 0, 0);
}

// 主线程函数
// 这个“业务处理线程”就属于中断下半部，负责实现具体业务。像ADC读数这种比较耗时的操作，就很适合在业务处理线程中实现。
// 在业务代码中通过阻塞方式读取消息队列，拿到消息则处理业务，没有消息则线程挂起，不占用CPU资源。
static void VoltageButtonTask(void *arg)
{
    (void)arg;

    // 用于存放消息内容
    int msg;

    // 用于存放ADC2通道的值
    unsigned short data = 0;

    // 工作循环
    while (1)
    {
        // 从消息队列中取出消息
        // 如果消息队列为空，则会等待消息队列的消息，直到消息队列有消息，才会取出消息
        osMessageQueueGet(buttons_queue, &msg, 0, osWaitForever);

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
        }
    }
}

// 入口函数
static void VoltageButtonDemo(void)
{
    // 创建消息队列
    buttons_queue = osMessageQueueNew(100, sizeof(int), NULL);
    // 判断消息队列是否创建成功
    if (buttons_queue == NULL)
    {
        printf("[%s] Create buttons queue failed!\n", __func__);
        return;
    }

    // 初始化GPIO模块
    IoTGpioInit(HI_IO_NAME_GPIO_5);

    // 设置GPIO5为GPIO功能。
    hi_io_set_func(HI_IO_NAME_GPIO_5, HI_IO_FUNC_GPIO_5_GPIO);

    // 设置GPIO5为输入模式（引脚方向为输入）。
    IoTGpioSetDir(HI_IO_NAME_GPIO_5, IOT_GPIO_DIR_IN);

    // 设置GPIO5为上拉模式（引脚上拉）。
    // 引脚上拉后，在按键没有被按下时，读取到的值为1高电平，在按键按下时，读取到的值为0低电平。
    hi_io_set_pull(HI_IO_NAME_GPIO_5, HI_IO_PULL_UP);

    // 注册GPIO5中断处理函数。
    IoTGpioRegisterIsrFunc(HI_IO_NAME_GPIO_5,           //GPIO-05引脚
                        IOT_INT_TYPE_EDGE,              //边沿触发
                        IOT_GPIO_EDGE_FALL_LEVEL_LOW,   //下降沿触发
                        OnButtonPressed,                //中断处理函数
                        NULL);                          //中断处理函数的参数

    // 定义线程属性
    osThreadAttr_t attr = {0};
    attr.name = "VoltageButtonTask";
    attr.stack_size = 4096;
    attr.priority = osPriorityNormal;

    // 创建线程
    if (osThreadNew(VoltageButtonTask, NULL, &attr) == NULL)
    {
        printf("[%s] Create VoltageButtonTask failed!\n", __func__);
    }
}

// 运行入口函数
APP_FEATURE_INIT(VoltageButtonDemo);
