#include "win_mem.h"
#include <opencv2/opencv.hpp>
#include <filesystem>
#include <fstream>
#include <vector>
#include <string>
#include <cstring>
#include <iostream>
#include <algorithm>
#include <unordered_set>
#include <cctype>
#include <stdexcept>

namespace fs = std::filesystem;

// 固定头尾
static const uint8_t FIXED_HEAD[6] = {0xA5,0x5A,0x5A,0xA5,0xA5,0x5A};
static const uint8_t FIXED_TAIL[6] = {0xEB,0x90,0x90,0xEB,0xEB,0x90};

// 文件类型枚举
enum class FileType : uint32_t {
    Unknown = 0, JPEG=1, PNG=2, BMP=3, TIFF=4, WEBP=5,
    GIF=6, JP2=7, TGA=8, PNM=9, HDR=10, EXR=11
};

// 小工具：转小写
static std::string lower(std::string s){ for(auto&c:s)c=(char)tolower((unsigned char)c); return s; }

// 扩展名->类型
static FileType detectFileTypeByExt(const fs::path& p) {
    auto ext = lower(p.extension().string());
    if (ext==".jpg"||ext==".jpeg") return FileType::JPEG;
    if (ext==".png") return FileType::PNG;
    if (ext==".bmp") return FileType::BMP;
    if (ext==".tif"||ext==".tiff") return FileType::TIFF;
    if (ext==".webp") return FileType::WEBP;
    if (ext==".gif") return FileType::GIF;
    if (ext==".jp2") return FileType::JP2;
    if (ext==".tga") return FileType::TGA;
    if (ext==".ppm"||ext==".pgm"||ext==".pnm"||ext==".pbm") return FileType::PNM;
    if (ext==".hdr") return FileType::HDR;
    if (ext==".exr") return FileType::EXR;
    return FileType::Unknown;
}

// 读取磁盘原始字节
static std::vector<uint8_t> readFileBytes(const fs::path& p) {
    std::ifstream ifs(p, std::ios::binary);
    if (!ifs) return {};
    ifs.seekg(0,std::ios::end);
    auto len=ifs.tellg();
    if(len<0){return {};}
    ifs.seekg(0,std::ios::beg);
    std::vector<uint8_t> buf((size_t)len);
    if(len>0) ifs.read((char*)buf.data(),len);
    return buf;
}

// OpenCV 验证是否可读图像
static bool isValidImageWithOpenCV(const fs::path& p) {
    auto img = cv::imread(p.string(), cv::IMREAD_UNCHANGED);
    return !img.empty();
}

// 写入到大块内存的工具
static void writeBytes(WinMem& wm, SIZE_T& offset, const void* src, SIZE_T n) {
    SIZE_T need = offset + n;
    if (need > wm.reserved) throw std::runtime_error("超过10GB保留区");
    if (!wm.ensureCommitted(need)) throw std::runtime_error("提交内存失败");
    std::memcpy((BYTE*)wm.base + offset, src, n);
    offset += n;
}

// 解析尺寸字符串：如 "10k", "20mb", "8g"
static uint64_t parseSize(const std::string& s) {
    if (s.empty()) return 0;
    std::string t = lower(s);
    // 拆数字与单位
    size_t i = 0;
    while (i<t.size() && (std::isdigit((unsigned char)t[i]) || t[i]=='.')) ++i;
    double val = std::stod(t.substr(0,i));
    std::string unit = t.substr(i);
    if (unit=="" ) return (uint64_t)val;
    if (unit=="k" || unit=="kb")  return (uint64_t)(val * 1024.0);
    if (unit=="m" || unit=="mb")  return (uint64_t)(val * 1024.0*1024.0);
    if (unit=="g" || unit=="gb")  return (uint64_t)(val * 1024.0*1024.0*1024.0);
    // 未知单位，当作字节
    return (uint64_t)val;
}

// 命令行解析
struct Options {
    fs::path folder;
    std::unordered_set<std::string> typeSet; // 小写扩展名（不带点），如 "jpg","png"
    bool useTypeFilter=false;
    uint64_t minBytes=0;
    uint64_t maxBytes=UINT64_MAX;
    bool useMin=false, useMax=false;
    uint64_t stopMaxFiles=UINT64_MAX;
    uint64_t stopMaxContentBytes=UINT64_MAX;
};

