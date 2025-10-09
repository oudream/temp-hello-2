// test_cxfilesystem.cpp
// 需要 C++17 及 GoogleTest (可用 vcpkg 安装: vcpkg install gtest)
// 测试目标：CxFilesystem 的全部公开接口

#include <gtest/gtest.h>
#include <filesystem>
#include <fstream>
#include <thread>
#include <unordered_set>

#include "cxfilesystem.h"   // 被测头文件

namespace fs = std::filesystem;

// --------- 测试辅助：临时沙箱夹具（为每个测试自动创建/销毁唯一目录） ---------
class FsSandbox : public ::testing::Test
{
protected:
    fs::path root_;
    fs::path old_cwd_;

    static fs::path UniqueName()
    {
        auto now = std::chrono::high_resolution_clock::now().time_since_epoch().count();
        auto tid = std::hash<std::thread::id>{}(std::this_thread::get_id());
        std::string name = "gtest_fs_sandbox_" + std::to_string(tid) + "_" + std::to_string(now);
        return fs::temp_directory_path() / fs::path(name);
    }

    void SetUp() override
    {
        root_ = UniqueName();
        ASSERT_NO_THROW(fs::create_directories(root_));
        // 记录当前工作目录
        std::error_code ec;
        old_cwd_ = fs::current_path(ec);
        // 切换到沙箱
        fs::current_path(root_, ec);
        ASSERT_FALSE(ec);
    }

    void TearDown() override
    {
        // 还原工作目录
        std::error_code ec;
        fs::current_path(old_cwd_, ec);
        // 尽力删除沙箱
        std::uintmax_t removed = fs::remove_all(root_, ec);
        (void) removed;
    }

    // 工具：将 std::vector<std::string> 转为不关心顺序的集合
    static std::unordered_set<std::string> ToSet(const std::vector<std::string> &v)
    {
        return std::unordered_set<std::string>(v.begin(), v.end());
    }
};

// ---------- 基础存在性/类型/大小 ----------
TEST_F(FsSandbox, Exists_IsFile_IsDir_Stat)
{
    std::string d = "a/b/c";
    EXPECT_TRUE(CxFilesystem::createDirs(d));
    EXPECT_TRUE(CxFilesystem::exists(d));
    EXPECT_TRUE(CxFilesystem::isDir(d));
    EXPECT_FALSE(CxFilesystem::isFile(d));

    std::string f = d + "/hello.txt";
    {
        std::ofstream ofs(f);
        ofs << "abc";
    }
    EXPECT_TRUE(CxFilesystem::exists(f));
    EXPECT_TRUE(CxFilesystem::isFile(f));
    EXPECT_FALSE(CxFilesystem::isDir(f));

    auto st_dir = CxFilesystem::stat(d);
    EXPECT_TRUE(st_dir.exists);
    EXPECT_TRUE(st_dir.isDir);
    EXPECT_FALSE(st_dir.isFile);

    auto st_file = CxFilesystem::stat(f);
    EXPECT_TRUE(st_file.exists);
    EXPECT_TRUE(st_file.isFile);
    EXPECT_GE(st_file.size, 3u);

    // 不存在
    auto st_none = CxFilesystem::stat("no_such_path____");
    EXPECT_FALSE(st_none.exists);
}

// ---------- 根路径/绝对相对 ----------
TEST_F(FsSandbox, Root_Absolute_Relative)
{
#ifdef _WIN32
    std::string absPath = "C:\\Windows";      // 用词法判断，不要求真实存在
    std::string relPath = "folder\\file.txt";
    EXPECT_TRUE(CxFilesystem::hasRootPath(absPath));
    EXPECT_TRUE(CxFilesystem::hasRootName(absPath));       // 盘符
    EXPECT_TRUE(CxFilesystem::hasRootDirectory(absPath));  // 根目录 '\'
    EXPECT_TRUE(CxFilesystem::isAbsolute(absPath));
    EXPECT_TRUE(CxFilesystem::isRelative(relPath));
#else
    std::string absPath = "/usr/bin";
    std::string relPath = "folder/file.txt";
    EXPECT_TRUE(CxFilesystem::hasRootPath(absPath));
    EXPECT_FALSE(CxFilesystem::hasRootName(absPath));      // POSIX 无 root name
    EXPECT_TRUE(CxFilesystem::hasRootDirectory(absPath));
    EXPECT_TRUE(CxFilesystem::isAbsolute(absPath));
    EXPECT_TRUE(CxFilesystem::isRelative(relPath));
#endif
}

