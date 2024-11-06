#include <stdio.h>          // 标准输入输出
#include <unistd.h>         // POSIX标准接口
#include <string.h>         // 字符串处理(操作字符数组)

#include "ohos_init.h"      // 用于初始化服务(services)和功能(features)
#include "cmsis_os2.h"      // CMSIS-RTOS API V2

// Wi-Fi设备接口：AP模式
// 用于：开启和关闭Wi-Fi设备的AP模式，查询AP状态，获取station列表，事件监听等
#include "wifi_hotspot.h"

#include "lwip/netifapi.h"  // lwIP TCP/IP协议栈：网络接口API

// 全局变量，用于标识热点是否成功启动
static volatile int g_hotspotStarted = 0;

// 热点状态变化回调函数
// 参数int state表示热点状态：WIFI_HOTSPOT_ACTIVE(热点成功开启) | WIFI_HOTSPOT_NOT_ACTIVE(热点关闭)
// 参见 foundation\communication\wifi_lite\interfaces\wifiservice\wifi_event.h
static void OnHotspotStateChanged(int state)
{
    // 输出日志
    printf("OnHotspotStateChanged: %d.\r\n", state);

    // 更新全局状态变量，在另外一个线程中轮询状态变量，这种方式实现起来比较简单
    if (state == WIFI_HOTSPOT_ACTIVE)   // 热点成功开启
    {
        g_hotspotStarted = 1;
    }
    else                                // 热点关闭
    {
        g_hotspotStarted = 0;
    }
}

