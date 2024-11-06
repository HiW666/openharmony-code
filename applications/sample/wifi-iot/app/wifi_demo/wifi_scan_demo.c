#include <stdio.h>          // 标准输入输出
#include <unistd.h>         // POSIX标准接口
#include <string.h>         // 字符串处理(操作字符数组)

#include "ohos_init.h"      // 用于初始化服务(services)和功能(features)
#include "cmsis_os2.h"      // CMSIS-RTOS API V2

// Wi-Fi设备接口：station模式
// 用于：开启和关闭Wi-Fi设备的station模式，连接或断开station，查询station状态，事件监听
#include "wifi_device.h"

// 全局变量，用于标识扫描是否完成
static int g_scanDone = 0;

// 返回字符串形式的 Wi-Fi security types，用于日志输出
static char *SecurityTypeName(WifiSecurityType type)
{
    switch (type)
    {
    case WIFI_SEC_TYPE_OPEN:
        return "OPEN";
    case WIFI_SEC_TYPE_WEP:
        return "WEP";
    case WIFI_SEC_TYPE_PSK:
        return "PSK";
    case WIFI_SEC_TYPE_SAE:
        return "SAE";
    default:
        break;
    }
    return "UNKNOW";
}

// 连接状态变化回调函数
// 该回调函数有两个参数state和info
//      state表示连接状态，WIFI_STATE_AVALIABLE表示连接成功，WIFI_STATE_NOT_AVALIABLE表示连接失败
//      info类型为WifiLinkedInfo*，有多个成员，包括ssid，bssid，rssi，connState，disconnectedReason等
void OnWifiConnectionChanged(int state, WifiLinkedInfo *info)
{
    (void)state;    // 忽略参数state
    (void)info;     // 忽略参数info

    // 简单输出日志信息，表明函数被执行了
    printf("%s %d\r\n", __FUNCTION__, __LINE__);
}

// 显示扫描结果
void PrintScanResult(void)
{
    // 创建一个WifiScanInfo数组，用于存放扫描结果
    // 扫描结果的数量由WIFI_SCAN_HOTSPOT_LIMIT定义。
    // 参见 foundation\communication\wifi_lite\interfaces\wifiservice\wifi_scan_info.h
    WifiScanInfo scanResult[WIFI_SCAN_HOTSPOT_LIMIT] = {0};
    uint32_t resultSize = WIFI_SCAN_HOTSPOT_LIMIT;

    // 初始化数组
    memset(&scanResult, 0, sizeof(scanResult));

    // 获取扫描结果
    // GetScanInfoList函数有两个参数：
    // result指向用于存放结果的数组，需要大于等于WIFI_SCAN_HOTSPOT_LIMIT
    // size类型为指针，是为了内部能够修改它的值，返回后size指向的值是实际搜索到的热点个数
    // 调用GetScanInfoList函数前，size指向的实际值不能为0，否则会报参数错误
    WifiErrorCode errCode = GetScanInfoList(scanResult, &resultSize);

    // 检查接口调用结果
    if (errCode != WIFI_SUCCESS)
    {
        printf("GetScanInfoList failed: %d\r\n", errCode);
        return;
    }

    // 打印扫描结果
    for (uint32_t i = 0; i < resultSize; i++)
    {
        // 存储MAC地址字符串
        static char macAddress[32] = {0};

        // 获取MAC地址
        WifiScanInfo info = scanResult[i];
        unsigned char *mac = info.bssid;

        // MAC地址转换为字符串
        snprintf(macAddress, sizeof(macAddress), "%02X:%02X:%02X:%02X:%02X:%02X", 
                 mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);

        // 输出日志
        printf("AP[%d]: %s, %4s, %d, %d, %d, %s\r\n",   // 格式
                i,                                      // 热点序号
                macAddress,                             // MAC地址
                SecurityTypeName(info.securityType),    // 安全类型
                info.rssi,                              // 信号强度(Received Signal Strength Indicator)
                info.band,                              // 频带类型
                info.frequency,                         // 频段
                info.ssid);                             // SSID
    }
}

