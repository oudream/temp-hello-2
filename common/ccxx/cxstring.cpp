#include "cxstring.h"

#include <cinttypes> // for PRId64, PRIu64 (最可移植的 C99/C++11 方式)


using namespace std;

// --------------------------- 内部辅助（不暴露在 .h） ---------------------------

namespace
{
    inline bool _isHexPrefix(const std::string &s)
    {
        return s.size() > 2 && s[0] == '0' && (s[1] == 'x' || s[1] == 'X');
    }

    inline const char *_skipSpaces(const char *p)
    {
        while (*p && std::isspace(static_cast<unsigned char>(*p))) ++p;
        return p;
    }

    inline bool _parsedAll(const char *endptr)
    {
        endptr = _skipSpaces(endptr);
        return *endptr == '\0';
    }

    template<typename T>
    bool _parseInt(const std::string &s, T &out, bool *ok = nullptr)
    {
        const char *p = _skipSpaces(s.c_str());
        char *end = nullptr;
        int base = _isHexPrefix(s) ? 16 : 10;

        if (std::numeric_limits<T>::is_signed)
        {
            long long v = strtoll(p, &end, base);
            bool success = (end != p) && _parsedAll(end);

            if (success && (v < std::numeric_limits<T>::min() || v > std::numeric_limits<T>::max()))
            {
                success = false;
            }

            if (ok) *ok = success;
            if (!success) return false;
            out = static_cast<T>(v);
            return true;
        }
        else
        {
            unsigned long long v = strtoull(p, &end, base);
            bool success = (end != p) && _parsedAll(end);

            if (success && (v > (unsigned long long) std::numeric_limits<T>::max()))
            {
                success = false;
            }

            if (ok) *ok = success;
            if (!success) return false;
            out = static_cast<T>(v);
            return true;
        }
    }

    template<typename T>
    inline bool _parseDouble(const std::string &s, T &out, bool *ok = nullptr)
    {
        const char *p = _skipSpaces(s.c_str());
        char *end = nullptr;
        double v = strtod(p, &end);
        bool success = (end != p) && _parsedAll(end);

        if (success && std::isnan(v)) success = false;
        auto lo = std::numeric_limits<T>::lowest();
        auto hi = std::numeric_limits<T>::max();
        if (success && (v < static_cast<double>(lo) || v > static_cast<double>(hi)))
            success = false;
        if (ok) *ok = success;
        if (!success) return false;
        out = static_cast<T>(v);
        return true;
    }

// toString(32-bit) 走 snprintf；64-bit 走 ostringstream（跨平台安全）
    template<typename TInt>
    inline std::string _toStringInt32(TInt v, const char *fmt)
    {
        char buf[32];
        int n = std::snprintf(buf, sizeof(buf), fmt, v);
        return (n > 0 && n < (int) sizeof(buf)) ? std::string(buf, (size_t) n) : std::string();
    }

// HEX：将单字节转为两字符
    inline void _byteToHex(unsigned char b, char *twoChars /*size>=2*/)
    {
        static const char HEX[] = "0123456789ABCDEF";
        twoChars[0] = HEX[(b >> 4) & 0xF];
        twoChars[1] = HEX[b & 0xF];
    }

// 任意标量（整型）转等宽 HEX（高位在前，宽度=sizeof(T)*2）
    template<typename T>
    std::string _toHexScalar(T value)
    {
        typedef typename std::make_unsigned<T>::type U;
        U u = static_cast<U>(value);
        std::string out(sizeof(T) * 2, '0');
        char *p = &out[0];
        for (int i = (int) out.size() - 1; i >= 0; --i)
        {
            static const char HEX[] = "0123456789ABCDEF";
            p[i] = HEX[u & 0xF];
            u >>= 4;
        }
        return out;
    }

// HEX 字符串解析：允许空格、可含“0x”，忽略非十六进制字符
    template<typename TByte=unsigned char>
    std::vector<TByte> _fromHexString(const std::string &s)
    {
        std::vector<TByte> r;
        if (s.empty()) return r;

        auto hexVal = [](char ch) -> int
        {
            if (ch >= '0' && ch <= '9') return ch - '0';
            if (ch >= 'a' && ch <= 'f') return ch - 'a' + 10;
            if (ch >= 'A' && ch <= 'F') return ch - 'A' + 10;
            return -1;
        };

        const char *p = _skipSpaces(s.c_str());
        if (p[0] == '0' && (p[1] == 'x' || p[1] == 'X')) p += 2; // 跳过 0x

        r.reserve((s.size() + 1) / 2);
        TByte cur = 0;
        bool high = true; // 正在填写高 4 位？

        while (*p)
        {
            if (std::isspace(static_cast<unsigned char>(*p)))
            {
                ++p; // 允许并跳过内部空格
                continue;
            }

            const int hv = hexVal(*p);
            if (hv < 0) return {}; // 【修正】严格模式：遇到无效字符，立即失败

            if (high)
            {
                cur = (TByte) (hv << 4);
                high = false;
            }
            else
            {
                cur |= (TByte) hv;
                r.push_back(cur);
                high = true;
            }
            ++p;
        }

        // 【修正】严格模式：如果是奇数个 hex 字符，解析失败
        if (!high) return {};

        return r;
    }

    inline std::string _trim(const std::string &s, const std::string &delims = " \t\r\n")
    {
        if (s.empty()) return s;
        size_t i = 0, j = s.size();
        while (i < j && delims.find(s[i]) != std::string::npos) ++i;
        while (j > i && delims.find(s[j - 1]) != std::string::npos) --j;
        return s.substr(i, j - i);
    }

    inline void _trim_inplace(std::string &s, const std::string &delims = " \t\r\n")
    {
        if (s.empty()) return;
        size_t i = 0, j = s.size();
        while (i < j && delims.find(s[i]) != std::string::npos) ++i;
        while (j > i && delims.find(s[j - 1]) != std::string::npos) --j;
        if (i > 0 || j < s.size()) s.assign(s, i, j - i);
    }

    inline std::string _lower(const std::string &s)
    {
        std::string t = s;
        for (char &c: t) c = (char) std::tolower((unsigned char) c);
        return t;
    }

    inline void _lower_inplace(std::string &s)
    {
        auto *p = reinterpret_cast<unsigned char *>(&s[0]);
        unsigned char *end = p + s.size();
        while (p < end)
        {
            unsigned char c = *p;
            if (c >= 'A' && c <= 'Z')
                *p = (unsigned char) (c + 32);
            ++p;
        }
    }

    inline void _upper_inplace(std::string &s)
    {
        auto *p = reinterpret_cast<unsigned char*>(&s[0]);
        unsigned char *end = p + s.size();
        while (p < end)
        {
            unsigned char c = *p;
            if (c >= 'a' && c <= 'z')
                *p = (unsigned char)(c - 32);
            ++p;
        }
    }

    // -------- equal（支持忽略顺序/大小写/trim）--------
    inline std::string _norm_ascii(std::string s, bool icase, bool trim)
    {
        if (trim) s = CxString::trim(s);
        if (icase) s = CxString::toLowerAscii(s);
        return s;
    }

    inline bool ascii_is_hex_digit(unsigned char c)
    {
        return (c >= '0' && c <= '9') ||
               (c >= 'a' && c <= 'f') ||
               (c >= 'A' && c <= 'F');
    }

