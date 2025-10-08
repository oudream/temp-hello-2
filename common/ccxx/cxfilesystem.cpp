#include "cxfilesystem.h"

#include <filesystem>
#include <fstream>
#include <sstream>
#include <system_error>
#include <thread>

namespace fs = std::filesystem;

// --- UTF-8 string <-> path ---
fs::path CxFilesystem::p8(const std::string &s) {
    return fs::u8path(s);
}

std::string CxFilesystem::u8(const fs::path &p) {
    return p.u8string();
}

// --- 生成同目录临时文件名（降低覆盖冲突概率；仍不保证唯一） ---
fs::path CxFilesystem::makeSiblingTemp(const fs::path &dst) {
    const auto now = std::chrono::high_resolution_clock::now().time_since_epoch().count();
    const auto tid = std::hash<std::thread::id>{}(std::this_thread::get_id());
    fs::path tmp = dst;
    tmp += fs::path(".tmp." + std::to_string(tid) + "." + std::to_string(now));
    return tmp;
}

// -------- 路径/状态 --------
bool CxFilesystem::exists(const std::string &path) {
    std::error_code ec;
    return fs::exists(p8(path), ec);
}

bool CxFilesystem::isFile(const std::string &path) {
    std::error_code ec;
    return fs::is_regular_file(p8(path), ec);
}

bool CxFilesystem::isDir(const std::string &path) {
    std::error_code ec;
    return fs::is_directory(p8(path), ec);
}

CxFilesystem::FileStat CxFilesystem::stat(const std::string &path) {
    std::error_code ec;
    FileStat st;
    fs::path p = p8(path);
    st.exists = fs::exists(p, ec);
    if (!st.exists) return st;
    st.isFile = fs::is_regular_file(p, ec);
    st.isDir = fs::is_directory(p, ec);
    if (st.isFile) st.size = fs::file_size(p, ec);
    return st;
}

// 根/绝对性
bool CxFilesystem::hasRootPath(const std::string &path) {
    return p8(path).has_root_path();
}

bool CxFilesystem::hasRootName(const std::string &path) {
    return p8(path).has_root_name();
}

bool CxFilesystem::hasRootDirectory(const std::string &path) {
    return p8(path).has_root_directory();
}

bool CxFilesystem::isAbsolute(const std::string &path) {
    return p8(path).is_absolute(); // 词法判断
}

bool CxFilesystem::isRelative(const std::string &path) {
    return p8(path).is_relative();
}

// -------- 目录操作 --------
bool CxFilesystem::createDirs(const std::string &dir) {
    std::error_code ec;
    const fs::path p = p8(dir);
    if (fs::exists(p, ec)) return true;
    return fs::create_directories(p, ec);
}

bool CxFilesystem::removeOne(const std::string &path) {
    std::error_code ec;
    return fs::remove(p8(path), ec);
}

std::uintmax_t CxFilesystem::removeAll(const std::string &path) {
    std::error_code ec;
    return fs::remove_all(p8(path), ec);
}

bool CxFilesystem::rename(const std::string &from, const std::string &to) {
    std::error_code ec;
    fs::rename(p8(from), p8(to), ec);
    return !ec;
}

// -------- 复制 --------
bool CxFilesystem::copyFile(const std::string &from, const std::string &to, bool overwrite) {
    std::error_code ec;
    // 无论是否覆盖，都尝试创建父目录（之前版本只在 overwrite=true 时创建，已修正）
    {
        std::error_code ec2;
        auto parent = p8(to).parent_path();
        if (!parent.empty()) fs::create_directories(parent, ec2);
    }
    auto opt = overwrite ? fs::copy_options::overwrite_existing
                         : fs::copy_options::none;
    return fs::copy_file(p8(from), p8(to), opt, ec);
}

bool CxFilesystem::copyDir(const std::string &from, const std::string &to, bool overwrite) {
    std::error_code ec;
    fs::copy_options opt = fs::copy_options::recursive;
    if (overwrite) opt |= fs::copy_options::overwrite_existing | fs::copy_options::update_existing;

    {
        std::error_code ec2;
        fs::create_directories(p8(to), ec2);
    }
    fs::copy(p8(from), p8(to), opt, ec);
    return !ec;
}

// -------- 枚举目录 --------
std::vector<std::string> CxFilesystem::listDir(const std::string &dir) {
    std::vector<std::string> out;
    std::error_code ec;
    for (fs::directory_iterator it{p8(dir), ec}, end; it != end && !ec; it.increment(ec)) {
        out.emplace_back(u8(it->path()));
    }
    return out;
}

std::vector<std::string> CxFilesystem::listDirRecursive(const std::string &dir) {
    std::vector<std::string> out;
    std::error_code ec;
    for (fs::recursive_directory_iterator it{p8(dir), ec}, end; it != end && !ec; it.increment(ec)) {
        out.emplace_back(u8(it->path()));
    }
    return out;
}

// -------- 文件读写 --------
bool CxFilesystem::readAllText(const std::string &path, std::string &out) {
    std::ifstream ifs(p8(path));                 // 文本模式
    if (!ifs) return false;
    std::ostringstream ss;
    ss << ifs.rdbuf();
    out = ss.str();
    return true;
}

