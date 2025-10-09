#include <gtest/gtest.h>

#include "datetime.h"

using cx::DateTime;

// 为了书写方便
static inline DateTime Utc(int y, int M, int d, int h = 0, int m = 0, int s = 0, int ms = 0)
{
    return DateTime(y, M, d, h, m, s, ms, DateTime::Kind::Utc);
}

static inline DateTime Loc(int y, int M, int d, int h = 0, int m = 0, int s = 0, int ms = 0)
{
    return DateTime(y, M, d, h, m, s, ms, DateTime::Kind::Local);
}

TEST(DateTime_Basic, DefaultAndValidity)
{
    DateTime a; // 默认构造：有效，1970-01-01 00:00:00.000（Unspecified）
    EXPECT_TRUE(a.isValid());
    EXPECT_EQ(a.ticksMs(), DateTime::minMs());
    EXPECT_EQ(a.kind(), DateTime::Kind::Unspecified);

    auto inv = DateTime::invalid();
    EXPECT_FALSE(inv.isValid());
}

TEST(DateTime_Range, MinMax)
{
    EXPECT_EQ(DateTime::minMs(), 0);
    // 9999-12-31T23:59:59.999
    EXPECT_EQ(DateTime::maxMs(), INT64_C(253402300799999));
}

TEST(DateTime_Construct, UtcAndLocalConstruct)
{
    // 1970-01-01 00:00:00.000 UTC -> epoch 0
    auto z0 = Utc(1970, 1, 1, 0, 0, 0, 0);
    EXPECT_TRUE(z0.isValid());
    EXPECT_EQ(z0.ticksMs(), 0);

    // 一个普通时间点
    auto z = Utc(2024, 1, 2, 3, 4, 5, 6); // UTC
    EXPECT_TRUE(z.isValid());
    // toIso8601 对 Utc 末尾应带 'Z'
    std::string iso = z.toIso8601();
    EXPECT_NE(iso.find('Z'), std::string::npos);

    // Local 构造（你的实现把 Local 解释为固定 +08:00）
    auto l = Loc(2024, 1, 2, 3, 4, 5, 6);
    EXPECT_TRUE(l.isValid());
    // 同一“墙钟数值”在 Local/UTC 上 ticksMs 不同
    EXPECT_NE(l.ticksMs(), z.ticksMs());
}

TEST(DateTime_NowToday, NowAndTodayAreValid)
{
    auto n = DateTime::now();     // Local
    auto u = DateTime::utcNow();  // Utc
    EXPECT_TRUE(n.isValid());
    EXPECT_TRUE(u.isValid());

    auto t = DateTime::today();   // 当地 00:00:00
    EXPECT_TRUE(t.isValid());
    int h = t.hour();
    int mi = t.minute();
    int se = t.second();
    int ms = t.millisecond();
    EXPECT_EQ(h, 0);
    EXPECT_EQ(mi, 0);
    EXPECT_EQ(se, 0);
    EXPECT_EQ(ms, 0);
}

TEST(DateTime_Fields, SplitFields)
{
    auto z = Utc(2000, 2, 29, 23, 59, 58, 999);
    EXPECT_EQ(z.year(), 2000);
    EXPECT_EQ(z.month(), 2);
    EXPECT_EQ(z.day(), 29);
    EXPECT_EQ(z.hour(), 23);
    EXPECT_EQ(z.minute(), 59);
    EXPECT_EQ(z.second(), 58);
    EXPECT_EQ(z.millisecond(), 999);

    EXPECT_EQ(z.dayOfWeek(), 2 /* 0=Sun -> 2000-02-29 是周二 */);
    EXPECT_EQ(z.dayOfYear(), 60); // 闰年，2月29日是当年的第60天
}

TEST(DateTime_LeapMonthDays, LeapAndDaysInMonth)
{
    EXPECT_TRUE(DateTime::isLeapYear(2000));
    EXPECT_FALSE(DateTime::isLeapYear(1900));
    EXPECT_TRUE(DateTime::isLeapYear(2024));
    EXPECT_EQ(DateTime::daysInMonth(2024, 2), 29);
    EXPECT_EQ(DateTime::daysInMonth(2023, 2), 28);
    EXPECT_EQ(DateTime::daysInMonth(2023, 4), 30);
    EXPECT_EQ(DateTime::daysInMonth(2023, 7), 31);
}

TEST(DateTime_AddArithmetic, AddMsSecMinHourDay)
{
    auto z = Utc(2024, 1, 1, 0, 0, 0, 0);
    EXPECT_EQ(z.addMilliseconds(500).millisecond(), 500);
    EXPECT_EQ(z.addSeconds(61).second(), 1);
    EXPECT_EQ(z.addMinutes(125).hour(), 2);
    EXPECT_EQ(z.addHours(26).day(), 2);
    EXPECT_EQ(z.addDays(31).month(), 2);
}

