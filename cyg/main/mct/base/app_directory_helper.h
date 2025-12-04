#ifndef CX_CT_X2_APP_DIRECTORY_HELPER_H
#define CX_CT_X2_APP_DIRECTORY_HELPER_H


#include <ccxx/cxfilesystem.h>


/**
 * @brief AppDirectoryHelper
 * 项目级“app 目录 helper”，统一管理以下目录：
 *   [exe 上级目录] / AppData /
 *       ├── Configs
 *       ├── Temp
 *       ├── Logs
 *       ├── Commands
 *       ├── Data        （数据库、数据文件）
 *       └── Projects
 *
 * 使用方式：
 *   1) 在 main() 开头调用 Init(argv[0]) 完成初始化（解析可执行文件路径与目录）。
 *   2) 调用 EnsureAppDataTree() 一键创建 AppData 及子目录。
 *   3) 通过 *Path() 获取目录，通过 *Join()/joinFullPath() 拼接子路径/文件名。
 *
 * 设计要点：
 *   - 以 argv[0] 为主进行解析，必要时使用平台 API 兜底。
 *   - 返回值统一为 UTF-8 字符串。
 *   - 小驼峰命名风格：init/ensureAppDataTree/joinFullPath/isFullPath 等。
 */
class AppDirectoryHelper
{
public:
    // —— 固定目录名（UTF-8）——
    static constexpr const char *kAppDataDir = "AppData";
    static constexpr const char *kConfigsDir = "Configs";
    static constexpr const char *kTempDir = "Temp";
    static constexpr const char *kLogsDir = "Logs";
    static constexpr const char *kCommandsDir = "Commands";
    static constexpr const char *kDataDir = "Data";      // 数据库、数据文件
    static constexpr const char *kProjectsDir = "Projects";

public:
    /**
     * @brief init 在 main() 开头调用；argv0 通常传入 argv[0]
     */
    static void init(const char *argv0);

    // —— 基础路径（UTF-8 字符串）——
    static const std::string &exeFileName();            // 可执行文件名
    static const std::string &exeFileNameNoExt();       // 不含扩展名
    static const std::string &exePath();                // 可执行文件完整路径
    static const std::string &exeDirectory();           // 可执行文件所在目录
    static std::string exeParentDirectory();            // 可执行目录的上一级目录

    // —— AppData 与各子目录（均位于 exe 上级目录之下）——
    static std::string appDataPath();  // .../AppData

    static std::string configsPath();  // .../AppData/Configs
    static std::string tempPath();     // .../AppData/Temp
    static std::string logsPath();     // .../AppData/Logs
    static std::string commandsPath(); // .../AppData/Commands
    static std::string dataPath();     // .../AppData/Data
    static std::string projectsPath(); // .../AppData/Projects

    /**
     * @brief ensureAppDataTree 确认/创建 AppData 及全部子目录
     */
    static void ensureAppDataTree();

    // —— 便捷拼接：将子路径拼到各目录下 —— //
    static std::string configsJoin(const std::string &subPath);

    static std::string tempJoin(const std::string &subPath);

    static std::string logsJoin(const std::string &subPath);

    static std::string commandsJoin(const std::string &subPath);

    static std::string dataJoin(const std::string &subPath);

    static std::string projectsJoin(const std::string &subPath);

    // 可选：多段拼接重载（便于一口气拼多级子目录/文件名）
    static std::string configsJoin(const std::vector<std::string> &parts);

    static std::string tempJoin(const std::vector<std::string> &parts);

    static std::string logsJoin(const std::vector<std::string> &parts);

    static std::string commandsJoin(const std::vector<std::string> &parts);

    static std::string dataJoin(const std::vector<std::string> &parts);

    static std::string projectsJoin(const std::vector<std::string> &parts);

    // —— 常用工具 —— //
    /**
     * @brief joinFullPath 若 path 为绝对路径则直接返回；
     *        否则将其视为“相对于可执行目录”，拼接并规范化返回。
     */
    static std::string joinFullPath(const std::string &path);

    static std::vector<std::string> printSelf();

private:
    // —— 内部工具 —— //
    static std::string resolveExeFromArgv0(const char *argv0);

    static std::string parentPath(const std::string &path);

    static bool mkDirs(const std::string &dir);

};

#endif //CX_CT_X2_APP_DIRECTORY_HELPER_H
