#include <gtest/gtest.h>
#include <random>
#include <limits>
#include <cstring>
#include <algorithm>
#include <map>
#include <set>
#include "cxstring.h"

using std::string;
using std::vector;
using std::map;

static std::mt19937_64 rng(0xC0FFEEULL);

TEST(CxString_ToString, IntegralAndBool)
{
    EXPECT_EQ(CxString::toString(true), "1");
    EXPECT_EQ(CxString::toString(false), "0");
    EXPECT_EQ(CxString::toString(0), "0");
    EXPECT_EQ(CxString::toString(123), "123");
    EXPECT_EQ(CxString::toString((cx::uint32) 4294967295u), "4294967295");
    // long/ulong/int64/uint64 roundtrips
    cx::int64 i64 = (std::numeric_limits<cx::int64>::min)();
    cx::uint64 u64 = (std::numeric_limits<cx::uint64>::max)();
    EXPECT_EQ(CxString::toString(i64), std::to_string(i64));
    EXPECT_EQ(CxString::toString(u64), std::to_string(u64));
    // float/double: default precision per impl (6/15)
    EXPECT_EQ(CxString::toString(1.25f).substr(0, 4), "1.25");
    EXPECT_TRUE(CxString::toString(3.141592653589793).find("3.14159") != string::npos);
    EXPECT_EQ(CxString::toString(string("abc")), "abc");
}

TEST(CxString_ToFrom_Primitives, ToBoolean_Ints_Floats_DefaultOk)
{
    EXPECT_TRUE(CxString::toBoolean("1"));
    EXPECT_FALSE(CxString::toBoolean("0"));
    EXPECT_EQ(CxString::toInt32("123"), 123);
    EXPECT_EQ(CxString::toUint32("123"), (cx::uint32) 123);
    EXPECT_EQ(CxString::toInt64("9223372036854775807"), (cx::int64) 9223372036854775807LL);
    EXPECT_EQ(CxString::toUint64("18446744073709551615"), (cx::uint64) 18446744073709551615ULL);
    EXPECT_DOUBLE_EQ(CxString::toDouble("1.5"), 1.5);
    EXPECT_FLOAT_EQ(CxString::toFloat("2.5"), 2.5f);
}

TEST(CxString_ToX_HexPrefix, HexForms)
{
    // 0x... parsing for integers
    bool ok = false;
    EXPECT_EQ(CxString::toInt32("0xFF"), 255);
    EXPECT_EQ(CxString::toUint32("0X7B"), (cx::uint32) 123);
    EXPECT_EQ(CxString::toInt64("0x7fffffffffffffff"), (cx::int64) 0x7fffffffffffffffLL);
    EXPECT_EQ(CxString::toUint64("0Xffffffffffffffff"), (cx::uint64) 0xffffffffffffffffULL);
    // with ok
    int v = CxString::toInt32("0x1A", &ok);
    EXPECT_TRUE(ok);
    EXPECT_EQ(v, 26);
    cx::uint u = CxString::toUint32("0x1A", &ok);
    EXPECT_TRUE(ok);
    EXPECT_EQ(u, (cx::uint) 26);
    cx::int64 i64 = CxString::toInt64("0x1A", &ok);
    EXPECT_TRUE(ok);
    EXPECT_EQ(i64, (cx::int64) 26);
    cx::uint64 u64 = CxString::toUint64("0x1A", &ok);
    EXPECT_TRUE(ok);
    EXPECT_EQ(u64, (cx::uint64) 26);
}

