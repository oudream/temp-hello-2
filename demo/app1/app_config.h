#ifndef CX_CT_X2_APP_CONFIG_H
#define CX_CT_X2_APP_CONFIG_H

#include <ccxx/configuration.h>

// -----------------------------
// FILE: include/example_app_config.hpp
// -----------------------------

struct RotationPolicy : public cfg::ConfigItem {
    cfg::FieldValue<int> maxSizeMB{"maxSizeMB", 100, false, "单文件最大(MB)", "1~4096",
                                   cfg::Validation::IntRange(1, 4096)};

    cfg::FieldValue<int> keepFiles{"keepFiles", 7, false, "最多保留", "1~1000",
                                   cfg::Validation::IntRange(1, 1000)};

    cfg::FieldValue<bool> dailyRotate{"dailyRotate", true, false, "是否按天", "true/false",
                                      cfg::Validation{}};

    const cfg::ConfigMeta &meta() const override {
        static const cfg::ConfigMeta m{"RotationPolicy", "日志滚动策略", "控制单文件大小、保留数量与按天滚动"};
        return m;
    }

    RotationPolicy() {
        registerField(&RotationPolicy::maxSizeMB);
        registerField(&RotationPolicy::keepFiles);
        registerField(&RotationPolicy::dailyRotate);
    }
};

struct SinkConfig : public cfg::ConfigItem {
    cfg::FieldValue<std::string> id{"id", "main", false, "Sink标识", "作为 Key（可用于引用）",
                                    cfg::Validation::StringRegex("^[A-Za-z0-9_\\-]{1,64}$")};

    cfg::FieldValue<std::string> type{"type", "file", false, "输出类型", "file/console/tcp",
                                      cfg::Validation::EnumSet({"file", "console", "tcp"})};

    cfg::FieldValue<std::string> level{"level", "info", false, "最低日志级别",
                                       "trace/debug/info/warn/error/critical/off",
                                       cfg::Validation::EnumSet(
                                               {"trace", "debug", "info", "warn", "error", "critical", "off"})};

    cfg::FieldObject<RotationPolicy> rotation{"rotation", false, "文件滚动策略", "type=file 时有效",
                                              cfg::Validation{}};

    cfg::FieldArray<std::string> tags{"tags", {"main"}, false, "标签", "非空且唯一",
                                      cfg::Validation::Array(true, true)};

    const cfg::ConfigMeta &meta() const override {
        static const cfg::ConfigMeta m{"SinkConfig", "日志输出(Sink)", "支持 file/console/tcp；file 可配置滚动策略"};
        return m;
    }

    SinkConfig() {
        registerField(&SinkConfig::id);
        registerField(&SinkConfig::type);
        registerField(&SinkConfig::level);
        registerField(&SinkConfig::rotation);
        registerField(&SinkConfig::tags);
    }
};

struct LoggerConfig : public cfg::ConfigItem {
    cfg::FieldValue<std::string> name{"name", "app", false, "日志器名称", "Key",
                                      cfg::Validation::StringRegex("^[A-Za-z0-9_\\-]{1,64}$")};

    cfg::FieldValue<std::string> format{"format", "%Y-%m-%dT%H:%M:%S [%l] %v", false, "格式模板", "",
                                        cfg::Validation::StringRegex(".+")};

    cfg::FieldValue<int> timeLong{"timeLong", 11, false, "时长", "",
                                  cfg::Validation::IntRange(0, 100)};

    cfg::FieldValue<int> timeCost{"timeCost", 99, false, "耗时", "",
                                  cfg::Validation::IntRange(0, 100)};

    cfg::FieldArrayObject<SinkConfig> sinks{"sinks", true, "输出列表", "至少一个",
                                            cfg::Validation::Array(true)};

    cfg::FieldArray<std::string> modules{"modules", {"core", "net"}, false, "模块过滤", "可为空；若非空需唯一",
                                         cfg::Validation::Array(false, true)};

    const cfg::ConfigMeta &meta() const override {
        static const cfg::ConfigMeta m{"LoggerConfig", "日志器(Logger)", "日志器包含若干输出(sinks)与模块过滤"};
        return m;
    }

    LoggerConfig() {
        registerField(&LoggerConfig::name);
        registerField(&LoggerConfig::format);
        registerField(&LoggerConfig::timeLong);
        registerField(&LoggerConfig::timeCost);
        registerField(&LoggerConfig::sinks);
        registerField(&LoggerConfig::modules);
    }
};

struct ServiceEndpoint : public cfg::ConfigItem {
    cfg::FieldValue<std::string> host{"host", "127.0.0.1", false, "主机", "主机名或 IP",
                                      cfg::Validation::StringRegex(R"(^.+$)", true, cfg::PrimarySemantic::HostName)};

