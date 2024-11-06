#include <stdio.h>              // 标准输入输出
#include <stdlib.h>             // 标准函数库
#include <string.h>             // 字符串处理(操作字符数组)

#include "ohos_init.h"          // 用于初始化服务(services)和功能(features)
#include "cmsis_os2.h"          // CMSIS-RTOS API V2

#include "temp-humi_sensor.h"   // 环境监测板AHT20数字温湿度传感器接口
#include "wifi_connecter.h"     // easy wifi (station模式)
#include "mqtt_task.h"          // MQTT接口

// 定义一系列宏，用于标识SSID、密码、加密方式、MQTT服务器IP地址等，请根据实际情况修改
#define PARAM_HOTSPOT_SSID "ohdev"           // AP名称
#define PARAM_HOTSPOT_PSK "openharmony"      // AP密码
#define PARAM_HOTSPOT_TYPE WIFI_SEC_TYPE_PSK // 安全类型，定义在wifi_device_config.h中
#define PARAM_SERVER_ADDR "192.168.8.10"     // MQTT服务器IP地址
#define PARAM_SERVER_PORT "1883"             // MQTT服务器端口
// #define PARAM_SERVER_ADDR "******"        // MQTT公网服务器IP地址
// #define PARAM_SERVER_PORT "****"          // MQTT公网服务器端口

// 主线程函数
static void mqttDemoTask(void *arg)
{
    (void)arg;

    // 连接AP
    WifiDeviceConfig config = {0};                      // 定义热点配置
    strcpy(config.ssid, PARAM_HOTSPOT_SSID);            // 设置热点配置中的SSID
    strcpy(config.preSharedKey, PARAM_HOTSPOT_PSK);     // 设置热点配置中的密码
    config.securityType = PARAM_HOTSPOT_TYPE;           // 设置热点配置中的加密方式
    osDelay(10);
    int netId = ConnectToHotspot(&config);              // 连接到热点
    if (netId < 0)                                      // 检查是否成功连接到热点
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
        printf("Connect to MQTT server failed!\r\n");
        return;
    }

    // 向主题"OpenHarmony/devices/Pegasus0001/THSensor/Message"发布上线消息
    char *ptopic = "OpenHarmony/devices/Pegasus0001/THSensor/Message";          // 主题
    char *payload = "{\"device_id\":\"Pegasus0001\", \"status\":\"online\"}";   // 消息内容
    int rc = MqttTaskPublish(ptopic, payload);                                  // 发布消息
    if (rc != 0)                                                                // 检查是否成功发布消息
        printf("MQTT Publish failed!\r\n");
    else
        printf("MQTT Publish OK\r\n");

    // 向主题"OpenHarmony/devices/Pegasus0001/THSensor/Message"上报数字温湿度传感器数据
    // 工作循环
    while (1)
    {
        char payload[64] = {0};                             // 消息内容
        float temperature = 0.0f;                           // 温度
        float humidity = 0.0f;                              // 湿度
        if (GetTempHumiLevel(&temperature, &humidity) == 0) //成功获取数字温湿度传感器数据
        {
            snprintf(payload, sizeof(payload),
                     "{\"device_id\":\"Pegasus0001\", \"temp\":%.2f, \"humi\":%.2f}",
                     temperature, humidity);                // 消息内容采用JSON格式
            rc = MqttTaskPublish(ptopic, payload);          // 发布消息
            if (rc != 0)                                    // 检查是否成功发布消息
                printf("MQTT Publish failed!\r\n");
            else
                printf("MQTT Publish OK\r\n");
        }
        osDelay(100);                                       // 发布消息间隔1秒。实际上报无需这么频繁
    }
}

// 入口函数
static void mqttDemoEntry(void)
{
    // 初始化AHT20数字温湿度传感器
    InitTempHumiSensor();

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
