// MQTT接口文件。
// 对paho-mqtt库进行了封装，提供一个简单的接口。
// 使用该接口可以实现MQTT的连接、发布、订阅、断开等功能。

// 定义条件编译宏，防止头文件的重复包含和编译
#ifndef MQTT_TASK_H
#define MQTT_TASK_H

// 声明接口函数

void MqttTaskInit(void);
int MqttTaskConnect(const char *host, unsigned short port,
                    const char *clientId, const char *username, const char *password);
int MqttTaskSubscribe(char *topic);
int MqttTaskPublish(char *topic, char *payload);
int MqttTaskUnSubscribe(char *topic);
int MqttTaskDisconnect(void);
void MqttTaskDeinit(void);

#endif // 条件编译结束
