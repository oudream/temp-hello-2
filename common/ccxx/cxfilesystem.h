#ifndef CXFILESYSTEM_H
#define CXFILESYSTEM_H

#include "cxglobal.h"

class CCXX_EXPORT CxFilesystem
{
public:
    struct FileStat
    {
        bool exists = false;
        bool isFile = false;
        bool isDir = false;
        std::uintmax_t size = 0;
    };

public:
    // --- 路径/状态 ---
    static bool exists(const std::string &path);

    static bool isFile(const std::string &path);

    static bool isDir(const std::string &path);

    static FileStat stat(const std::string &path);

    static bool hasRootPath(const std::string &path);

    static bool hasRootName(const std::string &path);

    static bool hasRootDirectory(const std::string &path);

    static bool isAbsolute(const std::string &path);

    static bool isRelative(const std::string &path);

    // --- 目录操作 ---
    static bool createDirs(const std::string &dir);

    static bool removeOne(const std::string &path);

    static std::uintmax_t removeAll(const std::string &path);

    static bool rename(const std::string &from, const std::string &to);

    // --- 复制 ---
    static bool copyFile(const std::string &from, const std::string &to, bool overwrite = false);

    static bool copyDir(const std::string &from, const std::string &to, bool overwrite = false);

    // --- 枚举目录 ---
    static std::vector<std::string> listDir(const std::string &dir);

    static std::vector<std::string> listDirRecursive(const std::string &dir);

    // --- 文件读写 ---
    static bool readAllText(const std::string &path, std::string &out);

    static bool writeAllText(const std::string &path, const std::string &text, bool createParent = true);

    static bool readAllBytes(const std::string &path, std::vector<char> &out);

    static bool writeAllBytes(const std::string &path, const std::vector<char> &data, bool createParent = true);

    static bool writeAllTextAtomic(const std::string &path, const std::string &text, bool createParent = true);

    static bool writeAllBytesAtomic(const std::string &path, const std::vector<char> &data, bool createParent = true);

    // --- 路径工具 ---
    static std::string parentPath(const std::string &path);

    static std::string filename(const std::string &path);

    static std::string stem(const std::string &path);

    static std::string extension(const std::string &path);

    static std::string replaceExtension(const std::string &path, const std::string &newExtWithDot);

    static std::string replaceFilename(const std::string &path, const std::string &name);

    static std::string join(const std::string &a, const std::string &b);

    static std::string absolutePath(const std::string &path);

    static std::string normalizeLexical(const std::string &path);

    static std::string normalizeGeneric(const std::string &path);

    // --- 工作目录 ---
    static bool setCwd(const std::string &dir);

    static std::string getCwd();

private:
    static std::filesystem::path p8(const std::string &s);

    static std::string u8(const std::filesystem::path &p);

    static std::filesystem::path makeSiblingTemp(const std::filesystem::path &dst);

};

#endif //CXFILESYSTEM_H
