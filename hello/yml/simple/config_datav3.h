#ifndef CX_CT_X2_CONFIG_DATAV3_H
#define CX_CT_X2_CONFIG_DATAV3_H

#include <string>
#include <vector>
#include <optional>
#include <limits>

namespace cfgx {

    // 字段类型
    enum class ValueKind {
        kInt, kDouble, kBool, kString, kStringArray, kObject, kObjectArray
    };

    // —— 互斥的主语义（只能选一个） ——
    enum class PrimarySemantic {
        None = 0,
        DateShort,     // YYYY-MM-DD
        DateTimeLong,  // YYYY-MM-DD HH:mm:ss
        IPAddress,     // IPv4/IPv6
        HostName,      // 域名
        Port,          // 1..65535
        FilePath,      // 系统文件路径
        Email,         // 邮箱
        RegionCode,    // 国家/地区码
        URLPath        // URL 路径 (/api/xxx)
    };

    // 校验规则
    struct Validation {
        std::optional<long long> minInt, maxInt;
        std::optional<double>    minReal, maxReal;
        std::optional<std::string> regex;
        std::vector<std::string>   enumSet;

        bool nonEmpty    = false;
        bool uniqueItems = false;

        PrimarySemantic primary = PrimarySemantic::None;

        // ===== 静态构造 =====
        static Validation IntRange(long long minV, long long maxV,
                                   PrimarySemantic ps = PrimarySemantic::None) {
            Validation v; v.minInt = minV; v.maxInt = maxV; v.primary = ps; return v;
        }
        static Validation RealRange(double minV, double maxV,
                                    PrimarySemantic ps = PrimarySemantic::None) {
            Validation v; v.minReal = minV; v.maxReal = maxV; v.primary = ps; return v;
        }
        static Validation StringRegex(const std::string& re,
                                      bool nonEmpty = true,
                                      PrimarySemantic ps = PrimarySemantic::None) {
            Validation v; v.regex = re; v.nonEmpty = nonEmpty; v.primary = ps; return v;
        }
        static Validation EnumSet(const std::vector<std::string>& es,
                                  bool nonEmpty = true,
                                  PrimarySemantic ps = PrimarySemantic::None) {
            Validation v; v.enumSet = es; v.nonEmpty = nonEmpty; v.primary = ps; return v;
        }
        static Validation Array(bool nonEmpty = false,
                                bool uniqueItems = false,
                                PrimarySemantic ps = PrimarySemantic::None) {
            Validation v; v.nonEmpty = nonEmpty; v.uniqueItems = uniqueItems; v.primary = ps; return v;
        }
    };

    // 字段元信息
    struct FieldMeta {
        std::string   key;
        std::string   displayName;
        std::string   comment;
        bool          required = false;
        ValueKind     kind     = ValueKind::kString;
        Validation    v;
    };

    // 对象元信息
    struct ObjectMeta {
        std::string            typeName;
        std::string            displayName;
        std::string            comment;
        std::vector<FieldMeta> fields;
    };

} // namespace cfgx


// ========== 示例配置结构 ==========

namespace cfg {

    // 用来表示基础字段，和你 v2 里的一样（这里只声明）
    template<typename T> struct FieldValue {
        std::string key;
        T defaultValue{};
        bool required;
        std::string comment;
    };
    template<typename T> struct FieldArray {
        std::string key;
        std::vector<T> defaultValue{};
        bool required;
        std::string comment;
    };
    template<typename T> struct FieldObject {
        std::string key;
    };
    template<typename T> struct FieldArrayObject {
        std::string key;
    };

    struct ConfigItem {
        template<typename F> void registerField(F&) {}
    };

    using Int = int;

} // namespace cfg


// ========== 日志子系统配置 ==========

struct RotationPolicy : public cfg::ConfigItem {
    cfg::FieldValue<cfg::Int>  f_maxSizeMB   {"maxSizeMB",  100,  false, "单文件最大(MB)"};
    cfg::FieldValue<cfg::Int>  f_keepFiles   {"keepFiles",  7,    false, "最多保留文件数"};
    cfg::FieldValue<bool>      f_dailyRotate {"dailyRotate", true, false, "是否按天滚动"};

    RotationPolicy() {
        registerField(f_maxSizeMB);
        registerField(f_keepFiles);
        registerField(f_dailyRotate);
    }

    static inline const cfgx::ObjectMeta kMeta {
            "RotationPolicy", "日志滚动策略", "控制单文件大小、保留数量与按天滚动。",
            {
                    { "maxSizeMB", "单文件最大(MB)", "范围 1~4096 MB；用于触发滚动阈值。",
                      false, cfgx::ValueKind::kInt,
                      cfgx::Validation::IntRange(1, 4096, cfgx::PrimarySemantic::FilePath) },
                    { "keepFiles", "最多保留文件数", "范围 1~1000。",
                      false, cfgx::ValueKind::kInt,
                      cfgx::Validation::IntRange(1, 1000) },
                    { "dailyRotate", "是否按天滚动", "true=每天新建；false=仅按大小。",
                      false, cfgx::ValueKind::kBool, cfgx::Validation{} }
            }
    };
};

struct SinkConfig : public cfg::ConfigItem {
    cfg::FieldValue<std::string> f_type  {"type", "file", false, "sink 类型:file/console/tcp"};
    cfg::FieldValue<std::string> f_level {"level", "info", false, "最低级别"};
    cfg::FieldObject<RotationPolicy> f_rotation {"rotation"};
    cfg::FieldArray<std::string> f_tags {"tags", {"main"}, false, "标签"};

    SinkConfig() {
        registerField(f_type);
        registerField(f_level);
        registerField(f_rotation);
        registerField(f_tags);
    }

