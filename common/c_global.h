#ifndef CX_CT_X2_C_GLOBAL_H
#define CX_CT_X2_C_GLOBAL_H


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


#ifdef _MSC_VER
#if _MSC_VER < 1900
#define snprintf _snprintf
#define vsnprintf _vsnprintf
#endif
//*warning
#pragma once
#pragma warning(push)
#pragma warning(disable : 4005)
#pragma warning(disable : 4100) /* -- Disable warning 'unreferenced formal parameter' -- */
#pragma warning(disable : 4251)
#pragma warning(disable : 4355)
#pragma warning(disable : 4290)
#pragma warning(disable : 4291)
#pragma warning(disable : 4661)
#pragma warning(disable : 4819)
#pragma warning(disable : 4996)
#pragma warning(pop)
#else
#ifndef GM_OS_WIN
#include <signal.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/types.h>
#endif
#include <stdint.h>
#include <unistd.h>
#endif // _MSC_VER

namespace cx
{
    typedef signed char int8;   /* 8 bit signed */
    typedef unsigned char uint8;   /* 8 bit unsigned */
    typedef short int16;   /* 16 bit signed */
    typedef unsigned short uint16;   /* 16 bit unsigned */
    typedef int int32;   /* 32 bit signed */
    typedef unsigned int uint32;   /* 32 bit unsigned */

#ifdef _MSC_VER
#define GM_INT64_C(c) c ## i64    /* signed 64 bit constant */
#define GM_UINT64_C(c) c ## ui64   /* unsigned 64 bit constant */
    typedef __int64 int64;            /* 64 bit signed */
    typedef unsigned __int64 uint64;  /* 64 bit unsigned */
#else
#define GM_INT64_C(c) static_cast<long long>(c ## LL)     /* signed 64 bit constant */
#define GM_UINT64_C(c) static_cast<unsigned long long>(c ## ULL) /* unsigned 64 bit constant */
    typedef long long int64;           /* 64 bit signed */
    typedef unsigned long long uint64; /* 64 bit unsigned */
#endif

    typedef unsigned char uchar;   /* 8 bit unsigned */
    typedef unsigned short ushort;   /* 16 bit unsigned */
    typedef unsigned int uint;   /* 32 bit unsigned */
    typedef unsigned long ulong;   /* 32 bit unsigned */
    typedef long long longlong;   /* 64 bit signed */
    typedef unsigned long long ulonglong;   /* 64 bit unsigned */
#ifndef byte
    typedef unsigned char byte;   /* 8 bit unsigned */
#endif
    typedef unsigned short word;   /* 16 bit unsigned */
    typedef unsigned int dword;   /* 32 bit unsigned */
    typedef void *pvoid;

//1970.1.1 0:0:0:00
//second do not use time_t , _USE_32BIT_TIME_T
    typedef int sepoch_t;
    typedef long long msepoch_t;    //millisecond
    typedef long long usepoch_t;    //microsecond
    typedef long long nsepoch_t;    //nanosecond
    typedef long timems_t;          //time size millisecond
    typedef long long mslong_t;     //time size millisecond
    typedef long long secondlong_t; //time size second

    typedef void (*fn_void_t)();

    typedef void (*fn_void_int_t)(int);

    typedef void (*fn_void_pchar_t)(const char *, int);

    typedef void (*fn_void_tlv_t)(int, const void *, int);

    typedef void (*fn_void_msg_tlv_t)(int, int, const void *, int, void *, void *);

    typedef int (*fn_int_void_t)();

    typedef int (*fn_int_int_t)(int);

    typedef int (*fn_int_pchar_t)(const char *, int);

    typedef int (*fn_int_tlv_t)(int, const void *, int);

    typedef int (*fn_int_object_tlv_t)(void *, int, int, const void *);

    typedef void (*fn_void_queue_msg_tlv_t)(fn_void_msg_tlv_t, int, int, const void *, int, void *, void *,
                                            bool); //bool bHandleOnce  = false

    // ================== Bool ==================
    constexpr bool BOOL_TRUE = true;
    constexpr bool BOOL_FALSE = false;

    // ================== int8 ==================
    constexpr int8 INT8_ZERO = static_cast<int8>(0);
    constexpr int8 INT8_MINUS_ONE = static_cast<int8>(-1);
    constexpr int8 INT8_MIN_VAL = std::numeric_limits<int8>::min();
    constexpr int8 INT8_MAX_VAL = std::numeric_limits<int8>::max();

