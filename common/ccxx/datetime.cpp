#include "datetime.h"

#include "cxstring.h"

#include <iomanip>

#ifdef _WIN32

#include <windows.h>

#else
#include <sys/time.h>
#endif


#define GM_TIME_UTC_MST (-7)
#define GM_TIME_UTC_    (0)
#define GM_TIME_UTC_CCT (+8)

#define GM_TIME_UTC_LOCAL GM_TIME_UTC_CCT
#define GM_TIME_UTC_DIFF_MS (GM_TIME_UTC_LOCAL * 60 * 60 * 1000)


static const char CCNumberCharZero127[128] =
        {
                '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0',
                '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0',
                '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0',
                0, 1, 2, 3, 4, 5, 6, 7, 8, 9, '0', '0', '0', '0', '0', '0',
                '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0',
                '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0',
                '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0',
                '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0'
        };

static const char CCHexChar16[16] =
        {
                '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'
        };


inline cx::uint32 julianDayFromGregorianDate1(int year, int month, int day)
{
    // Gregorian calendar starting from October 15, 1582
    // Algorithm from Henry F. Fliegel and Thomas C. Van Flandern
    return (1461 * (year + 4800 + (month - 14) / 12)) / 4
           + (367 * (month - 2 - 12 * ((month - 14) / 12))) / 12
           - (3 * ((year + 4900 + (month - 14) / 12) / 100)) / 4
           + day - 32075;
}

inline cx::uint32 julianDayFromDate1(int year, int month, int day)
{
    if (year < 0)
        ++year;

    if (year > 1582 || (year == 1582 && (month > 10 || (month == 10 && day >= 15))))
    {
        return julianDayFromGregorianDate1(year, month, day);
    }
    else if (year < 1582 || (year == 1582 && (month < 10 || (month == 10 && day <= 4))))
    {
        // Julian calendar until October 4, 1582
        // Algorithm from Frequently Asked Questions about Calendars by Claus Toendering
        int a = (14 - month) / 12;
        return (153 * (month + (12 * a) - 3) + 2) / 5
               + (1461 * (year + 4800 - a)) / 4
               + day - 32083;
    }
    else
    {
        // the day following October 4, 1582 is October 15, 1582
        return 0;
    }
}

inline cx::uint msecsFromDecomposed1(int hour, int minute, int sec, int msec = 0)
{
    return 3600000 * hour + 60000 * minute + 1000 * sec + msec;
}

namespace cx
{


// ----------------- 常量 -----------------
    DateTime::ms_epoch_t DateTime::minMs()
    { return 0; }

    DateTime::ms_epoch_t DateTime::maxMs()
    { return INT64_C(253402300799999); } // 9999-12-31T23:59:59.999

// ----------------- 构造 / 工厂 -----------------
    DateTime::DateTime() : _ms(0), _kind(Kind::Unspecified), _valid(true)
    {}

    DateTime::DateTime(ms_epoch_t ms, Kind k)
            : _ms(ms), _kind(k), _valid(true)
    {
        if (ms < minMs() || ms > maxMs())
        {
            _ms = 0;
            _kind = Kind::Unspecified;
            _valid = false;
        }
    }

    DateTime::DateTime(int year, int month, int day,
                       int hour, int minute, int second, int millisecond,
                       Kind kind)
            : _ms(0), _kind(kind), _valid(false)
    {
        if (!validateYMDHMSms(year, month, day, hour, minute, second, millisecond))
        {
            _kind = Kind::Unspecified;
            return;
        }

        if (kind == Kind::Utc)
        {
            bool ok = false;
            ms_epoch_t z = encodeUtcMs(year, month, day, hour, minute, second, millisecond, ok);
            if (!ok)
            {
                _kind = Kind::Unspecified;
                return;
            }
            if (z < minMs() || z > maxMs())
            {
                _kind = Kind::Unspecified;
                return;
            }
            _ms = z;
            _valid = true;
        }
        else
        {
            // 本地编码（复用 CxTime）
            ms_epoch_t z = encodeDateTime(year, month, day, hour, minute, second, millisecond);
            if (z < minMs() || z > maxMs())
            {
                _kind = Kind::Unspecified;
                return;
            }
            _ms = z;
            _kind = (kind == Kind::Local ? Kind::Local : Kind::Unspecified);
            _valid = true;
        }
    }

