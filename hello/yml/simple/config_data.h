#ifndef CX_CT_X2_CONFIG_DATA_H
#define CX_CT_X2_CONFIG_DATA_H

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
};

// ========== 应用总配置 ==========

struct ServiceEndpoint : public cfg::ConfigItem {
    cfg::FieldValue<std::string> f_host {"host", "127.0.0.1", false, "主机"};
    cfg::FieldValue<cfg::Int>    f_port {"port", 8080,        true,  "端口"};
    cfg::FieldArray<std::string> f_paths{"paths",{"/api","/health"}, false, "路径白名单"};
    ServiceEndpoint() { registerField(f_host); registerField(f_port); registerField(f_paths); }
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
};


#endif //CX_CT_X2_CONFIG_DATA_H
