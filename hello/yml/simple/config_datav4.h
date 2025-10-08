#ifndef CX_CT_X2_CONFIG_DATAV4_H
#define CX_CT_X2_CONFIG_DATAV4_H

#include <string>
#include <vector>
#include <optional>
#include <limits>
#include <utility>

//
// ========================= cfg：元信息与校验 =========================
//
namespace cfg
{

    // 字段类型（UI/导出可用）
    enum class ValueKind
    {
        kInt, kDouble, kBool, kString,
        kIntArray, kDoubleArray, kStringArray,
        kObject, kObjectArray
    };

    // —— 互斥的主语义（只能选一个）——
    enum class PrimarySemantic
    {
        None = 0,
        DateShort,     // YYYY-MM-DD
        DateTimeLong,  // YYYY-MM-DD HH:mm:ss
        IPAddress,     // IPv4/IPv6
        HostName,      // DNS 主机名
        Port,          // 1..65535
        FilePath,      // 系统路径
        Email,         // 邮箱
        RegionCode,    // 国家/地区码
        URLPath        // /api/xxx
    };

    // —— 校验规则（静态构造）——
    struct Validation
    {
        // 数值范围
        std::optional<long long> minInt, maxInt;
        std::optional<double> minReal, maxReal;

        // 字符串/数组约束
        std::optional<std::string> regex;       // 例如: "^[A-Za-z0-9_\\-]{1,64}$"
        std::vector<std::string> enumSet;     // 允许值集合
        bool nonEmpty = false;               // 字符串或数组非空
        bool uniqueItems = false;               // 数组元素唯一

        // 互斥主语义
        PrimarySemantic primary = PrimarySemantic::None;

        // —— 静态构造（与截图风格一致）——
        static Validation IntRange(long long minV, long long maxV,
                                   PrimarySemantic ps = PrimarySemantic::None)
        {
            Validation v;
            v.minInt = minV;
            v.maxInt = maxV;
            v.primary = ps;
            return v;
        }

        static Validation RealRange(double minV, double maxV,
                                    PrimarySemantic ps = PrimarySemantic::None)
        {
            Validation v;
            v.minReal = minV;
            v.maxReal = maxV;
            v.primary = ps;
            return v;
        }

        static Validation StringRegex(std::string re,
                                      bool nonEmpty = true,
                                      PrimarySemantic ps = PrimarySemantic::None)
        {
            Validation v;
            v.regex = std::move(re);
            v.nonEmpty = nonEmpty;
            v.primary = ps;
            return v;
        }

        static Validation EnumSet(std::vector<std::string> es,
                                  bool nonEmpty = true,
                                  PrimarySemantic ps = PrimarySemantic::None)
        {
            Validation v;
            v.enumSet = std::move(es);
            v.nonEmpty = nonEmpty;
            v.primary = ps;
            return v;
        }

        static Validation Array(bool nonEmpty = false,
                                bool uniqueItems = false,
                                PrimarySemantic ps = PrimarySemantic::None)
        {
            Validation v;
            v.nonEmpty = nonEmpty;
            v.uniqueItems = uniqueItems;
            v.primary = ps;
            return v;
        }
    };




    // —— 字段模板（方式B：把元信息放到字段上）——
    template<typename T>
    struct FieldValue
    {
        std::string key;
        T value{};
        T defaultValue{};
        bool required = false;

        std::string displayName;  // 字段显示名
        std::string comment;      // 字段注释
        Validation validation;            // 校验/语义

        // 让它能像普通类型一样赋值
        FieldValue &operator=(const T &v)
        {
            value = v;
            return *this;
        }

        // 隐式转换成 T，方便直接用
        operator T() const
        { return value; }

        // 类型映射
        static constexpr ValueKind kind =
                std::is_same_v<T, bool> ? ValueKind::kBool :
                std::is_same_v<T, int> ? ValueKind::kInt :
                std::is_same_v<T, long long> ? ValueKind::kInt :
                std::is_same_v<T, unsigned int> ? ValueKind::kInt :
                std::is_same_v<T, unsigned long long> ? ValueKind::kInt :
                std::is_same_v<T, float> ? ValueKind::kDouble :
                std::is_same_v<T, double> ? ValueKind::kDouble :
                std::is_same_v<T, std::string> ? ValueKind::kString :
                ValueKind::kString; // 兜底
    };

    template<typename T>
    struct FieldArray
    {
        std::string key;
        std::vector<T> defaultValue{};
        bool required = false;

        std::string displayName;
        std::string comment;
        Validation v;

