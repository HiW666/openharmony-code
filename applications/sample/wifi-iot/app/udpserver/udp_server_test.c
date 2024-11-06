#include <stdio.h>          // 标准输入输出
#include <unistd.h>         // POSIX标准接口
#include <errno.h>          // 错误码
#include <string.h>         // 字符串处理(操作字符数组)

#include "lwip/sockets.h"   // lwIP TCP/IP协议栈：Socket API

#include "ssd1306.h"        // OLED驱动接口

// 要收发的数据
static char message[128] = "";

/// @brief UDP服务端测试函数
/// @param port UDP服务端端口
void UdpServerTest(unsigned short port)
{
    // 用于接收Socket API接口返回值
    ssize_t retval = 0;

    // 创建一个UDP Socket，返回值为文件描述符
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);

    // 用于记录客户端的IP地址和端口号
    struct sockaddr_in clientAddr = {0};

    // 用于记录clientAddr的长度
    socklen_t clientAddrLen = sizeof(clientAddr);

    // 用于配置服务端的地址信息
    struct sockaddr_in serverAddr = {0};

    // 开始配置服务端的地址信息，包括协议、端口、允许接入的IP地址等

    // 使用IPv4协议
    serverAddr.sin_family = AF_INET;

    // 端口号，从主机字节序转为网络字节序
    serverAddr.sin_port = htons(port);

    // 允许任意主机接入，0.0.0.0
    serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);

    // 将sockfd和本服务器IP、端口号绑定
    // 这样与该IP和端口号相关的收/发数据都与sockfd关联
    retval = bind(sockfd, (struct sockaddr *)&serverAddr, sizeof(serverAddr));

    // 检查接口返回值，小于0表示绑定失败
    if (retval < 0)
    {
        // 绑定失败
        printf("bind failed, %ld!\r\n", retval);                        // 输出错误信息
        ssd1306_PrintString("port:bind failed\r\n", Font_6x8, White);   // 显示错误信息
        goto do_cleanup;                                                // 跳转到cleanup部分
    }

    // 绑定成功
    // 输出日志
    printf("bind to port %d success!\r\n", port);

    // OLED显示端口号
    ssd1306_PrintString("port:", Font_6x8, White);
    char strPort[5] = {0};
    snprintf(strPort, sizeof(strPort), "%d", port);
    ssd1306_PrintString(strPort, Font_6x8, White);

    // 接收来自客户端的消息，并用客户端的IP地址和端口号填充clientAddr
    // UDP socket是 “无连接的”，因此每次接收时并不知道消息来自何处，通过clientAddr参数可以得到发送方的信息（主机、端口号）
    retval = recvfrom(sockfd, message, sizeof(message), 0, (struct sockaddr *)&clientAddr, &clientAddrLen);

    // 接收失败，输出日志，跳转到cleanup部分
    if (retval < 0)
    {
        printf("recvfrom failed, %ld!\r\n", retval);
        goto do_cleanup;
    }

    // 接收成功，输出日志
    printf("recv message {%s} %ld done!\r\n", message, retval);

    // 显示发送方的地址信息
    printf("peer info: ipaddr = %s, port = %d\r\n", inet_ntoa(clientAddr.sin_addr), ntohs(clientAddr.sin_port));

    // OLED显示收到的数据
    ssd1306_PrintString("\r\nrecv:", Font_6x8, White);
    ssd1306_PrintString(message, Font_6x8, White);

    // 向客户端的IP地址和端口号发送消息
    // UDP socket是 “无连接的”，因此每次发送都必须先指定目标主机和端口，主机可以是多播地址
    retval = sendto(sockfd, message, strlen(message), 0, (struct sockaddr *)&clientAddr, sizeof(clientAddr));

    // 发送失败，输出日志，跳转到cleanup部分
    if (retval < 0)
    {
        printf("send failed, %ld!\r\n", retval);
        goto do_cleanup;
    }

    // 发送成功
    // 输出日志
    printf("send message {%s} %ld done!\r\n", message, retval);
    // OLED显示发送的数据
    ssd1306_PrintString("send:", Font_6x8, White);
    ssd1306_PrintString(message, Font_6x8, White);

// cleanup部分
do_cleanup:
    printf("do_cleanup...\r\n");    // 输出日志
    // 关闭socket
    lwip_close(sockfd);
}