    inline bool _isLittleEndian()
    {
        static const std::uint32_t i = 0x01;
        return *reinterpret_cast<const std::uint8_t *>(&i) == 0x01;
    }

    template<typename T>
    std::string _toHexPortable(T v)
    {
        unsigned char buf[sizeof(T)];
        std::memcpy(buf, &v, sizeof(T));

        // 【修正】统一转换为大端字节序 (Network Order)
        if (_isLittleEndian())
        {
            std::reverse(buf, buf + sizeof(T));
        }

        return CxString::toHexString(reinterpret_cast<const char *>(buf), (int) sizeof(T), false);
    }

    // 检查字符串是否是 T 类型 (float/double) 的合法实数字面量：
    // 语法正确 + 有限 + 范围内 + 精度内（包括小数）
    template<typename T>
    inline bool _isValidRealLiteralFor(const std::string& s)
    {
        const char* p = _skipSpaces(s.c_str());
        char* end = nullptr;
        long double v = strtold(p, &end);

        // 基础语法检查
        if ((end == p) || !_parsedAll(end) || !std::isfinite((double)v))
            return false;

        // 范围检查
        const auto lo = static_cast<long double>(std::numeric_limits<T>::lowest());
        const auto hi = static_cast<long double>(std::numeric_limits<T>::max());
        if (v < lo || v > hi)
            return false;

        // 有效数字检查（包括整数和小数部分）
        int digits_count = 0;
        bool in_exponent = false;
        bool has_digit = false;

        for (const char* q = p; q < end; ++q) {
            if (*q == 'e' || *q == 'E') {
                in_exponent = true;
                continue;
            }
            if (in_exponent)
                continue; // 跳过指数部分
            if (isdigit((unsigned char)*q)) {
                if (!(digits_count == 0 && *q == '0')) // 忽略前导0
                    ++digits_count;
                has_digit = true;
            }
        }

        if (!has_digit)
            return false;

        // 可保证的十进制有效数字数
        const int max_digits = std::numeric_limits<T>::digits10;

        // 容忍2位（因为 digits10 是“保证精度”，实际略高）
        if (digits_count > max_digits + 2)
            return false;

        return true;
    }

} // namespace

// --------------------------- toString ---------------------------

std::string CxString::toString(const bool &b)
{
    return b ? "1" : "0";
}

std::string CxString::toString(const int &i)
{
    return _toStringInt32<int>(i, "%d");
}

std::string CxString::toString(const cx::uint32 &n)
{
    return _toStringInt32<cx::uint32>(n, "%u");
}

std::string CxString::toString(const long &n)
{
    char buf[32]; // long 在 64 位系统上是 64-bit
    int len = std::snprintf(buf, sizeof(buf), "%ld", n);
    return (len > 0 && len < (int) sizeof(buf)) ? std::string(buf, (size_t) len) : std::string();
}

std::string CxString::toString(const cx::ulong &n)
{
    char buf[32];
    int len = std::snprintf(buf, sizeof(buf), "%lu", n);
    return (len > 0 && len < (int) sizeof(buf)) ? std::string(buf, (size_t) len) : std::string();
}

std::string CxString::toString(const cx::int64 &n)
{
    char buf[32]; // 2^63-1 is 19 digits + sign
    int len = std::snprintf(buf, sizeof(buf), "%" PRId64, n);
    return (len > 0 && len < (int) sizeof(buf)) ? std::string(buf, (size_t) len) : std::string();
}

std::string CxString::toString(const cx::uint64 &n)
{
    char buf[32]; // 2^64-1 is 20 digits
    int len = std::snprintf(buf, sizeof(buf), "%" PRIu64, n);
    return (len > 0 && len < (int) sizeof(buf)) ? std::string(buf, (size_t) len) : std::string();
}

std::string CxString::toString(const float &f)
{
    std::ostringstream ss;
    ss.precision(6);
    ss << f;
    return ss.str();
}

std::string CxString::toString(const double &d)
{
    std::ostringstream ss;
    ss.precision(15);
    ss << d;
    return ss.str();
}

std::string CxString::toString(const std::string &s)
{
    return s;
}

// --------------------------- toXxx（严格解析） ---------------------------

bool CxString::toBoolean(const std::string &sSource)
{
    std::string t = sSource;
    // trim + 小写
    _trim_inplace(t);
    for (char &c: t) c = (char) std::tolower((unsigned char) c);
    if (t == "true" || t == "yes" || t == "on" || t == "ok") return true;
    if (t == "false" || t == "no" || t == "off" || t == "ng") return false;
    long long iv = 0;
    bool ok = false;
    _parseInt<long long>(t, iv, &ok);
    return ok && (iv != 0);
}

int CxString::toInt32(const std::string &sSource)
{
    int v = 0;
    _parseInt<int>(sSource, v, nullptr);
    return v;
}

cx::uint CxString::toUint32(const std::string &sSource)
{
    cx::uint v = 0;
    _parseInt<cx::uint>(sSource, v, nullptr);
    return v;
}

cx::int64 CxString::toInt64(const std::string &sSource)
{
    cx::int64 v = 0;
    _parseInt<cx::int64>(sSource, v, nullptr);
    return v;
}

cx::uint64 CxString::toUint64(const std::string &sSource)
{
    cx::uint64 v = 0;
    _parseInt<cx::uint64>(sSource, v, nullptr);
    return v;
}

double CxString::toDouble(const std::string &sSource)
{
    double v = 0;
    _parseDouble(sSource, v, nullptr);
    return v;
}

float CxString::toFloat(const std::string &sSource)
{
    float v = 0;
    _parseDouble(sSource, v, nullptr);
    return v;
}

bool CxString::toBoolean(const std::string &sSource, bool *bOk)
{
    std::string t = sSource;
    _trim_inplace(t);
    for (char &c: t) c = (char) std::tolower((unsigned char) c);

    if (t == "true" || t == "yes" || t == "on")
    {
        if (bOk) *bOk = true;
        return true;
    }
    if (t == "false" || t == "no" || t == "off")
    {
        if (bOk) *bOk = true;
        return false;
    }

    long long iv = 0;
    bool ok = false;
    _parseInt<long long>(t, iv, &ok);
    if (bOk) *bOk = ok;
    return ok && (iv != 0);
}

int CxString::toInt32(const std::string &sSource, bool *bOk)
{
    int v = 0;
    bool ok = _parseInt<int>(sSource, v, bOk);
    if (bOk) *bOk = ok;
    return v;
}

cx::uint CxString::toUint32(const std::string &sSource, bool *bOk)
{
    cx::uint v = 0;
    bool ok = _parseInt<cx::uint>(sSource, v, bOk);
    if (bOk) *bOk = ok;
    return v;
}

cx::int64 CxString::toInt64(const std::string &sSource, bool *bOk)
{
    cx::int64 v = 0;
    bool ok = _parseInt<cx::int64>(sSource, v, bOk);
    if (bOk) *bOk = ok;
    return v;
}

cx::uint64 CxString::toUint64(const std::string &sSource, bool *bOk)
{
    cx::uint64 v = 0;
    bool ok = _parseInt<cx::uint64>(sSource, v, bOk);
    if (bOk) *bOk = ok;
    return v;
}

