#ifndef CX_CT_X2_FILESYSTEM_HELPER_H
#define CX_CT_X2_FILESYSTEM_HELPER_H


#include <c_global.h>

// 只有 c++17 才有 filesystem
#include <filesystem>


class FileSystemHelper
{
public:
    // 扫描目录，返回按数字顺序排序的 xxx 文件路径
    static std::vector<std::string> ScanFiles(const std::string &dirPath, const std::string &suffix = std::string());


};


#endif //CX_CT_X2_FILESYSTEM_HELPER_H
