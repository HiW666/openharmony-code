// MQTT接口文件。
// 对paho-mqtt库进行了封装，提供一个简单的接口。
// 使用该接口可以实现MQTT的连接、发布、订阅、断开等功能。

#include <stdio.h>          // 标准输入输出
#include <stdint.h>         // 几种扩展的整数类型和宏
#include <stdlib.h>         // 标准函数库
#include <string.h>         // 字符串处理(操作字符数组)

#include "cmsis_os2.h"      // CMSIS-RTOS API V2

#include "MQTTClient.h"     // MQTTClient-C库接口文件
#include "mqtt_ohos.h"      // OHOS(LiteOS)适配接口文件

#include "mqtt_task.h"      // MQTT接口

// 定义一个宏，用于输出日志
#define LOGI(fmt, ...) printf("[%d] %s " fmt "\n", osKernelGetTickCount(), osThreadGetName(osThreadGetId()), ##__VA_ARGS__)

// 控制MQTT任务循环
static volatile int running = 1;

// MQTT客户端
static MQTTClient client = {0};

// MQTT网络连接
static Network network = {0};

// 发送和接收的数据缓冲区
static unsigned char sendbuf[512], readbuf[512]; //增大缓冲区，修复连接华为云失败的问题


/// @brief MQTT任务循环
/// @param arg MQTTClient
static void MqttTask(void *arg)
{
    // 输出日志
    LOGI("MqttTask start!");

    // 转换参数的类型
    MQTTClient *c = (MQTTClient *)arg;

    // 任务循环
    while (c)
    {
        // 检查是否需要退出任务循环
        // paho_mqtt对互斥锁操作进行了一个简单的封装
        // 参见：applications\sample\wifi-iot\app\paho_mqtt\MQTTClient-C\src\ohos\mqtt_ohos_cmsis.c
        mqttMutexLock(&c->mutex);
        if (!running)
        {
            // 需要退出任务循环
            LOGI("MQTT background thread exit!");
            mqttMutexUnlock(&c->mutex);
            break;
        }
        mqttMutexUnlock(&c->mutex);

        // 尝试接收数据
        mqttMutexLock(&c->mutex);
        if (c->isconnected)
        {
            // LOGI("checking...");
            MQTTYield(c, 1);
        }
        mqttMutexUnlock(&c->mutex);

        // 等待1秒
        // 参见：applications\sample\wifi-iot\app\paho_mqtt\MQTTClient-C\src\ohos\mqtt_ohos_cmsis.c
        Sleep(1000);
        // ThreadYield();
    }
    // 输出日志
    LOGI("MqttTask exit!");
}

/// 初始化并启动MQTT任务
void MqttTaskInit(void)
{
    // 初始化MQTT客户端
    // 在MQTTClientInit中，c->ipstack = network;
    NetworkInit(&network);
    MQTTClientInit(&client, &network, 300, sendbuf, sizeof(sendbuf), readbuf, sizeof(readbuf));

    // 开启MQTT任务循环
    running = 1;

    // 创建MQTT任务线程
    // paho_mqtt对创建线程操作进行了一个简单的封装
    // 参见：applications\sample\wifi-iot\app\paho_mqtt\MQTTClient-C\src\ohos\mqtt_ohos_cmsis.c
    int rc = ThreadStart(&client.thread, MqttTask, &client);

    // 输出日志
    LOGI("MqttTaskInit done!");
}

/// 停止MQTT任务
void MqttTaskDeinit(void)
{
    mqttMutexLock(&client.mutex);
    // 停止MQTT任务循环
    running = 0;
    mqttMutexUnlock(&client.mutex);
    mqttMutexDeinit(&client.mutex);
}