double CxString::toDouble(const std::string &sSource, bool *bOk)
{
    double v = 0;
    bool ok = _parseDouble(sSource, v, bOk);
    if (bOk) *bOk = ok;
    return v;
}

float CxString::toFloat(const std::string &sSource, bool *bOk)
{
    float v = 0;
    bool ok = _parseDouble(sSource, v, bOk);
    if (bOk) *bOk = ok;
    return v;
}

// --------------------------- fromString / setFromString ---------------------------

bool CxString::fromString(const std::string &sSource, const bool &b, bool *bOk)
{
    bool ok = false;
    bool v = toBoolean(sSource, &ok);
    if (bOk) *bOk = ok;
    return ok ? v : b;
}

cx::int8 CxString::fromString(const std::string &sSource, const cx::int8 &i, bool *bOk)
{
    int v = 0;
    bool ok = _parseInt<int>(sSource, v, bOk);
    if (bOk)*bOk = ok;
    return ok ? static_cast<cx::int8>(v) : i;
}

cx::uint8 CxString::fromString(const std::string &sSource, const cx::uint8 &i, bool *bOk)
{
    cx::uint v = 0;
    bool ok = _parseInt<cx::uint>(sSource, v, bOk);
    if (bOk)*bOk = ok;
    return ok ? static_cast<cx::uint8>(v) : i;
}

cx::int16 CxString::fromString(const std::string &sSource, const cx::int16 &i, bool *bOk)
{
    int v = 0;
    bool ok = _parseInt<int>(sSource, v, bOk);
    if (bOk)*bOk = ok;
    return ok ? static_cast<cx::int16>(v) : i;
}

cx::uint16 CxString::fromString(const std::string &sSource, const cx::uint16 &i, bool *bOk)
{
    cx::uint v = 0;
    bool ok = _parseInt<cx::uint>(sSource, v, bOk);
    if (bOk)*bOk = ok;
    return ok ? static_cast<cx::uint16>(v) : i;
}

int CxString::fromString(const std::string &sSource, const int &i, bool *bOk)
{
    int v = 0;
    bool ok = _parseInt<int>(sSource, v, bOk);
    if (bOk)*bOk = ok;
    return ok ? v : i;
}

cx::uint32 CxString::fromString(const std::string &sSource, const cx::uint32 &n, bool *bOk)
{
    cx::uint32 v = 0;
    bool ok = _parseInt<cx::uint32>(sSource, v, bOk);
    if (bOk)*bOk = ok;
    return ok ? v : n;
}

cx::int64 CxString::fromString(const std::string &sSource, const cx::int64 &n, bool *bOk)
{
    cx::int64 v = 0;
    bool ok = _parseInt<cx::int64>(sSource, v, bOk);
    if (bOk)*bOk = ok;
    return ok ? v : n;
}

cx::uint64 CxString::fromString(const std::string &sSource, const cx::uint64 &n, bool *bOk)
{
    cx::uint64 v = 0;
    bool ok = _parseInt<cx::uint64>(sSource, v, bOk);
    if (bOk)*bOk = ok;
    return ok ? v : n;
}

double CxString::fromString(const std::string &sSource, const double &d, bool *bOk)
{
    double v = 0;
    bool ok = _parseDouble(sSource, v, bOk);
    if (bOk)*bOk = ok;
    return ok ? v : d;
}

float CxString::fromString(const std::string &sSource, const float &f, bool *bOk)
{
    float v = 0;
    bool ok = _parseDouble(sSource, v, bOk);
    if (bOk)*bOk = ok;
    return ok ? v : f;
}

std::string CxString::fromString(const std::string &sSource, const std::string &s, bool *bOk)
{
    if (bOk) *bOk = true;
    return sSource;
}

bool CxString::setFromString(const std::string &sSource, bool &b)
{
    bool ok = false;
    bool v = toBoolean(sSource, &ok);
    if (ok) b = v;
    return ok;
}

bool CxString::setFromString(const std::string &sSource, int &i)
{
    bool ok = false;
    int v = 0;
    ok = _parseInt<int>(sSource, v, &ok);
    if (ok) i = v;
    return ok;
}

bool CxString::setFromString(const std::string &sSource, cx::uint32 &n)
{
    bool ok = false;
    cx::uint32 v = 0;
    ok = _parseInt<cx::uint32>(sSource, v, &ok);
    if (ok) n = v;
    return ok;
}

bool CxString::setFromString(const std::string &sSource, cx::int64 &n)
{
    bool ok = false;
    cx::int64 v = 0;
    ok = _parseInt<cx::int64>(sSource, v, &ok);
    if (ok) n = v;
    return ok;
}

bool CxString::setFromString(const std::string &sSource, cx::uint64 &n)
{
    bool ok = false;
    cx::uint64 v = 0;
    ok = _parseInt<cx::uint64>(sSource, v, &ok);
    if (ok) n = v;
    return ok;
}

bool CxString::setFromString(const std::string &sSource, double &d)
{
    bool ok = false;
    double v = 0;
    ok = _parseDouble(sSource, v, &ok);
    if (ok) d = v;
    return ok;
}

bool CxString::setFromString(const std::string &sSource, float &f)
{
    bool ok = false;
    float v = 0;
    ok = _parseDouble(sSource, v, &ok);
    if (ok) f = v;
    return ok;
}

bool CxString::setFromString(const std::string &sSource, std::string &s)
{
    s = sSource;
    return true;
}

// --------------------------- isvalid*（严格：完整解析） ---------------------------

bool CxString::isvalidBoolean(const std::string &sSource)
{
    bool ok = false;
    (void) toBoolean(sSource, &ok);
    return ok;
}

bool CxString::isvalidInteger(const std::string &sSource)
{
    bool ok = false;
    (void) toInt32(sSource, &ok);
    return ok;
}

bool CxString::isvalidLong(const std::string &sSource)
{
    bool ok = false;
    (void) toInt64(sSource, &ok);
    return ok;
}

bool CxString::isvalidFloat(const std::string &sSource)
{
    return _isValidRealLiteralFor<float>(sSource);
}

bool CxString::isvalidDouble(const std::string &sSource)
{
    return _isValidRealLiteralFor<double>(sSource);
}

// --------------------------- HEX ---------------------------

std::string CxString::toHexString(const char *pData, int iLength, bool bHasEmptyChar)
{
    if (!pData || iLength <= 0) return {};
    if (bHasEmptyChar)
    {
        std::string out(iLength * 3, '\0'); // "AA " × N
        char *q = &out[0];
        for (int i = 0; i < iLength; ++i)
        {
            _byteToHex(static_cast<unsigned char>(pData[i]), q);
            q += 2;
            *q++ = ' ';
        }
        if (!out.empty()) out.pop_back(); // 去掉最后一个空格
        return out;
    }
    else
    {
        std::string out(iLength * 2, '\0');
        char *q = &out[0];
        for (int i = 0; i < iLength; ++i)
        {
            _byteToHex(static_cast<unsigned char>(pData[i]), q);
            q += 2;
        }
        return out;
    }
}

std::string CxString::toHexString(const cx::uchar *pData, int iLength, bool bHasEmptyChar)
{
    return pData ? toHexString(reinterpret_cast<const char *>(pData), iLength, bHasEmptyChar) : std::string();
}

