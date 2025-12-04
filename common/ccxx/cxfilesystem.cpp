#include "cxfilesystem.h"

#include <filesystem>
#include <fstream>
#include <sstream>
#include <system_error>
#include <thread>

#ifndef _WIN32
#include <sys/stat.h>
#include <unistd.h>
#endif

namespace fs = std::filesystem;

using PathKind = CxFilesystem::PathKind;

// ---- time helpers ----
std::tm _fileTime2tm(fs::file_time_type ft)
{
    using namespace std::chrono;
    const auto sctp = time_point_cast<system_clock::duration>(
            ft - fs::file_time_type::clock::now() + system_clock::now());
    std::time_t tt = system_clock::to_time_t(sctp);
    std::tm tm{};
#if defined(_WIN32)
    localtime_s(&tm, &tt);
#else
    tm = *std::localtime(&tt);
#endif
    return tm;
}

// ---- kind detection (symlink/device/fifo/socket) ----
CxFilesystem::PathKind _detectKind(const fs::directory_entry &de)
{
    std::error_code ec;
    if (!de.exists(ec)) return PathKind::eNone;
    if (de.is_symlink(ec)) return PathKind::eSoftLink;
    if (de.is_directory(ec)) return PathKind::eDir;
    if (de.is_regular_file(ec)) return PathKind::eFile;

#if defined(_WIN32)
    // Windows 上字符/块设备基本不可见于普通文件系统；其余统归 eSystem
    return PathKind::eSystem;
#else
    struct stat st{};
    if (lstat(de.path().c_str(), &st) == 0) {
        if (S_ISCHR(st.st_mode))  return PathKind::eDevChar;
        if (S_ISBLK(st.st_mode))  return PathKind::eDevBlock;
        if (S_ISFIFO(st.st_mode)) return PathKind::eSystem; // fifo
#ifdef S_ISSOCK
        if (S_ISSOCK(st.st_mode)) return PathKind::eSystem; // socket
#endif
    }
    return PathKind::eSystem;
#endif
}


// --- UTF-8 string <-> path ---
fs::path CxFilesystem::p8(const std::string &s)
{
    return fs::u8path(s);
}

std::string CxFilesystem::u8(const fs::path &p)
{
    return p.u8string();
}

// --- 生成同目录临时文件名（降低覆盖冲突概率；仍不保证唯一） ---
fs::path CxFilesystem::makeSiblingTemp(const fs::path &dst)
{
    const auto now = std::chrono::high_resolution_clock::now().time_since_epoch().count();
    const auto tid = std::hash<std::thread::id>{}(std::this_thread::get_id());
    fs::path tmp = dst;
    tmp += fs::path(".tmp." + std::to_string(tid) + "." + std::to_string(now));
    return tmp;
}

// -------- 路径/状态 --------
bool CxFilesystem::exists(const std::string &path)
{
    std::error_code ec;
    return fs::exists(p8(path), ec);
}

bool CxFilesystem::isFile(const std::string &path)
{
    std::error_code ec;
    return fs::is_regular_file(p8(path), ec);
}

bool CxFilesystem::isDir(const std::string &path)
{
    std::error_code ec;
    return fs::is_directory(p8(path), ec);
}