    cfg::FieldValue<int> port{"port", 8080, true, "端口", "1~65535",
                              cfg::Validation::IntRange(1, 65535, cfg::PrimarySemantic::Port)};

    cfg::FieldArray<std::string> paths{"paths", {"/api", "/health"}, false, "路径白名单", "每项形如 /xxx; 非空且唯一",
                                       [] {
                                           auto v = cfg::Validation::Array(true, true, cfg::PrimarySemantic::URLPath);
                                           v.regex = "^/.*$";
                                           return v;
                                       }()};

    const cfg::ConfigMeta &meta() const override {
        static const cfg::ConfigMeta m{"ServiceEndpoint", "服务端点", "主机/端口/路径白名单"};
        return m;
    }

    ServiceEndpoint() {
        registerField(&ServiceEndpoint::host);
        registerField(&ServiceEndpoint::port);
        registerField(&ServiceEndpoint::paths);
    }
};

struct NumericZooConfig : public cfg::ConfigItem {
    // 标量（显式用 signed/unsigned 规避 char 签名性差异）
    cfg::FieldValue<signed char>         i8 {"i8",   0,   false, "有符号8位",  "-128~127",
                                             cfg::Validation::IntRange((std::numeric_limits<signed char>::min)(),
                                                                       (std::numeric_limits<signed char>::max)())};

    cfg::FieldValue<unsigned char>       u8 {"u8",   255, false, "无符号8位",   "0~255",
                                             cfg::Validation::IntRange(0,
                                                                       (std::numeric_limits<unsigned char>::max)())};

    cfg::FieldValue<short>               i16{"i16",  -12, false, "有符号16位", "-32768~32767",
                                             cfg::Validation::IntRange((std::numeric_limits<short>::min)(),
                                                                       (std::numeric_limits<short>::max)())};

    cfg::FieldValue<unsigned short>      u16{"u16",  1024,false, "无符号16位",  "0~65535",
                                             cfg::Validation::IntRange(0,
                                                                       (std::numeric_limits<unsigned short>::max)())};

    cfg::FieldValue<int>                 i32{"i32",  -99, false, "有符号32位", "任意int",
                                             cfg::Validation::IntRange((std::numeric_limits<int>::min)(),
                                                                       (std::numeric_limits<int>::max)())};

    cfg::FieldValue<unsigned int>        u32{"u32",  123u,false, "无符号32位", "0~2^32-1",
                                             cfg::Validation::IntRange(0,
                                                                       (long long)(std::numeric_limits<unsigned int>::max)())};

    cfg::FieldValue<unsigned long long>  u64{"u64",  9000000000000000000ULL, false, "无符号64位", "0~2^64-1",
                                             cfg::Validation::IntRange(0, // 校验器是带号的，用上界时注意别溢出
                                                                       (long long)(std::numeric_limits<long long>::max)())}; // 仅作存在性演示

    // 浮点
    cfg::FieldValue<float>               f32{"f32",  3.25f, false, "float32",  "有限值",
                                             cfg::Validation::RealRange(-1e9, 1e9)};
    cfg::FieldValue<double>              f64{"f64",  6.5,   false, "float64",  "有限值",
                                             cfg::Validation::RealRange(-1e12, 1e12)};

    // 字符串/布尔
    cfg::FieldValue<std::string>         s {"s", "hello", false, "字符串", "任意非空",
                                            cfg::Validation::StringRegex(".+")};
    cfg::FieldValue<bool>                b {"b", true,    false, "布尔",   "true/false", {}};

    // DateTime（需要你已接了 kDateTime/kDateTimeArray）
    cfg::FieldValue<cx::DateTime>        dt {"dt",  cx::DateTime::invalid(), false,
                                             "时间", "ISO8601(推荐)", {}};
    cfg::FieldArray<cx::DateTime>        dtArr{"dtArr", {}, false, "时间数组", "非空且唯一(示例)",
                                               cfg::Validation::Array(false, true)};

