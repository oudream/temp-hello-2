#ifndef CX_CT_X2_APP_CONFIG_HELPER_H
#define CX_CT_X2_APP_CONFIG_HELPER_H


#include "app_config.h"
#include <mutex>


class AppConfigHelper
{
public:
    // 加载配置文件（JSON/YAML/TOML 取决于 ccxx 的实现）
    // 如果 fn 为空且之前已加载，会使用上次路径。
    static void load(const std::string &fn = std::string());

    // 保存（如传空字符串则使用 load 时的路径）
    static bool save(const std::string &fn = std::string());

    // 是否已成功加载
    static bool isLoaded();

    // 当前配置文件路径（可能为空，代表尚未加载或调用者只使用了默认配置）
    static const std::string &path();

    // 只读访问整个配置
    static const AppConfig *get();

    // 可写访问（注意：改完后可调用 save() 持久化）
    static AppConfig *mutableConfig();

    // 便捷只读访问子配置
    static const ProgramConfig *program();

    static const CtReconstructionConfig *ct();

    // 重置为默认（不触发保存）
    static void resetToDefaults();

private:
    // 实际读/写的适配（把这两处替换成你们 ccxx 的真实 API）
    static bool load_(const std::string &fn, AppConfig &out);

    static bool save_(const std::string &fn, const AppConfig &in);

private:
    static std::mutex s_mu;
    static AppConfig s_cfg;
    static std::string s_path;
    static bool s_loaded;

};


#endif //CX_CT_X2_APP_CONFIG_HELPER_H