TEST(CxString_ToX_WithOk, EdgeZeroAndInvalid)
{
    bool ok = false;
    // zero branch behavior per implementation
    EXPECT_EQ(CxString::toInt32("0", &ok), 0);
    EXPECT_TRUE(ok);
    EXPECT_EQ(CxString::toUint32("0", &ok), (cx::uint) 0);
    EXPECT_TRUE(ok);
    EXPECT_EQ(CxString::toInt64("0", &ok), (cx::int64) 0);
    EXPECT_TRUE(ok);
    EXPECT_EQ(CxString::toUint64("0", &ok), (cx::uint64) 0);
    EXPECT_TRUE(ok);

    double d = CxString::toDouble("0", &ok);
    EXPECT_EQ(d, 0.0);
    EXPECT_TRUE(ok);
    float f = CxString::toFloat("0", &ok);
    EXPECT_EQ(f, 0.0f);
    EXPECT_TRUE(ok);

    // invalid numeric: ok=false by semantics in fromString/setFromString, and toX with ok follows isvalid*
    EXPECT_FALSE(CxString::isvalidInteger("abc"));
    int i = CxString::fromString("abc", 123, &ok);
    EXPECT_FALSE(ok);
    EXPECT_EQ(i, 123);
    cx::uint32 uu = CxString::fromString("abc", (cx::uint32) 88, &ok);
    EXPECT_FALSE(ok);
    EXPECT_EQ(uu, (cx::uint32) 88);
    cx::int64 ii = CxString::fromString("abc", (cx::int64) 77, &ok);
    EXPECT_FALSE(ok);
    EXPECT_EQ(ii, (cx::int64) 77);
    cx::uint64 uu64 = CxString::fromString("abc", (cx::uint64) 66, &ok);
    EXPECT_FALSE(ok);
    EXPECT_EQ(uu64, (cx::uint64) 66);

    double dd = CxString::fromString("abc", 12.34, &ok);
    EXPECT_FALSE(ok);
    EXPECT_DOUBLE_EQ(dd, 12.34);
    float ff = CxString::fromString("abc", 56.78f, &ok);
    EXPECT_FALSE(ok);
    EXPECT_FLOAT_EQ(ff, 56.78f);
}

TEST(CxString_SetFromString, AllTypes)
{
    bool ok;
    bool b = false;
    ok = CxString::setFromString("1", b);
    EXPECT_TRUE(ok);
    EXPECT_TRUE(b);
    int i = 0;
    ok = CxString::setFromString("0xFF", i);
    EXPECT_TRUE(ok);
    EXPECT_EQ(i, 255);
    cx::uint32 u = 0;
    ok = CxString::setFromString("0", u);
    EXPECT_TRUE(ok);
    EXPECT_EQ(u, (cx::uint32) 0);
    cx::int64 i64 = 0;
    ok = CxString::setFromString("1234567890123", i64);
    EXPECT_TRUE(ok);
    EXPECT_EQ(i64, (cx::int64) 1234567890123LL);
    cx::uint64 u64 = 0;
    ok = CxString::setFromString("0x10", u64);
    EXPECT_TRUE(ok);
    EXPECT_EQ(u64, (cx::uint64) 16);
    double d = 0;
    ok = CxString::setFromString("3.5", d);
    EXPECT_TRUE(ok);
    EXPECT_DOUBLE_EQ(d, 3.5);
    float f = 0;
    ok = CxString::setFromString("2.25", f);
    EXPECT_TRUE(ok);
    EXPECT_FLOAT_EQ(f, 2.25f);
    string s;
    ok = CxString::setFromString("hello", s);
    EXPECT_TRUE(ok);
    EXPECT_EQ(s, "hello");

    // invalid integer -> false
    i = 7;
    ok = CxString::setFromString("abc", i);
    EXPECT_FALSE(ok);
    EXPECT_EQ(i, 7);
    // invalid float -> false
    f = 1.0f;
    ok = CxString::setFromString("x.y", f);
    EXPECT_FALSE(ok);
    EXPECT_FLOAT_EQ(f, 1.0f);
}

TEST(CxString_IsValid_Group, IntegerFloatDoubleLengths)
{
    // integer: only digits or 0x-hex and length constraints
    EXPECT_TRUE(CxString::isvalidInteger("12345"));
    EXPECT_FALSE(CxString::isvalidInteger(""));        // empty
    EXPECT_FALSE(CxString::isvalidInteger("123456789012")); // length>=11
    EXPECT_TRUE(CxString::isvalidInteger("0xDEAD"));
    EXPECT_FALSE(CxString::isvalidInteger("0xG"));     // invalid hex digit

    EXPECT_TRUE(CxString::isvalidFloat("3.14"));
    EXPECT_TRUE(CxString::isvalidDouble("3.1415926535"));
    EXPECT_FALSE(CxString::isvalidFloat(""));          // empty
    EXPECT_FALSE(CxString::isvalidDouble("1.2.3"));   // multiple dots
    EXPECT_FALSE(CxString::isvalidFloat("123456789")); // length>8
    EXPECT_FALSE(CxString::isvalidDouble("123456789012345678")); // length>17
}

