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

// utils\native\lite\include\kv_store.h
// utils\native\lite\kv_store\src\kvstore_impl_hal\kv_store.c
#include "kv_store.h"  // KV存储接口，操作键值对

// 入口函数
void test(void)
{
    // 定义键值对
    // key由小写字母、数字、下划线和"."构成
    const char *key = "name";
    // value长度最大32字节（包括字符串结束符）
    char value[32] = {0};
    // UtilsGetValue：从文件系统或cache中读取指定键的值
    // 操作成功则返回值的长度；参数错误返回-9；其他情况返回-1；如果值是从cache中读取的，则返回0
    int retGet = UtilsGetValue(key, value, 32);
    // 读取失败，添加键值对
    if (retGet < 0)
    {
        // 定义要添加的值
        const char *valueToWrite = "OpenHarmony";
        // UtilsSetValue：在文件系统或cache中添加或更新指定键的值
        // 操作成功则返回0；参数错误返回-9；其他情况返回-1
        int retSet = UtilsSetValue(key, valueToWrite);
        printf("SetValue, result = %d\n", retSet);
    }
    // 读取成功，打印值
    else
    {
        printf("GetValue, result = %d, value = %s\n", retGet, value);
    }
}

// 让test函数以“优先级2”在系统启动过程的“阶段8. application-layer feature”阶段执行。
APP_FEATURE_INIT(test);