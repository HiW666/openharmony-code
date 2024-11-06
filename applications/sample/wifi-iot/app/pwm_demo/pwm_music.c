//原理说明：https://harmonyos.51cto.com/posts/1511

#include <stdio.h>      // 标准输入输出

#include "ohos_init.h"  // 用于初始化服务(services)和功能(features)
#include "cmsis_os2.h"  // CMSIS-RTOS API V2

#include "iot_gpio.h"   // OpenHarmony API：IoT硬件设备操作接口-GPIO
#include "hi_io.h"      // 海思 Pegasus SDK：IoT硬件设备操作接口-IO
#include "hi_pwm.h"     // 海思 Pegasus SDK：IoT硬件设备操作接口-PWM

// 分频倍数数组
// 40MHz 对应的分频倍数：
static const uint16_t g_tuneFreqs[] = {
    0,     // 不使用
    38223, // 1046.5Hz  C6(do)
    34052, // 1174.7Hz  D6(re)
    30338, // 1318.5Hz  E6(mi)
    28635, // 1396.9Hz  F6(fa)
    25511, // 1568Hz    G6(so)
    22728, // 1760Hz    A6(la)
    20249, // 1975.5Hz  B6(si)
    51020  // 784Hz     G5(so)低八度
};

// 曲谱音符数组：《两只老虎》。
// 简谱：http://www.jianpu.cn/pu/33/33945.htm
static const uint8_t g_scoreNotes[] = {
    1, 2, 3, 1,        1, 2, 3, 1,        3, 4, 5,  3, 4, 5,
    5, 6, 5, 4, 3, 1,  5, 6, 5, 4, 3, 1,  1, 8, 1,  1, 8, 1, // 最后两个 5 应该是低八度的，链接图片中的曲谱不对，声音到最后听起来不太对劲
};

// 曲谱音符数组：《蜜雪冰城主题曲》
// static const uint8_t g_scoreNotes[] = {
//     3, 5, 5, 6, 5, 3, 1, 1, 2, 3, 3, 2, 1, 2,
//     3, 5, 5, 6, 5, 3, 1, 1, 2, 3, 3, 2, 2, 1,
//     4, 4, 4, 6, 5, 5, 3, 2,
//     3, 5, 5, 6, 5, 3, 1, 1, 2, 3, 3, 2, 2, 1
// };

// 曲谱时值数组：《两只老虎》。
// 根据简谱记谱方法转写。
// 4/4拍中下面划一条线是半拍，划两条线是四分之一拍，点是顺延半拍
static const uint8_t g_scoreDurations[] = {
    4, 4, 4, 4,        4, 4, 4, 4,        4, 4, 8,  4, 4, 8,
    3, 1, 3, 1, 4, 4,  3, 1, 3, 1, 4, 4,  4, 4, 8,  4, 4, 8,
};

// 曲谱时值数组：《蜜雪冰城主题曲》
// static const uint8_t g_scoreDurations[] = {
//     2, 2, 3, 1, 2, 2, 2, 1, 1, 2, 2, 2, 2, 8,
//     2, 2, 3, 1, 2, 2, 2, 1, 1, 2, 2, 2, 2, 8,
//     4, 4, 2, 6, 4, 3, 1, 8,
//     2, 2, 3, 1, 2, 2, 2, 1, 1, 2, 2, 2, 2, 8
// };

// 主线程函数
static void BeeperMusicTask(void *arg)
{
    (void)arg;

    printf("BeeperMusicTask start!\r\n");   // 日志输出

    hi_pwm_set_clock(PWM_CLK_XTAL); // 设置时钟源为外部晶体时钟（40MHz，默认时钟源160MHz）

    // 演奏音乐
    // 使用循环遍历曲谱音符数组
    for (size_t i = 0; i < sizeof(g_scoreNotes)/sizeof(g_scoreNotes[0]); i++) {
        // 获取音符，也就是分频倍数数组元素的下标
        uint32_t tune = g_scoreNotes[i];
        // 获取分频倍数
        uint16_t freqDivisor = g_tuneFreqs[tune];
        // 获取音符时间
        // 适当拉长时间，四分之一拍用时125ms，每小节2s
        uint32_t tuneInterval = g_scoreDurations[i] * (125*1000);
        // 日志输出
        printf("%d %d %d %d\r\n", tune, (40*1000*1000) / freqDivisor, freqDivisor, tuneInterval);
        // 开始输出PWM信号，占空比50%
        hi_pwm_start(HI_PWM_PORT_PWM0, freqDivisor/2, freqDivisor);
        // 等待音符时间，参数的单位是微秒（千分之一毫秒）
        usleep(tuneInterval);
        // 停止输出PWM信号
        hi_pwm_stop(HI_PWM_PORT_PWM0);
        // 停止一个音符后，等待20ms，让两次发音有个间隔，听起来更自然一些
        usleep(20*1000);
    }
}

// 入口函数
static void StartBeepMusicTask(void)
{
    // 定义线程属性
    osThreadAttr_t attr;
    attr.name = "BeeperMusicTask";
    attr.stack_size = 4096;
    attr.priority = osPriorityNormal;

    // 蜂鸣器引脚 设置为 PWM功能
    IoTGpioInit(HI_IO_NAME_GPIO_9);
    hi_io_set_func(HI_IO_NAME_GPIO_9, HI_IO_FUNC_GPIO_9_PWM0_OUT);
    IoTGpioSetDir(HI_IO_NAME_GPIO_9, IOT_GPIO_DIR_OUT);
    IoTPwmInit(HI_PWM_PORT_PWM0);

    // 创建线程
    if (osThreadNew(BeeperMusicTask, NULL, &attr) == NULL) {
        printf("[BeeperMusicExample] Falied to create BeeperMusicTask!\n");
    }
}

// 运行入口函数
SYS_RUN(StartBeepMusicTask);