#include <stdio.h>      // 标准输入输出
#include <stdbool.h>    // 布尔类型
#include <stdint.h>     // 几种扩展的整数类型和宏

#include "ohos_init.h"  // 用于初始化服务(services)和功能(features)
#include "cmsis_os2.h"  // CMSIS-RTOS API V2

#include "qrcodegen.h"  // 二维码生成器
#include "ssd1306.h"    // OLED驱动接口

/// @brief 在OLED上显示二维码
/// @param x_start 二维码左上角的X坐标
/// @param y_start 二维码左上角的Y坐标
/// @param qrcode 二维码数据
/// @param zoom 缩放比例
static void ssd1306_PrintQr(uint8_t x_start, uint8_t y_start, const uint8_t qrcode[], uint8_t zoom)
{
    // 检查zoom是否合法
    zoom = zoom == 0 ? 1 : zoom;

    // 获取二维码的边长，进行缩放
    int size = qrcodegen_getSize(qrcode) * zoom;

    // 显示二维码图像
    for (int y = 0; y < size; y++)
    {
        for (int x = 0; x < size; x++)
        {
            // 显示一个像素
            // qrcodegen_getModule returns false for white or true for black,
            // while the ssd1306_DrawPixel uses a reversed logic,
            // so we invert the value to get the correct color.
            // 
            // qrcodegen_getModule函数返回二维码指定坐标的像素颜色。false表示白色，true表示黑色
            // ssd1306_DrawPixel函数使用反向的逻辑，所以我们将值取反以获得正确的颜色。
            ssd1306_DrawPixel(x_start + x, y_start + y, !qrcodegen_getModule(qrcode, x / zoom, y / zoom));
        }
    }

    // 上屏显示
    ssd1306_UpdateScreen();
}

// /**
//  * @brief 创建一个简单的二维码，并在OLED上显示
//  * 
//  */
// static void doBasicDemo(void)
// {
//     const char *text = "Hello, world!";               // User-supplied text
//     enum qrcodegen_Ecc errCorLvl = qrcodegen_Ecc_LOW; // Error correction level

//     // Make and print the QR Code symbol
//     uint8_t qrcode[qrcodegen_BUFFER_LEN_MAX];
//     uint8_t tempBuffer[qrcodegen_BUFFER_LEN_MAX];
//     bool ok = qrcodegen_encodeText(text, tempBuffer, qrcode, errCorLvl,
//                                    qrcodegen_VERSION_MIN, qrcodegen_VERSION_MAX, qrcodegen_Mask_AUTO, true);
//     if (ok)
//         ssd1306_PrintQr(0, 0, qrcode, 2);
// }

// /**
//  * @brief 创建各种类型的二维码，并在OLED上显示，以测试接口能力。
//  * 
//  */
// static void doVarietyDemo(void)
// {
//     { // Numeric mode encoding (3.33 bits per digit)
//         uint8_t qrcode[qrcodegen_BUFFER_LEN_MAX];
//         uint8_t tempBuffer[qrcodegen_BUFFER_LEN_MAX];
//         bool ok = qrcodegen_encodeText("314159265358979323846264338327950288419716939937510", tempBuffer, qrcode,
//                                        qrcodegen_Ecc_MEDIUM, qrcodegen_VERSION_MIN, qrcodegen_VERSION_MAX, qrcodegen_Mask_AUTO, true);
//         if (ok)
//             ssd1306_PrintQr(0, 0, qrcode, 1);
//     }
//     osDelay(100);
//     ssd1306_Fill(Black);

//     { // Alphanumeric mode encoding (5.5 bits per character)
//         uint8_t qrcode[qrcodegen_BUFFER_LEN_MAX];
//         uint8_t tempBuffer[qrcodegen_BUFFER_LEN_MAX];
//         bool ok = qrcodegen_encodeText("DOLLAR-AMOUNT:$39.87 PERCENTAGE:100.00% OPERATIONS:+-*/", tempBuffer, qrcode,
//                                        qrcodegen_Ecc_HIGH, qrcodegen_VERSION_MIN, qrcodegen_VERSION_MAX, qrcodegen_Mask_AUTO, true);
//         if (ok)
//             ssd1306_PrintQr(0, 0, qrcode, 1);
//     }
//     osDelay(100);
//     ssd1306_Fill(Black);

//     { // Unicode text as UTF-8
//         const char *text = "\xE3\x81\x93\xE3\x82\x93\xE3\x81\xAB\xE3\x81\xA1wa\xE3\x80\x81"
//                            "\xE4\xB8\x96\xE7\x95\x8C\xEF\xBC\x81\x20\xCE\xB1\xCE\xB2\xCE\xB3\xCE\xB4";
//         uint8_t qrcode[qrcodegen_BUFFER_LEN_MAX];
//         uint8_t tempBuffer[qrcodegen_BUFFER_LEN_MAX];
//         bool ok = qrcodegen_encodeText(text, tempBuffer, qrcode,
//                                        qrcodegen_Ecc_QUARTILE, qrcodegen_VERSION_MIN, qrcodegen_VERSION_MAX, qrcodegen_Mask_AUTO, true);
//         if (ok)
//             ssd1306_PrintQr(0, 0, qrcode, 1);
//     }
//     osDelay(100);
//     ssd1306_Fill(Black);

