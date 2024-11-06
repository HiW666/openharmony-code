#include <stdio.h>                  // 标准输入输出
#include <string.h>                 // 字符串处理(操作字符数组)

#include "ohos_init.h"              // 用于初始化服务(services)和功能(features)
#include "cmsis_os2.h"              // CMSIS-RTOS API V2

#include "wifi_starter.h"           // easy wifi (ap模式)

// 主线程函数
static void WifiHotspotTask(void *arg)
{
    (void)arg;

    // WifiErrorCode类型的变量，用于接收接口返回值
    WifiErrorCode errCode;

    // 准备AP的配置参数
    // 包括：热点名称、热点密码、加密方式、频带类型、信道等
    HotspotConfig config = {0};                     // 定义热点配置
    strcpy(config.ssid, "Pegasus");                 // 设置热点名称Pegasus
    strcpy(config.preSharedKey, "OpenHarmony");     // 设置热点密码OpenHarmony
    config.securityType = WIFI_SEC_TYPE_PSK;        // 设置加密方式PSK
    config.band = HOTSPOT_BAND_TYPE_2G;             // 设置频带类型2.4G
    config.channelNum = 7;                          // 设置信道7

    // 等待100ms
    osDelay(10);

    // 输出日志
    printf("starting AP ...\r\n");

    // 开启热点
    // 参数config用于指定AP的配置，返回WIFI_SUCCESS或错误码
    errCode = StartHotspot(&config);

    // 输出接口调用结果
    printf("StartHotspot: %d\r\n", errCode);

    // 热点将开启1分钟
    // 可以通过串口工具发送 AT+PING 命令，验证WiFi客户端的连通性(比如连接到该热点的手机) 
    int timeout = 60;
    printf("After %d seconds Ap will turn off!\r\n", timeout);
    while (timeout--) {
        
        osDelay(100);
    }

    // 输出日志
    printf("stop AP ...\r\n");

    // 关闭热点
    StopHotspot();

    // 等待100ms
    osDelay(10);
}

// 入口函数
static void WifiHotspotDemo(void)
{
    // 定义线程属性
    osThreadAttr_t attr;
    attr.name = "WifiHotspotTask";
    attr.attr_bits = 0U;
    attr.cb_mem = NULL;
    attr.cb_size = 0U;
    attr.stack_mem = NULL;
    attr.stack_size = 10240;
    attr.priority = osPriorityNormal;

    // 创建线程
    if (osThreadNew(WifiHotspotTask, NULL, &attr) == NULL) {
        printf("[WifiHotspotDemo] Falied to create WifiHotspotTask!\n");
    }
}

// 运行入口函数
SYS_RUN(WifiHotspotDemo);
