#include "app_config_helper.h"

#include <stdexcept>

#include <helpers/yaml_helper.h>
#include <helpers/log_helper.h>

// =====================
// 静态区
// =====================
std::mutex      AppConfigHelper::s_mu;
AppConfig       AppConfigHelper::s_cfg;   // 含 program / ct / version 等字段 :contentReference[oaicite:3]{index=3}
std::string     AppConfigHelper::s_path;
bool            AppConfigHelper::s_loaded = false;

// =====================
// 公有接口
// =====================
void AppConfigHelper::load(const std::string &fn)
{
    std::lock_guard<std::mutex> lk(s_mu);

    // 1) 解析路径来源
    std::string useFn = fn;
    if (useFn.empty())
    {
        if (!s_path.empty()) useFn = s_path;   // 允许重复加载上次文件
        else
            throw std::invalid_argument("AppConfigHelper::load: empty filename and no previous path");
    }

    // 2) 调用适配层读入
    AppConfig tmp;
    if (!load_(useFn, tmp))
    {
        LogHelper::error() << "Config load fail";
        return;
    }

    // 3) 成功则切换
    s_cfg = std::move(tmp);
    s_path = useFn;
    s_loaded = true;

    auto lines = cfg::toLines(&s_cfg);
    LogHelper::debug() << "Config has loaded, value ->\n" << lines << "<-Config\n";
}

bool AppConfigHelper::save(const std::string &fn)
{
    std::lock_guard<std::mutex> lk(s_mu);
    std::string useFn = fn.empty() ? s_path : fn;
    if (useFn.empty())
    {
        return false;
    }

    return save_(useFn, s_cfg);                // ← 写盘（替换为你们 ccxx 的实际API）
}

bool AppConfigHelper::isLoaded()
{
    std::lock_guard<std::mutex> lk(s_mu);
    return s_loaded;
}

const std::string &AppConfigHelper::path()
{
    std::lock_guard<std::mutex> lk(s_mu);
    return s_path;
}

const AppConfig *AppConfigHelper::get()
{
    std::lock_guard<std::mutex> lk(s_mu);
    return &s_cfg;
}

AppConfig *AppConfigHelper::mutableConfig()
{
    // 注意：返回引用用于修改；调用方修改完可选择 save()。
    return &s_cfg; // 这里不加锁以避免长时间持锁，按需在外层同步
}

const ProgramConfig *AppConfigHelper::program()
{
    std::lock_guard<std::mutex> lk(s_mu);
    return s_cfg.program.getTItem();
}

const CtReconstructionConfig *AppConfigHelper::ct()
{
    std::lock_guard<std::mutex> lk(s_mu);
    return s_cfg.ct.getTItem();
}

void AppConfigHelper::resetToDefaults()
{
    std::lock_guard<std::mutex> lk(s_mu);
    s_cfg = AppConfig();
    s_loaded = false;
}

// =====================
// 适配层（把这里替换成你们 ccxx 的真实读写实现）
// =====================
bool AppConfigHelper::load_(const std::string &fn, AppConfig &out)
{
    cfg::ConfigManager mgr(new cfg::ConfigIOYAML());

    std::string err;

    // 声明需要解析的 FieldRef<T> 类型集合：<LoggerConfig>
    bool ok = mgr.loadAll<AppConfig>(fn, &out, &err);
    if (!ok)
    {
        LogHelper::error() << "Config load failed:" << err;
        return false;
    }

    return true;
}

bool AppConfigHelper::save_(const std::string &fn, const AppConfig &in)
{
    cfg::ConfigManager mgr(new cfg::ConfigIOYAML());

    std::string err;

    if (!mgr.save(fn, &s_cfg, &err))
    {
        LogHelper::error() << "Config save failed:" << err;
        return false;
    }

    return true;
}