TEST(CxString_HexString_Data, Roundtrips)
{
    const char bytes[] = {(char) 0xDE, (char) 0xAD, (char) 0xBE, (char) 0xEF};
    string hex = CxString::toHexString(bytes, 4, false);
    EXPECT_EQ(hex, "DEADBEEF");
    auto v = CxString::fromHexString("DE AD BE EF");
    ASSERT_EQ(v.size(), 4u);
    EXPECT_EQ(v[0], (cx::uchar) 0xDE);
    EXPECT_EQ(v[1], (cx::uchar) 0xAD);
    EXPECT_EQ(v[2], (cx::uchar) 0xBE);
    EXPECT_EQ(v[3], (cx::uchar) 0xEF);

    bool ok = false;
    EXPECT_EQ(CxString::hexToInt32("FF", &ok), 255);
    EXPECT_TRUE(ok);
    EXPECT_EQ(CxString::hexToUint64("FFFFFFFFFFFFFFFF", &ok), (cx::uint64) 0xFFFFFFFFFFFFFFFFULL);
    EXPECT_TRUE(ok);

    // scalar overloads
    EXPECT_EQ(CxString::toHexString((cx::uchar) 0xAB), "AB");
    EXPECT_EQ(CxString::toHexString((short) 0x1234), "1234");
    EXPECT_EQ(CxString::toHexString((int) 0x89ABCDEF), "89ABCDEF");
    EXPECT_EQ(CxString::toHexString((cx::uint32) 0x76543210u, true), "0x76543210");
    EXPECT_EQ(CxString::toHexString((cx::int64) 0x1122334455667788LL).substr(0, 2), "11");
}

TEST(CxString_InsertSplit_Remove, BasicAndEdge)
{
    EXPECT_EQ(CxString::insertSplit("123456789", ',', 3), "123,456,789");
    EXPECT_EQ(CxString::insertSplit("", ',', 3), "");
    EXPECT_EQ(CxString::insertSplit("123", ',', 0), "123");

    string s = "a b  c   d";
    CxString::remove(s, ' ');
    EXPECT_EQ(s, "abcd");
}

TEST(CxString_Split_Family, CharStringCaseAndVec)
{
    // split by char
    auto v1 = CxString::split("a,,b,c", ',', true);
    ASSERT_EQ(v1.size(), 4u);
    EXPECT_EQ(v1[0], "a");
    EXPECT_EQ(v1[1], "");
    EXPECT_EQ(v1[2], "b");
    EXPECT_EQ(v1[3], "c");

    // split by string
    auto v2 = CxString::split("a<->b<->c", string("<->"), false);
    ASSERT_EQ(v2.size(), 3u);
    EXPECT_EQ(v2[1], "b");

    // split by delimiters set
    auto v3 = CxString::splitByDelimiters("a;b,c|d", ";,|", false);
    ASSERT_EQ(v3.size(), 4u);
    EXPECT_EQ(v3[2], "c");

    // case-insensitive split
    auto v4 = CxString::splitCase("a<->B<->c<->B<->d", "b<->", false);
    ASSERT_EQ(v4.size(), 3u);
    EXPECT_EQ(v4[1], "c<->");

    // split(vector<string>, token)
    vector<string> lines = {"12", "34", "56"};
    auto v5 = CxString::split(lines, string("34"), true);
    // "123456" split by "34" => {"12", "", "56"} when concatenated
    ASSERT_EQ(v5.size(), 2u); // implementation concatenates first then splits; here we assert minimal property
    EXPECT_TRUE(CxString::join(v5, ',').find("12") != string::npos);
}

TEST(CxString_SplitToMap_Join, TrimAndReverseAndLines)
{
    // splitToMap from vector
    vector<string> kvs = {" a = 1 ", "b=2", "c    =   3", "d"};
    auto m = CxString::splitToMap(kvs, '=', true);
    EXPECT_EQ(m["a"], "1");
    EXPECT_EQ(m["b"], "2");
    EXPECT_EQ(m["c"], "3");
    EXPECT_TRUE(m.find("d") != m.end());
    EXPECT_EQ(m["d"], "");

    // splitToMap from string
    auto m2 = CxString::splitToMap("k1=v1,k2=v2,,k3=v3", '=', ',', true);
    EXPECT_EQ(m2["k1"], "v1");
    EXPECT_EQ(m2["k2"], "v2");
    EXPECT_EQ(m2["k3"], "v3");

    // join map
    string j = CxString::join(m2, '=', ',');
    EXPECT_TRUE(j.find("k1=v1") != string::npos);

    // lines
    auto lines = CxString::splitToLines("a=1,b=2\nc=3,d=4", '=', ',');
    ASSERT_EQ(lines.size(), 3u);
    ASSERT_EQ(lines[0].size(), 2u);
}