//     { // Moderately large QR Code using longer text (from Lewis Carroll's Alice in Wonderland)
//         const char *text =
//             "Alice was beginning to get very tired of sitting by her sister on the bank, "
//             "and of having nothing to do: once or twice she had peeped into the book her sister was reading, "
//             "but it had no pictures or conversations in it, 'and what is the use of a book,' thought Alice "
//             "'without pictures or conversations?' So she was considering in her own mind (as well as she could, "
//             "for the hot day made her feel very sleepy and stupid), whether the pleasure of making a "
//             "daisy-chain would be worth the trouble of getting up and picking the daisies, when suddenly "
//             "a White Rabbit with pink eyes ran close by her.";
//         uint8_t qrcode[qrcodegen_BUFFER_LEN_MAX];
//         uint8_t tempBuffer[qrcodegen_BUFFER_LEN_MAX];
//         bool ok = qrcodegen_encodeText(text, tempBuffer, qrcode,
//                                        qrcodegen_Ecc_HIGH, qrcodegen_VERSION_MIN, qrcodegen_VERSION_MAX, qrcodegen_Mask_AUTO, true);
//         if (ok)
//             ssd1306_PrintQr(0, 0, qrcode, 1);
//     }
// }

// /**
//  * @brief 创建大小和内容相同，但是mask不同的二维码，并在OLED上显示
//  * 
//  */
// static void doMaskDemo(void)
// {
//     { // Project Nayuki URL
//         uint8_t qrcode[qrcodegen_BUFFER_LEN_MAX];
//         uint8_t tempBuffer[qrcodegen_BUFFER_LEN_MAX];
//         bool ok;

//         ok = qrcodegen_encodeText("https://www.nayuki.io/", tempBuffer, qrcode,
//                                   qrcodegen_Ecc_HIGH, qrcodegen_VERSION_MIN, qrcodegen_VERSION_MAX, qrcodegen_Mask_AUTO, true);
//         if (ok)
//             ssd1306_PrintQr(0, 0, qrcode, 1);
//         osDelay(100);
//         ssd1306_Fill(Black);

//         ok = qrcodegen_encodeText("https://www.nayuki.io/", tempBuffer, qrcode,
//                                   qrcodegen_Ecc_HIGH, qrcodegen_VERSION_MIN, qrcodegen_VERSION_MAX, qrcodegen_Mask_3, true);
//         if (ok)
//             ssd1306_PrintQr(0, 0, qrcode, 1);
//         osDelay(100);
//         ssd1306_Fill(Black);
//     }

//     { // Chinese text as UTF-8
//         const char *text =
//             "\xE7\xB6\xAD\xE5\x9F\xBA\xE7\x99\xBE\xE7\xA7\x91\xEF\xBC\x88\x57\x69\x6B\x69\x70"
//             "\x65\x64\x69\x61\xEF\xBC\x8C\xE8\x81\x86\xE8\x81\xBD\x69\x2F\xCB\x8C\x77\xC9\xAA"
//             "\x6B\xE1\xB5\xBB\xCB\x88\x70\x69\xCB\x90\x64\x69\x2E\xC9\x99\x2F\xEF\xBC\x89\xE6"
//             "\x98\xAF\xE4\xB8\x80\xE5\x80\x8B\xE8\x87\xAA\xE7\x94\xB1\xE5\x85\xA7\xE5\xAE\xB9"
//             "\xE3\x80\x81\xE5\x85\xAC\xE9\x96\x8B\xE7\xB7\xA8\xE8\xBC\xAF\xE4\xB8\x94\xE5\xA4"
//             "\x9A\xE8\xAA\x9E\xE8\xA8\x80\xE7\x9A\x84\xE7\xB6\xB2\xE8\xB7\xAF\xE7\x99\xBE\xE7"
//             "\xA7\x91\xE5\x85\xA8\xE6\x9B\xB8\xE5\x8D\x94\xE4\xBD\x9C\xE8\xA8\x88\xE7\x95\xAB";
//         uint8_t qrcode[qrcodegen_BUFFER_LEN_MAX];
//         uint8_t tempBuffer[qrcodegen_BUFFER_LEN_MAX];
//         bool ok;

//         ok = qrcodegen_encodeText(text, tempBuffer, qrcode,
//                                   qrcodegen_Ecc_MEDIUM, qrcodegen_VERSION_MIN, qrcodegen_VERSION_MAX, qrcodegen_Mask_0, true);
//         if (ok)
//             ssd1306_PrintQr(0, 0, qrcode, 1);
//         osDelay(100);
//         ssd1306_Fill(Black);

