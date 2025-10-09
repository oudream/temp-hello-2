#ifndef CX_CT_X2_CXGLOBAL_H
#define CX_CT_X2_CXGLOBAL_H


#include <c_global.h>

// c++17
#include <functional>
#include <optional>
#include <iomanip>
#include <regex>

// IO include
#ifdef _WIN32

#include <winsock2.h>
#include <ws2tcpip.h>
#include <process.h>
#include <io.h>

#elif defined(__PTH__)
#include <pth.h>
#else
#include <pthread.h>
#ifdef __APPLE__
#include <sys/_endian.h>
#else
#include <byteswap.h>
#endif
#endif

// IO data type define
namespace cx
{
#ifdef _WIN32
    typedef DWORD pthread_t;
    typedef CRITICAL_SECTION pthread_mutex_t;
    typedef char *caddr_t;
    typedef HANDLE fd_t;
    typedef SOCKET socket_t;
    typedef PROCESS_INFORMATION pid_os_t;
#elif defined(__PTH__)
    typedef int socket_t;
    typedef int fd_t;
    typedef pid_t pid_os_t;
#define INVALID_SOCKET -1
#define INVALID_HANDLE_VALUE -1
#define pthread_mutex_t pth_mutex_t
#define pthread_cond_t pth_cond_t
#define pthread_t pth_t
#else
#define pthread_t pthread_t
#define pthread_mutex_t pthread_mutex_t
    typedef int socket_t;
    typedef int fd_t;
    typedef pid_t pid_os_t;
#define INVALID_SOCKET -1
#define INVALID_HANDLE_VALUE -1
#endif
}


#define CCXX_STATIC

#if defined(_WIN32) && !defined(CCXX_STATIC)
# if defined(ccxx_EXPORTS)
#  define CCXX_EXPORT __declspec(dllexport)
# else
#  define CCXX_EXPORT __declspec(dllimport)
# endif
#else
# define CCXX_EXPORT
#endif

class CxValueType
{
public:
    enum ValueTypeEnum
    {
        ValueType_Bool,
        ValueType_Int8,
        ValueType_UInt8,
        ValueType_Int16,
        ValueType_UInt16,
        ValueType_Int32,
        ValueType_UInt32,
        ValueType_Int64,
        ValueType_UInt64,
        ValueType_Float,
        ValueType_Double,
        ValueType_String,
        ValueType_Unknown
    };

    // ================= id<T>() =================
    template<typename T>
    static ValueTypeEnum id()
    { return ValueType_Unknown; }

    template<>
    ValueTypeEnum id<bool>()
    { return ValueType_Bool; }

    template<>
    ValueTypeEnum id<cx::int8>()
    { return ValueType_Int8; }

    template<>
    ValueTypeEnum id<cx::uint8>()
    { return ValueType_UInt8; }

    template<>
    ValueTypeEnum id<cx::int16>()
    { return ValueType_Int16; }

    template<>
    ValueTypeEnum id<cx::uint16>()
    { return ValueType_UInt16; }

    template<>
    ValueTypeEnum id<int>()
    { return ValueType_Int32; }

    template<>
    ValueTypeEnum id<cx::uint32>()
    { return ValueType_UInt32; }

    template<>
    ValueTypeEnum id<cx::int64>()
    { return ValueType_Int64; }

    template<>
    ValueTypeEnum id<cx::uint64>()
    { return ValueType_UInt64; }

    template<>
    ValueTypeEnum id<float>()
    { return ValueType_Float; }

    template<>
    ValueTypeEnum id<double>()
    { return ValueType_Double; }

    template<>
    ValueTypeEnum id<std::string>()
    { return ValueType_String; }

    // ================= simpleName<T>() =================
    template<typename T>
    static std::string name()
    { return "unknown"; }

    template<>
    std::string name<bool>()
    { return "bool"; }

    template<>
    std::string name<cx::int8>()
    { return "int8"; }

    template<>
    std::string name<cx::uint8>()
    { return "uint8"; }

    template<>
    std::string name<cx::int16>()
    { return "int16"; }

    template<>
    std::string name<cx::uint16>()
    { return "uint16"; }

    template<>
    std::string name<int>()
    { return "int32"; }

    template<>
    std::string name<cx::uint32>()
    { return "uint32"; }

    template<>
    std::string name<cx::int64>()
    { return "int64"; }

    template<>
    std::string name<cx::uint64>()
    { return "uint64"; }

    template<>
    std::string name<float>()
    { return "float"; }

    template<>
    std::string name<double>()
    { return "double"; }

    template<>
    std::string name<std::string>()
    { return "string"; }

    // ---------------- minValue ----------------
    template<typename T>
    static T minValue();

    template<>
    cx::int8 minValue<cx::int8>()
    { return cx::INT8_MIN_VAL; }

    template<>
    cx::uint8 minValue<cx::uint8>()
    { return 0; }

    template<>
    cx::int16 minValue<cx::int16>()
    { return cx::INT16_MIN_VAL; }

    template<>
    cx::uint16 minValue<cx::uint16>()
    { return 0; }

    template<>
    cx::int32 minValue<cx::int32>()
    { return cx::INT32_MIN_VAL; }

    template<>
    cx::uint32 minValue<cx::uint32>()
    { return 0; }

    template<>
    cx::int64 minValue<cx::int64>()
    { return cx::INT64_MIN_VAL; }

    template<>
    cx::uint64 minValue<cx::uint64>()
    { return 0; }

    template<>
    float minValue<float>()
    { return cx::FLOAT_MIN_VAL; }

    template<>
    double minValue<double>()
    { return cx::DOUBLE_MIN_VAL; }

    template<>
    std::string minValue<std::string>()
    { return ""; }

    // ---------------- maxValue ----------------
    template<typename T>
    static T maxValue();

    template<>
    cx::int8 maxValue<cx::int8>()
    { return cx::INT8_MAX_VAL; }

    template<>
    cx::uint8 maxValue<cx::uint8>()
    { return cx::UINT8_MAX_VAL; }

    template<>
    cx::int16 maxValue<cx::int16>()
    { return cx::INT16_MAX_VAL; }

    template<>
    cx::uint16 maxValue<cx::uint16>()
    { return cx::UINT16_MAX_VAL; }

    template<>
    cx::int32 maxValue<cx::int32>()
    { return cx::INT32_MAX_VAL; }

    template<>
    cx::uint32 maxValue<cx::uint32>()
    { return cx::UINT32_MAX_VAL; }

    template<>
    cx::int64 maxValue<cx::int64>()
    { return cx::INT64_MAX_VAL; }

    template<>
    cx::uint64 maxValue<cx::uint64>()
    { return cx::UINT64_MAX_VAL; }

    template<>
    float maxValue<float>()
    { return cx::FLOAT_MAX_VAL; }

    template<>
    double maxValue<double>()
    { return cx::DOUBLE_MAX_VAL; }

    template<>
    std::string maxValue<std::string>()
    { return ""; }

    // ================= originalValue<T>() =================
    template<typename T>
    static T originalValue()
    { return T(); }

    template<>
    float originalValue<float>()
    { return std::numeric_limits<float>::quiet_NaN(); }

    template<>
    double originalValue<double>()
    { return std::numeric_limits<double>::quiet_NaN(); }

    template<>
    std::string originalValue<std::string>()
    { return ""; }

    // ================= valueTo<T1, T2>(), impl by CxString =================
    template<typename T1, typename T2>
    static T2 valueTo(const T1 &t1, const T2 &defaultT2);
};


#include "cxassert.h"


#endif //CX_CT_X2_CXGLOBAL_H
