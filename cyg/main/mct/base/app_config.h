#ifndef CX_CT_X2_APP_CONFIG_H
#define CX_CT_X2_APP_CONFIG_H


#include <ccxx/configuration.h>
#include <ccxx/datetime.h>

// ==============================
// 1) 程序自身配置
// ==============================
struct ProgramConfig : public cfg::ConfigItem
{
    cfg::FieldValue<bool> enable{"enable", true, false, "是否启用", "true=启用",
                                 cfg::Validation{}
    };

    cfg::FieldValue<int> configVersion{"configVersion", 1, false, "配置版本", ">=1",
                                       cfg::Validation::IntRange(1, (std::numeric_limits<int>::max)())
    };

    cfg::FieldValue<std::string> appName{"appName", "MCT Studio", false, "程序名称", "",
                                         cfg::Validation::StringRegex(".+")
    };

    // 注意：使用 cx::DateTime；默认 invalid() -> isValid=false 且时间为 1970-01-01T00:00:00.000
    cfg::FieldValue<cx::DateTime> buildDate{"buildDate", cx::DateTime::invalid(), false, "构建日期（缺省=无效1970-01-01T00:00:00.000）", "",
                                            cfg::Validation{}
    };

    cfg::FieldValue<std::string> workspaceDir{"workspaceDir", "./workspace", false, "工作根目录", "工作根目录",
                                              cfg::Validation{}
    };

    cfg::FieldValue<std::string> theme{
            "theme", "auto", false, "主题", "light/dark/auto",
            cfg::Validation::EnumSet({"light", "dark", "auto"})
    };

    cfg::FieldArray<std::string> plugins{
            "plugins", {}, false, "启用的插件", "可空，唯一",
            cfg::Validation::Array(false, true)
    };

    cfg::FieldValue<double> timeoutSec{
            "timeoutSec", 3.5, false, "全局超时(秒)", "0~600",
            cfg::Validation::RealRange(0.0, 600.0)
    };

    const cfg::ConfigMeta &meta() const override
    {
        static const cfg::ConfigMeta m{"ProgramConfig", "程序自身配置", "应用通用参数"};
        return m;
    }

    ProgramConfig()
    {
        registerField(&ProgramConfig::enable);
        registerField(&ProgramConfig::configVersion);
        registerField(&ProgramConfig::appName);
        registerField(&ProgramConfig::buildDate);
        registerField(&ProgramConfig::workspaceDir);
        registerField(&ProgramConfig::theme);
        registerField(&ProgramConfig::plugins);
        registerField(&ProgramConfig::timeoutSec);
    }
};

// ==============================
// 2) CT 重建配置（核心）
// ==============================
struct CtReconstructionConfig : public cfg::ConfigItem
{
    cfg::FieldValue<std::string> algorithm{
            "algorithm", "FBP", true, "重建算法", "FBP/SART/CGLS/RTK",
            cfg::Validation::EnumSet({"FBP", "SART", "CGLS", "RTK"})
    };

    cfg::FieldValue<std::string> filter{
            "filter", "RamLak", false, "滤波器（FBP用）", "RamLak/SheppLogan/Cosine/Hann/Hamming",
            cfg::Validation::EnumSet({"RamLak", "SheppLogan", "Cosine", "Hann", "Hamming"})
    };

    cfg::FieldValue<double> cutoff{
            "cutoff", 1.0, false, "滤波截止(0~1)", "",
            cfg::Validation::RealRange(0.0, 1.0)
    };

    cfg::FieldValue<int> iterations{
            "iterations", 20, false, "迭代次数（迭代法生效）", "",
            cfg::Validation::IntRange(1, 10000)
    };

    cfg::FieldValue<double> relaxation{
            "relaxation", 0.5, false, "松弛参数（SART）", "",
            cfg::Validation::RealRange(0.0, 1.0)
    };

    // 标定日期：缺省为 invalid -> isValid=false
    cfg::FieldValue<cx::DateTime> calibDate{
            "calibDate", cx::DateTime::invalid(), false, "几何标定日期（缺省=无效1970-01-01T00:00:00.000）", "", cfg::Validation{}
    };

    // 体素/尺寸：给出最小可用的三元数组（留空允许 UI 自算）
    cfg::FieldArray<int> dimXYZ{
            "dimXYZ", {}, false, "体素尺寸 [x,y,z] (可空)", "",
//            cfg::Validation::ArrayLenOrEmpty(3)
            cfg::Validation{}
    };
    cfg::FieldArray<double> voxelSizeXYZ{
            "voxelSizeXYZ", {}, false, "体素间距(mm) [x,y,z] (可空)", "",
            cfg::Validation{}
    };

    const cfg::ConfigMeta &meta() const override
    {
        static const cfg::ConfigMeta m{"CtReconstructionConfig", "CT 重建配置", "算法与体素核心参数"};
        return m;
    }

    CtReconstructionConfig()
    {
        registerField(&CtReconstructionConfig::algorithm);
        registerField(&CtReconstructionConfig::filter);
        registerField(&CtReconstructionConfig::cutoff);
        registerField(&CtReconstructionConfig::iterations);
        registerField(&CtReconstructionConfig::relaxation);
        registerField(&CtReconstructionConfig::calibDate);
        registerField(&CtReconstructionConfig::dimXYZ);
        registerField(&CtReconstructionConfig::voxelSizeXYZ);
    }
};

// ==============================
// 3) 顶层 AppConfig
// ==============================
struct AppConfig : public cfg::ConfigItem
{
    cfg::FieldObject<ProgramConfig> program{"program", true, "程序自身配置", "程序自身配置",
                                            cfg::Validation{}
    };

    cfg::FieldObject<CtReconstructionConfig> ct{"ct", true, "CT 重建配置", "CT 重建配置",
                                                cfg::Validation{}
    };

    // 顶层版本号（与 program.configVersion 分离，便于整体迁移）
    cfg::FieldValue<int> version{"version", 1, false, "AppConfig 顶层版本", ">=1",
                                 cfg::Validation::IntRange(1, (std::numeric_limits<int>::max)())
    };

    const cfg::ConfigMeta &meta() const override
    {
        static const cfg::ConfigMeta m{"AppConfig", "应用配置", "包含程序自身与CT重建两部分"};
        return m;
    }

    AppConfig()
    {
        registerField(&AppConfig::program);
        registerField(&AppConfig::ct);
        registerField(&AppConfig::version);
    }
};


#endif //CX_CT_X2_APP_CONFIG_H
