#include "app_directory_helper.h"

#include <filesystem>
#include <ccxx/cxcontainer.h>

// 静态存储（避免 ODR/多重定义问题）
static std::string s_exe;
static std::string s_exeDir;

// —— 对外接口 —— //

void AppDirectoryHelper::init(const char *argv0)
{
    s_exe = resolveExeFromArgv0(argv0);
    s_exeDir = parentPath(s_exe);
    if (s_exeDir.empty())
    {
        s_exeDir = std::filesystem::current_path().u8string();
    }

    ensureAppDataTree();
}

const std::string &AppDirectoryHelper::exeFileName()
{
    static std::string s_name;
    if (s_name.empty() && !s_exe.empty())
    {
        std::filesystem::path p(s_exe);
        s_name = p.filename().u8string();
    }
    return s_name;
}

const std::string &AppDirectoryHelper::exeFileNameNoExt()
{
    static std::string s_name_no_ext;
    if (s_name_no_ext.empty() && !s_exe.empty())
    {
        std::filesystem::path p(s_exe);
        s_name_no_ext = p.stem().u8string();
    }
    return s_name_no_ext;
}

const std::string &AppDirectoryHelper::exePath()
{
    return s_exe;
}

const std::string &AppDirectoryHelper::exeDirectory()
{
    return s_exeDir;
}

std::string AppDirectoryHelper::exeParentDirectory()
{
    return parentPath(s_exeDir);
}

std::string AppDirectoryHelper::appDataPath()
{
    return CxFilesystem::join(exeParentDirectory(), kAppDataDir);
}

std::string AppDirectoryHelper::configsPath()
{
    return CxFilesystem::join(appDataPath(), kConfigsDir);
}

std::string AppDirectoryHelper::tempPath()
{
    return CxFilesystem::join(appDataPath(), kTempDir);
}

std::string AppDirectoryHelper::logsPath()
{
    return CxFilesystem::join(appDataPath(), kLogsDir);
}

std::string AppDirectoryHelper::commandsPath()
{
    return CxFilesystem::join(appDataPath(), kCommandsDir);
}

std::string AppDirectoryHelper::dataPath()
{
    return CxFilesystem::join(appDataPath(), kDataDir);
}

std::string AppDirectoryHelper::projectsPath()
{
    return CxFilesystem::join(appDataPath(), kProjectsDir);
}

void AppDirectoryHelper::ensureAppDataTree()
{
    mkDirs(appDataPath());
    mkDirs(configsPath());
    mkDirs(tempPath());
    mkDirs(logsPath());
    mkDirs(commandsPath());
    mkDirs(dataPath());
    mkDirs(projectsPath());
}

// —— 便捷拼接（单段） —— //
std::string AppDirectoryHelper::configsJoin(const std::string &sub)
{
    return CxFilesystem::join(configsPath(), sub);
}

std::string AppDirectoryHelper::tempJoin(const std::string &sub)
{
    return CxFilesystem::join(tempPath(), sub);
}

std::string AppDirectoryHelper::logsJoin(const std::string &sub)
{
    return CxFilesystem::join(logsPath(), sub);
}

std::string AppDirectoryHelper::commandsJoin(const std::string &sub)
{
    return CxFilesystem::join(commandsPath(), sub);
}

std::string AppDirectoryHelper::dataJoin(const std::string &sub)
{
    return CxFilesystem::join(dataPath(), sub);
}

std::string AppDirectoryHelper::projectsJoin(const std::string &sub)
{
    return CxFilesystem::join(projectsPath(), sub);
}

// —— 便捷拼接（多段） —— //
std::string AppDirectoryHelper::configsJoin(const std::vector<std::string> &parts)
{
    return CxFilesystem::joinMany(configsPath(), parts);
}

std::string AppDirectoryHelper::tempJoin(const std::vector<std::string> &parts)
{
    return CxFilesystem::joinMany(tempPath(), parts);
}

std::string AppDirectoryHelper::logsJoin(const std::vector<std::string> &parts)
{
    return CxFilesystem::joinMany(logsPath(), parts);
}

std::string AppDirectoryHelper::commandsJoin(const std::vector<std::string> &parts)
{
    return CxFilesystem::joinMany(commandsPath(), parts);
}

std::string AppDirectoryHelper::dataJoin(const std::vector<std::string> &parts)
{
    return CxFilesystem::joinMany(dataPath(), parts);
}

std::string AppDirectoryHelper::projectsJoin(const std::vector<std::string> &parts)
{
    return CxFilesystem::joinMany(projectsPath(), parts);
}

// —— 常用工具 —— //
std::string AppDirectoryHelper::joinFullPath(const std::string &path)
{
    return CxFilesystem::join(appDataPath(), path);
}

// —— 内部工具 —— //

std::string AppDirectoryHelper::resolveExeFromArgv0(const char *argv0)
{
    // 首选：基于 argv0 解析为绝对路径
    if (argv0 && *argv0)
    {
        std::filesystem::path p(argv0);
        if (!p.is_absolute())
        {
            // p = std::filesystem::absolute(p, std::filesystem::current_path());
            p = std::filesystem::current_path() / p;
            p = std::filesystem::absolute(p); // 再规范化一次
        }
        // 若路径存在，尽量进行 canonical/weakly_canonical 规范化
        if (std::filesystem::exists(p))
        {
            std::error_code ec;
            auto canon = std::filesystem::weakly_canonical(p, ec);
            return (ec ? p : canon).u8string();
        }
        return p.u8string();
    }

    // 次选：平台 API 兜底
#if defined(_WIN32)
    std::vector<wchar_t> buf(MAX_PATH);
    while (true)
    {
        DWORD len = GetModuleFileNameW(nullptr, buf.data(), static_cast<DWORD>(buf.size()));
        if (len == 0) break;
        if (len < buf.size())
        {
            std::filesystem::path p(buf.data());
            std::error_code ec;
            auto canon = std::filesystem::weakly_canonical(p, ec);
            return (ec ? p : canon).u8string();
        }
        buf.resize(buf.size() * 2);
    }
#elif defined(__linux__)
    std::error_code ec;
    std::filesystem::path p = std::filesystem::read_symlink("/proc/self/exe", ec);
    if (!ec && !p.empty()) {
        auto canon = std::filesystem::weakly_canonical(p, ec);
        return (ec ? p : canon).u8string();
    }
#endif
    // 兜底：当前工作目录 + 占位名
    return (std::filesystem::path(std::filesystem::current_path()) / "app").u8string();
}

std::string AppDirectoryHelper::parentPath(const std::string &path)
{
    std::filesystem::path p(path);
    return p.has_parent_path() ? p.parent_path().u8string() : std::string{};
}

bool AppDirectoryHelper::mkDirs(const std::string &dir)
{
    if (dir.empty()) return false;
    std::error_code ec;
    if (std::filesystem::exists(dir)) return true;
    return std::filesystem::create_directories(dir, ec) || std::filesystem::exists(dir);
}

std::vector<std::string> AppDirectoryHelper::printSelf()
{
    auto exeFS = CxFilesystem::fileStat(AppDirectoryHelper::exePath());
    auto exeDS = CxFilesystem::dirsStat(AppDirectoryHelper::exeDirectory());
    auto appDataDs = CxFilesystem::dirsStat(AppDirectoryHelper::appDataPath());

    auto sExeFS = CxFilesystem::toString(exeFS);
    std::vector<std::string> ss;
    ss.push_back(sExeFS);
    auto dss = CxContainer::merge(exeDS, appDataDs);
    for (const auto &ds: dss)
    {
        ss.push_back(CxFilesystem::toString(ds));
    }
    return ss;
}
