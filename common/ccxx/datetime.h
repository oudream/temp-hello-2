#ifndef CX_CT_X2_DATETIME_H
#define CX_CT_X2_DATETIME_H


#include "cxglobal.h"


namespace cx
{
    struct DateTimeParts
    {
        int year;
        int month;
        int day;
        int hour;
        int minute;
        int second;
        int millisecond;

        DateTimeParts()
                : year(0), month(0), day(0),
                  hour(0), minute(0), second(0), millisecond(0) {}

         std::string toString(char s1 = '/', char s2 = ' ', char s3 = ':') const
        {
            char buf[64];
            snprintf(buf, sizeof(buf), "%04d%c%02d%c%02d%c%02d%c%02d%c%02d.%03d",
                     year, s1, month, s1, day, s2,
                     hour, s3, minute, s3, second, millisecond);
            return buf;
        }
    };

    class CCXX_EXPORT DateTime
    {
    public:
        typedef int64_t ms_epoch_t; // 毫秒，自 1970-01-01 00:00:00 UTC 起
        enum class Kind
        {
            Unspecified = 0, Utc = 1, Local = 2
        };

        // --------- 构造 / 工厂 ----------
        DateTime();                                      // 1970-01-01 00:00:00.000（有效）
        explicit DateTime(ms_epoch_t ms, Kind k = Kind::Unspecified); // 直接从 epoch ms
        DateTime(int year, int month, int day,
                 int hour = 0, int minute = 0, int second = 0, int millisecond = 0,
                 Kind kind = Kind::Unspecified);          // 无异常；若参数非法 -> Invalid()

        static DateTime invalid();                       // 无效标记
        inline bool isValid() const { return _valid; }   // 是否有效

        // --------- 范围 ----------
        static ms_epoch_t minMs();                       // 1970-01-01T00:00:00.000
        static ms_epoch_t maxMs();                       // 9999-12-31T23:59:59.999

        // --------- 现在/今天 ----------
        static DateTime now();                           // 本地
        static DateTime utcNow();                        // UTC
        static DateTime today();                         // 本地日期 00:00:00.000

        // --------- 属性（按 kind 解释） ----------
        Kind kind() const;

        ms_epoch_t ticksMs() const;

        DateTimeParts toParts() const;

        int year() const;

        int month() const;

        int day() const;

        int hour() const;

        int minute() const;

        int second() const;

        int millisecond() const;

        // DayOfWeek：0=Sunday,1=Monday,... 与 C# 对齐；失败时返回 -1
        int dayOfWeek() const;

        // DayOfYear：1..366；失败时返回 -1
        int dayOfYear() const;

        // --------- 加/减（返回 Invalid() 表示失败/溢出） ----------
        DateTime addMilliseconds(int64_t value) const;

        DateTime addSeconds(double value) const;

        DateTime addMinutes(double value) const;

        DateTime addHours(double value) const;

        DateTime addDays(double value) const;

        DateTime addMonths(int months) const; // 月末对齐

        DateTime addYears(int years) const;

        // --------- 运算符 ----------
        // 与毫秒偏移量
        DateTime operator+(int64_t ms) const;  // 调用 addMilliseconds
        DateTime operator-(int64_t ms) const;  // 调用 addMilliseconds(-ms)
        DateTime &operator+=(int64_t ms);       // 就地修改；失败则置为 Invalid()
        DateTime &operator-=(int64_t ms);

        // 两个时间相减 -> 毫秒差（无效任一侧时返回 0）
        int64_t operator-(const DateTime &rhs) const;

        bool operator==(const DateTime &o) const;

        bool operator!=(const DateTime &o) const;

        bool operator<(const DateTime &o) const;

        bool operator>(const DateTime &o) const;

        bool operator<=(const DateTime &o) const;

        bool operator>=(const DateTime &o) const;

        // --------- Local/UTC 转换 ----------
        DateTime toLocalTime() const;      // 失败返回 Invalid()

        DateTime toUniversalTime() const;  // 失败返回 Invalid()

        // --------- 字符串 ----------
        // 默认沿用 CxTime::toString 风格：yyyy/MM/dd HH:mm:ss:fff
        std::string toString(char s1 = '/', char s2 = ' ', char s3 = ':') const;

        // 简易 ISO8601（kind==Utc 末尾加 'Z'；无效返回空串）
        std::string toIso8601() const;

        ms_epoch_t toCxMs() const;

    private:
        ms_epoch_t _ms;
        Kind _kind;
        bool _valid;

    public:
        // --------- 静态实用 ----------
        static bool isLeapYear(int year);

        static int daysInMonth(int year, int month); // 非法参数时返回 0

        // 比较（基于 epoch ms；无效视为最小）
        static int compare(const DateTime &a, const DateTime &b);

        // 宽松解析：yyyy[-/]MM[-/]dd[ HH[:mm[:ss[.fff]]]]；失败返回 Invalid()
        static DateTime parse(const std::string &s, Kind kind = Kind::Unspecified);

        static bool tryParse(const std::string &s, DateTime &out, Kind kind = Kind::Unspecified);

        // --------- 与 CxTime 交互 ----------
        static DateTime fromCxLocalString(const std::string &s); // 失败返回 Invalid()

        static DateTime fromCxLocalMs(ms_epoch_t ms);            // 标记为 Local

        static msepoch_t currentMsepoch();                       // os.time count ms:(since 1970-01-01 's count ms)

        static std::string currentDateTimeString(char sSplit1 = '/', char sSplit2 = ' ', char sSplit3 = ':');

        static std::string currentDateString(char sSplit1 = '/');

        static std::string currentTimeString(char sSplit3 = ':');
        static std::string currentTimeStringMs(char sSplit3 = ':');

        static void decodeUtcTm(const msepoch_t &dt, int &y, int &m, int &d, int &h, int &mi, int &se, int &ms);

        static void decodeLocalTm(const msepoch_t &dt, int &y, int &m, int &d, int &h, int &mi, int &se, int &ms);

        static msepoch_t encodeDateTime(int y, int m, int d, int h, int mi, int se, int ms);

        static bool decodeDateTime(const std::string &sDateTime, int &y, int &m, int &d, int &h, int &mi, int &se, int &ms);

        static bool fromString(const std::string &sDateTime, cx::msepoch_t &lMsepoch);

        static std::string toString(int y, int m, int d, int h, int mi, int se, int ms, char sSplit1 = '/', char sSplit2 = ' ', char sSplit3 = ':');

        static long long milliSecondDifferToNow(const msepoch_t &dt);

        static std::string format(const std::tm& t);

    private:
        // ----- 工具 -----
        static bool validateYMDHMSms(int y, int m, int d, int h, int mi, int se, int ms);

        static bool decodeDateTimeLoose(const std::string &s, int &y, int &m, int &d, int &h, int &mi, int &se, int &ms);

        static ms_epoch_t checkedAdd(ms_epoch_t a, ms_epoch_t b, bool &ok);

        static ms_epoch_t encodeUtcMs(int y, int m, int d, int h, int mi, int se, int ms, bool &ok);

        static ms_epoch_t timegmPortable(const std::tm &t, bool &ok);

        void split(int &y, int &m, int &d, int &h, int &mi, int &se, int &ms) const;

    };

}


#endif //CX_CT_X2_DATETIME_H
