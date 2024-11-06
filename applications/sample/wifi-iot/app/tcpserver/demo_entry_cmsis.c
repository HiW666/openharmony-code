#include <stdio.h>          // 标准输入输出
#include <unistd.h>         // POSIX标准接口
#include <string.h>         // 字符串处理(操作字符数组)

#include "ohos_init.h"      // 用于初始化服务(services)和功能(features)
#include "cmsis_os2.h"      // CMSIS-RTOS API V2

#include "wifi_connecter.h" // easy wifi (station模式)
#include "ssd1306.h"        // OLED驱动接口

// 定义一个宏，用于标识SSID。请根据实际情况修改
#define PARAM_HOTSPOT_SSID "ohdev"

// 定义一个宏，用于标识密码。请根据实际情况修改
#define PARAM_HOTSPOT_PSK "openharmony"

// 定义一个宏，用于标识加密方式
#define PARAM_HOTSPOT_TYPE WIFI_SEC_TYPE_PSK

// 定义一个宏，用于标识TCP服务器端口
#define PARAM_SERVER_PORT 5678

// 主线程函数
static void NetDemoTask(void *arg)
{
    (void)arg;

    // 定义热点配置
    WifiDeviceConfig config = {0};

    // 设置热点配置中的SSID
    strcpy(config.ssid, PARAM_HOTSPOT_SSID);

    // 设置热点配置中的密码
    strcpy(config.preSharedKey, PARAM_HOTSPOT_PSK);

    // 设置热点配置中的加密方式(Wi-Fi security types)
    config.securityType = PARAM_HOTSPOT_TYPE;

    // 等待100ms
    osDelay(10);

    // 连接到热点
    int netId = ConnectToHotspot(&config);

    // 检查是否成功连接到热点
    if (netId < 0)
    {
        // 连接到热点失败
        printf("ConnectToAP failed\n");                                     // 输出错误信息
        ssd1306_PrintString("Connect to AP failed\r\n", Font_6x8, White);   // 显示错误信息
        return;
    }

    // 连接到热点成功，显示连接成功信息
    ssd1306_PrintString("AP:connected\r\n", Font_6x8, White);

    // 运行TCP服务端测试
    TcpServerTest(PARAM_SERVER_PORT);

    // 断开热点连接
    printf("disconnect to AP ...\r\n");
    DisconnectWithHotspot(netId);
    printf("disconnect to AP done!\r\n");
}

// 入口函数
static void NetDemoEntry(void)
{
    // 初始化OLED
    ssd1306_Init();

    // 全屏填充黑色
    ssd1306_Fill(Black);

    // OLED显示APP标题
    ssd1306_PrintString("TcpServer Test\r\n", Font_6x8, White);

    // 定义线程属性
    osThreadAttr_t attr;
    attr.name = "NetDemoTask";
    attr.attr_bits = 0U;
    attr.cb_mem = NULL;
    attr.cb_size = 0U;
    attr.stack_mem = NULL;
    attr.stack_size = 10240;
    attr.priority = osPriorityNormal;

    // 创建线程
    if (osThreadNew(NetDemoTask, NULL, &attr) == NULL)
    {
        printf("[NetDemoEntry] Falied to create NetDemoTask!\n");
    }
}

// 运行入口函数
SYS_RUN(NetDemoEntry);