std::string CxString::toHexString(const std::vector<char> &data, bool bHasEmptyChar)
{
    return data.empty() ? std::string() : toHexString(&data.front(), (int) data.size(), bHasEmptyChar);
}

std::string CxString::toHexString(const std::vector<cx::uchar> &data, bool bHasEmptyChar)
{
    return data.empty() ? std::string() : toHexString(reinterpret_cast<const char *>(&data.front()), (int) data.size(), bHasEmptyChar);
}

std::string CxString::toHexString(const char &v)
{
    return _toHexScalar<char>(v);
}

std::string CxString::toHexString(const cx::uchar &v)
{
    return _toHexScalar<cx::uchar>(v);
}

std::string CxString::toHexString(const short &v)
{
    return _toHexScalar<short>(v);
}

std::string CxString::toHexString(const cx::ushort &v)
{
    return _toHexScalar<cx::ushort>(v);
}

std::string CxString::toHexString(const int &v)
{
    return _toHexScalar<int>(v);
}

std::string CxString::toHexString(const cx::uint32 &v)
{
    return _toHexScalar<cx::uint32>(v);
}

std::string CxString::toHexString(const cx::int64 &v)
{
    return _toHexScalar<cx::int64>(v);
}

std::string CxString::toHexString(const cx::uint64 &v)
{
    return _toHexScalar<cx::uint64>(v);
}

std::string CxString::toHexString(const float &v)
{
    static_assert(sizeof(float) == 4, "float must be 4 bytes");
    return _toHexPortable(v);
}

std::string CxString::toHexString(const double &v)
{
    static_assert(sizeof(double) == 8, "double must be 8 bytes");
    return _toHexPortable(v);
}

std::string CxString::toHexString(const int &v, bool bHasHead)
{
    std::string s = _toHexScalar<int>(v);
    return bHasHead ? std::string("0x") + s : s;
}

std::string CxString::toHexString(const cx::uint32 &v, bool bHasHead)
{
    std::string s = _toHexScalar<cx::uint32>(v);
    return bHasHead ? std::string("0x") + s : s;
}

std::vector<cx::uchar> CxString::fromHexString(const std::string &s)
{
    return _fromHexString<cx::uchar>(s);
}

std::vector<char> CxString::hexToData(const std::string &s)
{
    return _fromHexString<char>(s);
}

int CxString::hexToInt32(const std::string &s, bool *bOk)
{
    // 允许 "0x"；完整解析
    std::string t = s;
    _trim_inplace(t);
    if (_isHexPrefix(t)) t = t.substr(2);
    char *end = nullptr;
    long v = strtol(t.c_str(), &end, 16);
    bool ok = (end != t.c_str()) && _parsedAll(end);
    if (bOk) *bOk = ok;
    return ok ? (int) v : 0;
}

cx::uint CxString::hexToUint32(const std::string &s, bool *bOk)
{
    std::string t = s;
    _trim_inplace(t);
    if (_isHexPrefix(t)) t = t.substr(2);
    char *end = nullptr;
    unsigned long v = strtoul(t.c_str(), &end, 16);
    bool ok = (end != t.c_str()) && _parsedAll(end);
    if (bOk) *bOk = ok;
    return ok ? (cx::uint) v : 0u;
}

cx::int64 CxString::hexToInt64(const std::string &s, bool *bOk)
{
    std::string t = s;
    _trim_inplace(t);
    if (_isHexPrefix(t)) t = t.substr(2);
    char *end = nullptr;
    long long v = strtoll(t.c_str(), &end, 16);
    bool ok = (end != t.c_str()) && _parsedAll(end);
    if (bOk) *bOk = ok;
    return ok ? (cx::int64) v : 0;
}

cx::uint64 CxString::hexToUint64(const std::string &s, bool *bOk)
{
    std::string t = s;
    _trim_inplace(t);
    if (_isHexPrefix(t)) t = t.substr(2);
    char *end = nullptr;
    unsigned long long v = strtoull(t.c_str(), &end, 16);
    bool ok = (end != t.c_str()) && _parsedAll(end);
    if (bOk) *bOk = ok;
    return ok ? (cx::uint64) v : 0ull;
}

// --------------------------- 其它字符串工具 ---------------------------

std::string CxString::insertSplit(const std::string &s, char cSplit, int iStep)
{
    if (s.empty() || iStep <= 0) return s;
    std::string out;
    out.reserve(s.size() + s.size() / iStep);
    for (size_t i = 0; i < s.size(); ++i)
    {
        out.push_back(s[i]);
        if ((i + 1) % iStep == 0 && (i + 1) < s.size()) out.push_back(cSplit);
    }
    return out;
}

void CxString::remove(std::string &s, char c)
{
    s.erase(std::remove(s.begin(), s.end(), c), s.end());
}

int CxString::count(const std::string &ss, const char cCharacter)
{
    return (int) std::count(ss.begin(), ss.end(), cCharacter);
}

int CxString::count(const std::string &ss, const std::string &sSub)
{
    if (ss.empty() || sSub.empty()) return 0;
    int cnt = 0;
    size_t pos = 0;
    while ((pos = ss.find(sSub, pos)) != std::string::npos)
    {
        ++cnt;
        pos += sSub.size();
    }
    return cnt;
}

std::vector<std::string> CxString::split(const std::string &ss, char cSplitCharacter, bool bHasEmptyString)
{
    std::vector<std::string> list;
    size_t start = 0, end;
    while ((end = ss.find(cSplitCharacter, start)) != std::string::npos)
    {
        if (start != end) list.push_back(ss.substr(start, end - start));
        else if (bHasEmptyString) list.emplace_back();
        start = end + 1;
    }
    if (start != ss.size()) list.push_back(ss.substr(start));
    return list;
}

std::vector<std::string> CxString::splitByDelimiters(const std::string &ss, const std::string &sDelimiters, bool bHasEmptyString)
{
    std::vector<std::string> r;
    size_t start = 0, end;
    while ((end = ss.find_first_of(sDelimiters, start)) != std::string::npos)
    {
        if (start != end) r.push_back(ss.substr(start, end - start));
        else if (bHasEmptyString) r.emplace_back();
        start = end + 1;
    }
    if (start != ss.size()) r.push_back(ss.substr(start));
    return r;
}

std::vector<std::string> CxString::split(const std::string &ss, const std::string &sSplitString, bool bHasEmptyString)
{
    std::vector<std::string> list;
    if (sSplitString.empty())
    {
        list.push_back(ss);
        return list;
    }
    size_t start = 0, end;
    while ((end = ss.find(sSplitString, start)) != std::string::npos)
    {
        if (start != end) list.push_back(ss.substr(start, end - start));
        else if (bHasEmptyString) list.emplace_back();
        start = end + sSplitString.size();
    }
    if (start != ss.size()) list.push_back(ss.substr(start));
    return list;
}

std::vector<std::string> CxString::split(const std::vector<std::string> &ss, const std::string &sSplitString, bool bHasEmptyString)
{
    size_t total = 0;
    for (const auto &s: ss) total += s.size();
    std::string joined;
    joined.reserve(total);
    for (const auto &s: ss) joined.append(s);
    return split(joined, sSplitString, bHasEmptyString);
}