    constexpr uint8 UINT8_ZERO = static_cast<uint8>(0);
    constexpr uint8 UINT8_MAX_VAL = std::numeric_limits<uint8>::max();

    // ================== int16 ==================
    constexpr int16 INT16_ZERO = static_cast<int16>(0);
    constexpr int16 INT16_MINUS_ONE = static_cast<int16>(-1);
    constexpr int16 INT16_MIN_VAL = std::numeric_limits<int16>::min();
    constexpr int16 INT16_MAX_VAL = std::numeric_limits<int16>::max();

    constexpr uint16 UINT16_ZERO = static_cast<uint16>(0);
    constexpr uint16 UINT16_MAX_VAL = std::numeric_limits<uint16>::max();

    // ================== int32 ==================
    constexpr int32 INT32_ZERO = static_cast<int32>(0);
    constexpr int32 INT32_MINUS_ONE = static_cast<int32>(-1);
    constexpr int32 INT32_MIN_VAL = std::numeric_limits<int32>::min();
    constexpr int32 INT32_MAX_VAL = std::numeric_limits<int32>::max();

    constexpr uint32 UINT32_ZERO = static_cast<uint32>(0);
    constexpr uint32 UINT32_MAX_VAL = std::numeric_limits<uint32>::max();

    // ================== int64 ==================
    constexpr int64 INT64_ZERO = static_cast<int64>(0);
    constexpr int64 INT64_MINUS_ONE = static_cast<int64>(-1);
    constexpr int64 INT64_MIN_VAL = std::numeric_limits<int64>::min();
    constexpr int64 INT64_MAX_VAL = std::numeric_limits<int64>::max();

    constexpr uint64 UINT64_ZERO = static_cast<uint64>(0);
    constexpr uint64 UINT64_MAX_VAL = std::numeric_limits<uint64>::max();

    // ================== long / unsigned long ==================
    // long / unsigned long (Note the platform difference: Windows 64-bit long=32-bit, Linux long=64-bit)
    constexpr long LONG_ZERO = static_cast<long>(0);
    constexpr long LONG_MINUS_ONE = static_cast<long>(-1);
    constexpr long LONG_MIN_VAL = std::numeric_limits<long>::min();
    constexpr long LONG_MAX_VAL = std::numeric_limits<long>::max();

    constexpr ulong ULONG_ZERO = static_cast<ulong>(0);
    constexpr ulong ULONG_MAX_VAL = std::numeric_limits<ulong>::max();

    // ================== float ==================
    constexpr float FLOAT_ZERO = 0.0f;
    constexpr float FLOAT_MIN_VAL = std::numeric_limits<float>::lowest();
    constexpr float FLOAT_MAX_VAL = std::numeric_limits<float>::max();
    constexpr float FLOAT_NaN = std::numeric_limits<float>::quiet_NaN();

    // ================== double ==================
    constexpr double DOUBLE_ZERO = 0.0;
    constexpr double DOUBLE_MIN_VAL = std::numeric_limits<double>::lowest();
    constexpr double DOUBLE_MAX_VAL = std::numeric_limits<double>::max();
    constexpr double DOUBLE_NaN = std::numeric_limits<double>::quiet_NaN();


    // ================== Common character constants ==================
    constexpr char CHAR_SPACE = ' ';
    constexpr char CHAR_TAB = '\t';
    constexpr char CHAR_NEWLINE = '\n';

    // QString / std::string : You can also use it directly
    constexpr const char *STR_NEWLINE = "\n";
    constexpr const char *STR_SPACE = " ";
    constexpr const char *STR_TAB = "\t";

#ifdef _WIN32
    constexpr char CHAR_PATH = '\\';
    constexpr const char *STR_LINEBREAK = "\r\n";
#else
    constexpr char CHAR_PATH = '/';
    constexpr const char *STR_LINEBREAK = "\n";
#endif

    inline const std::map<std::string, std::string> MAP_STRING_EMPTY = {};

    // ================== int to bool ==================
#ifndef    TRUE
#define    TRUE 1
#endif

#ifndef    FALSE
#define    FALSE 0
#endif

}


#endif //CX_CT_X2_C_GLOBAL_H
