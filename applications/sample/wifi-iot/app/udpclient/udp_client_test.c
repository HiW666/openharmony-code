#include <stdio.h>          // 标准输入输出
#include <unistd.h>         // POSIX标准接口
#include <errno.h>          // 错误码
#include <string.h>         // 字符串处理(操作字符数组)

#include "lwip/sockets.h"   // lwIP TCP/IP协议栈：Socket API

#include "ssd1306.h"        // OLED驱动接口

// 要发送的数据
static char request[] = "Hello";

// 要接收的数据
static char response[128] = "";

/// @brief UDP客户端测试函数
/// @param host UDP服务端IP地址
/// @param port UDP服务端端口
void UdpClientTest(const char *host, unsigned short port)
{
    // 用于接收Socket API接口返回值
    ssize_t retval = 0;

    // 创建一个UDP Socket，返回值为文件描述符
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);

    // 用于设置服务端的地址信息
    struct sockaddr_in toAddr = {0};

    // 使用IPv4协议
    toAddr.sin_family = AF_INET;

    // 端口号，从主机字节序转为网络字节序
    toAddr.sin_port = htons(port);

    // 将服务端IP地址从“点分十进制”字符串，转化为标准格式（32位整数）
    if (inet_pton(AF_INET, host, &toAddr.sin_addr) <= 0)
    {
        // 转化失败
        printf("inet_pton failed!\r\n");    // 输出日志

        // 跳转到cleanup部分
        goto do_cleanup;
    }

    // 发送数据
    // UDP socket是 “无连接的”，因此每次发送都必须先指定目标主机和端口，主机可以是多播地址
    // 发送数据的时候，使用本地随机端口N
    // 
    // 参数：
    // s：socket文件描述符
    // dataptr：要发送的数据
    // size：要发送的数据的长度，最大65332字节
    // flags：消息传输标志位
    // to：目标的地址信息
    // tolen：目标的地址信息长度
    // 
    // 返回值：
    // 发送的字节数，如果出错，返回-1
    retval = sendto(sockfd, request, sizeof(request), 0, (struct sockaddr *)&toAddr, sizeof(toAddr));

    // 检查接口返回值，小于0表示发送失败
    if (retval < 0)
    {
        // 发送失败
        printf("sendto failed!\r\n");   // 输出日志
        goto do_cleanup;                // 跳转到cleanup部分
    }

    // 发送成功
    // 输出日志
    printf("send UDP message {%s} %ld done!\r\n", request, retval);
    // OLED显示发送的数据
    ssd1306_PrintString("send:", Font_7x10, White);
    ssd1306_PrintString(request, Font_7x10, White);

    // 用于记录发送方的地址信息(IP地址和端口号)
    struct sockaddr_in fromAddr = {0};

    // 用于记录发送方的地址信息长度
    socklen_t fromLen = sizeof(fromAddr);

    // 在本地随机端口N上面接收数据
    // UDP socket是 “无连接的”，因此每次接收时并不知道消息来自何处，通过fromAddr参数可以得到发送方的信息（主机、端口号）
    // device\hisilicon\hispark_pegasus\sdk_liteos\third_party\lwip_sack\include\lwip\sockets.h -> lwip_recvfrom
    //
    // 参数：
    // s：socket文件描述符
    // buffer：接收数据的缓冲区的地址
    // length：接收数据的缓冲区的长度
    // flags：消息接收标志位
    // address：发送方的地址信息
    // address_len：发送方的地址信息长度
    //
    // 返回值：
    // 接收的字节数，如果出错，返回-1
    retval = recvfrom(sockfd, &response, sizeof(response), 0, (struct sockaddr *)&fromAddr, &fromLen);

    // 检查接口返回值，小于0表示接收失败
    if (retval <= 0)
    {
        // 接收失败，或者收到0长度的数据(忽略掉)
        printf("recvfrom failed or abort, %ld, %d!\r\n", retval, errno);    // 输出日志
        goto do_cleanup;                                                    // 跳转到cleanup部分
    }

    // 接收成功
    // 末尾添加字符串结束符'\0'，以便后续的字符串操作
    response[retval] = '\0';

    // 输出日志
    printf("recv UDP message {%s} %ld done!\r\n", response, retval);

    // 显示发送方的地址信息
    printf("peer info: ipaddr = %s, port = %d\r\n", inet_ntoa(fromAddr.sin_addr), ntohs(fromAddr.sin_port));

    // OLED显示收到的数据
    ssd1306_PrintString("\r\nrecv:", Font_7x10, White);
    ssd1306_PrintString(response, Font_7x10, White);

// cleanup部分
do_cleanup:
    printf("do_cleanup...\r\n");    // 输出日志
    // 关闭socket
    lwip_close(sockfd);
}
