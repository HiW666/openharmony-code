#include <stdio.h>          // 标准输入输出
#include <unistd.h>         // POSIX标准接口
#include <string.h>         // 字符串处理(操作字符数组)

#include "lwip/sockets.h"   // lwIP TCP/IP协议栈：Socket API

#include "ssd1306.h"        // OLED驱动接口

// 要发送的数据
static char request[] = "Hello";

// 要接收的数据
static char response[128] = "";

/// @brief TCP客户端测试函数
/// @param host TCP服务端IP地址
/// @param port TCP服务端端口
void TcpClientTest(const char *host, unsigned short port)
{
    // 用于接收Socket API接口返回值
    ssize_t retval = 0;

    // 创建一个TCP Socket，返回值为文件描述符
    // 函数名也可以加"lwip_"前缀，写成"lwip_socket"
    // 这个函数要右键，转到声明，跟踪两次才能定位到正确位置：
    // (device\hisilicon\hispark_pegasus\sdk_liteos\third_party\lwip_sack\include\lwip\sockets.h)
    // 下面其他Socket API函数也是如此(除了closesocket)
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);

    // 用于设置服务端的地址信息
    struct sockaddr_in serverAddr = {0};

    // 设置服务端的地址信息，包括协议、端口、IP地址等
    serverAddr.sin_family = AF_INET;    // AF_INET表示IPv4协议
    serverAddr.sin_port = htons(port);  // 端口号，使用htons函数从主机字节序转为网络字节序

    // 将服务端IP地址从“点分十进制”字符串，转化为标准格式（32位整数）
    // 函数名也可以加"lwip_"前缀，写成"lwip_inet_pton"
    if (inet_pton(AF_INET, host, &serverAddr.sin_addr) <= 0)
    {
        // 转化失败
        printf("inet_pton failed!\r\n");    // 输出日志

        // 跳转到cleanup部分，主要是关闭连接
        goto do_cleanup;
    }

    // 尝试和目标主机建立连接，连接成功会返回0，失败返回-1
    // 函数名也可以加"lwip_"前缀，写成"lwip_connect"
    if (connect(sockfd, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) < 0)
    {
        // 连接失败
        printf("connect failed!\r\n");      // 输出日志

        // OLED显示信息
        ssd1306_PrintString("conn:failed\r\n", Font_7x10, White);

        // 跳转到cleanup部分
        goto do_cleanup;
    }

    // 连接成功
    // 输出日志
    printf("connect to server %s success!\r\n", host);
    // OLED显示服务端IP地址
    ssd1306_PrintString("conn:", Font_7x10, White);
    ssd1306_PrintString(host, Font_7x10, White);

    // 建立连接成功之后，sockfd就具有了“连接状态”，
    // 后继的发送和接收，都是针对指定的目标主机和端口

    // 发送数据
    // 函数名也可以加"lwip_"前缀，写成"lwip_send"
    retval = send(sockfd, request, sizeof(request), 0);

    // 检查接口返回值，小于0表示发送失败
    if (retval < 0)
    {
        // 发送失败
        printf("send request failed!\r\n");     // 输出日志
        goto do_cleanup;                        // 跳转到cleanup部分
    }

    // 发送成功
    // 输出日志
    printf("send request{%s} %ld to server done!\r\n", request, retval);
    // OLED显示发送的数据
    ssd1306_PrintString("\r\nsend:", Font_7x10, White);
    ssd1306_PrintString(request, Font_7x10, White);

    // 接收数据
    // 函数名也可以加"lwip_"前缀，写成"lwip_recv"
    retval = recv(sockfd, &response, sizeof(response), 0);

    // 检查接口返回值，小于0表示接收失败
    // 对方的通信端关闭时，返回值为0
    if (retval <= 0)
    {
        // 接收失败或对方的通信端关闭
        printf("send response from server failed or done, %ld!\r\n", retval);   // 输出日志
        goto do_cleanup;                                                        // 跳转到cleanup部分
    }

    // 接收成功
    // 在末尾添加字符串结束符'\0'，以便后续的字符串操作
    response[retval] = '\0';

    // 输出日志
    printf("recv response{%s} %ld from server done!\r\n", response, retval);
    
    // OLED显示服务端返回的数据
    ssd1306_PrintString("\r\nrecv:", Font_7x10, White);  // 显示字符串（支持回车换行）
    ssd1306_PrintString(response, Font_7x10, White); // 连续输出不需要重新定位（驱动处理向右偏移）

// cleanup部分，主要是关闭连接
do_cleanup:
    printf("do_cleanup...\r\n");    // 输出日志
    // 关闭连接
    // 函数名不能直接加"lwip_"前缀，它对应"lwip_close"
    closesocket(sockfd);
}
