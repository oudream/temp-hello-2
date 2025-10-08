#ifndef CX_CT_X2_CONFIG_DATAV2_H
#define CX_CT_X2_CONFIG_DATAV2_H

// ========== 统一的字段元数据与校验规范（仅声明，不实现校验逻辑） ==========

#include <string>
#include <vector>
#include <optional>
#include <limits>

namespace cfgx {

    // 字段类型枚举（可按需扩展）
    enum class ValueKind {
        kInt, kDouble, kBool, kString, kStringArray, kObject, kObjectArray
    };

    // 基础校验：数值范围、字符串正则、枚举集合、非空/唯一等
    struct Validation {
        // 数值范围（整型与浮点型二选一使用）
        std::optional<long long> minInt;
        std::optional<long long> maxInt;
        std::optional<double>    minReal;
        std::optional<double>    maxReal;

        // 字符串/数组校验
        std::optional<std::string> regex;             // 例如: "^[A-Za-z0-9_\\-]+$"
        std::vector<std::string>   enumSet;           // 允许值集合
        bool nonEmpty    = false;                     // 字符串或数组非空
        bool uniqueItems = false;                     // 数组元素唯一（如 tags/modules/plugins）

        // 语义约束（可由上层实现）：端口、路径、主机、时长、文件大小等
        bool asPort              = false; // 1..65535
        bool asMegabytes         = false; // >=1
        bool asSeconds           = false; // >=0
        bool asHostnameOrIP      = false; // host 校验
        bool asURLPath           = false; // "/api" 形态
    };

    // 字段元信息：键名、显示名、注释、是否必填、默认值是否允许、类型与校验
    struct FieldMeta {
        std::string   key;           // JSON/YAML 键名
        std::string   displayName;   // UI显示名（回答“显示名是不是已经有了”：==> 现在“有了”）
        std::string   comment;       // 注释/说明
        bool          required = false;
        ValueKind     kind     = ValueKind::kString;
        Validation    v;             // 合法性规范
    };

    // 结构体元信息：用于为对象/对象数组提供描述
    struct ObjectMeta {
        std::string                typeName;    // C++类型名
        std::string                displayName; // UI显示名
        std::string                comment;     // 注释
        std::vector<FieldMeta>     fields;      // 字段清单
    };

} // namespace cfgx

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

    // —— 元数据（显示名 / 注释 / 合法性）——
    static inline const cfgx::ObjectMeta kMeta {
            /*typeName*/   "RotationPolicy",
            /*displayName*/"日志滚动策略",
            /*comment*/    "控制单文件大小、保留数量与按天滚动。",
            /*fields*/ {
                                   {
                                           "maxSizeMB", "单文件最大(MB)",
                                           "范围 1~4096 MB；用于触发滚动的单文件阈值。",
                                           /*required*/false, cfgx::ValueKind::kInt,
                                           cfgx::Validation{
                                                   /*minInt*/1, /*maxInt*/4096, /*minReal*/std::nullopt, /*maxReal*/std::nullopt,
                                                   /*regex*/std::nullopt, /*enumSet*/{}, /*nonEmpty*/false, /*uniqueItems*/false,
                                                   /*asPort*/false, /*asMegabytes*/true
                                           }
                                   },
                                   {
                                           "keepFiles", "最多保留文件数",
                                           "范围 1~1000；超过后按策略删除最旧文件。",
                                           false, cfgx::ValueKind::kInt,
                                           cfgx::Validation{ 1, 1000 }
                                   },
                                   {
                                           "dailyRotate", "是否按天滚动",
                                           "true=每天新建日志文件；false=仅按大小滚动。",
                                           false, cfgx::ValueKind::kBool,
                                           cfgx::Validation{}
                                   },
                           }
    };
};

// 每个输出（sink）是一个对象，里面还有对象与数组
struct SinkConfig : public cfg::ConfigItem {
    // 基本字段
    cfg::FieldValue<std::string> f_type    {"type",    "file", false, "sink 类型:file/console/tcp"};
    cfg::FieldValue<std::string> f_level   {"level",   "info", false, "最低级别"};
    // file sink 的专属选项（对象）
    cfg::FieldObject<RotationPolicy> f_rotation {"rotation"};
    // 标签（数组）
    cfg::FieldArray<std::string> f_tags {"tags", {"main"}, false, "标签"};