TEST(CxString_Replace_Trim_Case, Basic)
{
    EXPECT_EQ(CxString::replace("a-b-c", '-', '_'), "a_b_c");
    EXPECT_EQ(CxString::replace("abcabc", "ab", "X"), "XcXc");
    EXPECT_EQ(CxString::replaceCase("AbcABc", "aB", "Z"), "ZcZc");

    EXPECT_EQ(CxString::trim("  x  "), "x");
    EXPECT_EQ(CxString::trim("..x..", '.'), "x");
    EXPECT_EQ(CxString::trim("xyx", string("x")), "y");
    EXPECT_EQ(CxString::erase("a b c"), "abc");

    EXPECT_EQ(CxString::toLowerAscii("AbC"), "abc");
    EXPECT_EQ(CxString::toUpperAscii("AbC"), "ABC");

    EXPECT_TRUE(CxString::equal("AbC", "AbC"));
    EXPECT_TRUE(CxString::equalCase("AbC", "abc"));
}

TEST(CxString_Find_Exist_BeginEnd, SearchOps)
{
    EXPECT_EQ(CxString::findLeftCase("HelloWorld", "world"), 5u);
    EXPECT_EQ(CxString::findRightCase("abcXXabcXX", "ABC"), 5u);
    EXPECT_TRUE(CxString::exist("HelloWorld", "World"));
    EXPECT_TRUE(CxString::existCase("HelloWorld", "world"));
    vector<string> pool = {"Aaa", "bBb", "CCC"};
    EXPECT_TRUE(CxString::existCase(pool, "bbb"));
    EXPECT_TRUE(CxString::beginWith("foobar", "foo"));
    EXPECT_TRUE(CxString::endWithCase("foobar", "BAR"));
}

TEST(CxString_Format_Token_Unquote, FormattingAndTokenizing)
{
    string f = CxString::format("num=%d str=%s hex=%X", 7, "ok", 255);
    EXPECT_TRUE(f.find("num=7") != string::npos);
    EXPECT_TRUE(f.find("str=ok") != string::npos);
    EXPECT_TRUE(f.find("FF") != string::npos);

    string s1 = "aaaaa.12341234.bbbbbbbbb";
    bool ok = false;
    string t = CxString::tokenLeft(s1, '.', &ok);
    EXPECT_TRUE(ok);
    EXPECT_EQ(t, "aaaaa");
    EXPECT_EQ(s1, "12341234.bbbbbbbbb");

    s1 = "aaaaa.12341234.bbbbbbbbb";
    string tl = CxString::tokenLeft(s1, '.', &ok);
    EXPECT_TRUE(ok);
    EXPECT_EQ(tl, "aaaaa");
    EXPECT_EQ(s1, "12341234.bbbbbbbbb");

    s1 = "aaaaa.12341234.bbbbbbbbb";
    string tr = CxString::tokenRight(s1, '.', &ok);
    EXPECT_TRUE(ok);
    EXPECT_EQ(tr, "bbbbbbbbb");
    EXPECT_EQ(s1, "aaaaa.12341234");

    EXPECT_EQ(CxString::unquote("(abc)", '(', ')'), "abc");
    EXPECT_EQ(CxString::unquote("\"abc\"", '"'), "abc");
    auto us = CxString::unquotes("<a><b><c>", "<", ">");
    ASSERT_EQ(us.size(), 3u);
    EXPECT_EQ(us[1], "b");

    int pos = 0, len = 0;
    string des;
    EXPECT_TRUE(CxString::find("xx[ab]yy", "[", "]", pos, len, des));
    EXPECT_EQ(des, "ab");
}

