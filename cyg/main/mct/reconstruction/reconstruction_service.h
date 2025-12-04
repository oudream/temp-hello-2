#ifndef CX_CT_X2_RECONSTRUCTION_SERVICE_H
#define CX_CT_X2_RECONSTRUCTION_SERVICE_H


// 简洁易用的 C++ 门面；依赖 C 接口，不要求 C++17 以上
#include <vector>
#include <string>
#include <functional>
#include <cstring>

extern "C" {
#include <base/reconstruction_api.h>
}

/* ReconstructionService
 * - RAII 管理 ct_recon_init/ct_recon_deinit
 * - 以 std::function 包装进度/日志回调
 * - 提供从文件/从内存两种重建接口
 * - C++11/14 兼容（不使用 C++17 特性）
 */
class ReconstructionService
{
public:
    struct Callbacks
    {
        std::function<void(int, RECON_UserData)> onProgress; // 进度（0..100）
        std::function<void(RECON_LogLevel /*lv*/, const char * /*msg*/, RECON_UserData)> onLog; // 日志
    };

    static bool init(int gpuIndex = 0 /*<0 表示 CPU 或默认*/);

    static void deInit();

    static void setProgressCallback(RECON_ProgressCallback progressCallback);

    // 外部取消：线程安全性由外部保证（如在同线程或原子封装，此处简单按 int 语义）
    static void requestCancel();

    static void resetCancel();

    // 从“文件路径列表”发起重建
    static int reconstructFromFiles(const RECON_AlgoOptions &algo,
                                    const RECON_Geometry &geom,
                                    const RECON_IterParams &iter,
                                    const std::vector<std::string> &filePathsUtf8,
                                    int pixelBits,
                                    std::vector<float> &outVolume /*输出：大小=volx*voly*volz*/,
                                    RECON_UserData userData,
                                    unsigned alignBytes = 0);

    // 从“内存块集合”发起重建
    struct MemoryChunk
    {
        const void *data = nullptr;
        unsigned long long bytes = 0ULL;
        int pixelBits = 16; // 8/16/32
    };

    static int reconstructFromMemory(const RECON_AlgoOptions &algo,
                                     const RECON_Geometry &geom,
                                     const RECON_IterParams &iter,
                                     const std::vector<MemoryChunk> &chunks,
                                     std::vector<float> &outVolume,
                                     unsigned alignBytes = 0);

private:
    static bool ensureReady_();

    static void progressTrampoline_(int percent, RECON_UserData ud);


    static void logTrampoline_(RECON_LogLevel lv, const char *msg, RECON_UserData ud);

};

#endif //CX_CT_X2_RECONSTRUCTION_SERVICE_H