static void printUsage(const char* exe) {
    std::cout <<
              "用法:\n"
              "  " << exe << " <图片文件夹> [--types=jpg,png,tiff] [--min=10k] [--max=20mb]\n"
                             "                      [--max-files=1000] [--max-bytes=8gb]\n"
                             "\n"
                             "说明:\n"
                             "  --types       仅处理这些扩展名(逗号分隔)，大小写不敏感\n"
                             "  --min/--max   过滤文件大小(按字节), 支持后缀 k, m, g (或 kb/mb/gb)\n"
                             "  --max-files   读够指定张数就停止\n"
                             "  --max-bytes   装入的\"文件内容字节\"总量达到上限即停止\n";
}

static Options parseArgs(int argc, char** argv) {
    Options o;
    if (argc < 2) { printUsage(argv[0]); std::exit(0); }
    o.folder = fs::path(argv[1]);
    if (!fs::exists(o.folder) || !fs::is_directory(o.folder)) {
        std::cerr << "错误：路径不存在或不是文件夹\n"; std::exit(1);
    }
    for (int i=2;i<argc;i++) {
        std::string a = argv[i];
        if (a.rfind("--types=",0)==0) {
            o.useTypeFilter = true;
            std::string s = a.substr(8);
            // 解析逗号分隔
            size_t start=0;
            while (start<=s.size()) {
                size_t pos = s.find(',', start);
                std::string one = (pos==std::string::npos) ? s.substr(start) : s.substr(start, pos-start);
                one = lower(one);
                // 去掉可能的点
                if (!one.empty() && one[0]=='.') one = one.substr(1);
                if(!one.empty()) o.typeSet.insert(one);
                if (pos==std::string::npos) break;
                start = pos+1;
            }
        } else if (a.rfind("--min=",0)==0) {
            o.minBytes = parseSize(a.substr(6));
            o.useMin = true;
        } else if (a.rfind("--max=",0)==0) {
            o.maxBytes = parseSize(a.substr(6));
            o.useMax = true;
        } else if (a.rfind("--max-files=",0)==0) {
            o.stopMaxFiles = std::stoull(a.substr(12));
        } else if (a.rfind("--max-bytes=",0)==0) {
            o.stopMaxContentBytes = parseSize(a.substr(12));
        } else {
            std::cerr << "未知参数: " << a << "\n";
            printUsage(argv[0]); std::exit(1);
        }
    }
    return o;
}

// 解析器：从内存扫描并打印每条记录的文件名、大小、内容起始地址
// layout: [6头][128名][4大小][4类型][4备用1][4备用2][96备用3][内容N][6尾]
static void scanAndPrint(void* base, SIZE_T usedBytes) {
    BYTE* p = (BYTE*)base;
    SIZE_T off = 0;
    size_t idx = 0;

    while (off + 6 + 128 + 4 + 4 + 4 + 4 + 96 + 6 <= usedBytes) {
        // 头
        if (std::memcmp(p + off, FIXED_HEAD, 6) != 0) {
            // 找不到头，尝试向前推进一个字节（容忍意外字节）
            off += 1;
            continue;
        }
        SIZE_T pos = off + 6;

        // 文件名 128
        char nameBuf[129] = {0};
        std::memcpy(nameBuf, p + pos, 128);
        pos += 128;

        // 大小、类型、备用
        if (pos + 4 + 4 + 4 + 4 + 96 > usedBytes) break;
        uint32_t fsize=0, ftype=0, r1=0, r2=0;
        std::memcpy(&fsize, p+pos, 4); pos+=4;
        std::memcpy(&ftype, p+pos, 4); pos+=4;
        std::memcpy(&r1,    p+pos, 4); pos+=4;
        std::memcpy(&r2,    p+pos, 4); pos+=4;
        pos += 96; // 备用3

        // 内容
        if (pos + fsize + 6 > usedBytes) break;
        void* contentPtr = (void*)(p + pos);
        pos += fsize;

        // 尾
        if (std::memcmp(p + pos, FIXED_TAIL, 6) != 0) {
            // 结构异常，尝试跳过一个字节继续
            off += 1;
            continue;
        }
        pos += 6;

        // 打印信息
        std::printf("#%zu  name=\"%s\"  size=%u  content@%p\n",
                    ++idx, nameBuf, fsize, contentPtr);

        // 下一个记录
        off = pos;
    }

    std::printf("共解析出 %zu 条记录。\n", idx);
}