//         ok = qrcodegen_encodeText(text, tempBuffer, qrcode,
//                                   qrcodegen_Ecc_MEDIUM, qrcodegen_VERSION_MIN, qrcodegen_VERSION_MAX, qrcodegen_Mask_1, true);
//         if (ok)
//             ssd1306_PrintQr(0, 0, qrcode, 1);
//         osDelay(100);
//         ssd1306_Fill(Black);

//         ok = qrcodegen_encodeText(text, tempBuffer, qrcode,
//                                   qrcodegen_Ecc_MEDIUM, qrcodegen_VERSION_MIN, qrcodegen_VERSION_MAX, qrcodegen_Mask_5, true);
//         if (ok)
//             ssd1306_PrintQr(0, 0, qrcode, 1);
//         osDelay(100);
//         ssd1306_Fill(Black);

//         ok = qrcodegen_encodeText(text, tempBuffer, qrcode,
//                                   qrcodegen_Ecc_MEDIUM, qrcodegen_VERSION_MIN, qrcodegen_VERSION_MAX, qrcodegen_Mask_7, true);
//         if (ok)
//             ssd1306_PrintQr(0, 0, qrcode, 1);
//     }
// }

// 主线程函数
static void QrTask(void *arg)
{
    (void)arg;

    // 初始化OLED
    ssd1306_Init();

    // 全屏填充黑色
    ssd1306_Fill(Black);

    // 工作循环
    while (1)
    {
        // 存放文本数据二维码
        uint8_t qrcode[qrcodegen_BUFFER_LEN_MAX];
        uint8_t tempBuffer[qrcodegen_BUFFER_LEN_MAX];

        // 将openharmony官网地址编码为二维码
        // 二维码边长 = qrcodegen_VERSION * 4 + 17
        // 版本  边长
        // 1	21
        // 2	25
        // 3	29
        // 4	33
        // 5	37
        // 6	41
        // 7	45
        // 8	49
        // 9	53
        // 10	57
        // 11	61
        // 12	65
        // ...
        bool ok = qrcodegen_encodeText("https://www.openharmony.cn",
                                       tempBuffer, qrcode, qrcodegen_Ecc_LOW,
                                       3, qrcodegen_VERSION_MAX,
                                       qrcodegen_Mask_AUTO, true);
        if (!ok)
            return;

        // 在左上角显示
        ssd1306_PrintQr(0, 0, qrcode, 1);   // 1倍大小
        osDelay(100);                       // 等待1秒
        ssd1306_Fill(Black);                // 全屏填充黑色
        ssd1306_PrintQr(0, 0, qrcode, 2);   // 2倍大小
        osDelay(100);                       // 等待1秒
        ssd1306_Fill(Black);                // 全屏填充黑色

        // 在右上角显示
        ssd1306_Fill(White);                // 全屏填充白色
        ssd1306_PrintQr(SSD1306_WIDTH - qrcodegen_getSize(qrcode) * 2, 0, qrcode, 2);   //2倍大小
        osDelay(100);                       // 等待1秒
        ssd1306_Fill(Black);                // 全屏填充黑色

        // 存放二进制数据二维码
        uint8_t dataAndTemp[qrcodegen_BUFFER_LEN_FOR_VERSION(7)] = {0xE3, 0x81, 0x82};
        uint8_t qrBinary[qrcodegen_BUFFER_LEN_FOR_VERSION(7)];

        // 将二进制数据编码为二维码
        ok = qrcodegen_encodeBinary(dataAndTemp, 3, qrBinary,
                                    qrcodegen_Ecc_HIGH, 2, 7, qrcodegen_Mask_4, false);
        if (!ok)
            return;
        
        // 在左上角显示
        ssd1306_PrintQr(0, 0, qrBinary, 2);     // 2倍大小
        osDelay(100);                           // 等待1秒
        ssd1306_Fill(Black);                    // 全屏填充黑色

        // 其他demo
        // doBasicDemo();
        // osDelay(100);
        // ssd1306_Fill(Black);
        // doVarietyDemo();
        // osDelay(100);
        // ssd1306_Fill(Black);
        // doSegmentDemo();
        // doMaskDemo();
        // osDelay(100);
        // ssd1306_Fill(Black);
    }
}


// 入口函数
static void QrEntry(void)
{
    // 定义线程属性
    osThreadAttr_t attr;
    attr.name = "QrTask";
    attr.stack_size = 1024 * 40;
    attr.priority = osPriorityNormal;

    // 创建线程
    if (osThreadNew(QrTask, NULL, &attr) == NULL)
    {
        printf("[QrEntry] Create QrTask failed!\n");
    }
}

// 运行入口函数
APP_FEATURE_INIT(QrEntry);