        // 数组类型映射
        static constexpr ValueKind kind =
                std::is_same_v<T, int> ? ValueKind::kIntArray :
                std::is_same_v<T, long long> ? ValueKind::kIntArray :
                std::is_same_v<T, unsigned int> ? ValueKind::kIntArray :
                std::is_same_v<T, unsigned long long> ? ValueKind::kIntArray :
                std::is_same_v<T, float> ? ValueKind::kDoubleArray :
                std::is_same_v<T, double> ? ValueKind::kDoubleArray :
                std::is_same_v<T, std::string> ? ValueKind::kStringArray :
                ValueKind::kStringArray; // 默认当作 string array
    };

    template<typename T>
    struct FieldObject
    {
        std::string key;
        bool required = false;

        std::string displayName;
        std::string comment;
        Validation v; // 通常 primary=None

        static constexpr ValueKind kind = ValueKind::kObject;
    };

    template<typename T>
    struct FieldArrayObject
    {
        std::string key;
        bool required = false;

        std::string displayName;
        std::string comment;
        Validation v;

        static constexpr ValueKind kind = ValueKind::kObjectArray;
    };

    // —— 对象自身元信息 ——（每个 ConfigItem 子类各自提供一份）
    struct ConfigMeta
    {
        std::string nodeName;     // 支点名
        std::string displayName;  // UI显示名
        std::string comment;      // 注释/说明
    };

    struct ConfigItem
    {
        // 指向自身静态元信息（由子类构造函数赋值）
        const cfg::ConfigMeta *selfInfo = nullptr;

        // 记录字段指针，便于统一遍历（UI渲染/校验/序列化）
        std::vector<void *> registeredFields;

        template<typename FieldT>
        void registerField(FieldT *f)
        {
            registeredFields.push_back(static_cast<void *>(f));
        }

        // 访问自身元信息
        const cfg::ConfigMeta *meta() const
        { return selfInfo; }
    };

} // namespace cfg


//
// ========================= 具体配置结构体 =========================
//

// ---- 日志滚动策略 配置 ----
struct RotationPolicy : public cfg::ConfigItem
{
    cfg::FieldValue<int> maxSizeMB{
            "maxSizeMB", 100, false,
            "单文件最大(MB)",
            "范围 1~4096 MB；用于触发滚动阈值。",
            cfg::Validation::IntRange(1, 4096) // 单位MB属业务含义，不当作主语义
    };

    cfg::FieldValue<int> keepFiles{
            "keepFiles", 7, false,
            "最多保留文件数",
            "范围 1~1000。",
            cfg::Validation::IntRange(1, 1000)
    };

    cfg::FieldValue<bool> dailyRotate{
            "dailyRotate", true, false,
            "是否按天滚动",
            "true=每天新建；false=仅按大小。",
            cfg::Validation{}
    };

    static inline const cfg::ConfigMeta kInfo{
            "RotationPolicy", "日志滚动策略", "控制单文件大小、保留数量与按天滚动。"
    };

    RotationPolicy()
    {
        selfInfo = &kInfo;
        registerField(&maxSizeMB);
        registerField(&keepFiles);
        registerField(&dailyRotate);
    }

};

// ---- 日志输出（Sink） 配置 ----
struct SinkConfig : public cfg::ConfigItem
{
    cfg::FieldValue<std::string> type{
            "type", "file", false,
            "输出类型",
            "允许值: file / console / tcp。",
            cfg::Validation::EnumSet({"file", "console", "tcp"})
    };
    cfg::FieldValue<std::string> level{
            "level", "info", false,
            "最低日志级别",
            "允许值: trace/debug/info/warn/error/critical/off。",
            cfg::Validation::EnumSet({"trace", "debug", "info", "warn", "error", "critical", "off"})
    };
    cfg::FieldObject<RotationPolicy> rotation{
            "rotation", false,
            "文件滚动策略",
            "当 type=file 时有效。",
            cfg::Validation{}
    };
    cfg::FieldArray<std::string> tags{
            "tags", {"main"}, false,
            "标签",
            "非空且唯一（用于分类过滤）。",
            cfg::Validation::Array(/*nonEmpty*/true, /*uniqueItems*/true)
    };

    static inline const cfg::ConfigMeta kInfo{
            "SinkConfig", "日志输出(Sink)", "支持 file/console/tcp 等类型；file 可配置滚动策略。"
    };

    SinkConfig()
    {
        selfInfo = &kInfo;
        registerField(&type);
        registerField(&level);
        registerField(&rotation);
        registerField(&tags);
    }
};

