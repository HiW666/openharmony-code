#include <stdio.h>          // 标准输入输出
#include <stdlib.h>         // 标准函数库
#include <string.h>         // 字符串处理(操作字符数组)

#include "ohos_init.h"      // 用于初始化服务(services)和功能(features)
#include "cmsis_os2.h"      // CMSIS-RTOS API V2

#include "wifi_connecter.h" // easy wifi (station模式)
#include "mqtt_task.h"      // MQTT接口

// 定义一个宏，用于标识SSID。请根据实际情况修改
#define PARAM_HOTSPOT_SSID "ohdev"

// 定义一个宏，用于标识密码。请根据实际情况修改
#define PARAM_HOTSPOT_PSK "openharmony"

// 定义一个宏，用于标识加密方式
#define PARAM_HOTSPOT_TYPE WIFI_SEC_TYPE_PSK

// 定义一个宏，用于标识MQTT服务器IP地址。请根据实际情况修改
#define PARAM_SERVER_ADDR "192.168.8.10"

// 定义一个宏，用于标识MQTT服务器端口
#define PARAM_SERVER_PORT "1883"

// 主线程函数
static void mqttDemoTask(void *arg)
{
    (void)arg;

    // 连接AP
    // 定义热点配置
    WifiDeviceConfig config = {0};
    // 设置热点配置中的SSID
    strcpy(config.ssid, PARAM_HOTSPOT_SSID);
    // 设置热点配置中的密码
    strcpy(config.preSharedKey, PARAM_HOTSPOT_PSK);
    // 设置热点配置中的加密方式
    config.securityType = PARAM_HOTSPOT_TYPE;
    // 等待100ms
    osDelay(10);
    // 连接到热点
    int netId = ConnectToHotspot(&config);
    // 检查是否成功连接到热点
    if (netId < 0)
    {
        printf("Connect to AP failed!\r\n");
        return;
    }

    // 初始化并启动MQTT任务，连接MQTT服务器
    MqttTaskInit();                                                     // 初始化并启动MQTT任务
    const char *host = PARAM_SERVER_ADDR;                               // MQTT服务器IP地址
    unsigned short port = atoi(PARAM_SERVER_PORT);                      // MQTT服务器端口
    const char *clientId = "Pegasus0001";                               // MQTT客户端ID
    const char *username = NULL;                                        // MQTT服务器用户名
    const char *password = NULL;                                        // MQTT服务器密码
    if (MqttTaskConnect(host, port, clientId, username, password) != 0) // 连接MQTT服务器
    {
        // 连接失败，输出错误信息并退出
        printf("Connect to MQTT server failed!\r\n");
        return;
    }

    // 订阅主题"test/b"
    char *stopic = "test/b";            // 主题
    int rc = MqttTaskSubscribe(stopic); // 订阅主题
    if (rc != 0)
    {
        // 订阅失败，输出错误信息并退出
        printf("MQTT Subscribe failed!\r\n");
        return;
    }
    // 输出订阅成功信息
    printf("MQTT Subscribe OK\r\n");

    // 向主题"test/a"发布一条消息
    char *ptopic = "test/a";                            // 主题
    char *payload = "(MQTT Client) This is a Pegasus."; // 消息内容
    rc = MqttTaskPublish(ptopic, payload);              // 发布消息
    if (rc != 0)
        printf("MQTT Publish failed!\r\n");             // 发布失败，输出错误信息
    else
        printf("MQTT Publish OK\r\n");                  // 发布成功，输出成功信息
}

// 入口函数
static void mqttDemoEntry(void)
{
    // 定义线程属性
    osThreadAttr_t attr;
    attr.name = "mqttDemoTask";
    attr.attr_bits = 0U;
    attr.cb_mem = NULL;
    attr.cb_size = 0U;
    attr.stack_mem = NULL;
    attr.stack_size = 10240;
    attr.priority = osPriorityNormal;

    // 创建线程
    if (osThreadNew(mqttDemoTask, NULL, &attr) == NULL)
    {
        printf("[mqttDemoEntry] Falied to create mqttDemoTask!\n");
    }
}

// 运行入口函数
SYS_RUN(mqttDemoEntry);