// ---------- 目录操作 及 重命名 ----------
TEST_F(FsSandbox, CreateDirs_Remove_Rename)
{
    std::string d = "p/q/r";
    EXPECT_TRUE(CxFilesystem::createDirs(d));
    EXPECT_TRUE(CxFilesystem::createDirs(d));   // 幂等

    std::string f1 = d + "/data.bin";
    {
        std::ofstream ofs(f1, std::ios::binary);
        ofs.write("XYZ", 3);
    }
    EXPECT_TRUE(CxFilesystem::exists(f1));

    std::string f2 = d + "/data2.bin";
    EXPECT_TRUE(CxFilesystem::rename(f1, f2));
    EXPECT_FALSE(CxFilesystem::exists(f1));
    EXPECT_TRUE(CxFilesystem::exists(f2));

    EXPECT_TRUE(CxFilesystem::removeOne(f2));
    EXPECT_FALSE(CxFilesystem::exists(f2));

    // removeAll 删除整个目录树
    std::uintmax_t cnt = CxFilesystem::removeAll("p");
    EXPECT_GE(cnt, 1u);
    EXPECT_FALSE(CxFilesystem::exists("p"));
}

// ---------- 文件读写（文本/二进制） ----------
TEST_F(FsSandbox, ReadWrite_Text_And_Bytes)
{
    std::string ftxt = "x/y/z/hello.txt";
    std::string content = u8"中文UTF-8 + ascii ABC 123\n第二行";
    EXPECT_TRUE(CxFilesystem::writeAllText(ftxt, content, /*createParent=*/true));

    std::string out;
    EXPECT_TRUE(CxFilesystem::readAllText(ftxt, out));
    EXPECT_EQ(out, content);

    // 二进制
    std::string fbin = "x/y/z/raw.bin";
    std::vector<char> bytes = {0, 1, 2, 3, 4, 5, char(0xFF)};
    EXPECT_TRUE(CxFilesystem::writeAllBytes(fbin, bytes, /*createParent=*/true));

    std::vector<char> outb;
    EXPECT_TRUE(CxFilesystem::readAllBytes(fbin, outb));
    EXPECT_EQ(outb, bytes);

    // 若不创建父目录，应在父目录不存在时失败（文本/二进制各测一次）
    std::string ftxt2 = "no_parent_dir/xx.txt";
    EXPECT_FALSE(CxFilesystem::writeAllText(ftxt2, "x", /*createParent=*/false));

    std::string fbin2 = "no_parent_dir/xx.bin";
    EXPECT_FALSE(CxFilesystem::writeAllBytes(fbin2, bytes, /*createParent=*/false));
}

// ---------- 原子写入（文本/二进制） ----------
TEST_F(FsSandbox, AtomicWrites_Text_And_Bytes)
{
    std::string f = "atom/a.txt";
    // 初始内容
    EXPECT_TRUE(CxFilesystem::writeAllText(f, "old", true));

    // 原子替换
    EXPECT_TRUE(CxFilesystem::writeAllTextAtomic(f, "new中文", true));
    std::string out;
    EXPECT_TRUE(CxFilesystem::readAllText(f, out));
    EXPECT_EQ(out, "new中文");

    // 二进制
    std::string fb = "atom/b.bin";
    std::vector<char> oldb = {1, 2, 3};
    std::vector<char> newb = {9, 8, 7, 6};
    EXPECT_TRUE(CxFilesystem::writeAllBytes(fb, oldb, true));
    EXPECT_TRUE(CxFilesystem::writeAllBytesAtomic(fb, newb, true));
    std::vector<char> got;
    EXPECT_TRUE(CxFilesystem::readAllBytes(fb, got));
    EXPECT_EQ(got, newb);
}

// ---------- 复制（文件/目录，含覆盖与创建父目录） ----------
TEST_F(FsSandbox, CopyFile_And_CopyDir)
{
    // 文件复制
    std::string src = "srcdir/file.txt";
    EXPECT_TRUE(CxFilesystem::writeAllText(src, "A", true));

    // 目标父目录不存在，也应自动创建
    std::string dst = "dst1/sub/file.txt";
    EXPECT_TRUE(CxFilesystem::copyFile(src, dst, /*overwrite=*/false));
    std::string out;
    EXPECT_TRUE(CxFilesystem::readAllText(dst, out));
    EXPECT_EQ(out, "A");

    // 不覆盖时再复制同路径应失败（返回 false）
    EXPECT_FALSE(CxFilesystem::copyFile(src, dst, /*overwrite=*/false));

    // 覆盖
    EXPECT_TRUE(CxFilesystem::writeAllText(src, "B", true));
    EXPECT_TRUE(CxFilesystem::copyFile(src, dst, /*overwrite=*/true));
    EXPECT_TRUE(CxFilesystem::readAllText(dst, out));
    EXPECT_EQ(out, "B");

    // 目录复制（递归）
    EXPECT_TRUE(CxFilesystem::writeAllText("srcdir/inner/a.txt", "1", true));
    EXPECT_TRUE(CxFilesystem::writeAllText("srcdir/inner/b.txt", "2", true));
    EXPECT_TRUE(CxFilesystem::copyDir("srcdir", "dst2", /*overwrite=*/false));
    EXPECT_TRUE(CxFilesystem::exists("dst2/inner/a.txt"));
    EXPECT_TRUE(CxFilesystem::exists("dst2/inner/b.txt"));

    // 覆盖复制
    EXPECT_TRUE(CxFilesystem::writeAllText("srcdir/inner/b.txt", "22", true));
    EXPECT_TRUE(CxFilesystem::copyDir("srcdir", "dst2", /*overwrite=*/true));
    std::string btxt;
    EXPECT_TRUE(CxFilesystem::readAllText("srcdir/inner/b.txt", btxt));
    EXPECT_EQ(btxt, "22");
}

