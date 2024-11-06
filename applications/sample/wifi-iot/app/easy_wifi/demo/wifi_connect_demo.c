#include <stdio.h>                  // 标准输入输出
#include <string.h>                 // 字符串处理(操作字符数组)

#include "ohos_init.h"              // 用于初始化服务(services)和功能(features)
#include "cmsis_os2.h"              // CMSIS-RTOS API V2

#include "wifi_connecter.h"         // easy wifi (station模式)

// 主线程函数
static void WifiConnectTask(void *arg)
{
    (void)arg;

    // 等待100ms
    osDelay(10);

    // 定义热点配置
    WifiDeviceConfig apConfig = {0};

    // 设置热点配置中的SSID
    strcpy(apConfig.ssid, "ohdev");

    // 设置热点配置中的密码
    strcpy(apConfig.preSharedKey, "openharmony");

    // 设置热点配置中的加密方式(Wi-Fi security types)
    apConfig.securityType = WIFI_SEC_TYPE_PSK;

    // 连接到热点
    // 参数apConfig用于指定热点配置，返回netId
    int netId = ConnectToHotspot(&apConfig);

    // 模拟一段时间的联网业务
    // 可以通过串口工具发送 AT+PING 命令，验证网络连通性
    int timeout = 60;
    printf("After %d seconds I will disconnect with AP!\r\n", timeout);
    while (timeout--) {
        osDelay(100);
    }

    // 断开热点连接
    // 参数netId由ConnectToHotspot生成
    DisconnectWithHotspot(netId);
}

// 入口函数
static void WifiConnectDemo(void)
{
    // 定义线程属性
    osThreadAttr_t attr;
    attr.name = "WifiConnectTask";
    attr.attr_bits = 0U;
    attr.cb_mem = NULL;
    attr.cb_size = 0U;
    attr.stack_mem = NULL;
    attr.stack_size = 10240;
    attr.priority = osPriorityNormal;

    // 创建线程
    if (osThreadNew(WifiConnectTask, NULL, &attr) == NULL) {
        printf("[WifiConnectDemo] Falied to create WifiConnectTask!\n");
    }
}

// 运行入口函数
SYS_RUN(WifiConnectDemo);
