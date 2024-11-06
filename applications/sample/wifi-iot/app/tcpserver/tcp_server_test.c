#include <stdio.h>          // 标准输入输出
#include <unistd.h>         // POSIX标准接口
#include <stddef.h>         // 标准类型定义
#include <errno.h>          // 错误码
#include <string.h>         // 字符串处理(操作字符数组)

#include "lwip/sockets.h"   // lwIP TCP/IP协议栈：Socket API

#include "ssd1306.h"        // OLED驱动接口

// 要收发的数据
static char request[128] = "";

/// @brief TCP服务端测试函数
/// @param port TCP服务端端口
void TcpServerTest(unsigned short port)
{
    // 用于接收Socket API接口返回值
    ssize_t retval = 0;

    // 最大等待队列长度
    int backlog = 1;

    // 创建一个TCP Socket，返回值为文件描述符
    // 用于监听客户端的连接请求
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);

    // 这个socket用于连接成功后和客户端通信
    int connfd = -1;

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
    if (retval < 0) {
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

    // 开始监听，最大等待队列长度为backlog
    // 使用listen接口将sockfd设置为监听模式，在指定的IP和端口上监听客户端发起的连接请求。
    // 该函数不是阻塞的，它只是告诉内核，客户端对指定IP地址和端口号发起的三次握手成功后，内核应该将连接请求放入sockfd的等待队列中。
    retval = listen(sockfd, backlog);

    // 检查接口返回值，小于0表示监听失败
    if (retval < 0) {
        // 监听失败
        printf("listen failed!\r\n");                                   // 输出错误信息
        ssd1306_PrintString("\r\nlisten:failed\r\n", Font_6x8, White);  // 显示错误信息
        goto do_cleanup;                                                // 跳转到cleanup部分
    }

    // 监听成功
    // 输出日志
    printf("listen with %d backlog success!\r\n", backlog);

    // OLED显示监听成功
    ssd1306_PrintString(" listen\r\n", Font_6x8, White);

    // 使用accept接口阻塞式等待客户端连接（sockfd的等待队列中出现完成三次握手的连接），创建一个新socket与客户端建立连接。
    // 成功会返回一个表示连接的socket，clientAddr参数将会携带客户端主机和端口信息；失败返回-1。
    // 之后sockfd依然可以继续接受其他客户端的连接。
    // 
    // UNIX系统上经典的并发模型是“每个连接一个进程”——创建子进程处理连接，父进程继续接受其他客户端的连接
    // liteos-a内核，可以使用UNIX的“每个连接一个进程”的并发模型
    // liteos-m内核，可以使用“每个连接一个线程”的并发模型
    connfd = accept(sockfd, (struct sockaddr *)&clientAddr, &clientAddrLen);

    // 检查接口返回值，小于0表示建立连接失败
    if (connfd < 0) {
        // 建立连接失败
        printf("accept failed, %d, %d\r\n", connfd, errno);     // 输出错误信息
        goto do_cleanup;                                        // 跳转到cleanup部分
    }

    // 建立连接成功
    // 输出日志
    printf("accept success, connfd = %d!\r\n", connfd);
    printf("client addr info: host = %s, port = %d\r\n",
           inet_ntoa(clientAddr.sin_addr), ntohs(clientAddr.sin_port));

    // OLED显示客户端IP地址
    ssd1306_PrintString("cli:", Font_6x8, White);
    ssd1306_PrintString(inet_ntoa(clientAddr.sin_addr), Font_6x8, White);

    // 后续的收/发都在connfd上进行

    // 接收收据
    retval = recv(connfd, request, sizeof(request), 0);

    // 接收失败或对方的通信端关闭，输出日志，跳转到disconnect部分
    if (retval <= 0) {
        printf("recv request failed or done, %ld!\r\n", retval);
        goto do_disconnect;
    }

    // 接收成功，输出日志，OLED显示收到的收据
    printf("recv request{%s} from client done!\r\n", request);
    ssd1306_PrintString("\r\nrecv:", Font_6x8, White);
    ssd1306_PrintString(request, Font_6x8, White);

    // 发送数据
    retval = send(connfd, request, strlen(request), 0);

    // 发送失败，输出日志，跳转到disconnect部分
    if (retval < 0) {
        printf("send response failed, %ld!\r\n", retval);
        goto do_disconnect;
    }

    // 发送成功，输出日志，OLED显示发送的数据
    printf("send response{%s} to client done!\r\n", request);
    ssd1306_PrintString("\r\nsend:", Font_6x8, White);
    ssd1306_PrintString(request, Font_6x8, White);

// disconnect部分
do_disconnect:
    // 关闭连接
    sleep(1);
    lwip_close(connfd);
    sleep(1);

// cleanup部分
do_cleanup:
    // 关闭监听socket
    printf("do_cleanup...\r\n");
    lwip_close(sockfd);
}