std::vector<std::string> CxString::splitCase(const std::string &ss, const std::string &sSplitString, bool bHasEmptyString)
{
    return split(_lower(ss), _lower(sSplitString), bHasEmptyString);
}

std::vector<int> CxString::splitCaseToInt(const std::string &sVal, const std::string &split)
{
    std::vector<int> out;
    if (sVal.empty()) return out;

    std::vector<std::string> v = splitCase(sVal, split);
    out.reserve(v.size());
    for (const auto &s: v)
    {
        out.push_back(CxString::toInt32(s));
    }
    return out;
}

std::vector<double> CxString::splitCaseToDouble(const std::string &sVal, const std::string &split)
{
    std::vector<double> out;
    if (sVal.empty()) return out;

    std::vector<std::string> v = splitCase(sVal, split);
    out.reserve(v.size());
    for (const auto &s: v)
    {
        out.push_back(CxString::toDouble(s));
    }
    return out;
}

void CxString::splitCase(std::vector<std::vector<char> > &v, std::string &sVal, std::string &split)
{
    if (sVal.empty()) return;
    std::vector<std::string> v1 = splitCase(sVal, split);
    for (const auto &i: v1)
    {
        std::vector<char> v2 = hexToData(i);
        if (!v2.empty()) v.push_back(v2);
    }
}

std::map<std::string, std::string> CxString::splitToMap(const std::vector<std::string> &ss, char cMid, bool bTrim)
{
    std::map<std::string, std::string> r;
    for (const auto &sContent: ss)
    {
        size_t pos = sContent.find(cMid);
        if (pos != std::string::npos)
        {
            std::string k = sContent.substr(0, pos);
            std::string v = sContent.substr(pos + 1);
            if (bTrim)
            {
                _trim_inplace(k);
                _trim_inplace(v);
            }
            if (!k.empty()) r[k] = v;
        }
        else
        {
            std::string k = sContent;
            if (bTrim)
            {
                _trim_inplace(k);
            }
            r[k] = std::string();
        }
    }
    return r;
}

template<typename MapType=std::map<std::string, std::string>>
MapType splitToMap_(const std::string &s, char kvSep, char pairSep, bool bTrim, bool keepEmptyPair)
{
    MapType r;
    size_t start = 0;
    while (start <= s.size())
    {
        size_t end = s.find(pairSep, start);
        std::string pair = (end == std::string::npos) ? s.substr(start) : s.substr(start, end - start);
        if (!pair.empty() || keepEmptyPair)
        {
            size_t mid = pair.find(kvSep);
            std::string k, v;
            if (mid == std::string::npos)
            {
                k = pair;
                v.clear();
            }
            else
            {
                k = pair.substr(0, mid);
                v = pair.substr(mid + 1);
            }
            if (bTrim)
            {
                _trim_inplace(k);
                _trim_inplace(v);
            }
            if (!k.empty() || keepEmptyPair) r[k] = v;
        }
        if (end == std::string::npos) break;
        start = end + 1;
    }
    return r;
}

std::map<std::string, std::string> CxString::splitToMap(const std::string &s, char kvSep, char pairSep, bool bTrim, bool keepEmptyPair)
{
    return splitToMap_<std::map<std::string, std::string>>(s, kvSep, pairSep, bTrim, keepEmptyPair);
}

std::unordered_map<std::string, std::string> CxString::splitToUMap(const std::string &s, char kvSep, char pairSep, bool bTrim, bool keepEmptyPair)
{
    return splitToMap_<std::unordered_map<std::string, std::string>>(s, kvSep, pairSep, bTrim, keepEmptyPair);
}

std::map<std::string, std::string> CxString::splitToMap_mix(const std::string &ss, char cMid, char cSplitCharacter)
{
    std::map<std::string, std::string> r;
    auto items = split(ss, cSplitCharacter, false);
    for (auto &it: items)
    {
        size_t pos = it.find(cMid);
        if (pos != std::string::npos)
        {
            r[it.substr(0, pos)] = it.substr(pos + 1);
        }
        else
        {
            _trim_inplace(it);
            r[it] = std::string();
        }
    }
    return r;
}

std::map<std::string, std::string> CxString::splitToMap_reverse(const std::string &ss, char cMid, char cSplitCharacter, bool bTrim)
{
    std::map<std::string, std::string> r;
    auto items = split(ss, cSplitCharacter, false);
    for (auto &it: items)
    {
        size_t pos = it.find(cMid);
        if (pos != std::string::npos)
        {
            std::string v = it.substr(0, pos);
            std::string k = it.substr(pos + 1);
            if (bTrim)
            {
                _trim_inplace(k);
                _trim_inplace(v);
            }
            r[k] = v;
        }
    }
    return r;
}

std::vector<std::vector<std::string> > CxString::splitToLines(const std::string &ss, char cMid, char cSplitCharacter)
{
    std::vector<std::vector<std::string>> lines;
    auto items = split(ss, cSplitCharacter, false);
    lines.reserve(items.size());
    for (auto &it: items)
    {
        auto kv = split(it, cMid, true);
        lines.push_back(kv);
    }
    return lines;
}

std::vector<std::map<std::string, std::string> > CxString::splitToLines(const std::vector<std::string> &ss, char cMid, char cSplitCharacter)
{
    std::vector<std::map<std::string, std::string>> r;
    r.reserve(ss.size());
    for (auto &line: ss) r.push_back(splitToMap(line, cMid, cSplitCharacter, false));
    return r;
}

std::vector<std::map<std::string, std::string> > CxString::sortToLines(const std::map<std::string, std::map<std::string, std::string> > &ss)
{
    std::vector<std::map<std::string, std::string>> r;
    r.reserve(ss.size());
    for (auto &kv: ss) r.push_back(kv.second);
    return r;
}

std::string CxString::join(const std::vector<std::string> &ss, char cJoin)
{
    if (ss.empty()) return {};
    size_t total = ss.size() - 1; // joins
    for (const auto &s: ss) total += s.size();
    std::string out;
    out.reserve(total);
    for (size_t i = 0; i < ss.size(); ++i)
    {
        out += ss[i];
        if (i + 1 < ss.size()) out.push_back(cJoin);
    }
    return out;
}

std::string CxString::join(const std::vector<std::string> &ss, const std::string &sJoin)
{
    if (ss.empty()) return {};
    size_t total = (ss.size() - 1) * sJoin.size();
    for (const auto &s: ss) total += s.size();
    std::string out;
    out.reserve(total);
    for (size_t i = 0; i < ss.size(); ++i)
    {
        out += ss[i];
        if (i + 1 < ss.size()) out += sJoin;
    }
    return out;
}

std::string CxString::join(const std::map<std::string, std::string> &pairs, char cMid, char cSplit)
{
    std::vector<std::string> items;
    items.reserve(pairs.size());
    for (auto &kv: pairs)
    {
        std::string item;
        item.reserve(kv.first.size() + kv.second.size() + 1);
        item += kv.first;
        item.push_back(cMid);
        item += kv.second;
        items.push_back(std::move(item));
    }
    return join(items, cSplit);
}