    SinkConfig() {
        registerField(f_type);
        registerField(f_level);
        registerField(f_rotation);
        registerField(f_tags);
    }

    static inline const cfgx::ObjectMeta kMeta {
            "SinkConfig",
            "日志输出(Sink)",
            "支持 file/console/tcp 等类型；当为 file 时可配置滚动策略。",
            {
                    {
                            "type", "输出类型",
                            "允许值: file / console / tcp。",
                            false, cfgx::ValueKind::kString,
                            cfgx::Validation{
                                    /*minInt*/std::nullopt, /*maxInt*/std::nullopt,
                                    /*minReal*/std::nullopt, /*maxReal*/std::nullopt,
                                    /*regex*/std::nullopt,
                                    /*enumSet*/{"file","console","tcp"},
                                    /*nonEmpty*/true
                            }
                    },
                    {
                            "level", "最低日志级别",
                            "允许值: trace/debug/info/warn/error/critical/off。",
                            false, cfgx::ValueKind::kString,
                            cfgx::Validation{
                                    std::nullopt,std::nullopt,std::nullopt,std::nullopt,
                                    std::nullopt,
                                    {"trace","debug","info","warn","error","critical","off"},
                                    true
                            }
                    },
                    {
                            "rotation", "文件滚动策略",
                            "当 type=file 时使用；其他类型可忽略。",
                            false, cfgx::ValueKind::kObject,
                            cfgx::Validation{}
                    },
                    {
                            "tags", "标签",
                            "用于对输出进行分类过滤；必须非空且唯一。",
                            false, cfgx::ValueKind::kStringArray,
                            cfgx::Validation{
                                    std::nullopt,std::nullopt,std::nullopt,std::nullopt,
                                    std::nullopt,{}, /*nonEmpty*/true, /*uniqueItems*/true
                            }
                    }
            }
    };
};

struct LoggerConfig : public cfg::ConfigItem {
    // 简单值
    cfg::FieldValue<std::string> f_name  {"name",  "app",  false, "日志器名称"};
    cfg::FieldValue<std::string> f_format{"format","%Y-%m-%d %H:%M:%S [%l] %v", false, "格式模板"};
    // 对象数组（递归：数组元素是 SinkConfig 对象）
    cfg::FieldArrayObject<SinkConfig> f_sinks {"sinks"};
    // 允许过滤的模块名（数组）
    cfg::FieldArray<std::string> f_modules{"modules", {"core","net"}, false, "启用模块过滤"};

    LoggerConfig() {
        registerField(f_name);
        registerField(f_format);
        registerField(f_sinks);
        registerField(f_modules);
    }

    static inline const cfgx::ObjectMeta kMeta {
            "LoggerConfig",
            "日志器(Logger)",
            "日志器包含若干输出(sinks)与模块过滤。",
            {
                    { "name", "日志器名称",
                      "非空、由字母数字/下划线/中划线组成。",
                      false, cfgx::ValueKind::kString,
                      cfgx::Validation{
                              std::nullopt,std::nullopt,std::nullopt,std::nullopt,
                              "^[A-Za-z0-9_\\-]{1,64}$", {}, true
                      }
                    },
                    { "format", "格式模板",
                      "例如: %Y-%m-%d %H:%M:%S [%l] %v。",
                      false, cfgx::ValueKind::kString,
                      cfgx::Validation{ std::nullopt,std::nullopt,std::nullopt,std::nullopt, std::nullopt, {}, true }
                    },
                    { "sinks", "输出列表",
                      "至少配置一个输出。",
                      false, cfgx::ValueKind::kObjectArray,
                      cfgx::Validation{ std::nullopt,std::nullopt,std::nullopt,std::nullopt, std::nullopt, {}, true /*nonEmpty*/ }
                    },
                    { "modules", "模块名过滤",
                      "用于限定输出的模块集合；可为空；若非空需唯一。",
                      false, cfgx::ValueKind::kStringArray,
                      cfgx::Validation{ std::nullopt,std::nullopt,std::nullopt,std::nullopt, std::nullopt, {}, false, true }
                    },
            }
    };
};

// ========== 应用总配置 ==========

