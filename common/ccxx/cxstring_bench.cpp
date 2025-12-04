#include <benchmark/benchmark.h>
#include <string>
#include <vector>
#include <map>
#include <random>
#include <cstring>
#include <cstdarg>
#include <algorithm>
#include "cxstring.h"

// ========== 工具 & 数据集生成 ==========

using std::string;
using std::vector;

static std::mt19937_64 &rng()
{
    static std::mt19937_64 gen(123456789ULL);
    return gen;
}

static string rand_ascii(size_t n)
{
    static const char table[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz,.-:_/ \\t\\r\\n";
    std::uniform_int_distribution<size_t> dist(0, sizeof(table) - 2);
    string s(n, '\0');
    for (size_t i = 0; i < n; ++i) s[i] = table[dist(rng())];
    return s;
}

static vector<char> rand_bytes(size_t n)
{
    std::vector<char> v(n);
    std::uniform_int_distribution<int> dist(0, 255);
    for (auto &b: v) b = char(dist(rng()));
    return v;
}

static vector<string> mk_tokens(size_t n, const string &tok = "tok")
{
    return vector<string>(n, tok);
}

static string join_with(const vector<string> &vs, char sep)
{
    string out;
    for (size_t i = 0; i < vs.size(); ++i)
    {
        if (i) out.push_back(sep);
        out += vs[i];
    }
    return out;
}

static string mk_kv_line(size_t n)
{
    // k1=v1,k2=v2, ...（含一些空格与空值）
    string out;
    for (size_t i = 1; i <= n; ++i)
    {
        if (i > 1) out += (i % 3 == 0) ? " , " : ",";
        out += "k" + std::to_string(i);
        if (i % 5 == 0)
        {
            out += " ";
            continue;
        } // 故意无等号以测混合情形
        out += "=";
        if (i % 7 == 0) out += " "; // 空值
        else out += "v" + std::to_string(i);
    }
    return out;
}

// ========== 1) toString 全族 ==========

template<typename T>
static void BM_toString_T(benchmark::State &st, const T &v)
{
    for (auto _: st)
    {
        auto s = CxString::toString(v);
        benchmark::DoNotOptimize(s);
    }
}

BENCHMARK_CAPTURE(BM_toString_T, bool, true);
BENCHMARK_CAPTURE(BM_toString_T, int, 123456789);
BENCHMARK_CAPTURE(BM_toString_T, uint32, (cx::uint32) 4294967295u);
BENCHMARK_CAPTURE(BM_toString_T, long, (long) -123456789);
BENCHMARK_CAPTURE(BM_toString_T, ulong, (cx::ulong) 123456789ul);
BENCHMARK_CAPTURE(BM_toString_T, int64, (cx::int64) -9223372036854775807LL);
BENCHMARK_CAPTURE(BM_toString_T, uint64, (cx::uint64) 18446744073709551615ULL);
BENCHMARK_CAPTURE(BM_toString_T, float, 3.1415926f);
BENCHMARK_CAPTURE(BM_toString_T, double, 3.141592653589793);
BENCHMARK_CAPTURE(BM_toString_T, string, string("Hello中文🙂"));

// ========== 2) 基本类型 from/to（含 bOk 版本 & 边界/非法） ==========

static void BM_toBoolean(benchmark::State &st)
{
    const string samples[] = {"0", "1", "true", "false", "", "00", " 1", "9"};
    for (auto _: st)
    {
        for (auto &s: samples)
        {
            auto v = CxString::toBoolean(s);
            benchmark::DoNotOptimize(v);
        }
    }
}

BENCHMARK(BM_toBoolean);

static void BM_toInt32_variants(benchmark::State &st)
{
    const string samples[] = {"0", "42", "-7", "000123", "0xFF", "0XdeadBEEF", "abc", "9999999999"};
    bool ok = false;
    for (auto _: st)
    {
        for (auto &s: samples)
        {
            auto a = CxString::toInt32(s);
            auto b = CxString::toInt32(s, &ok);
            benchmark::DoNotOptimize(a);
            benchmark::DoNotOptimize(b);
            benchmark::DoNotOptimize(ok);
        }
    }
}

BENCHMARK(BM_toInt32_variants);

static void BM_toUint64_variants(benchmark::State &st)
{
    const string samples[] = {"0", "18446744073709551615", "0xFFFFFFFFFFFFFFFF", "-1", "xyz"};
    bool ok = false;
    for (auto _: st)
    {
        for (auto &s: samples)
        {
            auto a = CxString::toUint64(s);
            auto b = CxString::toUint64(s, &ok);
            benchmark::DoNotOptimize(a);
            benchmark::DoNotOptimize(b);
            benchmark::DoNotOptimize(ok);
        }
    }
}

BENCHMARK(BM_toUint64_variants);

static void BM_toFloatDouble(benchmark::State &st)
{
    const string samples[] = {"0", "-0", "3.14", "-2.5", "1e10", "-1e-10", ".5", "5.", "nan", "inf", "x3.1"};
    bool ok = false;
    for (auto _: st)
    {
        for (auto &s: samples)
        {
            auto d = CxString::toDouble(s);
            auto f = CxString::toFloat(s);
            auto d2 = CxString::toDouble(s, &ok);
            auto f2 = CxString::toFloat(s, &ok);
            benchmark::DoNotOptimize(d);
            benchmark::DoNotOptimize(f);
            benchmark::DoNotOptimize(d2);
            benchmark::DoNotOptimize(f2);
            benchmark::DoNotOptimize(ok);
        }
    }
}

BENCHMARK(BM_toFloatDouble);

// fromString / setFromString 全族（选取代表性类型与非法输入）
template<typename T, typename Ret=T>
static void BM_from_set_String_T(benchmark::State &st, const vector<string> &ins, const T &defVal)
{
    bool ok = false;
    for (auto _: st)
    {
        for (auto &s: ins)
        {
            auto r = CxString::fromString(s, defVal, &ok);
            Ret out = defVal;
            bool b2 = CxString::setFromString(s, out);
            benchmark::DoNotOptimize(r);
            benchmark::DoNotOptimize(out);
            benchmark::DoNotOptimize(ok);
            benchmark::DoNotOptimize(b2);
        }
    }
}

static void Register_from_set()
{
    static vector<string> ints = {"0", "-1", "42", "0007", "0xFF", "xyz"};
    static vector<string> u64s = {"0", "18446744073709551615", "-1", "0xFFFFFFFFFFFFFFFF", "abc"};
    static vector<string> flts = {"0", ".5", "5.", "-3.14", "nan", "inf", "x7"};
    static vector<string> strs = {"", " hello ", "\t\n", "中文🙂", "0xFF"};
    benchmark::RegisterBenchmark("from/set::int", BM_from_set_String_T<int, int>, ints, 123);
    benchmark::RegisterBenchmark("from/set::u32", BM_from_set_String_T<cx::uint32, cx::uint32>, ints, (cx::uint32) 7);
    benchmark::RegisterBenchmark("from/set::i64", BM_from_set_String_T<cx::int64, cx::int64>, u64s, (cx::int64) -5);
    benchmark::RegisterBenchmark("from/set::u64", BM_from_set_String_T<cx::uint64, cx::uint64>, u64s, (cx::uint64) 9);
    benchmark::RegisterBenchmark("from/set::float", BM_from_set_String_T<float, float>, flts, 1.25f);
    benchmark::RegisterBenchmark("from/set::double", BM_from_set_String_T<double, double>, flts, 2.5);
    benchmark::RegisterBenchmark("from/set::string", BM_from_set_String_T<string, string>, strs, string("d"));
}

static int _keepalive = []()
{
    Register_from_set();
    return 0;
}();
// 自注册
//struct AutoRegister_from_set { AutoRegister_from_set(){ Register_from_set(); } } _auto_from_set;

// ========== 3) isvalid* 校验族（边界长度、多个点、小数等） ==========

static void BM_isvalid_all(benchmark::State &st)
{
    const string ints[] = {"0", "42", "-1", "0001", "0xFF", "abcdef", "12345678901"};
    const string longs[] = {"0", "123456789012345678", "abc", "-1"};
    const string floats[] = {"0", ".5", "5.", "-3.14", "3.14.15", "3e8", "1e+10", ""};
    const string dbls[] = {"0", ".5", "5.", "-3.1415926", "3.14.15", ""};
    for (auto _: st)
    {
        for (auto &s: ints) benchmark::DoNotOptimize(CxString::isvalidInteger(s));
        for (auto &s: longs) benchmark::DoNotOptimize(CxString::isvalidLong(s));
        for (auto &s: floats) benchmark::DoNotOptimize(CxString::isvalidFloat(s));
        for (auto &s: dbls) benchmark::DoNotOptimize(CxString::isvalidDouble(s));
    }
}

BENCHMARK(BM_isvalid_all);

// ========== 4) Hex 编解码族（有/无空格，大块数据，奇数字符等） ==========

static void BM_toHexString_vec(benchmark::State &st)
{
    size_t n = static_cast<size_t>(st.range(0));
    auto bytes = rand_bytes(n);
    for (auto _: st)
    {
        auto s1 = CxString::toHexString(bytes, true);
        auto s2 = CxString::toHexString(bytes, false);
        benchmark::DoNotOptimize(s1);
        benchmark::DoNotOptimize(s2);
    }
    st.SetBytesProcessed(st.iterations() * n * 2);
}

BENCHMARK(BM_toHexString_vec)->RangeMultiplier(4)->Range(64, 1 << 20);

static void BM_fromHexString_hexToData(benchmark::State &st)
{
    size_t n = static_cast<size_t>(st.range(0));
    auto bytes = rand_bytes(n);
    auto spaced = CxString::toHexString(bytes, true);
    auto nospace = CxString::toHexString(bytes, false);
    string weird = nospace + "Zz"; // 非法字符尾部
    for (auto _: st)
    {
        auto v1 = CxString::fromHexString(spaced);
        auto v2 = CxString::fromHexString(nospace);
        auto v3 = CxString::hexToData(weird);
        benchmark::DoNotOptimize(v1);
        benchmark::DoNotOptimize(v2);
        benchmark::DoNotOptimize(v3);
    }
    st.SetBytesProcessed(st.iterations() * n * 3);
}

BENCHMARK(BM_fromHexString_hexToData)->RangeMultiplier(4)->Range(64, 1 << 20);

static void BM_hexToIntegers(benchmark::State &st)
{
    bool ok = false;
    const string samples[] = {"FF", "ffffffff", "7fffffff", "FFFFFFFFFFFFFFFF", "xyz"};
    for (auto _: st)
    {
        for (auto &s: samples)
        {
            auto a = CxString::hexToInt32(s, &ok);
            auto b = CxString::hexToUint32(s, &ok);
            auto c = CxString::hexToInt64(s, &ok);
            auto d = CxString::hexToUint64(s, &ok);
            benchmark::DoNotOptimize(a);
            benchmark::DoNotOptimize(b);
            benchmark::DoNotOptimize(c);
            benchmark::DoNotOptimize(d);
            benchmark::DoNotOptimize(ok);
        }
    }
}

BENCHMARK(BM_hexToIntegers);

// 标量 toHexString（带/不带0x）
static void BM_toHex_scalars(benchmark::State &st)
{
    int i = -1;
    cx::uint32 u = 0xDEADBEEF;
    cx::int64 l = -0x112233445566778;
    cx::uint64 ul = 0xFEDCBA9876543210ULL;
    float f = 3.14f;
    double d = 6.28;
    for (auto _: st)
    {
        auto s1 = CxString::toHexString(i);
        auto s2 = CxString::toHexString(u);
        auto s3 = CxString::toHexString(l);
        auto s4 = CxString::toHexString(ul);
        auto s5 = CxString::toHexString(f);
        auto s6 = CxString::toHexString(d);
        auto s7 = CxString::toHexString(i, true);
        auto s8 = CxString::toHexString(u, true);
        benchmark::DoNotOptimize(s1);
        benchmark::DoNotOptimize(s2);
        benchmark::DoNotOptimize(s3);
        benchmark::DoNotOptimize(s4);
        benchmark::DoNotOptimize(s5);
        benchmark::DoNotOptimize(s6);
        benchmark::DoNotOptimize(s7);
        benchmark::DoNotOptimize(s8);
    }
}

BENCHMARK(BM_toHex_scalars);

// ========== 5) 分割 / 合并 / 插入分隔 / 统计 ==========

static void BM_insertSplit(benchmark::State &st)
{
    size_t n = static_cast<size_t>(st.range(0));
    string s = CxString::toHexString(rand_bytes(n), false);
    for (auto _: st)
    {
        auto r1 = CxString::insertSplit(s, '-', 2);
        auto r2 = CxString::insertSplit(s, ' ', 3);
        benchmark::DoNotOptimize(r1);
        benchmark::DoNotOptimize(r2);
    }
    st.SetBytesProcessed(st.iterations() * s.size());
}

BENCHMARK(BM_insertSplit)->RangeMultiplier(4)->Range(64, 1 << 20);

static void BM_split_char_string(benchmark::State &st)
{
    size_t t = static_cast<size_t>(st.range(0));
    auto vs = mk_tokens(t);
    string s_char = join_with(vs, ',');
    string s_str = join_with(vs, '|');
    for (auto _: st)
    {
        auto v1 = CxString::split(s_char, ',');
        auto v2 = CxString::split(s_str, "|");
        auto v3 = CxString::splitCase("AaAXYZBbBXYZcC", "xyz");
        benchmark::DoNotOptimize(v1);
        benchmark::DoNotOptimize(v2);
        benchmark::DoNotOptimize(v3);
    }
    st.SetItemsProcessed(st.iterations() * t * 3);
}

BENCHMARK(BM_split_char_string)->RangeMultiplier(4)->Range(8, 8 << 12);

// 多分隔符
static void BM_splitByDelimiters(benchmark::State &st)
{
    size_t t = static_cast<size_t>(st.range(0));
    auto vs = mk_tokens(t);
    string s = join_with(vs, ',');
    std::replace(s.begin(), s.end(), ',', (t % 2) ? ';' : ','); // 混合 , ;
    for (auto _: st)
    {
        auto v = CxString::splitByDelimiters(s, ",;");
        benchmark::DoNotOptimize(v);
    }
    st.SetItemsProcessed(st.iterations() * t);
}

BENCHMARK(BM_splitByDelimiters)->RangeMultiplier(4)->Range(8, 8 << 12);

// map/lines 系列
static void BM_splitToMap_lines(benchmark::State &st)
{
    size_t t = static_cast<size_t>(st.range(0));
    string kv = mk_kv_line(t);
    for (auto _: st)
    {
        auto m1 = CxString::splitToMap(kv, '=', ',', true);
        auto m2 = CxString::splitToMap(kv, '=', ',', false);
        auto m3 = CxString::splitToMap_mix(kv, '=', ',');
        auto m4 = CxString::splitToMap_reverse(kv, '=', ',');
        auto lines = CxString::splitToLines(kv, '=', ',');
        benchmark::DoNotOptimize(m1);
        benchmark::DoNotOptimize(m2);
        benchmark::DoNotOptimize(m3);
        benchmark::DoNotOptimize(m4);
        benchmark::DoNotOptimize(lines);
    }
    st.SetItemsProcessed(st.iterations() * t);
}

BENCHMARK(BM_splitToMap_lines)->RangeMultiplier(4)->Range(8, 8 << 12);

static void BM_join_variants(benchmark::State &st)
{
    size_t t = static_cast<size_t>(st.range(0));
    auto vs = mk_tokens(t);
    std::map<string, string> m;
    for (size_t i = 0; i < t; ++i) m["k" + std::to_string(i)] = "v" + std::to_string(i);
    for (auto _: st)
    {
        auto s1 = CxString::join(vs, ',');
        auto s2 = CxString::join(vs, string("||"));
        auto s3 = CxString::join(m, '=', ',');
        auto s4 = CxString::join(m, "==", "&&");
        auto l = CxString::joinToStringList(m, "=");
        benchmark::DoNotOptimize(s1);
        benchmark::DoNotOptimize(s2);
        benchmark::DoNotOptimize(s3);
        benchmark::DoNotOptimize(s4);
        benchmark::DoNotOptimize(l);
    }
    st.SetItemsProcessed(st.iterations() * t);
}

BENCHMARK(BM_join_variants)->RangeMultiplier(4)->Range(8, 8 << 12);

// count
static void BM_count(benchmark::State &st)
{
    size_t n = static_cast<size_t>(st.range(0));
    auto s = rand_ascii(n);
    for (auto _: st)
    {
        auto c1 = CxString::count(s, ',');
        auto c2 = CxString::count(s, "ab");
        benchmark::DoNotOptimize(c1);
        benchmark::DoNotOptimize(c2);
    }
    st.SetBytesProcessed(st.iterations() * n);
}

BENCHMARK(BM_count)->RangeMultiplier(4)->Range(64, 1 << 20);

// ========== 6) 替换 / 裁剪 / 大小写 / 等价 / 忽略空白 ==========

static void BM_replace_trim_case(benchmark::State &st)
{
    size_t n = static_cast<size_t>(st.range(0));
    auto s = rand_ascii(n);
    for (auto _: st)
    {
        auto r1 = CxString::replace(s, 'a', 'A');
        auto r2 = CxString::replace(s, "abc", "XYZ");
        auto r3 = CxString::replaceCase(s, "AbC", "xyz");
        auto t1 = CxString::trim("   \t\r\n" + s + "  ");
        auto t2 = CxString::trim(s, ' ');
        auto t3 = CxString::trim(s, string(" \t\r\n"));
        auto t4 = CxString::trim(s, ' ');
        string l = CxString::toLowerAscii(s);
        string u = CxString::toUpperAscii(s);
        benchmark::DoNotOptimize(r1);
        benchmark::DoNotOptimize(r2);
        benchmark::DoNotOptimize(r3);
        benchmark::DoNotOptimize(t1);
        benchmark::DoNotOptimize(t2);
        benchmark::DoNotOptimize(t3);
        benchmark::DoNotOptimize(t4);
        benchmark::DoNotOptimize(l);
        benchmark::DoNotOptimize(u);
    }
    st.SetBytesProcessed(st.iterations() * n);
}

BENCHMARK(BM_replace_trim_case)->RangeMultiplier(4)->Range(64, 1 << 20);

static void BM_equal_exist_begin_end(benchmark::State &st)
{
    const string a = "Hello  World", b = "hello  world", sub = "world";
    for (auto _: st)
    {
        bool e1 = CxString::equal(a, b);
        bool e2 = CxString::equalCase("a b\tc", "ab c");
        bool f1 = CxString::exist(a, "World");
        bool f2 = CxString::existCase(a, "world");
        bool bw = CxString::beginWith(a, "Hello");
        bool bwc = CxString::beginWithCase(a, "hello");
        bool ew = CxString::endWith(a, "World");
        bool ewc = CxString::endWithCase(a, "world");
        benchmark::DoNotOptimize(e1);
        benchmark::DoNotOptimize(e2);
        benchmark::DoNotOptimize(f1);
        benchmark::DoNotOptimize(f2);
        benchmark::DoNotOptimize(bw);
        benchmark::DoNotOptimize(bwc);
        benchmark::DoNotOptimize(ew);
        benchmark::DoNotOptimize(ewc);
    }
}

BENCHMARK(BM_equal_exist_begin_end);

// ========== 7) 查找 & Token & Quote & Format & Find ==========

static void BM_findLeftRight(benchmark::State &st)
{
    const string s = "abcXYZdefXYZghi";
    vector<string> subs = {"XYZ", "def", "none"};
    for (auto _: st)
    {
        auto p1 = CxString::findLeftCase(s, "xyz");
        auto p2 = CxString::findRightCase(s, "xyz");
        auto p3 = CxString::findLeftCase(s, subs);
        auto p4 = CxString::findLeftCase(vector<string>{"aaa", "bbb", "ccc"}, string("BBB"));
        auto p5 = CxString::findEqualCase(vector<string>{"aaa", "bbb", "ccc"}, string("BBB"));
        benchmark::DoNotOptimize(p1);
        benchmark::DoNotOptimize(p2);
        benchmark::DoNotOptimize(p3);
        benchmark::DoNotOptimize(p4);
        benchmark::DoNotOptimize(p5);
    }
}

BENCHMARK(BM_findLeftRight);

static void BM_token_series(benchmark::State &st)
{
    string s1 = "aaaaa.12341234.bbbbbbbbb";
    string s2 = s1, s3 = s1, s4 = s1, s5 = s1, s6 = s1;
    bool ok = false;
    for (auto _: st)
    {
        auto r = CxString::tokenLeft(s1, '.');
        auto r2 = CxString::tokenLeft(s2, ".");
        auto l = CxString::tokenLeft(s3, '.');
        auto l2 = CxString::tokenLeft(s4, ".");
        auto r3 = CxString::tokenRight(s5, '.');
        auto r4 = CxString::tokenRight(s6, ".");
        benchmark::DoNotOptimize(r);
        benchmark::DoNotOptimize(r2);
        benchmark::DoNotOptimize(l);
        benchmark::DoNotOptimize(l2);
        benchmark::DoNotOptimize(r3);
        benchmark::DoNotOptimize(r4);
        benchmark::DoNotOptimize(ok);
    }
}

BENCHMARK(BM_token_series);

static void BM_quote_unquote(benchmark::State &st)
{
    const string s = "[hello](world)<ok>";
    for (auto _: st)
    {
        auto u1 = CxString::unquote(s, '[', ']');
        auto u2 = CxString::unquote(s, '(', ')');
        auto u3 = CxString::unquote(s, "[", "]");
        auto u4 = CxString::unquote(s, 0, "[", "]", nullptr);
        auto vs = CxString::unquotes("a(b)c(d)e", "(", ")");
        benchmark::DoNotOptimize(u1);
        benchmark::DoNotOptimize(u2);
        benchmark::DoNotOptimize(u3);
        benchmark::DoNotOptimize(u4);
        benchmark::DoNotOptimize(vs);
    }
}

BENCHMARK(BM_quote_unquote);

static void BM_format_find(benchmark::State &st)
{
    for (auto _: st)
    {
        auto s = CxString::format("val=%d, hex=0x%X, str=%s", 123, 0xABCD, "ok");
        int pos = 0, len = 0;
        string des;
        bool f = CxString::find("prefix[VALUE]suffix", "[", "]", pos, len, des);
        benchmark::DoNotOptimize(s);
        benchmark::DoNotOptimize(f);
        benchmark::DoNotOptimize(pos);
        benchmark::DoNotOptimize(len);
        benchmark::DoNotOptimize(des);
    }
}

BENCHMARK(BM_format_find);

// ========== 8) CString 族 & Buffer -> Uint32 ==========

static void BM_length_copy_CString(benchmark::State &st)
{
    const char *a = "hello";
    const char *b = "world";
    const char *c = "中文";
    char buf[256];
    for (auto _: st)
    {
        int L = CxString::lengthCString(3, a, b, c);
        int n = CxString::copyCString(buf, 3, a, b, c);
        benchmark::DoNotOptimize(L);
        benchmark::DoNotOptimize(n);
        benchmark::DoNotOptimize(buf[0]);
    }
}

BENCHMARK(BM_length_copy_CString);

static void BM_toUint32_buffer(benchmark::State &st)
{
    size_t n = static_cast<size_t>(st.range(0));
    auto bytes = rand_bytes(n);
    auto ub = reinterpret_cast<cx::uchar *>(bytes.data());
    for (auto _: st)
    {
        auto v1 = CxString::toUint32(bytes.data(), (int) n, true);
        auto v2 = CxString::toUint32(ub, (int) n, false);
        benchmark::DoNotOptimize(v1);
        benchmark::DoNotOptimize(v2);
    }
    st.SetBytesProcessed(st.iterations() * n * 2);
}

BENCHMARK(BM_toUint32_buffer)->RangeMultiplier(4)->Range(4, 1 << 20);

// ========== 9) IP / Port / Path / FileName / CodeName / HexChar 校验 ==========

static void BM_validations(benchmark::State &st)
{
    const string ips[] = {"127.0.0.1", "192.168.1.1", "256.1.1.1", "a.b.c.d", "0.0.0.0"};
    const int ports[] = {0, 80, 65535, -1, 70000};
    const string paths[] = {
#ifdef _WIN32
            "C:\\Windows\\System32\\drivers\\etc\\hosts",
            "C:\\Program Files\\",
            "C:\\Invalid|Name"
#else
            "/usr/local/bin", "/tmp", "/inv:alid"
#endif
    };
    const string names[] = {"OK_Name-1", "bad name", "中文名", "_code1", "1*bad"};
    const string hexes[] = {"0123ABCD", "01 23 ab cd", "zz", ""};
    for (auto _: st)
    {
        for (auto &s: ips) benchmark::DoNotOptimize(CxString::isValidIp(s));
        for (auto p: ports)benchmark::DoNotOptimize(CxString::isValidPort(p));
        for (auto &s: paths)benchmark::DoNotOptimize(CxString::isValidPath(s));
        for (auto &s: paths)benchmark::DoNotOptimize(CxString::isValidFullPath(s));
        for (auto &s: paths)benchmark::DoNotOptimize(CxString::isValidFileName(s));
        for (auto &s: paths)benchmark::DoNotOptimize(CxString::isValidPathName(s));
        for (auto &s: names)benchmark::DoNotOptimize(CxString::isValidCodeName(s));
        for (auto &s: hexes)benchmark::DoNotOptimize(CxString::isValidHexCharater(s));
    }
}

BENCHMARK(BM_validations);

// ========== 10) 拷贝/统计/构造新串 ==========

static void BM_copy_size_newStrings(benchmark::State &st)
{
    size_t t = static_cast<size_t>(st.range(0));
    auto vs = mk_tokens(t);
    std::vector<std::vector<std::string>> lines = {vs, vs, vs};

    const int need = CxString::sizeOf(vs) + 64;   // 预留一点
    std::vector<char> buf((std::max)(need, 1));  // 堆分配

    for (auto _: st)
    {
        int sz = CxString::sizeOf(vs);
        // 若预估不够就扩容（极端情况下）
        if (sz > static_cast<int>(buf.size())) buf.resize(sz);

        int n = CxString::copyTo(vs, buf.data(), static_cast<int>(buf.size()));
        auto v1 = CxString::newStrings(vs);
        auto j = CxString::join(lines, '=', ',');
        auto s2 = CxString::newString("abc", 2);
        auto s3 = CxString::newString("abcdef");
        benchmark::DoNotOptimize(sz);
        benchmark::DoNotOptimize(n);
        benchmark::DoNotOptimize(v1);
        benchmark::DoNotOptimize(j);
        benchmark::DoNotOptimize(s2);
        benchmark::DoNotOptimize(s3);
    }
    st.SetItemsProcessed(st.iterations() * static_cast<long long>(t));
}

BENCHMARK(BM_copy_size_newStrings)->RangeMultiplier(4)->Range(8, 8 << 12);

// ========== main ==========

BENCHMARK_MAIN();