std::string CxString::join(const std::map<std::string, std::string> &pairs, const std::string &sMid, const std::string &sSplit)
{
    std::vector<std::string> items;
    items.reserve(pairs.size());
    for (auto &kv: pairs) items.push_back(kv.first + sMid + kv.second);
    return join(items, sSplit);
}

std::vector<std::string> CxString::joinToStringList(const std::map<std::string, std::string> &pairs, const std::string &sMid)
{
    std::vector<std::string> items;
    items.reserve(pairs.size());
    for (auto &kv: pairs) items.push_back(kv.first + sMid + kv.second);
    return items;
}

std::string CxString::join(const std::vector<std::vector<std::string> > &lines, char cMid, char cSplitCharacter)
{
    std::vector<std::string> items;
    items.reserve(lines.size());
    for (auto &ln: lines) items.push_back(join(ln, cMid));
    return join(items, cSplitCharacter);
}

std::string CxString::replace(const std::string &strBase, char cSrc, char cDes)
{
    std::string r = strBase;
    std::replace(r.begin(), r.end(), cSrc, cDes);
    return r;
}

std::string CxString::replace(const std::string &strBase, const std::string &strSrc, const std::string &strDes)
{
    if (strSrc.empty()) return strBase;
    std::string r = strBase;
    size_t pos = 0;
    while ((pos = r.find(strSrc, pos)) != std::string::npos)
    {
        r.replace(pos, strSrc.size(), strDes);
        pos += strDes.size();
    }
    return r;
}

std::string CxString::replaceCase(const std::string &strBase, const std::string &strSrc, const std::string &strDes)
{
    if (strSrc.empty()) return strBase;

    std::string lowerBase = _lower(strBase);
    std::string lowerSrc = _lower(strSrc);
    std::string result;
    result.reserve(strBase.size());

    size_t lastPos = 0;
    size_t pos;

    while ((pos = lowerBase.find(lowerSrc, lastPos)) != std::string::npos)
    {
        result.append(strBase, lastPos, pos - lastPos);
        result.append(strDes);
        lastPos = pos + strSrc.size();
    }
    result.append(strBase, lastPos, std::string::npos);

    return result;
}

std::string CxString::trim(const std::string &s)
{
    return _trim(s);
}

std::string CxString::trim(const std::string &s, const char cDelete)
{
    std::string delims(1, cDelete);
    return _trim(s, delims);
}

std::string CxString::trim(const std::string &s, const std::string &sDelete)
{
    return _trim(s, sDelete);
}

void CxString::trimInplace(string &s)
{
    _trim_inplace(s);
}

void CxString::trimInplace(string &s, char cDelete)
{
    std::string delims(1, cDelete);
    _trim_inplace(s, delims);
}

void CxString::trimInplace(string &s, const string &sDelete)
{
    _trim_inplace(s, sDelete);
}

std::string CxString::erase(const std::string &src, char cDelete)
{
    std::string r = src;
    r.erase(std::remove(r.begin(), r.end(), cDelete), r.end());
    return r;
}

void CxString::toLowerAsciiSelf(string &r)
{
    _lower_inplace(r);
}

string CxString::toLowerAscii(const string &s)
{
    std::string r = s;
    toLowerAsciiSelf(r);
    return r;
}

void CxString::toUpperAsciiSelf(string &s)
{
    _upper_inplace(s);
}

string CxString::toUpperAscii(const string &s)
{
    std::string r = s;
    toUpperAsciiSelf(r);
    return r;
}

bool CxString::equal(const std::string &s1, const std::string &s2)
{
    return s1 == s2;
}

bool CxString::equalCase(const std::string &s1, const std::string &s2)
{
    if (s1.size() != s2.size()) return false;
    for (size_t i = 0; i < s1.size(); ++i)
        if (std::tolower((unsigned char) s1[i]) != std::tolower((unsigned char) s2[i]))
            return false;
    return true;
}

size_t CxString::findLeftCase(const std::string &sMaster, const std::string &sSub, size_t iPos)
{
    std::string lowerMaster = _lower(sMaster);
    std::string lowerSub = _lower(sSub);
    return lowerMaster.find(lowerSub, iPos);
}

size_t CxString::findLeftCase(const std::string &sMaster, const std::vector<std::string> &sSubs)
{
    size_t pos = std::string::npos;
    for (const auto &sub: sSubs)
    {
        size_t p = sMaster.find(_lower(sub));
        if (p != std::string::npos)
        {
            pos = p;
            break;
        }
    }
    return pos;
}

size_t CxString::findLeftCase(const std::vector<std::string> &sMaster, const std::string &sSubs)
{
    for (size_t i = 0; i < sMaster.size(); ++i)
    {
        if (sMaster[i].find(_lower(sSubs)) != std::string::npos) return i;
    }
    return std::string::npos;
}

size_t CxString::findEqualCase(const std::vector<std::string> &sMaster, const std::string &sSubs)
{
    auto sub = _lower(sSubs);
    for (size_t i = 0; i < sMaster.size(); ++i)
    {
        if (_lower(sMaster[i]) == sub) return i;
    }
    return std::string::npos;
}

size_t CxString::findRightCase(const std::string &sMaster, const std::string &sSub, size_t iPos)
{
    return _lower(sMaster).rfind(_lower(sSub), iPos);
}

bool CxString::exist(const std::string &sMaster, const std::string &sSub)
{
    return sMaster.find(sSub) != std::string::npos;
}

bool CxString::existCase(const std::string &sMaster, const std::string &sSub)
{
    return _lower(sMaster).find(_lower(sSub)) != std::string::npos;
}

bool CxString::existCase(const std::string &sMaster, const std::vector<std::string> &sSubs)
{
    std::string lm = _lower(sMaster);
    return std::any_of(sSubs.begin(), sSubs.end(), [&](const std::string &sub)
    {
        return lm.find(_lower(sub)) != std::string::npos;
    });
}

bool CxString::existCase(const std::vector<std::string> &sMaster, const std::string &sSubs)
{
    std::string ls = _lower(sSubs);
    return std::any_of(sMaster.begin(), sMaster.end(), [&](const std::string &m)
    {
        return _lower(m).find(ls) != std::string::npos;
    });
}

bool CxString::existEqualCase(const std::vector<std::string> &sMaster, const std::string &sSubs)
{
    std::string ls = _lower(sSubs);
    return std::any_of(sMaster.begin(), sMaster.end(), [&](const std::string &m)
    {
        return _lower(m) == ls;
    });
}

bool CxString::beginWith(const std::string &sMaster, const std::string &sSub)
{
    return sMaster.size() >= sSub.size() && std::equal(sSub.begin(), sSub.end(), sMaster.begin());
}

bool CxString::beginWithCase(const std::string &sMaster, const std::string &sSub)
{
    return beginWith(_lower(sMaster), _lower(sSub));
}

bool CxString::endWith(const std::string &sMaster, const std::string &sSub)
{
    return sMaster.size() >= sSub.size() &&
           std::equal(sSub.rbegin(), sSub.rend(), sMaster.rbegin());
}

bool CxString::endWithCase(const std::string &sMaster, const std::string &sSub)
{
    return endWith(_lower(sMaster), _lower(sSub));
}