// 扫描状态变化回调函数
// 该回调函数有两个参数state和size
//      state表示扫描状态，WIFI_STATE_AVALIABLE表示扫描动作完成，WIFI_STATE_NOT_AVALIABLE表示扫描动作未完成
//      size表示扫描到的热点个数
void OnWifiScanStateChanged(int state, int size)
{
    // 输出日志
    printf("%s %d, state = %X, size = %d\r\n", __FUNCTION__, __LINE__, state, size);

    // 扫描完成，并且找到了热点
    if (state == WIFI_STATE_AVALIABLE && size > 0)
    {
        // 不能直接调用GetScanInfoList函数，否则会有运行时异常报错
        // 可以更新全局状态变量，在另外一个线程中轮询状态变量，这种方式实现起来比较简单
        // 但需要保证更新和查询操作的原子性，逻辑才是严格正确的
        // 或者使用信号量进行通知，这种方式更好一些，更优雅
        g_scanDone = 1;
    }
}

// 主线程函数
static void WifiScanTask(void *arg)
{
    (void)arg;

    // WifiErrorCode类型的变量，用于接收接口返回值
    WifiErrorCode errCode;

    // 创建WiFi事件监听器
    // 参见 foundation\communication\wifi_lite\interfaces\wifiservice\wifi_event.h
    // 开启WiFi设备的STA模式之前，需要使用RegisterWifiEvent接口，向系统注册状态监听函数，用于接收状态通知
    // STA模式需要绑定如下两个回调函数：
    //  OnWifiScanStateChanged用于绑定扫描状态监听函数
    //  OnWifiConnectionChanged用于绑定连接状态监听函数
    WifiEvent eventListener = {
        // 在连接状态发生变化时，调用OnWifiConnectionChanged回调函数
        .OnWifiConnectionChanged = OnWifiConnectionChanged,
        // 在扫描状态发生变化时，调用OnWifiScanStateChanged回调函数
        .OnWifiScanStateChanged = OnWifiScanStateChanged};

    // 等待100ms
    osDelay(10);

    // 使用RegisterWifiEvent接口，注册WiFi事件监听器
    errCode = RegisterWifiEvent(&eventListener);

    // 功能相关接口都有WifiErrorCode类型的返回值，
    // 需要接收并判断返回值是否为WIFI_SUCCESS，用于确认是否调用成功。
    // 不为WIFI_SUCCESS表示失败，通过枚举值查找错误原因。
    // 这里简单做个日志输出
    printf("RegisterWifiEvent: %d\r\n", errCode);

    // 工作循环
    while (1)
    {
        // 开启WiFi设备的STA模式。使其可以扫描，并且连接到AP。
        // 进行WiFi的STA模式开发前，必须先调用EnableWifi函数。
        errCode = EnableWifi();
        printf("EnableWifi: %d\r\n", errCode);
        osDelay(100);

        // 开始扫描WiFi热点。只是触发扫描动作，并不会等到扫描完成才返回。
        // 不返回扫描结果，只是通过OnWifiScanStateChanged事件通知扫描结果。
        // 在事件处理函数中，可以通过调用GetScanInfoList获取扫描结果。
        g_scanDone = 0;
        errCode = Scan();
        printf("Scan: %d\r\n", errCode);

        // 等待扫描完成
        while (!g_scanDone)
        {
            osDelay(5);
        }

        // 打印扫描结果
        // 扫描完成后要及时调用GetScanInfoList函数获取扫描结果
        // 如果间隔时间太长（例如5秒以上），可能会无法获得上次扫描结果
        PrintScanResult();

        // 关闭WiFi设备的STA模式。
        errCode = DisableWifi();
        printf("DisableWifi: %d\r\n", errCode);
        osDelay(500);
    }
}

// 入口函数
static void WifiScanDemo(void)
{
    // 定义线程属性
    osThreadAttr_t attr;
    attr.name = "WifiScanTask";
    attr.attr_bits = 0U;
    attr.cb_mem = NULL;
    attr.cb_size = 0U;
    attr.stack_mem = NULL;
    attr.stack_size = 10240;
    attr.priority = osPriorityNormal;

    // 创建线程
    if (osThreadNew(WifiScanTask, NULL, &attr) == NULL)
    {
        printf("[WifiScanDemo] Falied to create WifiScanTask!\n");
    }
}

// 运行入口函数
APP_FEATURE_INIT(WifiScanDemo);