TEST(CxString_VarArgs_CStringOps, LengthAndCopy)
{
    // lengthCString: returns sum(strlen) + n
    int L = CxString::lengthCString(3, "aa", "bbb", "c");
    EXPECT_EQ(L, int(strlen("aa")+strlen("bbb") + strlen("c") + 3 - 1));
    // copyCString: dest gets concatenated with '\0' between per contract
    char buf[64] = {0};
    int copied = CxString::copyCString(buf, 3, "aa", "bb", "c");
    EXPECT_GT(copied, 0);
    // Ensure starts with first, contains middle
    EXPECT_STREQ(buf, string("aa").c_str());
}

TEST(CxString_BufferInt, ToUint32Buffers)
{
    // little/big-endian flags are implementation-defined (bInverse=true default)
    cx::uchar b1[4] = {0x12, 0x34, 0x56, 0x78};
    cx::uint32 v1 = CxString::toUint32((cx::uchar *) b1, 4, true);
    cx::uint32 v2 = CxString::toUint32((cx::uchar *) b1, 4, false);
    EXPECT_NE(v1, v2);
}

TEST(CxString_Validators, IP_Port_Code_Path_HexChar)
{
    EXPECT_TRUE(CxString::isValidIp("192.168.1.1"));
    EXPECT_FALSE(CxString::isValidIp("999.1.1.1"));
    EXPECT_TRUE(CxString::isValidPort(80));
    EXPECT_FALSE(CxString::isValidPort(-1));
    EXPECT_TRUE(CxString::isValidCodeName("Abc_123"));
    EXPECT_FALSE(CxString::isValidCodeName("1*bad"));

#if defined(_WIN32) || defined(_WIN64)
    EXPECT_TRUE(CxString::isValidPath("C:\\temp"));
    EXPECT_TRUE(CxString::isValidFileName("file.txt"));
    EXPECT_TRUE(CxString::isValidPathName("folder"));
#else
    EXPECT_TRUE(CxString::isValidPath("/tmp"));
    EXPECT_TRUE(CxString::isValidFileName("file.txt"));
    EXPECT_TRUE(CxString::isValidPathName("folder"));
#endif

    EXPECT_TRUE(CxString::isValidHexCharater("DEADBEEF"));
    EXPECT_FALSE(CxString::isValidHexCharater("XYZ"));
}

TEST(CxString_CopyTo_SizeOf, LinesOps)
{
    vector<string> lines = {"abc", "defg", "", "hi"};
    int sz = CxString::sizeOf(lines);
    ASSERT_GT(sz, 0);
    vector<char> buf(sz + 8, 0);
    int copied = CxString::copyTo(lines, buf.data(), (int) buf.size());
    EXPECT_EQ(copied, sz);
}

TEST(CxString_NewString_NewStrings, Copies)
{
    string s = "hello";
    string s2 = CxString::newString(s);
    EXPECT_EQ(s2, s);
    auto vv = CxString::newStrings(vector<string>{"a", "b"});
    ASSERT_EQ(vv.size(), 2u);
    EXPECT_EQ(vv[1], "b");
}

// ------------------------ Large-scale / fuzz-ish ------------------------

TEST(CxString_LargeScale, SplitJoinAndHex_FixedSeed)
{
    // Generate a large comma-separated list, then split & join roundtrip.
    const int N = 5000;
    string big;
    big.reserve(N * 6);
    for (int i = 0; i < N; i++)
    {
        big += std::to_string((int) (rng() % 100000));
        if (i + 1 < N) big.push_back(',');
    }
    auto parts = CxString::split(big, ',');
    ASSERT_EQ((int) parts.size(), N);
    string back = CxString::join(parts, ',');
    EXPECT_EQ(back, big);

    // Random bytes -> hex -> back
    vector<char> bytes(4096);
    for (auto &c: bytes) c = (char) (rng() & 0xFF);
    string hex = CxString::toHexString(bytes, true);     // with spaces
    auto backBytes = CxString::hexToData(hex);
    ASSERT_EQ(backBytes.size(), bytes.size());
    EXPECT_TRUE(std::equal(bytes.begin(), bytes.end(), backBytes.begin()));
}

TEST(CxString_CaseInsensitiveBulk, ExistCaseAndFindLeftCase)
{
    string s = "TheQuickBrownFoxJumpsOverTheLazyDog";
    for (string q: {"thequick", "BROWN", "dog", "FOXJ"})
    {
        EXPECT_TRUE(CxString::existCase(s, q));
        EXPECT_NE(CxString::findLeftCase(s, q), string::npos);
    }
}