CxFilesystem::FileStat CxFilesystem::stat(const std::string &path)
{
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
bool CxFilesystem::hasRootPath(const std::string &path)
{
    return p8(path).has_root_path();
}

bool CxFilesystem::hasRootName(const std::string &path)
{
    return p8(path).has_root_name();
}

bool CxFilesystem::hasRootDirectory(const std::string &path)
{
    return p8(path).has_root_directory();
}

bool CxFilesystem::isAbsolute(const std::string &path)
{
    return p8(path).is_absolute(); // 词法判断
}

bool CxFilesystem::isRelative(const std::string &path)
{
    return p8(path).is_relative();
}

// -------- 目录操作 --------
bool CxFilesystem::createDirs(const std::string &dir)
{
    std::error_code ec;
    const fs::path p = p8(dir);
    if (fs::exists(p, ec)) return true;
    return fs::create_directories(p, ec);
}

bool CxFilesystem::ensureDirs(const std::string &dir)
{
    return createDirs(dir);
}

bool CxFilesystem::removeOne(const std::string &path)
{
    std::error_code ec;
    return fs::remove(p8(path), ec);
}

std::uintmax_t CxFilesystem::removeAll(const std::string &path)
{
    std::error_code ec;
    return fs::remove_all(p8(path), ec);
}

bool CxFilesystem::rename(const std::string &from, const std::string &to)
{
    std::error_code ec;
    fs::rename(p8(from), p8(to), ec);
    return !ec;
}

// -------- 复制 --------
bool CxFilesystem::copyFile(const std::string &from, const std::string &to, bool overwrite)
{
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

bool CxFilesystem::copyDir(const std::string &from, const std::string &to, bool overwrite)
{
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
std::vector<std::string> CxFilesystem::listDir(const std::string &dir)
{
    std::vector<std::string> out;
    std::error_code ec;
    for (fs::directory_iterator it{p8(dir), ec}, end; it != end && !ec; it.increment(ec))
    {
        out.emplace_back(u8(it->path()));
    }
    return out;
}

std::vector<std::string> CxFilesystem::listDirRecursive(const std::string &dir)
{
    std::vector<std::string> out;
    std::error_code ec;
    for (fs::recursive_directory_iterator it{p8(dir), ec}, end; it != end && !ec; it.increment(ec))
    {
        out.emplace_back(u8(it->path()));
    }
    return out;
}

// -------- 文件读写 --------
bool CxFilesystem::readAllText(const std::string &path, std::string &out)
{
    std::ifstream ifs(p8(path));                 // 文本模式
    if (!ifs) return false;
    std::ostringstream ss;
    ss << ifs.rdbuf();
    out = ss.str();
    return true;
}

bool CxFilesystem::writeAllText(const std::string &path, const std::string &text, bool createParent)
{
    if (createParent)
    {
        std::error_code ec;
        auto parent = p8(path).parent_path();
        if (!parent.empty()) fs::create_directories(parent, ec);
    }
    std::ofstream ofs(p8(path));                 // 覆盖
    if (!ofs) return false;
    ofs << text;
    return bool(ofs);
}

bool CxFilesystem::readAllBytes(const std::string &path, std::vector<char> &out)
{
    std::ifstream ifs(p8(path), std::ios::binary);
    if (!ifs) return false;
    ifs.seekg(0, std::ios::end);
    const auto n = static_cast<std::size_t>(ifs.tellg());
    ifs.seekg(0, std::ios::beg);
    out.resize(n);
    if (n) ifs.read(reinterpret_cast<char *>(out.data()), static_cast<std::streamsize>(n));
    return bool(ifs);
}

bool CxFilesystem::writeAllBytes(const std::string &path, const std::vector<char> &data, bool createParent)
{
    if (createParent)
    {
        std::error_code ec;
        auto parent = p8(path).parent_path();
        if (!parent.empty()) fs::create_directories(parent, ec);
    }
    std::ofstream ofs(p8(path), std::ios::binary | std::ios::trunc);
    if (!ofs) return false;
    if (!data.empty())
    {
        ofs.write(reinterpret_cast<const char *>(data.data()),
                  static_cast<std::streamsize>(data.size()));
    }
    return bool(ofs);
}

// -------- 原子写：写临时文件 → flush/close → rename 覆盖 --------
bool CxFilesystem::writeAllTextAtomic(const std::string &path, const std::string &text, bool createParent)
{
    std::error_code ec;
    const fs::path dst = p8(path);
    const fs::path tmp = makeSiblingTemp(dst);

    if (createParent)
    {
        auto parent = dst.parent_path();
        if (!parent.empty()) fs::create_directories(parent, ec);
    }

    // 写临时文件
    {
        std::ofstream ofs(tmp);
        if (!ofs) return false;
        ofs << text;
        ofs.flush();
        if (!ofs)
        {
            std::error_code _;
            fs::remove(tmp, _);
            return false;
        }
    }
    // 原子替换（同目录 rename）
    fs::rename(tmp, dst, ec);
    if (ec)
    {
        std::error_code _;
        fs::remove(tmp, _);
        return false;
    }
    return true;
}

bool CxFilesystem::writeAllBytesAtomic(const std::string &path, const std::vector<char> &data, bool createParent)
{
    std::error_code ec;
    const fs::path dst = p8(path);
    const fs::path tmp = makeSiblingTemp(dst);

    if (createParent)
    {
        auto parent = dst.parent_path();
        if (!parent.empty()) fs::create_directories(parent, ec);
    }

    {
        std::ofstream ofs(tmp, std::ios::binary | std::ios::trunc);
        if (!ofs) return false;
        if (!data.empty())
        {
            ofs.write(reinterpret_cast<const char *>(data.data()),
                      static_cast<std::streamsize>(data.size()));
        }
        ofs.flush();
        if (!ofs)
        {
            std::error_code _;
            fs::remove(tmp, _);
            return false;
        }
    }
    fs::rename(tmp, dst, ec);
    if (ec)
    {
        std::error_code _;
        fs::remove(tmp, _);
        return false;
    }
    return true;
}

// -------- 路径工具 --------
std::string CxFilesystem::parentPath(const std::string &path)
{
    return u8(p8(path).parent_path());
}

std::string CxFilesystem::filename(const std::string &path)
{
    return u8(p8(path).filename());
}

std::string CxFilesystem::stem(const std::string &path)
{
    return u8(p8(path).stem());
}

std::string CxFilesystem::extension(const std::string &path)
{
    return u8(p8(path).extension());
}

std::string CxFilesystem::replaceExtension(const std::string &path, const std::string &newExtWithDot)
{
    fs::path p = p8(path);
    p.replace_extension(p8(newExtWithDot));
    return u8(p);
}

std::string CxFilesystem::replaceFilename(const std::string &path, const std::string &name)
{
    fs::path p = p8(path);
    p.replace_filename(p8(name));
    return u8(p);
}

std::string CxFilesystem::join(const std::string &a, const std::string &b)
{
    fs::path p = p8(a);
    p /= p8(b);
    return u8(p);
}

// -------- 路径拼接补全 --------
std::string CxFilesystem::join(const std::string &a, const std::string &b, const std::string &c)
{
    fs::path p = p8(a);
    p /= p8(b);
    p /= p8(c);
    return u8(p);
}

std::string CxFilesystem::join(const std::string &a, const std::string &b, const std::string &c, const std::string &d)
{
    fs::path p = p8(a);
    p /= p8(b);
    p /= p8(c);
    p /= p8(d);
    return u8(p);
}

std::string CxFilesystem::join(const std::string &a, const std::string &b, const std::string &c, const std::string &d, const std::string &e)
{
    fs::path p = p8(a);
    p /= p8(b);
    p /= p8(c);
    p /= p8(d);
    p /= p8(e);
    return u8(p);
}

std::string CxFilesystem::join(const std::string &a, const std::string &b, const std::string &c, const std::string &d, const std::string &e, const std::string &f)
{
    fs::path p = p8(a);
    p /= p8(b);
    p /= p8(c);
    p /= p8(d);
    p /= p8(e);
    p /= p8(f);
    return u8(p);
}

std::string CxFilesystem::joinMany(const std::string &base, const std::vector<std::string> &parts)
{
    std::filesystem::path p(base);
    for (const auto &s: parts) p /= s;
    return p.u8string();
}

std::string CxFilesystem::absolutePath(const std::string &path)
{
    std::error_code ec;
    return u8(fs::absolute(p8(path), ec));
}

std::string CxFilesystem::normalizeLexical(const std::string &path)
{
    fs::path p = p8(path).lexically_normal();
    return u8(p);
}

std::string CxFilesystem::normalizeGeneric(const std::string &path)
{
    fs::path p = p8(path).lexically_normal();
    return p.generic_u8string();
}

// -------- 工作目录（进程级） --------
bool CxFilesystem::setCwd(const std::string &dir)
{
    std::error_code ec;
    fs::current_path(p8(dir), ec);
    return !ec;
}

std::string CxFilesystem::getCwd()
{
    std::error_code ec;
    return u8(fs::current_path(ec));
}


// ---- pathStat / fileStat / dirStat / dirsStat ----
CxFilesystem::PathStat CxFilesystem::pathStat(const std::string &path)
{
    PathStat st;
    std::error_code ec;
    const fs::path p = p8(path);                // uses your helper :contentReference[oaicite:2]{index=2}
    st.path = u8(p);                           // uses your helper :contentReference[oaicite:3]{index=3}
    st.name = u8(p.filename());

    fs::directory_entry de(p, ec);
    st.exists = de.exists(ec);
    if (!st.exists) return st;

    st.isFile = de.is_regular_file(ec);
    st.isDir = de.is_directory(ec);
    st.kind = _detectKind(de);

    if (st.isFile)
    {
        std::error_code ec2;
        st.size = de.file_size(ec2);
    }
    else
    {
        st.size = 0;
    }

    std::error_code ec3;
    auto mtime = de.last_write_time(ec3);
    if (!ec3) st.modifyTime = _fileTime2tm(mtime);

    return st;
}

CxFilesystem::FileStat CxFilesystem::fileStat(const std::string &filePath)
{
    FileStat fsst;
    PathStat base = pathStat(filePath);
    static_cast<PathStat &>(fsst) = base;

    if (base.exists && base.isFile)
    {
        fsst.extension = u8(p8(filePath).extension());
    }
    return fsst;
}

CxFilesystem::DirStat CxFilesystem::dirStat(const std::string &dirPath, bool calcSize)
{
    DirStat ds;
    PathStat base = pathStat(dirPath);
    static_cast<PathStat &>(ds) = base;

    if (!(base.exists && base.isDir)) return ds;

    std::error_code ec;
    std::uintmax_t sz = 0;
    std::size_t files = 0, dirs = 0;

    for (fs::directory_iterator it{p8(dirPath), ec}, end; it != end && !ec; it.increment(ec))
    {
        std::error_code ec2;
        if (it->is_directory(ec2))
        {
            ++dirs;
        }
        else if (it->is_regular_file(ec2))
        {
            ++files;
            if (calcSize)
            {
                std::error_code ec3;
                sz += it->file_size(ec3);
            }
        }
    }
    ds.fileCount = files;
    ds.dirCount = dirs;
    if (calcSize) ds.size = sz;
    return ds;
}

std::vector<CxFilesystem::DirStat> CxFilesystem::dirsStat(const std::string &dirPath, bool calcSize)
{
    std::vector<DirStat> out;
    std::error_code ec;
    if (!fs::is_directory(p8(dirPath), ec)) return out;

    for (fs::directory_iterator it{p8(dirPath), ec}, end; it != end && !ec; it.increment(ec))
    {
        std::error_code ec2;
        if (it->is_directory(ec2))
        {
            out.emplace_back(dirStat(u8(it->path()), calcSize));
        }
    }
    return out;
}

// ---- stringify ----
constexpr char CHAR_SPLIT = ':';

static inline void _kv(std::ostringstream &oss, const char *k, const std::string &v, char sp)
{
    oss << sp << k << CHAR_SPLIT << v;
}

static inline void _kv(std::ostringstream &oss, const char *k, bool v, char sp)
{
    oss << sp << k << ":" << (v ? "true" : "false");
}

static inline void _kv(std::ostringstream &oss, const char *k, std::uintmax_t v, char sp)
{
    oss << sp << k << ":" << v;
}

static inline const char *_kindStr(CxFilesystem::PathKind k)
{
    using K = CxFilesystem::PathKind;
    switch (k)
    {
        case K::eNone:
            return "none";
        case K::eFile:
            return "file";
        case K::eDir:
            return "dir";
        case K::eSoftLink:
            return "link";
        case K::eDevChar:
            return "devchar";
        case K::eDevBlock:
            return "devblock";
        case K::eSystem:
            return "system";
    }
    return "none";
}

std::string CxFilesystem::toString(const PathStat &st, char spitChar)
{
    std::ostringstream oss;
    oss << "name" << CHAR_SPLIT << st.name;
    _kv(oss, "path", st.path, spitChar);
    _kv(oss, "kind", std::string(_kindStr(st.kind)), spitChar);
    _kv(oss, "exists", st.exists, spitChar);
    _kv(oss, "isFile", st.isFile, spitChar);
    _kv(oss, "isDir", st.isDir, spitChar);
    _kv(oss, "size", st.size, spitChar);
    {
        std::ostringstream ss;
        ss << std::put_time(&st.modifyTime, "%Y-%m-%d %H:%M:%S");
        _kv(ss, "modify", ss.str(), spitChar);
    }
//    if (!s.empty()) s.back() = '\n';
    return oss.str();
}

std::string CxFilesystem::toString(const FileStat &st, char spitChar)
{
    std::ostringstream oss;
    oss << toString(static_cast<const PathStat &>(st), spitChar);
    _kv(oss, "ext", st.extension, spitChar);
    return oss.str();
}

std::string CxFilesystem::toString(const DirStat &st, char spitChar)
{
    std::ostringstream oss;
    oss << toString(static_cast<const PathStat &>(st), spitChar);
    _kv(oss, "files", st.fileCount, spitChar);
    _kv(oss, "dirs", st.dirCount, spitChar);
    return oss.str();
}