int main(int argc,char**argv) try {
    Options opt = parseArgs(argc, argv);

    // 预留10GB
    const SIZE_T TEN_GB = 10ull * 1024ull * 1024ull * 1024ull;
    WinMem wm;
    if(!wm.reserve(TEN_GB)) return 2;
    std::cout<<"[OK] 预留10GB at "<< wm.base <<"\n";

    // 收集文件并排序
    std::vector<fs::path> files;
    for (auto& e: fs::directory_iterator(opt.folder)) {
        if (e.is_regular_file()) files.push_back(e.path());
    }
    std::sort(files.begin(), files.end());

    SIZE_T offset = 0;
    uint64_t totalContentBytes = 0;
    size_t writtenFiles = 0, skipped = 0;

    for (const auto& p : files) {
        // ---- 过滤：类型
        if (opt.useTypeFilter) {
            std::string ext = lower(p.extension().string());
            if (!ext.empty() && ext[0]=='.') ext = ext.substr(1);
            if (opt.typeSet.count(ext)==0) { skipped++; continue; }
        }
        // ---- 过滤：大小
        std::error_code ec;
        uint64_t sz = (uint64_t)fs::file_size(p, ec);
        if (ec) { skipped++; continue; }
        if (opt.useMin && sz < opt.minBytes) { skipped++; continue; }
        if (opt.useMax && sz > opt.maxBytes) { skipped++; continue; }

        // ---- OpenCV 验证是有效图像
        if (!isValidImageWithOpenCV(p)) { skipped++; continue; }

        // ---- 停止条件（写入前快速判断是否已够）
        if (writtenFiles >= opt.stopMaxFiles) break;
        if (totalContentBytes >= opt.stopMaxContentBytes) break;

        // 读取原始字节
        auto raw = readFileBytes(p);
        if (raw.empty()) { skipped++; continue; }
        uint32_t fsize = (uint32_t)raw.size();          // 单文件 <4GB
        uint32_t ftype = (uint32_t)detectFileTypeByExt(p);
        uint32_t zero = 0;

        // 文件名（UTF-8）最长127字节 + '\0'，不足补零
        std::string fname = p.filename().u8string();
        std::vector<uint8_t> fnameBuf(128, 0);
        if (fname.size() > 127) fname.resize(127);
        if (!fname.empty()) {
            std::memcpy(fnameBuf.data(), fname.data(), fname.size());
            fnameBuf[std::min<size_t>(127, fname.size())] = 0;
        }

        // 计算本条记录总大小，检查是否会超过保留区（防御）
        uint64_t recordBytes = 6 + 128 + 4 + 4 + 4 + 4 + 96 + raw.size() + 6;
        if ((uint64_t)offset + recordBytes > (uint64_t)wm.reserved) {
            std::cerr << "[WARN] 剩余空间不足，停止写入。\n";
            break;
        }

        // ---- 写入（边写边提交）
        writeBytes(wm, offset, FIXED_HEAD, sizeof(FIXED_HEAD));
        writeBytes(wm, offset, fnameBuf.data(), fnameBuf.size());
        writeBytes(wm, offset, &fsize, 4);
        writeBytes(wm, offset, &ftype, 4);
        writeBytes(wm, offset, &zero, 4);
        writeBytes(wm, offset, &zero, 4);
        uint8_t zeros96[96] = {0};
        writeBytes(wm, offset, zeros96, 96);
        // 记录内容起始地址（如需实时打印可在这里计算）
        void* contentPtr = (BYTE*)wm.base + offset;
        writeBytes(wm, offset, raw.data(), (SIZE_T)raw.size());
        writeBytes(wm, offset, FIXED_TAIL, sizeof(FIXED_TAIL));

        writtenFiles++;
        totalContentBytes += raw.size();

        if (writtenFiles % 20 == 0) {
            std::printf("[Progress] files=%zu  used=%.2f GB  committed=%.2f GB\n",
                        writtenFiles,
                        (double)offset / (1024.0*1024.0*1024.0),
                        (double)wm.committed / (1024.0*1024.0*1024.0));
        }

        // ---- 写入后再检查停止条件（保障“读够就好”）
        if (writtenFiles >= opt.stopMaxFiles) break;
        if (totalContentBytes >= opt.stopMaxContentBytes) break;
    }

    std::cout << "\n=== 写入完成 ===\n";
    std::cout << "成功: " << writtenFiles
              << "  跳过: " << skipped
              << "  内容总字节: " << (double)totalContentBytes/(1024.0*1024.0) << " MB\n";
    std::cout << "内存占用(偏移): " << (double)offset/(1024.0*1024.0*1024.0) << " GB\n";
    std::cout << "已提交: " << (double)wm.committed/(1024.0*1024.0*1024.0) << " GB\n\n";

    // ---- 从内存解析并打印每条记录（文件名、大小、内容地址）
    std::cout << "=== 内存解析 ===\n";
    scanAndPrint(wm.base, offset);

    // 如果后续还要用这块内存，可以不释放；这里演示释放
    wm.release();
    return 0;

} catch (const std::exception& ex) {
    std::fprintf(stderr, "异常: %s\n", ex.what());
    return 10;
}