// ---- 日志器 ----
struct LoggerConfig : public cfg::ConfigItem
{
    cfg::FieldValue<std::string> name{
            "name", "app", false,
            "日志器名称",
            "非空，字母数字/下划线/中划线，长度<=64。",
            cfg::Validation::StringRegex("^[A-Za-z0-9_\\-]{1,64}$")
    };

    cfg::FieldValue<std::string> format{
            "format", "%Y-%m-%d %H:%M:%S [%l] %v", false,
            "格式模板",
            "例如: %Y-%m-%d %H:%M:%S [%l] %v。",
            cfg::Validation::StringRegex(".+")
    };

    cfg::FieldArrayObject<SinkConfig> sinks{
            "sinks", true,
            "输出列表",
            "至少配置一个输出。",
            cfg::Validation::Array(/*nonEmpty*/true)
    };

    cfg::FieldArray<std::string> modules{
            "modules", {"core", "net"}, false,
            "模块过滤",
            "可为空；若非空需唯一。",
            cfg::Validation::Array(/*nonEmpty*/false, /*uniqueItems*/true)
    };

    static inline const cfg::ConfigMeta kInfo{
            "LoggerConfig", "日志器(Logger)", "日志器包含若干输出(sinks)与模块过滤。"
    };

    LoggerConfig()
    {
        selfInfo = &kInfo;
        registerField(&name);
        registerField(&format);
        registerField(&sinks);
        registerField(&modules);
    }
};

// ---- 服务端点 ----
struct ServiceEndpoint : public cfg::ConfigItem
{

    cfg::FieldValue<std::string> host{
            "host", "127.0.0.1", false,
            "主机",
            "主机名或IPv4/IPv6；示例 127.0.0.1 / ::1 / example.com。",
            // 允许 HostName 或 IP；此处以 HostName 作为主语义，并配合正则放宽
            cfg::Validation::StringRegex(R"(^.+$)", true, cfg::PrimarySemantic::HostName)
    };

    cfg::FieldValue<int> port{
            "port", 8080, true,
            "端口",
            "1~65535；建议避免保留端口。",
            cfg::Validation::IntRange(1, 65535, cfg::PrimarySemantic::Port)
    };

    cfg::FieldArray<std::string> paths{
            "paths", {"/api", "/health"}, false,
            "路径白名单",
            "每项形如“/xxx”；非空且唯一。",
            []
            {
                auto v = cfg::Validation::Array(true, true, cfg::PrimarySemantic::URLPath);
                v.regex = "^/.*$";
                return v;
            }()
    };

    static inline const cfg::ConfigMeta kInfo{
            "ServiceEndpoint", "服务端点", "服务访问主机/端口与路径白名单。"
    };

    ServiceEndpoint()
    {
        selfInfo = &kInfo;
        registerField(&host);
        registerField(&port);
        registerField(&paths);
    }
};

// ---- 应用总配置 ----
struct AppConfig : public cfg::ConfigItem
{
    cfg::FieldValue<bool> enable{
            "enable", true, false,
            "是否启用",
            "true=启用；false=禁用。",
            cfg::Validation{}
    };
    cfg::FieldValue<double> timeout{
            "timeoutSec", 3.5, false,
            "全局超时(秒)",
            "范围 0~600；建议 0~120。",
            cfg::Validation::RealRange(0.0, 600.0) // 秒是单位，不作为主语义
    };
    cfg::FieldValue<int> version{
            "configVersion", 4, false,
            "配置版本",
            "正整数；用于兼容升级。",
            cfg::Validation::IntRange(1, static_cast<long long>(std::numeric_limits<int>::max()))
    };

    cfg::FieldObject<ServiceEndpoint> endpoint{"endpoint", false, "服务端点", "主机/端口/路径白名单。",
                                               cfg::Validation{}};
    cfg::FieldArrayObject<LoggerConfig> loggers{"loggers", true, "日志器列表", "至少一个。",
                                                cfg::Validation::Array(true)};
    cfg::FieldArray<std::string> plugins{
            "plugins", {"metrics", "trace"}, false,
            "插件名",
            "非空且唯一；建议短小标识符（字母数字/下划线/中划线）。",
            cfg::Validation::Array(true, true)
    };

    static inline const cfg::ConfigMeta kInfo{
            "AppConfig", "应用配置", "应用全局参数与子模块配置入口。"
    };

    AppConfig()
    {
        selfInfo = &kInfo;
        registerField(&enable);
        registerField(&timeout);
        registerField(&version);
        registerField(&endpoint);
        registerField(&loggers);
        registerField(&plugins);
    }
};

void helloConfig()
{
    AppConfig appConfig{};
    appConfig.timeout = 1.0;

}

#endif // CX_CT_X2_CONFIG_DATAV4_H
