/*
 * Copyright (c) 2022, Dragon. HeBei University. email:dragon@hbu.edu.cn
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its
 *    contributors may be used to endorse or promote products derived from
 *    this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <stdio.h>     // 标准输入输出
#include "ohos_init.h" // 用于初始化服务(services)和功能(features)

// utils\native\lite\include\utils_file.h
// utils\native\lite\file\src\file_impl_hal\file.c
#include "utils_file.h" // 文件操作接口

/**
 * @brief 入口函数。如果文件不存在，则创建文件，写入内容。如果文件存在，则读取文件内容，删除文件。
 * @return 无
 */
void entry(void)
{
    // 定义文件名
    char fileName[] = "wifiAccount";

    // 获取文件大小，可以用来检查文件是否存在
    int fileLen = 0;
    // 失败返回-1
    int ret = UtilsFileStat(fileName, &fileLen);

    // 如果文件不存在，则创建文件，写入内容。
    if (ret == -1)
    {
        // 输出文件不存在
        printf("file %s not exist\n", fileName);

        // 定义文件数据
        // 格式: ssid,password
        static const char dataWrite[] = "dragon,123456";

        // 打开文件
        // O_WRONLY_FS: write-only (只有写操作)
        // O_CREAT_FS: create file if not exist (如果文件不存在就创建文件)
        // O_TRUNC_FS: clear file content if the file exists and can be opened in write mode (如果文件存在，并且可以以写模式打开就清除文件的内容)
        int fd = UtilsFileOpen(fileName, O_WRONLY_FS | O_CREAT_FS | O_TRUNC_FS, 0);
        printf("file handle = %d\n", fd);

        // 写入数据
        ret = UtilsFileWrite(fd, dataWrite, strlen(dataWrite));
        printf("write ret = %d\n", ret);

        // 关闭文件
        ret = UtilsFileClose(fd);
    }
    // 如果文件存在，则读取文件内容，删除文件。
    else
    {
        // 输出文件存在
        printf("file %s exist\n", fileName);

        // 打开文件
        // O_RDWR_FS：读写模式
        int fd = UtilsFileOpen(fileName, O_RDWR_FS, 0);
        printf("file handle = %d\n", fd);

        // 重新定位文件读/写偏移量
        ret = UtilsFileSeek(fd, 0, SEEK_SET_FS);
        printf("seek ret = %d\n", ret);

        // 读取ssid数据
        char ssid[64] = {0};
        int readLen = UtilsFileRead(fd, ssid, 6);
        printf("read len = %d : ssid = %s\n", readLen, ssid);

        // 重新定位文件读/写偏移量
        ret = UtilsFileSeek(fd, 7, SEEK_SET_FS);
        printf("seek ret = %d\n", ret);

        // 读取password数据
        char password[64] = {0};
        readLen = UtilsFileRead(fd, password, 6);
        printf("read len = %d : password = %s\n", readLen, password);

        // 关闭文件
        ret = UtilsFileClose(fd);

        // 获取文件大小
        fileLen = 0;
        ret = UtilsFileStat(fileName, &fileLen);
        printf("file size = %d\n", fileLen);

        // 删除文件（只是为了展示接口，实际开发中不应当删除）
        ret = UtilsFileDelete(fileName);
        printf("delete ret = %d\n", ret);
    }
}

// 让entry函数以“优先级2”在系统启动过程的“阶段8. application-layer feature”阶段执行。
APP_FEATURE_INIT(entry);