    DateTime DateTime::invalid()
    {
        DateTime dt;
        dt._ms = 0;
        dt._kind = Kind::Unspecified;
        dt._valid = false;
        return dt;
    }

    bool DateTime::isValid() const
    { return _valid; }

// ----------------- 现在/今天 -----------------
    DateTime DateTime::now()
    {
        ms_epoch_t ms = currentMsepoch();
        return DateTime(ms, Kind::Local);
    }

    DateTime DateTime::utcNow()
    {
        ms_epoch_t ms = currentMsepoch();
        return DateTime(ms, Kind::Utc);
    }

    DateTime DateTime::today()
    {
        int y, m, d, h, mi, se, ms;
        decodeLocalTm(currentMsepoch(), y, m, d, h, mi, se, ms);
        return DateTime(y, m, d, 0, 0, 0, 0, Kind::Local);
    }

// ----------------- 属性 -----------------
    DateTime::Kind DateTime::kind() const
    { return _kind; }

    DateTime::ms_epoch_t DateTime::ticksMs() const
    { return _ms; }

    int DateTime::year() const
    {
        int y, m, d, h, mi, se, ms;
        split(y, m, d, h, mi, se, ms);
        return _valid ? y : -1;
    }

    int DateTime::month() const
    {
        int y, m, d, h, mi, se, ms;
        split(y, m, d, h, mi, se, ms);
        return _valid ? m : -1;
    }

    int DateTime::day() const
    {
        int y, m, d, h, mi, se, ms;
        split(y, m, d, h, mi, se, ms);
        return _valid ? d : -1;
    }

    int DateTime::hour() const
    {
        int y, m, d, h, mi, se, ms;
        split(y, m, d, h, mi, se, ms);
        return _valid ? h : -1;
    }

    int DateTime::minute() const
    {
        int y, m, d, h, mi, se, ms;
        split(y, m, d, h, mi, se, ms);
        return _valid ? mi : -1;
    }

    int DateTime::second() const
    {
        int y, m, d, h, mi, se, ms;
        split(y, m, d, h, mi, se, ms);
        return _valid ? se : -1;
    }

    int DateTime::millisecond() const
    {
        int y, m, d, h, mi, se, ms;
        split(y, m, d, h, mi, se, ms);
        return _valid ? ms : -1;
    }

    int DateTime::dayOfWeek() const
    {
        if (!_valid) return -1;
        // 借助 C 库（以本地为准）来得出 wday
        int y, m, d, h, mi, se, ms;
        split(y, m, d, h, mi, se, ms);
        std::tm t = {};
        t.tm_year = y - 1900;
        t.tm_mon = m - 1;
        t.tm_mday = d;
        t.tm_hour = h;
        t.tm_min = mi;
        t.tm_sec = se;
        std::time_t ts = std::mktime(&t);
        if (ts == (std::time_t) -1) return -1;
        std::tm *lt = std::localtime(&ts);
        return lt ? lt->tm_wday : -1;
    }

    int DateTime::dayOfYear() const
    {
        if (!_valid) return -1;
        int y, m, d, h, mi, se, ms;
        split(y, m, d, h, mi, se, ms);
        static const int doy_norm[] = {0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334};
        static const int doy_leap[] = {0, 31, 60, 91, 121, 152, 182, 213, 244, 274, 305, 335};
        bool leap = isLeapYear(y);
        return (leap ? doy_leap : doy_norm)[m - 1] + d;
    }

// ----------------- 静态实用 -----------------
    bool DateTime::isLeapYear(int year)
    {
        return ((year % 4 == 0 && year % 100 != 0) || (year % 400 == 0));
    }