/// 连接MQTT服务器
/// @param[in] host 服务器地址
/// @param[in] port 服务器端口
/// @param[in] clientId 客户端ID
/// @param[in] username 用户名
/// @param[in] password 密码
/// @return 0：成功，-1：失败
int MqttTaskConnect(const char *host, unsigned short port,
                    const char *clientId, const char *username, const char *password)
{
    // 用于接收接口返回值
    int rc = 0;

    // 初始化MQTT连接信息
    MQTTPacket_connectData connectData = MQTTPacket_connectData_initializer;

    // 使用TCP socket连接MQTT服务器
    // 在MQTTClientInit中，将c->ipstack指向了network，所以连接成功后，c->ipstack中保有socket句柄
    if ((rc = NetworkConnect(&network, (char *)host, port)) != 0)
    {
        // 连接失败，输出日志并返回-1
        LOGI("NetworkConnect is %d", rc);
        return -1;
    }

    // 设置用户名和密码
    if (username != NULL && password != NULL)
    {
        connectData.username.cstring = (char *)username;
        connectData.password.cstring = (char *)password;
    }

    // 设置MQTT版本
    // 3 = 3.1
    // 4 = 3.1.1
    connectData.MQTTVersion = 3;

    // 设置MQTT客户端ID
    connectData.clientID.cstring = (char *)clientId;

    // 发送MQTT连接包
    if ((rc = MQTTConnect(&client, &connectData)) != 0)
    {
        // 连接失败，输出日志并返回-1
        LOGI("MQTTConnect failed: %d", rc);
        return -1;
    }

    // 成功连接到MQTT服务器，输出日志并返回0
    LOGI("MQTT Connected!");
    return 0;
}

// 消息到达的回调函数
static void OnMessageArrived(MessageData *data)
{
    // 日志输出
    LOGI("Message arrived on topic %.*s: %.*s",
         (int)data->topicName->lenstring.len, (char *)data->topicName->lenstring.data,
         (int)data->message->payloadlen, (char *)data->message->payload);

    // 可以在此处理消息
    // ...

}

/// 订阅主题
/// @param topic 主题
int MqttTaskSubscribe(char *topic)
{
    // 用于接收接口返回值
    int rc = 0;

    // 输出日志
    LOGI("Subscribe: [%s] from broker", topic);

    // 发送订阅包，并等待订阅确认
    // Send an MQTT subscribe packet and wait for suback before returning.
    if ((rc = MQTTSubscribe(&client, topic, QOS2, OnMessageArrived)) != 0)
    {
        // 订阅失败，输出日志并返回-1
        LOGI("MQTTSubscribe failed: %d", rc);
        return -1;
    }

    // 成功订阅，返回0
    return 0;
}

/// 取消主题订阅
/// @param topic 主题
int MqttTaskUnSubscribe(char *topic)
{
    // 用于接收接口返回值
    int rc = 0;

    // 输出日志
    LOGI("UnSubscribe: [%s] from broker", topic);

    // 发送取消订阅包，并等待取消订阅确认
    // send an MQTT unsubscribe packet and wait for unsuback before returning.
    if ((rc = MQTTUnsubscribe(&client, topic)) != 0)
    {
        // 取消订阅失败，输出日志并返回-1
        LOGI("MQTTUnsubscribe failed: %d", rc);
        return -1;
    }

    // 成功取消订阅，返回0
    return 0;
}

/// 发布消息
/// @param topic 主题
/// @param payload 消息内容
int MqttTaskPublish(char *topic, char *payload)
{
    // 用于接收接口返回值
    int rc = 0;

    // 定义一个MQTT消息数据包
    MQTTMessage message;

    // 确保订阅者只收到一次消息
    message.qos = QOS2;

    // 订阅者重新连接到MQTT服务器时，不需要接收该主题的最新消息。
    // Retained消息是指在MQTTMessage数据包中retained标识设为1的消息。
    // MQTT服务器收到这样的MQTTMessage数据包以后，将保存这个消息，
    // 当有一个新的订阅者订阅相应主题的时候，MQTT服务器会马上将这个消息发送给订阅者。
    message.retained = 0;

    // 设置消息的内容
    message.payload = payload;

    // 设置消息的长度
    message.payloadlen = strlen(payload);

    // 输出日志
    LOGI("Publish: #'%s': '%s' to broker", topic, payload);

    // 发布消息
    if ((rc = MQTTPublish(&client, topic, &message)) != 0)
    {
        // 发布消息失败，输出日志并返回-1
        LOGI("MQTTPublish failed: %d", rc);
        return -1;
    }

    // 发布消息成功，返回0
    return 0;
}

/// 断开和MQTT服务器的连接
int MqttTaskDisconnect(void)
{
    // 用于接收接口返回值
    int rc = 0;

    // 发送MQTT断开连接包，关闭连接
    if ((rc = MQTTDisconnect(&client)) != 0)
    {
        // 断开连接失败，输出日志并返回-1
        LOGI("MQTTDisconnect failed: %d", rc);
        return -1;
    }

    // 断开和MQTT服务器的TCP socket连接
    NetworkDisconnect(&network);

    // 断开连接成功，返回0
    return 0;
}
