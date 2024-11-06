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

#include "hal_sys_param.h"
#include <securec.h>

static const char OHOS_PRODUCT_TYPE[] = {"demo"};
static const char OHOS_MANUFACTURE[] = {"hbu"};
static const char OHOS_BRAND[] = {"hbucpi"};
static const char OHOS_MARKET_NAME[] = {"****"};
static const char OHOS_PRODUCT_SERIES[] = {"****"};
static const char OHOS_PRODUCT_MODEL[] = {"****"};
static const char OHOS_SOFTWARE_MODEL[] = {"****"};
static const char OHOS_HARDWARE_MODEL[] = {"****"};
static const char OHOS_HARDWARE_PROFILE[] = {"aout:true,display:true"};
static const char OHOS_BOOTLOADER_VERSION[] = {"bootloader"};
static const char OHOS_ABI_LIST[] = {"****"};
static const char OHOS_SERIAL[] = {"1234567890"};  // provided by OEM.

char* HalGetProductType(void)
{
    char* value = (char*)malloc(strlen(OHOS_PRODUCT_TYPE) + 1);
    if (value == NULL) {
        return NULL;
    }
    if (strcpy_s(value, strlen(OHOS_PRODUCT_TYPE) + 1, OHOS_PRODUCT_TYPE) != 0) {
        free(value);
        return NULL;
    }
    return value;
}

char* HalGetManufacture(void)
{
    char* value = (char*)malloc(strlen(OHOS_MANUFACTURE) + 1);
    if (value == NULL) {
        return NULL;
    }
    if (strcpy_s(value, strlen(OHOS_MANUFACTURE) + 1, OHOS_MANUFACTURE) != 0) {
        free(value);
        return NULL;
    }
    return value;
}

char* HalGetBrand(void)
{
    char* value = (char*)malloc(strlen(OHOS_BRAND) + 1);
    if (value == NULL) {
        return NULL;
    }
    if (strcpy_s(value, strlen(OHOS_BRAND) + 1, OHOS_BRAND) != 0) {
        free(value);
        return NULL;
    }
    return value;
}

char* HalGetMarketName(void)
{
    char* value = (char*)malloc(strlen(OHOS_MARKET_NAME) + 1);
    if (value == NULL) {
        return NULL;
    }
    if (strcpy_s(value, strlen(OHOS_MARKET_NAME) + 1, OHOS_MARKET_NAME) != 0) {
        free(value);
        return NULL;
    }
    return value;
}

char* HalGetProductSeries(void)
{
    char* value = (char*)malloc(strlen(OHOS_PRODUCT_SERIES) + 1);
    if (value == NULL) {
        return NULL;
    }
    if (strcpy_s(value, strlen(OHOS_PRODUCT_SERIES) + 1, OHOS_PRODUCT_SERIES) != 0) {
        free(value);
        return NULL;
    }
    return value;
}

char* HalGetProductModel(void)
{
    char* value = (char*)malloc(strlen(OHOS_PRODUCT_MODEL) + 1);
    if (value == NULL) {
        return NULL;
    }
    if (strcpy_s(value, strlen(OHOS_PRODUCT_MODEL) + 1, OHOS_PRODUCT_MODEL) != 0) {
        free(value);
        return NULL;
    }
    return value;
}

char* HalGetSoftwareModel(void)
{
    char* value = (char*)malloc(strlen(OHOS_SOFTWARE_MODEL) + 1);
    if (value == NULL) {
        return NULL;
    }
    if (strcpy_s(value, strlen(OHOS_SOFTWARE_MODEL) + 1, OHOS_SOFTWARE_MODEL) != 0) {
        free(value);
        return NULL;
    }
    return value;
}

char* HalGetHardwareModel(void)
{
    char* value = (char*)malloc(strlen(OHOS_HARDWARE_MODEL) + 1);
    if (value == NULL) {
        return NULL;
    }
    if (strcpy_s(value, strlen(OHOS_HARDWARE_MODEL) + 1, OHOS_HARDWARE_MODEL) != 0) {
        free(value);
        return NULL;
    }
    return value;
}

char* HalGetHardwareProfile(void)
{
    char* value = (char*)malloc(strlen(OHOS_HARDWARE_PROFILE) + 1);
    if (value == NULL) {
        return NULL;
    }
    if (strcpy_s(value, strlen(OHOS_HARDWARE_PROFILE) + 1, OHOS_HARDWARE_PROFILE) != 0) {
        free(value);
        return NULL;
    }
    return value;
}

char* HalGetSerial(void)
{
    char* value = (char*)malloc(strlen(OHOS_SERIAL) + 1);
    if (value == NULL) {
        return NULL;
    }
    if (strcpy_s(value, strlen(OHOS_SERIAL) + 1, OHOS_SERIAL) != 0) {
        free(value);
        return NULL;
    }
    return value;
}

char* HalGetBootloaderVersion(void)
{
    char* value = (char*)malloc(strlen(OHOS_BOOTLOADER_VERSION) + 1);
    if (value == NULL) {
        return NULL;
    }
    if (strcpy_s(value, strlen(OHOS_BOOTLOADER_VERSION) + 1, OHOS_BOOTLOADER_VERSION) != 0) {
        free(value);
        return NULL;
    }
    return value;
}

char* HalGetAbiList(void)
{
    char* value = (char*)malloc(strlen(OHOS_ABI_LIST) + 1);
    if (value == NULL) {
        return NULL;
    }
    if (strcpy_s(value, strlen(OHOS_ABI_LIST) + 1, OHOS_ABI_LIST) != 0) {
        free(value);
        return NULL;
    }
    return value;
}