    int DateTime::daysInMonth(int year, int month)
    {
        static const int mdays[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
        if (month < 1 || month > 12) return 0;
        if (month == 2) return isLeapYear(year) ? 29 : 28;
        return mdays[month - 1];
    }

// ----------------- 加/减 -----------------
    DateTime DateTime::addMilliseconds(int64_t value) const
    {
        if (!_valid) return invalid();
        bool ok = false;
        ms_epoch_t nm = checkedAdd(_ms, value, ok);
        if (!ok) return invalid();
        return DateTime(nm, _kind);
    }

    DateTime DateTime::addSeconds(double value) const
    {
        return addMilliseconds((int64_t) std::llround(value * 1000.0));
    }

    DateTime DateTime::addMinutes(double value) const
    {
        return addMilliseconds((int64_t) std::llround(value * 60.0 * 1000.0));
    }

    DateTime DateTime::addHours(double value) const
    {
        return addMilliseconds((int64_t) std::llround(value * 3600.0 * 1000.0));
    }

    DateTime DateTime::addDays(double value) const
    {
        return addMilliseconds((int64_t) std::llround(value * 86400.0 * 1000.0));
    }

    DateTime DateTime::addMonths(int months) const
    {
        if (!_valid) return invalid();
        int y, m, d, h, mi, se, ms;
        split(y, m, d, h, mi, se, ms);
        int total = (y * 12) + (m - 1) + months;
        int ny = total / 12;
        int nm = (total % 12) + 1;
        int md = daysInMonth(ny, nm);
        if (md == 0) return invalid();
        int nd = (std::min)(d, md);
//        int nd = (((d) < (md)) ? (d) : (md));
        DateTime r(ny, nm, nd, h, mi, se, ms, _kind);
        return r.isValid() ? r : invalid();
    }

    DateTime DateTime::addYears(int years) const
    {
        return addMonths(years * 12);
    }

// ----------------- 运算符 -----------------
    DateTime DateTime::operator+(int64_t ms) const
    { return addMilliseconds(ms); }

    DateTime DateTime::operator-(int64_t ms) const
    { return addMilliseconds(-ms); }

    DateTime &DateTime::operator+=(int64_t ms)
    {
        *this = addMilliseconds(ms);
        return *this;
    }

    DateTime &DateTime::operator-=(int64_t ms)
    {
        *this = addMilliseconds(-ms);
        return *this;
    }

    int64_t DateTime::operator-(const DateTime &rhs) const
    {
        if (!isValid() || !rhs.isValid()) return 0;
        return _ms - rhs._ms;
    }

// ----------------- 比较 -----------------
    int DateTime::compare(const DateTime &a, const DateTime &b)
    {
        if (!a._valid && !b._valid) return 0;
        if (!a._valid) return -1;
        if (!b._valid) return +1;
        if (a._ms < b._ms) return -1;
        if (a._ms > b._ms) return +1;
        return 0;
    }

    bool DateTime::operator==(const DateTime &o) const
    { return compare(*this, o) == 0; }

    bool DateTime::operator!=(const DateTime &o) const
    { return !(*this == o); }

    bool DateTime::operator<(const DateTime &o) const
    { return compare(*this, o) < 0; }

    bool DateTime::operator>(const DateTime &o) const
    { return compare(*this, o) > 0; }

    bool DateTime::operator<=(const DateTime &o) const
    { return compare(*this, o) <= 0; }

    bool DateTime::operator>=(const DateTime &o) const
    { return compare(*this, o) >= 0; }

// ----------------- Local/UTC 转换 -----------------
    DateTime DateTime::toLocalTime() const
    {
        if (!_valid) return invalid();
        if (_kind == Kind::Local) return *this;
        int y, m, d, h, mi, se, ms;
        decodeLocalTm(_ms, y, m, d, h, mi, se, ms);
        DateTime r(y, m, d, h, mi, se, ms, Kind::Local);
        return r.isValid() ? r : invalid();
    }

    DateTime DateTime::toUniversalTime() const
    {
        if (!_valid) return invalid();
        if (_kind == Kind::Utc) return *this;
        int y, m, d, h, mi, se, ms;
        decodeUtcTm(_ms, y, m, d, h, mi, se, ms);
        bool ok = false;
        ms_epoch_t z = encodeUtcMs(y, m, d, h, mi, se, ms, ok);
        if (!ok) return invalid();
        return DateTime(z, Kind::Utc);
    }

// ----------------- 字符串 -----------------
    std::string DateTime::toString(char s1, char s2, char s3) const
    {
        if (!_valid) return std::string();
        int y, m, d, h, mi, se, ms;
        split(y, m, d, h, mi, se, ms);
        return toString(y, m, d, h, mi, se, ms, s1, s2, s3);
    }

    DateTime DateTime::parse(const std::string &s, Kind kind)
    {
        DateTime out;
        if (!tryParse(s, out, kind)) return invalid();
        return out;
    }

    static inline int fast_atoi(const std::string &s)
    {
        // 宽松：std::atoi 即可（C++11）
        return std::atoi(s.c_str());
    }

    bool DateTime::tryParse(const std::string &s, DateTime &out, Kind kind)
    {
        int y = 0, m = 0, d = 0, h = 0, mi = 0, se = 0, ms = 0;
        if (!decodeDateTimeLoose(s, y, m, d, h, mi, se, ms))
        {
            out = invalid();
            return false;
        }
        DateTime r(y, m, d, h, mi, se, ms, kind);
        if (!r.isValid())
        {
            out = invalid();
            return false;
        }
        out = r;
        return true;
    }

    std::string DateTime::toIso8601() const
    {
        if (!_valid) return std::string();
        int y, m, d, h, mi, se, ms;
        split(y, m, d, h, mi, se, ms);
        std::ostringstream oss;
        oss << std::setfill('0')
            << std::setw(4) << y << '-' << std::setw(2) << m << '-' << std::setw(2) << d
            << 'T' << std::setw(2) << h << ':' << std::setw(2) << mi << ':' << std::setw(2) << se
            << '.' << std::setw(3) << ms;
        if (_kind == Kind::Utc) oss << 'Z';
        return oss.str();
    }

// ----------------- 与 CxTime 交互 -----------------
    DateTime DateTime::fromCxLocalString(const std::string &s)
    {
        msepoch_t ms = 0;
        if (!fromString(s, ms)) return invalid();
        DateTime r(ms, Kind::Local);
        return r.isValid() ? r : invalid();
    }

    DateTime DateTime::fromCxLocalMs(ms_epoch_t ms)
    { return DateTime(ms, Kind::Local); }

    msepoch_t DateTime::currentMsepoch()
    {
#ifdef _WIN32
        FILETIME ft;
        GetSystemTimeAsFileTime(&ft);

        const uint64 iShift = 116444736000000000ULL; // (27111902 << 32) + 3577643008

        union
        {
            FILETIME as_file_time;
            msepoch_t as_integer;   // 100-nanos since 1601-Jan-01
        } caster;
        caster.as_file_time = ft;
        caster.as_integer -= iShift; // filetime is now 100-nanos since 1970-Jan-01
        return caster.as_integer / 10000LL; // truncate to microseconds
#else
        struct timeval tv;
        gettimeofday(&tv, 0); //gettimeofday does not support TZ adjust on Linux.
        time_t t = tv.tv_sec;
        uint32 sub_sec = tv.tv_usec;
        msepoch_t dt = t;
        return dt * 1000 + sub_sec / 1000;
//#else
//#error Now however neither gettimeofday nor FILETIME support is detected.
#endif
    }

    void DateTime::decodeUtcTm(const msepoch_t &dt, int &y, int &m, int &d, int &h, int &mi, int &se, int &ms)
    {
        msepoch_t msecs = dt;
        int ddays = msecs / 86400000;
        msecs %= 86400000;
        if (msecs < 0)
        {
            // negative
            --ddays;
            msecs += 86400000;
        }

        uint32 jd = julianDayFromDate1(1970, 1, 1);
        if (ddays >= 0)
            ddays = (jd + ddays >= jd) ? jd + ddays : 0;
        else
            ddays = (jd + ddays < jd) ? jd + ddays : 0;

        if (ddays >= 2299161)
        {
            // Gregorian calendar starting from October 15, 1582
            // This algorithm is from Henry F. Fliegel and Thomas C. Van Flandern
            msepoch_t ell, n, i, j;
            ell = msepoch_t(ddays) + 68569;
            n = (4 * ell) / 146097;
            ell = ell - (146097 * n + 3) / 4;
            i = (4000 * (ell + 1)) / 1461001;
            ell = ell - (1461 * i) / 4 + 31;
            j = (80 * ell) / 2447;
            d = ell - (2447 * j) / 80;
            ell = j / 11;
            m = j + 2 - (12 * ell);
            y = 100 * (n - 49) + i + ell;
        }
        else
        {
            // Julian calendar until October 4, 1582
            // Algorithm from Frequently Asked Questions about Calendars by Claus Toendering
            ddays += 32082;
            int dd = (4 * ddays + 3) / 1461;
            int ee = ddays - (1461 * dd) / 4;
            int mm = ((5 * ee) + 2) / 153;
            d = ee - (153 * mm + 2) / 5 + 1;
            m = mm + 3 - 12 * (mm / 10);
            y = dd - 4800 + (mm / 10);
            if (y <= 0)
                --y;
        }

        int mds = msecs % 86400000;
        if (msecs < 0)
        {
            // % not well-defined for -ve, but / is.
            int negdays = (86400000 - msecs) / 86400000;
            mds = (msecs + negdays * 86400000) % 86400000;
        }

        h = mds / 3600000;
        mi = (mds % 3600000) / 60000;
        se = (mds / 1000) % 60;
        ms = (mds % 1000);
    }

    void DateTime::decodeLocalTm(const msepoch_t &dt, int &y, int &m, int &d, int &h, int &mi, int &se, int &ms)
    {
        msepoch_t msecs = dt + GM_TIME_UTC_DIFF_MS;
        decodeUtcTm(dt + GM_TIME_UTC_DIFF_MS, y, m, d, h, mi, se, ms);
    }

    msepoch_t DateTime::encodeDateTime(int y, int m, int d, int h, int mi, int se, int ms)
    {
        tm dt;
        dt.tm_year = y - 1900;
        dt.tm_mon = m - 1;
        dt.tm_mday = d;
        dt.tm_hour = h;
        dt.tm_min = mi;
        dt.tm_sec = se;
        dt.tm_isdst = 0;
        dt.tm_wday = 0;
        dt.tm_yday = 0;

        msepoch_t dtResult = mktime(&dt);
        return dtResult * 1000 + ms;
    }

    bool DateTime::decodeDateTime(const std::string &sDateTime, int &y, int &m, int &d, int &h, int &mi, int &se, int &ms)
    {
        //48 49 50 51 .. 57
        // 0  1  2  3     9
        std::string sDateTime2 = CxString::trim(sDateTime);
        if (sDateTime2.size() < 18)
        {
            return false;
        }
        const unsigned char *dts = (const unsigned char *) sDateTime2.data();;

        {
            y = CCNumberCharZero127[dts[0]] * 1000 + CCNumberCharZero127[dts[1]] * 100 +
                CCNumberCharZero127[dts[2]] * 10
                + CCNumberCharZero127[dts[3]];
            m = CCNumberCharZero127[dts[5]] * 10 + CCNumberCharZero127[dts[6]];
            d = CCNumberCharZero127[dts[8]] * 10 + CCNumberCharZero127[dts[9]];
            if (m > 12) return false;
            if (d > 31) return false;
        }
        h = 0;
        mi = 0;
        se = 0;
        ms = 0;
        if (sDateTime2.size() > 18)
        {
            h = CCNumberCharZero127[dts[11]] * 10 + CCNumberCharZero127[dts[12]];
            mi = CCNumberCharZero127[dts[14]] * 10 + CCNumberCharZero127[dts[15]];
            se = CCNumberCharZero127[dts[17]] * 10 + CCNumberCharZero127[dts[18]];
            if (h > 23) return false;
            if (mi > 59) return false;
            if (se > 59) return false;
        }
        if (sDateTime2.size() > 22)
        {
            ms = CCNumberCharZero127[dts[20]] * 100 + CCNumberCharZero127[dts[21]] * 10 + CCNumberCharZero127[dts[22]];
            if (ms > 999) return false;
        }
        return true;
    }

    bool DateTime::fromString(const std::string &sDateTime, cx::msepoch_t &lMsepoch)
    {
        int y, m, d, h, mi, se, ms = 0;
        if (decodeDateTime(sDateTime, y, m, d, h, mi, se, ms))
        {
            cx::msepoch_t dt;
            dt = msecsFromDecomposed1(h, mi, se, ms);
            dt += cx::msepoch_t(julianDayFromGregorianDate1(y, m, d) - julianDayFromGregorianDate1(1970, 1, 1))
                  * GM_INT64_C(86400000);
            lMsepoch = dt - GM_TIME_UTC_DIFF_MS;
            return true;
        }
        return false;
    }

    std::string DateTime::toString(int y, int m, int d, int h, int mi, int se, int ms, char sSplit1, char sSplit2, char sSplit3)
    {
        if (sSplit1 < ' ' || sSplit3 < ' ' || sSplit3 < ' ')
        {
            char dts[18] = {'2', '0', '1', '2', '1', '2', '1', '2', '1', '2', '1', '2', '1', '2', '1', '2', '2', '\0'};

            if (y > -1 && y < 10000 && m > 0 && m < 13 && d > 0 && d < 32 && h > -1 && h < 24 && mi > -1 && mi < 60
                && se > -1 && se < 100)
            {
                int hc, lc;

                div_t y1 = div(y, 1000);
                div_t y2 = div(y1.rem, 100);
                div_t y3 = div(y2.rem, 10);
                dts[0] = CCHexChar16[y1.quot];
                dts[1] = CCHexChar16[y2.quot];
                dts[2] = CCHexChar16[y3.quot];
                dts[3] = CCHexChar16[y3.rem];

                hc = m / 10;
                lc = m % 10;
                dts[4] = CCHexChar16[hc];
                dts[5] = CCHexChar16[lc];

                hc = d / 10;
                lc = d % 10;
                dts[6] = CCHexChar16[hc];
                dts[7] = CCHexChar16[lc];

                hc = h / 10;
                lc = h % 10;
                dts[8] = CCHexChar16[hc];
                dts[9] = CCHexChar16[lc];

                hc = mi / 10;
                lc = mi % 10;
                dts[10] = CCHexChar16[hc];
                dts[11] = CCHexChar16[lc];

                hc = se / 10;
                lc = se % 10;
                dts[12] = CCHexChar16[hc];
                dts[13] = CCHexChar16[lc];

                hc = ms / 100;
                lc = (ms % 100) / 10;
                dts[14] = CCHexChar16[hc];
                dts[15] = CCHexChar16[lc];
                lc = ms % 10;
                dts[16] = CCHexChar16[lc];
            }
            else
            {
            }

            return std::string(dts);
        }
        else
        {
            char dts[24] = {'2', '0', '1', '2', sSplit1, '1', '2', sSplit1, '1', '2', sSplit2, '1', '2', sSplit3, '1',
                            '2',
                            sSplit3, '1', '2', sSplit3, '1', '2', '2', '\0'};

            if (y > -1 && y < 10000 && m > 0 && m < 13 && d > 0 && d < 32 && h > -1 && h < 24 && mi > -1 && mi < 60
                && se > -1 && se < 100)
            {
                int hc, lc;

                div_t y1 = div(y, 1000);
                div_t y2 = div(y1.rem, 100);
                div_t y3 = div(y2.rem, 10);
                dts[0] = CCHexChar16[y1.quot];
                dts[1] = CCHexChar16[y2.quot];
                dts[2] = CCHexChar16[y3.quot];
                dts[3] = CCHexChar16[y3.rem];
//            int yy = y % 100;

//            hc = yy / 10;
//            lc = yy % 10;
//            dts[2] = CCHexChar16[ hc ];
//            dts[3] = CCHexChar16[ lc ];

                hc = m / 10;
                lc = m % 10;
                dts[5] = CCHexChar16[hc];
                dts[6] = CCHexChar16[lc];

                hc = d / 10;
                lc = d % 10;
                dts[8] = CCHexChar16[hc];
                dts[9] = CCHexChar16[lc];

                hc = h / 10;
                lc = h % 10;
                dts[11] = CCHexChar16[hc];
                dts[12] = CCHexChar16[lc];

                hc = mi / 10;
                lc = mi % 10;
                dts[14] = CCHexChar16[hc];
                dts[15] = CCHexChar16[lc];

                hc = se / 10;
                lc = se % 10;
                dts[17] = CCHexChar16[hc];
                dts[18] = CCHexChar16[lc];

                hc = ms / 100;
                lc = (ms % 100) / 10;
                dts[20] = CCHexChar16[hc];
                dts[21] = CCHexChar16[lc];
                lc = ms % 10;
                dts[22] = CCHexChar16[lc];
            }
            else
            {
            }

            return std::string(dts);
        }
    }

    DateTime::ms_epoch_t DateTime::toCxMs() const
    { return _ms; }

// ----------------- 工具 -----------------
    bool DateTime::validateYMDHMSms(int y, int m, int d, int h, int mi, int se, int ms)
    {
        if (y < 1 || y > 9999) return false;
        if (m < 1 || m > 12) return false;
        int md = daysInMonth(y, m);
        if (d < 1 || d > md) return false;
        if (h < 0 || h > 23) return false;
        if (mi < 0 || mi > 59) return false;
        if (se < 0 || se > 59) return false;
        if (ms < 0 || ms > 999) return false;
        return true;
    }

    bool DateTime::decodeDateTimeLoose(const std::string &s,
                                       int &y, int &m, int &d, int &h, int &mi, int &se, int &ms)
    {
        y = m = d = h = mi = se = ms = 0;
        int parts[7] = {0};
        int cnt = 0;
        std::string num;
        for (size_t i = 0; i < s.size(); ++i)
        {
            char c = s[i];
            if (c >= '0' && c <= '9')
            {
                num.push_back(c);
            }
            else
            {
                if (!num.empty())
                {
                    if (cnt < 7) parts[cnt++] = std::atoi(num.c_str());
                    num.clear();
                }
            }
        }
        if (!num.empty() && cnt < 7) parts[cnt++] = std::atoi(num.c_str());

        if (cnt < 3) return false;
        y = parts[0];
        m = parts[1];
        d = parts[2];
        if (cnt > 3) h = parts[3];
        if (cnt > 4) mi = parts[4];
        if (cnt > 5) se = parts[5];
        if (cnt > 6) ms = parts[6];

        return validateYMDHMSms(y, m, d, h, mi, se, ms);
    }

    DateTime::ms_epoch_t DateTime::checkedAdd(ms_epoch_t a, ms_epoch_t b, bool &ok)
    {
        ok = true;
        if (b > 0 && a > maxMs() - b)
        {
            ok = false;
            return 0;
        }
        if (b < 0 && a < minMs() - b)
        {
            ok = false;
            return 0;
        }
        ms_epoch_t r = a + b;
        if (r < minMs() || r > maxMs())
        {
            ok = false;
            return 0;
        }
        return r;
    }

// 把 (y,m,d,h,mi,se,ms) 视为 UTC 直接编码到 epoch ms
    DateTime::ms_epoch_t DateTime::encodeUtcMs(int y, int m, int d, int h, int mi, int se, int ms, bool &ok)
    {
        ok = false;
        std::tm utm = {};
        utm.tm_year = y - 1900;
        utm.tm_mon = m - 1;
        utm.tm_mday = d;
        utm.tm_hour = h;
        utm.tm_min = mi;
        utm.tm_sec = se;
        ms_epoch_t base = timegmPortable(utm, ok);
        if (!ok) return 0;
        ms_epoch_t r = base + ms;
        if (r < minMs() || r > maxMs())
        {
            ok = false;
            return 0;
        }
        ok = true;
        return r;
    }

// 将 tm 当作 UTC 处理，转为 epoch ms（不经本地时区）；可移植实现
    DateTime::ms_epoch_t DateTime::timegmPortable(const std::tm &t, bool &ok)
    {
        ok = false;
        int y = t.tm_year + 1900;
        int m = t.tm_mon + 1;
        int d = t.tm_mday;
        int H = t.tm_hour;
        int M = t.tm_min;
        int S = t.tm_sec;

        if (y < 1 || y > 9999 || m < 1 || m > 12) return 0;
        if (d < 1 || d > daysInMonth(y, m)) return 0;
        if (H < 0 || H > 23 || M < 0 || M > 59 || S < 0 || S > 59) return 0;

        static const int doy_norm[] = {0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334};
        static const int doy_leap[] = {0, 31, 60, 91, 121, 152, 182, 213, 244, 274, 305, 335};
        bool leap = isLeapYear(y);
        int yday = (leap ? doy_leap : doy_norm)[m - 1] + (d - 1);

        // days_before_year：从 0001-01-01 到 year-01-01 的天数
        const auto days_before_year = [](int Y) -> int64_t
        {
            int64_t y = Y - 1;
            return y * 365 + y / 4 - y / 100 + y / 400;
        };
        int64_t days = (days_before_year(y) - days_before_year(1970)) + yday;

        int64_t seconds = days * 86400LL + H * 3600 + M * 60 + S;
        ok = true;
        return (ms_epoch_t) seconds * 1000LL;
    }

    void DateTime::split(int &y, int &m, int &d, int &h, int &mi, int &se, int &ms) const
    {
        if (!_valid)
        {
            y = m = d = h = mi = se = ms = 0;
            return;
        }
        if (_kind == Kind::Utc)
        {
            decodeUtcTm(_ms, y, m, d, h, mi, se, ms);
        }
        else
        {
            decodeLocalTm(_ms, y, m, d, h, mi, se, ms);
        }
    }

    std::string DateTime::currentDateTimeString()
    {
        DateTime now = DateTime::now();
        return now.toString('/', ' ', ':');
    }

    std::string DateTime::currentDateString()
    {
        DateTime now = DateTime::now();
        int y, m, d, h, mi, se, ms;
        now.split(y, m, d, h, mi, se, ms);
        return toString(y, m, d, 0, 0, 0, 0, '/', 0, 0).substr(0, 10); // "yyyy/MM/dd"
    }

    std::string DateTime::currentTimeString()
    {
        DateTime now = DateTime::now();
        int y, m, d, h, mi, se, ms;
        now.split(y, m, d, h, mi, se, ms);
        char buf[16];
        std::snprintf(buf, sizeof(buf), "%02d:%02d:%02d", h, mi, se);
        return {buf};
    }

}
