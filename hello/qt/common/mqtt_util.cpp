//
// Created by root on 2021/8/25.
//

#include "mqtt_util.h"


//stl
#include <typeinfo>
#include <vector>
#include <deque>
#include <list>
#include <set>
#include <map>
#include <bitset>
#include <stack>
#include <queue>
#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <limits>

//clib
#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <float.h>
#include <iso646.h>
#include <locale.h>
#include <math.h>
#include <setjmp.h>
#include <signal.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <wchar.h>
#include <wctype.h>

#define CLIENT_TIME_LENGTH 		10

#include <openssl/ossl_typ.h>
#include <openssl/hmac.h>


void* StrMemSet(void *_Dst, int _Val, size_t _Size) {
    memset(_Dst, _Val, _Size);
    return NULL;
}

char* MqttUtil::GetClientTimesStamp()
{
    time_t t;
    struct tm *lt;

    time(&t); //get Unix time stamp
    //lt = gmtime(&t); //transform into time struct
    lt = localtime(&t); //transform into time struct
    if (lt == NULL) {
        return NULL;
    }
    char *dest_str = static_cast<char *>(malloc(CLIENT_TIME_LENGTH + 1)); //length of yyyyMMDDhh + 1
    if (dest_str == NULL) {
        return NULL;
    } else {
        StrMemSet(dest_str, 0, CLIENT_TIME_LENGTH + 1);
        snprintf(dest_str, CLIENT_TIME_LENGTH + 1, "%d%.2d%.2d%.2d", lt->tm_year + 1900, lt->tm_mon + 1, lt->tm_mday, lt->tm_hour);
        return dest_str;
    }
}

int StringLength(const char *str) {
    if (str == NULL) {
        return 0;
    }
    int len = 0;
    const char *temp_str = str;
    while (*temp_str++ != '\0') {
        len++;
    }
    return len;
}
/**NOTE: "*dst" will be "malloc" inside this function, and the invocation needs to free it after used.
 * If this function is recalled with the same "**dst", you should free the pointer "*dst" before invoking this function in case of memory leak.
 */
int CopyStrValue(char **dst, const char *src, int length) {
    if (length <= 0) {
        return 0;
    }
    *dst = static_cast<char *>(malloc(length + 1));
    if (*dst == NULL) {
        return -1;
    }
    StrMemSet(*dst, 0, length);
    strncat(*dst, src, length);
    return 0;
}
#define PASSWORD_ENCRYPT_LENGTH 		64
void StringMalloc(char **str, int length) {
    if (length <= 0) {
        return;
    }
    *str = static_cast<char *>(malloc(length));
    if (*str == NULL) {
        return;
    }
    memset(*str, 0, length);
}

void _MemFree(char **str) {
    if (*str != NULL) {
        free(*str);
        *str = NULL;
    }
}

#define IOTA_SUCCESS 					0
#define ENCRYPT_LENGTH 					32
#define TIME_STAMP_LENGTH 			10
#define TRY_MAX_TIME 				100
#define SHA256_ENCRYPTION_LENGRH 	32
int EncryWithHMacSha256(const char *inputData, char **inputKey, int inEncryDataLen, char *outData) {

    if (inputData == NULL || (*inputKey) == NULL) {
        return -1;
    }

    if (TIME_STAMP_LENGTH != strlen(*inputKey)) {
        return -1;
    }

    char *end = NULL;
    unsigned int mac_length = 0;
    unsigned int tryTime = 1;
    int lenData = strlen(inputData);
    long timeTmp = strtol(*inputKey, &end, 10);
    unsigned char *temp = HMAC(EVP_sha256(), *inputKey, TIME_STAMP_LENGTH, (const unsigned char*) inputData, lenData, NULL, &mac_length);

    while ((strlen(reinterpret_cast<const char *>(temp)) != SHA256_ENCRYPTION_LENGRH) && (mac_length != SHA256_ENCRYPTION_LENGRH)) {
        tryTime++;
        if (tryTime > TRY_MAX_TIME) {
            return -1;
        }

        timeTmp++;
        snprintf(*inputKey, TIME_STAMP_LENGTH + 1, "%ld", timeTmp);
        temp = HMAC(EVP_sha256(), *inputKey, TIME_STAMP_LENGTH, (const unsigned char*) inputData, lenData, NULL, &mac_length);
    }

    int uiIndex, uiLoop;
    char ucHex;

    for (uiIndex = 0, uiLoop = 0; uiLoop < inEncryDataLen; uiLoop++) {
        ucHex = (temp[uiLoop] >> 4) & 0x0F;
        outData[uiIndex++] = (ucHex <= 9) ? (ucHex + '0') : (ucHex + 'a' - 10);

        ucHex = temp[uiLoop] & 0x0F;
        outData[uiIndex++] = (ucHex <= 9) ? (ucHex + '0') : (ucHex + 'a' - 10);
    }

    outData[uiIndex] = '\0';

    return IOTA_SUCCESS;
}

int MqttUtil::GetEncryptedPassword(const char *password, char **timestamp, char **encryptedPwd) {
    if (password == NULL) {
        return -1;
    }

    char *temp_pwd = NULL;
    if (CopyStrValue(&temp_pwd, (const char*) password, StringLength(password)) < 0) {
        return -1;
    }

    char *temp_encrypted_pwd = NULL;
    StringMalloc(&temp_encrypted_pwd, PASSWORD_ENCRYPT_LENGTH + 1);
    if (temp_encrypted_pwd == NULL) {
        _MemFree(&temp_pwd);
        return -1;
    }

    int ret = EncryWithHMacSha256(temp_pwd, timestamp, ENCRYPT_LENGTH, temp_encrypted_pwd);
    if (ret != IOTA_SUCCESS) {
        _MemFree(&temp_pwd);
        _MemFree(&temp_encrypted_pwd);
        return -1;
    }

    if (CopyStrValue(encryptedPwd, (const char*) temp_encrypted_pwd, PASSWORD_ENCRYPT_LENGTH) < 0) {
        _MemFree(&temp_pwd);
        _MemFree(&temp_encrypted_pwd);
        return -1;
    }

    _MemFree(&temp_pwd);
    _MemFree(&temp_encrypted_pwd);
    return IOTA_SUCCESS;
}