    static inline const cfgx::ObjectMeta kMeta {
            "SinkConfig", "日志输出(Sink)", "支持 file/console/tcp。",
            {
                    { "type", "输出类型", "允许值: file / console / tcp。",
                      false, cfgx::ValueKind::kString,
                      cfgx::Validation::EnumSet({"file","console","tcp"}) },
                    { "level", "最低日志级别", "允许值: trace/debug/info/warn/error。",
                      false, cfgx::ValueKind::kString,
                      cfgx::Validation::EnumSet({"trace","debug","info","warn","error"}) },
                    { "rotation", "文件滚动策略", "仅 file 类型使用。",
                      false, cfgx::ValueKind::kObject, cfgx::Validation{} },
                    { "tags", "标签", "非空且唯一。",
                      false, cfgx::ValueKind::kStringArray,
                      cfgx::Validation::Array(true, true) }
            }
    };
};

struct LoggerConfig : public cfg::ConfigItem {
    cfg::FieldValue<std::string> f_name  {"name",  "app",  false, "日志器名称"};
    cfg::FieldValue<std::string> f_format{"format","%Y-%m-%d %H:%M:%S [%l] %v", false, "格式模板"};
    cfg::FieldArrayObject<SinkConfig> f_sinks {"sinks"};
    cfg::FieldArray<std::string> f_modules{"modules", {"core","net"}, false, "启用模块过滤"};

    LoggerConfig() {
        registerField(f_name);
        registerField(f_format);
        registerField(f_sinks);
        registerField(f_modules);
    }

    static inline const cfgx::ObjectMeta kMeta {
            "LoggerConfig", "日志器(Logger)", "日志器包含输出与模块过滤。",
            {
                    { "name", "日志器名称", "非空字符串。",
                      false, cfgx::ValueKind::kString,
                      cfgx::Validation::StringRegex("^[A-Za-z0-9_\\-]{1,64}$") },
                    { "format", "格式模板", "如 %Y-%m-%d %H:%M:%S [%l] %v。",
                      false, cfgx::ValueKind::kString,
                      cfgx::Validation::StringRegex(".+") },
                    { "sinks", "输出列表", "至少一个。",
                      false, cfgx::ValueKind::kObjectArray,
                      cfgx::Validation::Array(true) },
                    { "modules", "模块过滤", "唯一；可空。",
                      false, cfgx::ValueKind::kStringArray,
                      cfgx::Validation::Array(false, true) }
            }
    };
};

struct ServiceEndpoint : public cfg::ConfigItem {
    cfg::FieldValue<std::string> f_host {"host", "127.0.0.1", false, "主机"};
    cfg::FieldValue<cfg::Int>    f_port {"port", 8080, true, "端口"};
    cfg::FieldArray<std::string> f_paths{"paths",{"/api","/health"}, false, "路径白名单"};

    ServiceEndpoint() {
        registerField(f_host); registerField(f_port); registerField(f_paths);
    }

    static inline const cfgx::ObjectMeta kMeta {
            "ServiceEndpoint", "服务端点", "服务访问配置。",
            {
                    { "host", "主机", "IPv4/IPv6/域名。",
                      false, cfgx::ValueKind::kString,
                      cfgx::Validation::StringRegex(".+", true, cfgx::PrimarySemantic::HostName) },
                    { "port", "端口", "1~65535。",
                      true, cfgx::ValueKind::kInt,
                      cfgx::Validation::IntRange(1, 65535, cfgx::PrimarySemantic::Port) },
                    { "paths", "路径白名单", "非空唯一，以 / 开头。",
                      false, cfgx::ValueKind::kStringArray,
                      cfgx::Validation::Array(true, true, cfgx::PrimarySemantic::URLPath) }
            }
    };
};

struct AppConfig : public cfg::ConfigItem {
    cfg::FieldValue<bool>   f_enable {"enable", true, false, "是否启用"};
    cfg::FieldValue<double> f_timeout{"timeoutSec", 3.5, false, "全局超时(秒)"};
    cfg::FieldValue<cfg::Int> f_version{"configVersion", 3, false, "版本"};

    cfg::FieldObject<ServiceEndpoint> f_endpoint {"endpoint"};
    cfg::FieldArrayObject<LoggerConfig> f_loggers {"loggers"};
    cfg::FieldArray<std::string> f_plugins{"plugins", {"metrics","trace"}, false, "插件名"};

    AppConfig() {
        registerField(f_enable);
        registerField(f_timeout);
        registerField(f_version);
        registerField(f_endpoint);
        registerField(f_loggers);
        registerField(f_plugins);
    }

    static inline const cfgx::ObjectMeta kMeta {
            "AppConfig", "应用配置", "应用全局参数与子模块配置。",
            {
                    { "enable", "是否启用", "true/false。",
                      false, cfgx::ValueKind::kBool, cfgx::Validation{} },
                    { "timeoutSec", "全局超时", "范围 0~600 秒。",
                      false, cfgx::ValueKind::kDouble,
                      cfgx::Validation::RealRange(0.0, 600.0, cfgx::PrimarySemantic::DateTimeLong) },
                    { "configVersion", "配置版本", "正整数。",
                      false, cfgx::ValueKind::kInt,
                      cfgx::Validation::IntRange(1, std::numeric_limits<int>::max()) },
                    { "endpoint", "服务端点", "主机/端口/路径。",
                      false, cfgx::ValueKind::kObject, cfgx::Validation{} },
                    { "loggers", "日志器列表", "至少一个。",
                      false, cfgx::ValueKind::kObjectArray,
                      cfgx::Validation::Array(true) },
                    { "plugins", "插件名", "非空唯一。",
                      false, cfgx::ValueKind::kStringArray,
                      cfgx::Validation::Array(true, true) }
            }
    };
};

#endif // CX_CT_X2_CONFIG_DATAV3_H