bool CxFilesystem::writeAllText(const std::string &path, const std::string &text, bool createParent) {
    if (createParent) {
        std::error_code ec;
        auto parent = p8(path).parent_path();
        if (!parent.empty()) fs::create_directories(parent, ec);
    }
    std::ofstream ofs(p8(path));                 // 覆盖
    if (!ofs) return false;
    ofs << text;
    return bool(ofs);
}

bool CxFilesystem::readAllBytes(const std::string &path, std::vector<char> &out) {
    std::ifstream ifs(p8(path), std::ios::binary);
    if (!ifs) return false;
    ifs.seekg(0, std::ios::end);
    const auto n = static_cast<std::size_t>(ifs.tellg());
    ifs.seekg(0, std::ios::beg);
    out.resize(n);
    if (n) ifs.read(reinterpret_cast<char *>(out.data()), static_cast<std::streamsize>(n));
    return bool(ifs);
}

bool CxFilesystem::writeAllBytes(const std::string &path, const std::vector<char> &data, bool createParent) {
    if (createParent) {
        std::error_code ec;
        auto parent = p8(path).parent_path();
        if (!parent.empty()) fs::create_directories(parent, ec);
    }
    std::ofstream ofs(p8(path), std::ios::binary | std::ios::trunc);
    if (!ofs) return false;
    if (!data.empty()) {
        ofs.write(reinterpret_cast<const char *>(data.data()),
                  static_cast<std::streamsize>(data.size()));
    }
    return bool(ofs);
}

// -------- 原子写：写临时文件 → flush/close → rename 覆盖 --------
bool CxFilesystem::writeAllTextAtomic(const std::string &path, const std::string &text, bool createParent) {
    std::error_code ec;
    const fs::path dst = p8(path);
    const fs::path tmp = makeSiblingTemp(dst);

    if (createParent) {
        auto parent = dst.parent_path();
        if (!parent.empty()) fs::create_directories(parent, ec);
    }

    // 写临时文件
    {
        std::ofstream ofs(tmp);
        if (!ofs) return false;
        ofs << text;
        ofs.flush();
        if (!ofs) {
            std::error_code _;
            fs::remove(tmp, _);
            return false;
        }
    }
    // 原子替换（同目录 rename）
    fs::rename(tmp, dst, ec);
    if (ec) {
        std::error_code _;
        fs::remove(tmp, _);
        return false;
    }
    return true;
}

bool CxFilesystem::writeAllBytesAtomic(const std::string &path, const std::vector<char> &data, bool createParent) {
    std::error_code ec;
    const fs::path dst = p8(path);
    const fs::path tmp = makeSiblingTemp(dst);

    if (createParent) {
        auto parent = dst.parent_path();
        if (!parent.empty()) fs::create_directories(parent, ec);
    }

    {
        std::ofstream ofs(tmp, std::ios::binary | std::ios::trunc);
        if (!ofs) return false;
        if (!data.empty()) {
            ofs.write(reinterpret_cast<const char *>(data.data()),
                      static_cast<std::streamsize>(data.size()));
        }
        ofs.flush();
        if (!ofs) {
            std::error_code _;
            fs::remove(tmp, _);
            return false;
        }
    }
    fs::rename(tmp, dst, ec);
    if (ec) {
        std::error_code _;
        fs::remove(tmp, _);
        return false;
    }
    return true;
}

// -------- 路径工具 --------
std::string CxFilesystem::parentPath(const std::string &path) {
    return u8(p8(path).parent_path());
}

std::string CxFilesystem::filename(const std::string &path) {
    return u8(p8(path).filename());
}

std::string CxFilesystem::stem(const std::string &path) {
    return u8(p8(path).stem());
}

std::string CxFilesystem::extension(const std::string &path) {
    return u8(p8(path).extension());
}

std::string CxFilesystem::replaceExtension(const std::string &path, const std::string &newExtWithDot) {
    fs::path p = p8(path);
    p.replace_extension(p8(newExtWithDot));
    return u8(p);
}

std::string CxFilesystem::replaceFilename(const std::string &path, const std::string &name) {
    fs::path p = p8(path);
    p.replace_filename(p8(name));
    return u8(p);
}

std::string CxFilesystem::join(const std::string &a, const std::string &b) {
    fs::path p = p8(a);
    p /= p8(b);
    return u8(p);
}

std::string CxFilesystem::absolutePath(const std::string &path) {
    std::error_code ec;
    return u8(fs::absolute(p8(path), ec));
}

std::string CxFilesystem::normalizeLexical(const std::string &path) {
    fs::path p = p8(path).lexically_normal();
    return u8(p);
}

std::string CxFilesystem::normalizeGeneric(const std::string &path) {
    fs::path p = p8(path).lexically_normal();
    return p.generic_u8string();
}

// -------- 工作目录（进程级） --------
bool CxFilesystem::setCwd(const std::string &dir) {
    std::error_code ec;
    fs::current_path(p8(dir), ec);
    return !ec;
}

std::string CxFilesystem::getCwd() {
    std::error_code ec;
    return u8(fs::current_path(ec));
}