inline std::string vstring_format(const char *fmt, va_list ap)
{
    char buf[256];
    va_list ap2;
    va_copy(ap2, ap);
    int n = std::vsnprintf(buf, sizeof(buf), fmt, ap2);
            va_end(ap2);
    if (n >= 0 && n < (int) sizeof(buf))
    {
        return {buf, static_cast<size_t>(n)};
    }

    int needed = (n > 0) ? (n + 1) : 0;

    if (n < 0)
    {
#ifdef _MSC_VER
        va_list ap3;
        va_copy(ap3, ap);
        needed = _vscprintf(fmt, ap3) + 1;
                va_end(ap3);
        if (needed <= 0) return {};
#else
        return {};
#endif
    }

    std::vector<char> tmp(needed);
    std::vsnprintf(tmp.data(), tmp.size(), fmt, ap);
    return {tmp.data(), tmp.data() + needed - 1};
}


string CxString::format(const char *sFormat, ...)
{
    va_list ap;
            va_start(ap, sFormat);
    std::string out = vstring_format(sFormat, ap);
            va_end(ap);
    return out;
}

std::string CxString::tokenLeft(std::string &s, char cToken, bool *ok)
{
    size_t pos = s.find(cToken);
    if (pos == std::string::npos)
    {
        if (ok) *ok = false;
        return {};
    }
    std::string r = s.substr(0, pos);
    s.erase(0, pos + 1);
    if (ok) *ok = true;
    return r;
}

std::string CxString::tokenLeft(std::string &s, const std::string &sToken, bool *ok)
{
    size_t pos = s.find(sToken);
    if (pos == std::string::npos)
    {
        if (ok) *ok = false;
        return {};
    }
    std::string r = s.substr(0, pos);
    s.erase(0, pos + sToken.size());
    if (ok) *ok = true;
    return r;
}

std::string CxString::tokenRight(std::string &s, char cToken, bool *ok)
{
    size_t pos = s.rfind(cToken);
    if (pos == std::string::npos)
    {
        if (ok) *ok = false;
        return {};
    }
    std::string r = s.substr(pos + 1);
    s.erase(pos);
    if (ok) *ok = true;
    return r;
}

std::string CxString::tokenRight(std::string &s, const std::string &sToken, bool *ok)
{
    size_t pos = s.rfind(sToken);
    if (pos == std::string::npos)
    {
        if (ok) *ok = false;
        return {};
    }
    std::string r = s.substr(pos + sToken.size());
    s.erase(pos);
    if (ok) *ok = true;
    return r;
}

std::string CxString::unquote(const std::string &s, char quote_l, char quote_r)
{
    size_t l = s.find(quote_l);
    if (l == std::string::npos) return {};
    size_t r = s.find(quote_r, l + 1);
    if (r == std::string::npos || r <= l) return {};
    return s.substr(l + 1, r - l - 1);
}

std::string CxString::unquote(const std::string &s, char c)
{
    return unquote(s, c, c);
}

std::string CxString::unquote(const std::string &s, const std::string &quote_l, const std::string &quote_r)
{
    size_t l = s.find(quote_l);
    if (l == std::string::npos) return {};
    size_t r = s.find(quote_r, l + quote_l.size());
    if (r == std::string::npos || r <= l) return {};
    return s.substr(l + quote_l.size(), r - (l + quote_l.size()));
}

std::vector<std::string> CxString::unquotes(const std::string &s, const std::string &quote_l, const std::string &quote_r)
{
    std::vector<std::string> out;
    size_t pos = 0;
    while (true)
    {
        size_t l = s.find(quote_l, pos);
        if (l == std::string::npos) break;
        size_t r = s.find(quote_r, l + quote_l.size());
        if (r == std::string::npos || r <= l) break;
        out.push_back(s.substr(l + quote_l.size(), r - (l + quote_l.size())));
        pos = r + quote_r.size();
    }
    return out;
}

std::string CxString::unquote(const std::string &s, size_t sPos, const std::string &quote_l, const std::string &quote_r, size_t *pPos)
{
    if (sPos >= s.size()) return {};
    size_t l = s.find(quote_l, sPos);
    if (l == std::string::npos) return {};
    size_t r = s.find(quote_r, l + quote_l.size());
    if (r == std::string::npos || r <= l) return {};
    if (pPos) *pPos = r;
    return s.substr(l + quote_l.size(), r - (l + quote_l.size()));
}

bool CxString::find(const std::string &src, const std::string &start, const std::string &end, int &pos, int &len, std::string &des)
{
    size_t p1 = src.find(start);
    if (p1 == std::string::npos) return false;
    size_t p2 = src.find(end, p1 + start.size());
    if (p2 == std::string::npos || p2 <= p1) return false;
    pos = (int) p1;
    len = (int) (p2 - (p1 + start.size()));
    des = src.substr(p1 + start.size(), len);
    return true;
}

int CxString::lengthCString(int n, ...)
{
    va_list ap;
            va_start(ap, n);
    int len = 0;
    for (int i = 0; i < n; ++i)
    {
        const char *s = va_arg(ap, const char*);
        len += (int) std::strlen(s);
    }
            va_end(ap);
    return len + n - 1; // n-1 个分隔（如果调用者这么需要）
}

int CxString::copyCString(char *dest, int n, ...)
{
    if (!dest || n <= 0) return 0;
    va_list ap;
            va_start(ap, n);
    int wrote = 0;
    for (int i = 0; i < n; ++i)
    {
        const char *s = va_arg(ap, const char*);
        int l = (int) std::strlen(s);
        std::memcpy(dest + wrote, s, (size_t) l);
        wrote += l;
        if (i + 1 < n)
        { dest[wrote++] = '\0'; } // 按原注释风格可自定义分隔
    }
            va_end(ap);
    return wrote;
}

cx::uint32 CxString::toUint32(const char *pBuff, int iLenth, bool bInverse)
{
    if (!pBuff || iLenth <= 0) return 0;
    cx::uint32 r = 0;
    if (bInverse)
    {
        for (int i = 0; i < iLenth && i < 4; ++i) r = (r << 8) | (unsigned char) pBuff[i];
    }
    else
    {
        for (int i = iLenth - 1; i >= 0 && i >= iLenth - 4; --i) r = (r << 8) | (unsigned char) pBuff[i];
    }
    return r;
}

cx::uint32 CxString::toUint32(cx::uchar *pBuff, int iLenth, bool bInverse)
{
    return pBuff ? toUint32(reinterpret_cast<char *>(pBuff), iLenth, bInverse) : 0u;
}

bool CxString::isValidIp(const std::string &sIp)
{
    auto parts = split(sIp, '.');
    if (parts.size() != 4) return false;
    for (auto &p: parts)
    {
        bool ok = false;
        int v = toInt32(p, &ok);
        if (!ok || v < 0 || v > 255) return false;
    }
    return true;
}

bool CxString::isValidPort(int iPort)
{
    return iPort > 0 && iPort <= 65535;
}

bool CxString::isValidPath(const std::string &sPath)
{
    if (sPath.empty()) return false;

    return std::all_of(sPath.begin(), sPath.end(), [](unsigned char c)
    {
        return std::isalnum(c) || c == '_' || c == '-' || c == '.' ||
               c == '/' || c == '\\' || c == ':';  // 允许路径符号
    });
}

bool CxString::isValidFullPath(const std::string &sPath)
{
    return isValidPath(sPath);
}

