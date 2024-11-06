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

// 标准输入输出
#include <stdio.h>

// ohos_init.h是OpenHarmony的特有头文件
// 位置：utils\native\lite\include\ohos_init.h
// Provides the entries for initializing services and features during service development
// 在开发中，它提供了一系列入口，用于初始化服务(services)和功能(features)。
//
// 在系统启动过程中，服务和功能按以下顺序初始化：
// 阶段1. core
// 阶段2. core system service
// 阶段3. core system feature
// 阶段4. system startup
// 阶段5. system service
// 阶段6. system feature
// 阶段7. application-layer service
// 阶段8. application-layer feature
//
#include "ohos_init.h"

// 定义一个函数，输出hello world
void hello(void)
{
    printf("Hello world!\r\n");
}

// ohos_init.h中定义了以下8个宏，用于让一个函数以“优先级2”在系统启动过程的1-8阶段执行。
// 即：函数会被标记为入口，在系统启动过程的1-8阶段，以“优先级2”被调用。
// 优先级范围：0-4。
// 优先级顺序：0, 1, 2, 3, 4
// CORE_INIT()：          阶段1. core
// SYS_SERVICE_INIT()：   阶段2. core system service
// SYS_FEATURE_INIT()：   阶段3. core system feature
// SYS_RUN()：            阶段4. system startup
// SYSEX_SERVICE_INIT()： 阶段5. system service
// SYSEX_FEATURE_INIT()： 阶段6. system feature
// APP_SERVICE_INIT()：   阶段7. application-layer service
// APP_FEATURE_INIT()：   阶段8. application-layer feature

// SYS_RUN() 是ohos_init.h中定义的宏，让函数在系统启动时执行。
// 让hello函数以“优先级2”在系统启动过程的“阶段4. system startup”阶段执行。
SYS_RUN(hello);