// 输出WiFi设备端的信息
static void PrintStationInfo(StationInfo *info)
{
    // 检查参数的合法性
    if (!info)
        return;
    
    // 存储MAC地址字符串
    static char macAddress[32] = {0};

    // 获取MAC地址
    unsigned char *mac = info->macAddress;

    // mac地址转换为字符串
    snprintf(macAddress, sizeof(macAddress), "%02X:%02X:%02X:%02X:%02X:%02X",
             mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
    
    // 输出日志
    printf(" PrintStationInfo: mac=%s, reason=%d.\r\n", 
            macAddress,                     // MAC地址
            info->disconnectedReason);      // 断开原因
}

// 全局变量，用于记录连接到热点的wifi客户端数量
static volatile int g_joinedStations = 0;

// WiFi设备连接上当前热点的回调函数
// 参数StationInfo* info，其中包含macAddress和disconnectedReason
static void OnHotspotStaJoin(StationInfo *info)
{
    // 连接到热点的wifi客户端数量+1
    g_joinedStations++;

    // 输出WiFi设备端的信息
    PrintStationInfo(info);

    // 输出日志
    printf("+OnHotspotStaJoin: active stations = %d.\r\n", g_joinedStations);
}

// WiFi设备断开当前热点的回调函数
// 参数StationInfo* info，其中包含macAddress和disconnectedReason
static void OnHotspotStaLeave(StationInfo *info)
{
    // 连接到热点的wifi客户端数量-1
    g_joinedStations--;

    // 输出WiFi设备端的信息
    PrintStationInfo(info);

    // 输出日志
    printf("-OnHotspotStaLeave: active stations = %d.\r\n", g_joinedStations);
}

// 建立WiFi事件监听器
// 参见 foundation\communication\wifi_lite\interfaces\wifiservice\wifi_event.h
// 开启WiFi设备的AP模式之前，需要使用RegisterWifiEvent接口，向系统注册状态监听函数，用于接收状态通知
// AP模式需要绑定以下三个回调函数；
// OnHotspotStaJoin回调函数，其他设备连上当前热点时会被调用
// OnHotspotStaLeave回调函数，其他设备断开当前热点时会被调用
// OnHotspotStateChanged回调函数，当热点本身状态变化时会被调用
WifiEvent g_defaultWifiEventListener = {
    // 在WiFi设备成功连接当前热点时，调用OnHotspotStaJoin回调函数
    .OnHotspotStaJoin = OnHotspotStaJoin,
    // 在WiFi设备断开当前热点时，调用OnHotspotStaLeave回调函数
    .OnHotspotStaLeave = OnHotspotStaLeave,
    // 在热点本身状态发生变化时，调用OnHotspotStateChanged回调函数
    .OnHotspotStateChanged = OnHotspotStateChanged,
};

// 网络接口
static struct netif *g_iface = NULL;

// 创建热点
int StartHotspot(const HotspotConfig *config)
{
    // WifiErrorCode类型的变量，用于接收接口返回值
    WifiErrorCode errCode = WIFI_SUCCESS;

    // 使用RegisterWifiEvent接口，向系统注册状态监听函数，AP模式需要绑定三个回调函数
    errCode = RegisterWifiEvent(&g_defaultWifiEventListener);

    // 打印接口调用结果
    printf("RegisterWifiEvent: %d\r\n", errCode);

    // 通过SetHotspotConfig接口，向系统设置当前热点配置信息
    errCode = SetHotspotConfig(config);

    // 打印接口调用结果
    printf("SetHotspotConfig: %d\r\n", errCode);

    // 使用EnableHotspot接口，开启热点，无需参数
    g_hotspotStarted = 0;
    errCode = EnableHotspot();

    // 打印接口调用结果
    printf("EnableHotspot: %d\r\n", errCode);

    // 等待热点开启成功
    while (!g_hotspotStarted)
    {
        osDelay(10);
    }

    // 输出日志
    printf("g_hotspotStarted = %d.\r\n", g_hotspotStarted);

    // 热点开启成功之后，需要启动DHCP服务端，Hi3861使用如下接口：
    // 使用netifapi_netif_find("ap0")获取AP模式的网络接口
    // 使用netifapi_netif_set_addr接口设置热点本身的IP地址、网关、子网掩码
    // 使用netifapi_dhcps_start接口启动DHCP服务端
    // 使用netifapi_dhcps_stop接口停止DHCP服务端

    // 获取AP模式的网络接口
    g_iface = netifapi_netif_find("ap0");

    // 获取网络接口成功
    if (g_iface)
    {
        // 存储IP地址
        ip4_addr_t ipaddr;

        // 存储网关
        ip4_addr_t gateway;

        // 存储子网掩码
        ip4_addr_t netmask;

        // 设置IP地址
        IP4_ADDR(&ipaddr, 192, 168, 12, 1);    /* input your IP for example: 192.168.12.1 */

        // 设置子网掩码
        IP4_ADDR(&netmask, 255, 255, 255, 0); /* input your netmask for example: 255.255.255.0 */

        // 设置网关
        IP4_ADDR(&gateway, 192, 168, 12, 1);   /* input your gateway for example: 192.168.12.1 */

        // 设置热点的IP地址、子网掩码、网关
        err_t ret = netifapi_netif_set_addr(g_iface, &ipaddr, &netmask, &gateway);

        // 打印接口调用结果
        printf("netifapi_netif_set_addr: %d\r\n", ret);

        // 停止DHCP服务 (DHCP服务有可能默认是开启状态)
        ret = netifapi_dhcps_stop(g_iface);

        // 打印接口调用结果
        printf("netifapi_dhcps_stop: %d\r\n", ret);

        // 启动DHCP服务
        ret = netifapi_dhcps_start(g_iface, 0, 0);

        // 打印接口调用结果
        printf("netifapi_dhcps_start: %d\r\n", ret);
    }

    return errCode;
}

// 停止热点
void StopHotspot(void)
{
    // 如果之前已经成功获取网络接口
    if (g_iface)
    {
        // 停止DHCP服务
        err_t ret = netifapi_dhcps_stop(g_iface);

        // 打印接口调用结果
        printf("netifapi_dhcps_stop: %d\r\n", ret);
    }

    // 使用UnRegisterWifiEvent接口解除事件监听
    WifiErrorCode errCode = UnRegisterWifiEvent(&g_defaultWifiEventListener);

    // 打印接口调用结果
    printf("UnRegisterWifiEvent: %d\r\n", errCode);

    // 使用DisableHotspot接口关闭热点
    errCode = DisableHotspot();

    // 打印接口调用结果
    printf("DisableHotspot: %d\r\n", errCode);
}

// 主线程函数
static void WifiHotspotTask(void *arg)
{
    (void)arg;

    // WifiErrorCode类型的变量，用于接收接口返回值
    WifiErrorCode errCode;

    // 准备AP的配置参数
    // 包括：热点名称、热点密码、加密方式、频带类型、信道等
    HotspotConfig config = {0};                 // 定义热点配置
    strcpy(config.ssid, "Pegasus");             // 设置热点名称Pegasus
    strcpy(config.preSharedKey, "OpenHarmony"); // 设置热点密码OpenHarmony
    config.securityType = WIFI_SEC_TYPE_PSK;    // 设置加密方式PSK
    config.band = HOTSPOT_BAND_TYPE_2G;         // 设置频带类型2.4G
    config.channelNum = 7;                      // 设置信道7

    // 等待100ms
    osDelay(10);

    // 开启热点
    printf("starting AP ...\r\n");
    errCode = StartHotspot(&config);

    // 打印接口调用结果
    printf("StartHotspot: %d\r\n", errCode);

    // 热点将开启1分钟
    // 可以通过串口工具发送 AT+PING 命令，验证WiFi客户端的连通性(比如连接到该热点的手机) 
    int timeout = 60;
    printf("After %d seconds Ap will turn off!\r\n", timeout);
    while (timeout--)
    {
        // printf("After %d seconds Ap will turn off!\r\n", timeout);
        osDelay(100);
    }

    // 关闭热点
    printf("stop AP ...\r\n");
    StopHotspot();
    printf("AP stopped.\r\n");
}

// 入口函数
static void WifiHotspotDemo(void)
{
    // 定义线程属性
    osThreadAttr_t attr;
    attr.name = "WifiHotspotTask";
    attr.attr_bits = 0U;
    attr.cb_mem = NULL;
    attr.cb_size = 0U;
    attr.stack_mem = NULL;
    attr.stack_size = 10240;
    attr.priority = osPriorityNormal;

    // 创建线程
    if (osThreadNew(WifiHotspotTask, NULL, &attr) == NULL)
    {
        printf("[WifiHotspotDemo] Falied to create WifiHotspotTask!\n");
    }
}

// 运行入口函数
APP_FEATURE_INIT(WifiHotspotDemo);
