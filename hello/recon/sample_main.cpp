// examples/sample_main.cpp
#include <iostream>
#include <vector>
#include <string>
#include <cstdio>
#include <cstdlib>
#include <cstring>

#include "ct_recon_wrapper.hpp"

static void print_result_stats(const std::vector<float>& vol,
                               unsigned vx, unsigned vy, unsigned vz) {
    double sum = 0.0;
    double mn = 1e300, mx = -1e300;
    for (size_t i = 0; i < vol.size(); ++i) {
        double v = vol[i];
        sum += v;
        if (v < mn) mn = v;
        if (v > mx) mx = v;
    }
    double mean = vol.empty() ? 0.0 : (sum / (double)vol.size());
    std::cout << "[结果统计] 体素数=" << vol.size()
              << " min=" << mn << " max=" << mx << " mean=" << mean << "\n";
}

int main(int argc, char** argv) {
    // 1) 创建客户端 + 初始化
    CtReconClient cli;
    if (!cli.init(-1 /*CPU 或默认*/)) {
        std::cerr << "ct_recon_init 失败\n";
        return 1;
    }

    // 2) 设置回调
    CtReconClient::Callbacks cbs;
    cbs.onProgress = [](int p) {
        std::printf("进度: %3d%%\r", p);
        std::fflush(stdout);
    };
    cbs.onLog = [](RECON_LogLevel lv, const char* msg) {
        std::printf("[LOG %d] %s\n", (int)lv, msg);
    };
    cli.setCallbacks(cbs);

    // 3) 构造参数（示例值）
    const unsigned volx = 128, voly = 128, volz = 64;
    const unsigned detU = 256, detV = 192, projAngles = 180;

    RECON_Geometry geom = CtReconClient::makeGeometry(
        volx, voly, volz,
        projAngles, detU, detV,
        /*pixelSize*/ 0.1f, /*voxelSize*/ 0.2f,
        /*radiusSrc*/ 500.0f, /*radiusDet*/ 500.0f,
        /*sid*/ 400.0f, /*sdd*/ 800.0f);

    RECON_AlgoOptions algo = CtReconClient::makeAlgo(RECON_ALGO_FDK, /*flags*/ 0x3 /*保存切片+前处理(示例)*/);
    RECON_IterParams iters = CtReconClient::makeIters(50, 0.0f, 1.0f);

    // 4) 示例一：从“文件列表”重建（把真实投影路径放进来）
    std::vector<std::string> fileList;
    // 示例：fileList.push_back("projs/p000.raw"); ... // 自行替换

    std::vector<float> volA;
    if (!fileList.empty()) {
        std::cout << "\n=== 从文件列表重建 ===\n";
        int rc = cli.reconstructFromFiles(algo, geom, iters, fileList, /*pixelBits*/16, volA);
        std::cout << "\n返回码 rc=" << rc << "\n";
        if (rc == RECON_OK) print_result_stats(volA, volx, voly, volz);
    } else {
        std::cout << "\n(跳过文件列表示例：fileList 为空)\n";
    }

    // 5) 示例二：从“内存块集合”重建（模拟 16bit 原始投影）
    std::cout << "\n=== 从内存块重建（模拟数据） ===\n";
    // 创建一块“模拟投影数据”：projAngles * detV * detU * sizeof(uint16_t)
    const unsigned long long pixelsPerProj = (unsigned long long)detU * detV;
    const unsigned long long totalPixels = pixelsPerProj * projAngles;
    const unsigned long long bytes = totalPixels * sizeof(uint16_t);

    std::vector<uint16_t> fakeProj;
    try { fakeProj.resize((size_t)totalPixels); }
    catch (...) { std::cerr << "内存不足\n"; return 2; }

    // 填充一些伪数据（渐变）
    for (unsigned long long i = 0; i < totalPixels; ++i) {
        fakeProj[i] = (uint16_t)(i % 4096); // 0..4095 循环
    }

    CtReconClient::MemoryChunk chunk;
    chunk.data = (const void*)fakeProj.data();
    chunk.bytes = bytes;
    chunk.pixelBits = 16;

    std::vector<CtReconClient::MemoryChunk> chunks;
    chunks.push_back(chunk);

    std::vector<float> volB;
    int rc2 = cli.reconstructFromMemory(algo, geom, iters, chunks, volB);
    std::cout << "返回码 rc=" << rc2 << "\n";
    if (rc2 == RECON_OK) print_result_stats(volB, volx, voly, volz);

    // 6) （可选）演示取消：在另一个线程把 cli.requestCancel() 设为 1。
    //    这里用单线程不演示多线程取消。

    return 0;
}