    // 数组（含唯一/非空校验）
    cfg::FieldArray<signed char>         ai8 {"ai8",  { -1, 0, 1 }, false, "i8数组",  "可重复可为空",
                                              cfg::Validation::Array(false, false)};
    cfg::FieldArray<unsigned char>       au8 {"au8",  { 1, 2, 3 },  false, "u8数组",  "唯一",
                                              cfg::Validation::Array(false, true)};
    cfg::FieldArray<short>               ai16{"ai16", { -5, 7 },    false, "i16数组", "非空",
                                              cfg::Validation::Array(true)};
    cfg::FieldArray<unsigned short>      au16{"au16", { 9, 10 },    false, "u16数组", "任意", {}};
    cfg::FieldArray<int>                 ai32{"ai32", { 1, 2, 2 },  false, "i32数组", "允许重复", {}};
    cfg::FieldArray<unsigned int>        au32{"au32", { 3u, 4u },   false, "u32数组", "唯一",
                                              cfg::Validation::Array(false, true)};
    cfg::FieldArray<unsigned long long>  au64{"au64", { 7ULL },     false, "u64数组", "任意", {}};

    cfg::FieldArray<float>               af32{"af32", { 1.25f, 2.5f }, false, "f32数组", "非空",
                                              cfg::Validation::Array(true)};
    cfg::FieldArray<double>              af64{"af64", { 0.5, 1.0 },   false, "f64数组", "唯一",
                                              cfg::Validation::Array(false, true)};

    const cfg::ConfigMeta& meta() const override {
        static const cfg::ConfigMeta m{"NumericZooConfig", "类型覆盖测试", "覆盖整型/浮点/字符串/布尔/时间及数组"};
        return m;
    }

    NumericZooConfig() {
        registerField(&NumericZooConfig::i8);
        registerField(&NumericZooConfig::u8);
        registerField(&NumericZooConfig::i16);
        registerField(&NumericZooConfig::u16);
        registerField(&NumericZooConfig::i32);
        registerField(&NumericZooConfig::u32);
        registerField(&NumericZooConfig::u64);
        registerField(&NumericZooConfig::f32);
        registerField(&NumericZooConfig::f64);
        registerField(&NumericZooConfig::s);
        registerField(&NumericZooConfig::b);
        registerField(&NumericZooConfig::dt);
        registerField(&NumericZooConfig::dtArr);
        registerField(&NumericZooConfig::ai8);
        registerField(&NumericZooConfig::au8);
        registerField(&NumericZooConfig::ai16);
        registerField(&NumericZooConfig::au16);
        registerField(&NumericZooConfig::ai32);
        registerField(&NumericZooConfig::au32);
        registerField(&NumericZooConfig::au64);
        registerField(&NumericZooConfig::af32);
        registerField(&NumericZooConfig::af64);
    }
};

struct AppConfig : public cfg::ConfigItem {
    cfg::FieldValue<bool> enable{"enable", true, false, "是否启用", "true=启用",
                                 cfg::Validation{}};

    cfg::FieldValue<double> timeout{"timeoutSec", 3.5, false, "全局超时(秒)", "0~600",
                                    cfg::Validation::RealRange(0.0, 600.0)};

    cfg::FieldValue<int> version{"configVersion", 6, false, "配置版本", ">=1",
                                 cfg::Validation::IntRange(1, (std::numeric_limits<int>::max)())};

    cfg::FieldValue<long long> buildDate{"buildDate", 0LL, false, "构建日期", "YYYY-MM-DD",
                                         cfg::Validation{}};
//                                         cfg::Validation{.primary=cfg::PrimarySemantic::DateShort}};

    cfg::FieldObject<ServiceEndpoint> endpoint{"endpoint", false, "服务端点", "主机/端口/路径白名单。",
                                               cfg::Validation{}};

    cfg::FieldArrayObject<LoggerConfig> loggers{"loggers", true, "日志器列表", "至少一个。",
                                                cfg::Validation::Array(true)};

    cfg::FieldArray<std::string> plugins{"plugins", {"metrics", "trace"}, false, "插件名", "非空且唯一",
                                         cfg::Validation::Array(true, true)
    };

    cfg::FieldObject<NumericZooConfig> numericZoo{"numericZoo", false, "打靶测试", "覆盖所有数值类型的验证",
                                                  cfg::Validation{}};

    const cfg::ConfigMeta &meta() const override {
        static const cfg::ConfigMeta m{"AppConfig", "应用配置", "应用全局参数与子模块配置入口"};
        return m;
    }

    AppConfig() {
        loggers
                .emplace([](LoggerConfig &l) { l.name = "core"; })
                .emplace([](LoggerConfig &l) { l.name = "ui"; })
                .emplace([](LoggerConfig &l) { l.name = "net"; });

        registerField(&AppConfig::enable);
        registerField(&AppConfig::timeout);
        registerField(&AppConfig::version);
        registerField(&AppConfig::buildDate);
        registerField(&AppConfig::endpoint);
        registerField(&AppConfig::loggers);
        registerField(&AppConfig::plugins);
        registerField(&AppConfig::numericZoo);
    }
};

#endif //CX_CT_X2_APP_CONFIG_H