struct ServiceEndpoint : public cfg::ConfigItem {
    cfg::FieldValue<std::string> f_host {"host", "127.0.0.1", false, "主机"};
    cfg::FieldValue<cfg::Int>    f_port {"port", 8080,        true,  "端口"};
    cfg::FieldArray<std::string> f_paths{"paths",{"/api","/health"}, false, "路径白名单"};
    ServiceEndpoint() { registerField(f_host); registerField(f_port); registerField(f_paths); }

    static inline const cfgx::ObjectMeta kMeta {
            "ServiceEndpoint",
            "服务端点",
            "服务访问地址配置。",
            {
                    { "host", "主机",
                      "主机名或IPv4/IPv6；示例 127.0.0.1 / ::1 / example.com。",
                      false, cfgx::ValueKind::kString,
                      cfgx::Validation{ std::nullopt,std::nullopt,std::nullopt,std::nullopt, std::nullopt, {}, true, false, /*asPort*/false, /*asMegabytes*/false, /*asSeconds*/false, /*asHostnameOrIP*/true }
                    },
                    { "port", "端口",
                      "1~65535；建议避免保留端口。",
                      true, cfgx::ValueKind::kInt,
                      cfgx::Validation{ 1, 65535, std::nullopt, std::nullopt, std::nullopt, {}, false, false, /*asPort*/true }
                    },
                    { "paths", "路径白名单",
                      "每项形如“/xxx”；非空、唯一。",
                      false, cfgx::ValueKind::kStringArray,
                      cfgx::Validation{
                              std::nullopt,std::nullopt,std::nullopt,std::nullopt,
                              "^/.*$", {}, true, true, /*asPort*/false, /*asMegabytes*/false, /*asSeconds*/false, /*asHostnameOrIP*/false, /*asURLPath*/true
                      }
                    },
            }
    };
};

struct AppConfig : public cfg::ConfigItem {
    // 简单值
    cfg::FieldValue<bool>   f_enable   {"enable",   true,  false, "是否启用"};
    cfg::FieldValue<double> f_timeout  {"timeoutSec", 3.5, false, "全局超时(秒)"};
    cfg::FieldValue<cfg::Int> f_version{"configVersion", 2, false, "版本"};

    // 嵌套对象
    cfg::FieldObject<ServiceEndpoint> f_endpoint {"endpoint"};

    // 嵌套对象数组（多日志器，每个日志器内还有对象数组/对象/数组）
    cfg::FieldArrayObject<LoggerConfig> f_loggers {"loggers"};

    // 简单数组
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
            "AppConfig",
            "应用配置",
            "应用全局参数与子模块配置入口。",
            {
                    { "enable", "是否启用",
                      "true=启用；false=禁用。",
                      false, cfgx::ValueKind::kBool,
                      cfgx::Validation{}
                    },
                    { "timeoutSec", "全局超时(秒)",
                      "范围 0~600；建议 0~120。",
                      false, cfgx::ValueKind::kDouble,
                      cfgx::Validation{ std::nullopt,std::nullopt, 0.0, 600.0, std::nullopt, {}, false, false, /*asPort*/false, /*asMegabytes*/false, /*asSeconds*/true }
                    },
                    { "configVersion", "配置版本",
                      "正整数；用于兼容升级。",
                      false, cfgx::ValueKind::kInt,
                      cfgx::Validation{ 1, static_cast<long long>(std::numeric_limits<int>::max()) }
                    },
                    { "endpoint", "服务端点",
                      "服务访问主机/端口与路径白名单。",
                      false, cfgx::ValueKind::kObject,
                      cfgx::Validation{}
                    },
                    { "loggers", "日志器列表",
                      "至少一个；用于定义日志输出与格式。",
                      false, cfgx::ValueKind::kObjectArray,
                      cfgx::Validation{ std::nullopt,std::nullopt,std::nullopt,std::nullopt, std::nullopt, {}, true /*nonEmpty*/ }
                    },
                    { "plugins", "插件名",
                      "非空、唯一；建议为短小的标识符（字母数字/下划线/中划线）。",
                      false, cfgx::ValueKind::kStringArray,
                      cfgx::Validation{
                              std::nullopt,std::nullopt,std::nullopt,std::nullopt,
                              "^[A-Za-z0-9_\\-]{1,64}$", {}, true, true
                      }
                    },
            }
    };
};

#endif //CX_CT_X2_CONFIG_DATAV2_H