bool CxString::isValidFileName(const std::string &sPath)
{
    return !sPath.empty() && sPath.find_first_of("\\/:*?\"<>|") == std::string::npos;
}

bool CxString::isValidPathName(const std::string &sPath)
{
    return isValidPath(sPath);
}

bool CxString::isValidCodeName(const std::string &sName)
{
    if (sName.empty()) return false;
    if (!(std::isalpha((unsigned char) sName[0]) || sName[0] == '_')) return false;
    const auto *p = reinterpret_cast<const unsigned char *>(sName.data());
    const unsigned char *e = p + sName.size();
    for (; p < e; ++p)
    {
        unsigned char c = *p;
        if (!(std::isalnum(c) || c == '_')) return false;
    }
    return true;
}

bool CxString::isValidHexCharater(const std::string &s)
{
    if (s.empty()) return false;
    return std::all_of(s.begin(), s.end(), [](unsigned char c)
    {
        return ascii_is_hex_digit(c) || c == ' ' || c == 'x' || c == 'X';
    });
}

int CxString::copyTo(const std::vector<std::string> &sLines, char *dest, int iLength)
{
    if (!dest || iLength <= 0) return 0;
    int pos = 0;
    for (size_t i = 0; i < sLines.size(); ++i)
    {
        const std::string &s = sLines[i];
        int l = (int) std::min<size_t>(s.size(), (std::max)(0, iLength - pos));
        std::memcpy(dest + pos, s.data(), (size_t) l);
        pos += l;
        if (pos < iLength && i + 1 < sLines.size()) dest[pos++] = '\n';
        if (pos >= iLength) break;
    }
    return pos;
}

int CxString::sizeOf(const std::vector<std::string> &sLines)
{
    int n = 0;
    if (sLines.empty()) return 0;
    for (const auto &sLine: sLines) n += (int) sLine.size();
    n += (int) sLines.size() - 1; // 行间换行
    return n;
}

// -------- newStrings --------
std::vector<std::string> CxString::newStrings(const std::vector<std::string> &ss)
{
    std::vector<std::string> r;
    r.reserve(ss.size());
    for (const auto &s: ss) r.emplace_back(s.data(), s.size());
    return r;
}

std::map<std::string, std::string> CxString::newStrings(const std::map<std::string, std::string> &ss)
{
    std::map<std::string, std::string> r;
    for (const auto &kv: ss)
    {
        const std::string &k = kv.first, &v = kv.second;
        r.emplace(std::piecewise_construct,
                  std::forward_as_tuple(k.data(), k.size()),
                  std::forward_as_tuple(v.data(), v.size()));
    }
    return r;
}

// -------- countOfString --------
int CxString::countOfString(const std::string &src, const std::string &sub)
{
    if (sub.empty()) return 0;
    int cnt = 0;
    std::string::size_type pos = 0;
    while ((pos = src.find(sub, pos)) != std::string::npos)
    {
        ++cnt;
        pos += sub.size();
    }
    return cnt;
}

int CxString::countOfString(const std::string &src, char sub)
{
    int cnt = 0;
    const char *p = src.data();
    const char *e = p + src.size();
    for (; p < e; ++p) if (*p == sub) ++cnt;
    return cnt;
}

// -------- parseToInts --------
// 例： "1~3,7,10~12"  -> [1,2,3,7,10,11,12]
std::vector<int> CxString::parseToInts(const std::string &sSrc, char cJoin, char cSplit)
{
    std::vector<int> out;
    auto parts = split(sSrc, cSplit);
    for (auto s: parts)
    {
        if (s.find(cJoin) != std::string::npos)
        {
            std::string sBegin = tokenLeft(s, cJoin); // 取左边
            std::string sEnd = s;                 // token 会改写 s，剩右边
            int a = CxString::toInt32(sBegin);
            int b = CxString::toInt32(sEnd);
            if (a <= b) for (int x = a; x <= b; ++x) out.push_back(x);
            else for (int x = a; x >= b; --x) out.push_back(x); // 兼容递减
        }
        else
        {
            out.push_back(CxString::toInt32(s));
        }
    }
    return out;
}

// -------- UTF-8 <-> wide --------
// 优先使用 UTF-8。Windows 走 MultiByteToWideChar/WideCharToMultiByte；
// 其他平台用 std::wstring_convert（已弃用，但便携且简单，足够实用）。
std::wstring CxString::toWstring(const std::string &sSrc)
{
#if defined(_WIN32)
    if (sSrc.empty()) return {};
    int n = MultiByteToWideChar(CP_UTF8, 0, sSrc.c_str(), (int) sSrc.size(), nullptr, 0);
    std::wstring w(n, L'\0');
    MultiByteToWideChar(CP_UTF8, 0, sSrc.c_str(), (int) sSrc.size(), &w[0], n);
    return w;
#else
    // 注意：codecvt 被弃用，但在多数实现上仍可用；更严谨可换 ICU。
    std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> cvt;
    try { return cvt.from_bytes(sSrc); } catch (...) { return std::wstring(); }
#endif
}

std::string CxString::fromWstring(const std::wstring &sSrc)
{
#if defined(_WIN32)
    if (sSrc.empty()) return {};
    int n = WideCharToMultiByte(CP_UTF8, 0, sSrc.c_str(), (int) sSrc.size(), nullptr, 0, nullptr, nullptr);
    std::string s(n, '\0');
    WideCharToMultiByte(CP_UTF8, 0, sSrc.c_str(), (int) sSrc.size(), &s[0], n, nullptr, nullptr);
    return s;
#else
    std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> cvt;
    try { return cvt.to_bytes(sSrc); } catch (...) { return std::string(); }
#endif
}

bool CxString::equal(const std::vector<std::string> &ss1, const std::vector<std::string> &ss2, bool bIgnoreOrder, bool bIgnoreCase, bool bTrim)
{
    if (ss1.size() != ss2.size()) return false;
    if (ss1.empty()) return true;

    if (!bIgnoreOrder)
    {
        for (size_t i = 0; i < ss1.size(); ++i)
        {
            if (_norm_ascii(ss1[i], bIgnoreCase, bTrim) != _norm_ascii(ss2[i], bIgnoreCase, bTrim))
                return false;
        }
        return true;
    }

    // 忽略顺序：用计数表避免 O(n^2)
    auto build = [&](const std::vector<std::string> &v)
    {
        std::unordered_map<std::string, int> m;
        m.reserve(v.size() * 2);
        for (auto s: v) ++m[_norm_ascii(std::move(s), bIgnoreCase, bTrim)];
        return m;
    };
    auto m1 = build(ss1), m2 = build(ss2);
    if (m1.size() != m2.size()) return false;
    for (auto &kv: m1)
    {
        auto it = m2.find(kv.first);
        if (it == m2.end() || it->second != kv.second) return false;
    }
    return true;
}

std::vector<std::pair<const char *, int>> CxString::splitToPair(const char *src, int len, int splitSize)
{
    std::vector<std::pair<const char *, int>> result;
    if (!src || len <= 0 || splitSize <= 0) return result;
    result.reserve((len + splitSize - 1) / splitSize);
    for (int i = 0; i < len; i += splitSize)
    {
        int l = (std::min)(splitSize, len - i);
        result.emplace_back(src + i, l);
    }
    return result;
}