TEST(DateTime_AddMonthsYears, EndOfMonthAlignment)
{
    // 月末对齐：1月31日 + 1 月 -> 2月最后一天（闰/平年）
    auto a = Utc(2023, 1, 31, 12, 0, 0, 0).addMonths(1);
    EXPECT_TRUE(a.isValid());
    EXPECT_EQ(a.year(), 2023);
    EXPECT_EQ(a.month(), 2);
    EXPECT_EQ(a.day(), 28);

    auto b = Utc(2024, 1, 31, 12, 0, 0, 0).addMonths(1);
    EXPECT_TRUE(b.isValid());
    EXPECT_EQ(b.year(), 2024);
    EXPECT_EQ(b.month(), 2);
    EXPECT_EQ(b.day(), 29);

    auto c = Utc(2022, 2, 28, 23, 59, 59, 999).addYears(1);
    EXPECT_TRUE(c.isValid());
    EXPECT_EQ(c.year(), 2023);
    EXPECT_EQ(c.month(), 2);
    EXPECT_EQ(c.day(), 28);
}

TEST(DateTime_Compare, OperatorsAndDiff)
{
    auto a = Utc(2024, 5, 1, 0, 0, 0, 0);
    auto b = a.addSeconds(1);
    EXPECT_LT(a, b);
    EXPECT_GT(b, a);
    EXPECT_EQ(DateTime::compare(a, a), 0);
    EXPECT_TRUE(a != b);
    EXPECT_TRUE(a <= b);
    EXPECT_TRUE(b >= a);

    // 差值为毫秒
    EXPECT_EQ(b - a, 1000);
}

TEST(DateTime_LocalUtc, ToLocalAndToUTC)
{
    // 你的实现中 Local = UTC+8（固定差值）
    auto z = Utc(2024, 1, 2, 3, 4, 5, 6);
    auto l = z.toLocalTime();
    ASSERT_TRUE(l.isValid());
    EXPECT_EQ(l.year(), 2024);
    EXPECT_EQ(l.month(), 1);
    EXPECT_EQ(l.day(), 2);
    EXPECT_EQ(l.hour(), 11);   // 3:04:05Z + 8h = 11:04:05 local
    EXPECT_EQ(l.minute(), 4);
    EXPECT_EQ(l.second(), 5);
    EXPECT_EQ(l.millisecond(), 6);
    EXPECT_EQ(l.kind(), DateTime::Kind::Local);

    auto backZ = l.toUniversalTime();
    ASSERT_TRUE(backZ.isValid());
    EXPECT_EQ(backZ.ticksMs(), z.ticksMs());
    EXPECT_EQ(backZ.kind(), DateTime::Kind::Utc);
}

TEST(DateTime_FormatParse, ToStringIsoAndParseLoose)
{
    auto z = Utc(2025, 10, 6, 12, 34, 56, 789);
    // toString 默认格式 yyyy/MM/dd HH:mm:ss:fff（按 kind 解释）
    std::string s1 = z.toString('/', ' ', ':');
    EXPECT_FALSE(s1.empty());

    // toIso8601 包含毫秒，UTC 带 Z
    std::string iso = z.toIso8601();
    EXPECT_NE(iso.find("2025-10-06T12:34:56.789"), std::string::npos);
    EXPECT_NE(iso.find('Z'), std::string::npos);

    // 宽松解析：yyyy[-/]MM[-/]dd[ HH[:mm[:ss[.fff]]]]
    DateTime out;
    EXPECT_TRUE(DateTime::tryParse("2025/10/06 12:34:56.789", out, DateTime::Kind::Utc));
    EXPECT_TRUE(out.isValid());
    EXPECT_EQ(out.year(), 2025);
    EXPECT_EQ(out.month(), 10);
    EXPECT_EQ(out.day(), 6);
    EXPECT_EQ(out.second(), 56);
    EXPECT_EQ(out.millisecond(), 789);

    // 不完整时间也可（到分钟）
    EXPECT_TRUE(DateTime::tryParse("2025-10-06 12:34", out, DateTime::Kind::Utc));
    EXPECT_EQ(out.second(), 0);
    EXPECT_EQ(out.millisecond(), 0);

    // 非法
    EXPECT_FALSE(DateTime::tryParse("2025-13-99 99:99:99.999", out, DateTime::Kind::Utc));
    EXPECT_FALSE(out.isValid()); // tryParse 失败时 out = invalid()
}

TEST(DateTime_OperatorsWithMs, PlusMinusMs)
{
    auto a = Utc(2024, 1, 1, 0, 0, 0, 0);
    auto b = a + 1500; // +1.5s
    EXPECT_EQ(b - a, 1500);
    b -= 500;
    EXPECT_EQ(b - a, 1000);
    b += -1000;
    EXPECT_EQ(b - a, 0);
}