// ---------- 枚举（非递归/递归） ----------
TEST_F(FsSandbox, ListDir_And_ListDirRecursive)
{
    EXPECT_TRUE(CxFilesystem::createDirs("L1"));
    EXPECT_TRUE(CxFilesystem::writeAllText("L1/a.txt", "a", true));
    EXPECT_TRUE(CxFilesystem::createDirs("L1/sub"));
    EXPECT_TRUE(CxFilesystem::writeAllText("L1/sub/b.txt", "b", true));

    auto v1 = CxFilesystem::listDir("L1");
    auto s1 = ToSet(v1);
    // 只应包含 L1/a.txt 与 L1/sub（平台路径风格可能不同，统一用 generic 判断）
    EXPECT_EQ(s1.size(), 2u);

    auto v2 = CxFilesystem::listDirRecursive("L1");
    auto s2 = ToSet(v2);
    // 递归应至少包含三项：L1/a.txt, L1/sub, L1/sub/b.txt
    EXPECT_GE(s2.size(), 3u);
}

// ---------- 路径工具 ----------
TEST_F(FsSandbox, PathUtilities)
{
    std::string p = CxFilesystem::join("foo", "bar.txt");
#ifdef _WIN32
    // Windows 下 join 会用 '\'
    EXPECT_TRUE(p.find("foo\\bar.txt") != std::string::npos);
#else
    EXPECT_EQ(p, "foo/bar.txt");
#endif

    EXPECT_EQ(CxFilesystem::parentPath(p), CxFilesystem::normalizeLexical("foo"));
    EXPECT_EQ(CxFilesystem::filename(p), "bar.txt");
    EXPECT_EQ(CxFilesystem::stem(p), "bar");
    EXPECT_EQ(CxFilesystem::extension(p), ".txt");
    EXPECT_EQ(CxFilesystem::replaceExtension(p, ".dat"),
              CxFilesystem::join("foo", "bar.dat"));
    EXPECT_EQ(CxFilesystem::replaceFilename(p, "baz.bin"),
              CxFilesystem::join("foo", "baz.bin"));

    // 规范化（去掉 ./ 与 .. 等）
    EXPECT_EQ(CxFilesystem::normalizeLexical("a/./b/../c"), CxFilesystem::normalizeLexical("a/c"));
    // generic 使用 '/' 作为分隔
    auto g = CxFilesystem::normalizeGeneric(p);
    EXPECT_TRUE(g.find('/') != std::string::npos);

    // 绝对路径可被计算（不要求存在）
    auto abs = CxFilesystem::absolutePath("alpha/beta");
    EXPECT_TRUE(CxFilesystem::isAbsolute(abs));
}

// ---------- 工作目录 ----------
TEST_F(FsSandbox, SetGetCwd)
{
    // 记录原 cwd（在夹具中已经被改成 root_，这里只验证 API 正常工作）
    auto cwd1 = CxFilesystem::getCwd();
    EXPECT_FALSE(cwd1.empty());

    // 新目录
    std::string d = "work/cwd_test";
    EXPECT_TRUE(CxFilesystem::createDirs(d));
    EXPECT_TRUE(CxFilesystem::setCwd(d));

    auto cwd2 = CxFilesystem::getCwd();
#ifdef _WIN32
    // 大小写不强制；只要以 d 结尾即可
    EXPECT_TRUE(fs::path(cwd2).filename().u8string() == fs::path(d).filename().u8string());
#else
    EXPECT_TRUE(cwd2.size() >= d.size());
    EXPECT_EQ(fs::path(cwd2).filename().u8string(), fs::path(d).filename().u8string());
#endif
}
