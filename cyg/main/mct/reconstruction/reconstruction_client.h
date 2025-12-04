#ifndef CX_CT_X2_RECONSTRUCTION_CLIENT_H
#define CX_CT_X2_RECONSTRUCTION_CLIENT_H


// 简洁易用的 C++ 门面；依赖 C 接口，不要求 C++17 以上
#include <vector>
#include <string>
#include <functional>
#include <cstring>

extern "C" {
#include <base/reconstruction_api.h>
}

/* CtReconClient
 * - RAII 管理 ct_recon_init/ct_recon_deinit
 * - 以 std::function 包装进度/日志回调
 * - 提供从文件/从内存两种重建接口
 * - C++11/14 兼容（不使用 C++17 特性）
 */
class CtReconClient
{
public:
    // 构造 RECON_Geometry / RECON_AlgoOptions / RECON_IterParams 的辅助
    static RECON_Geometry makeGeometry(unsigned volx, unsigned voly, unsigned volz,
                                       unsigned projAngles, unsigned detU, unsigned detV,
                                       float pixelSize, float voxelSize,
                                       float radiusSrc, float radiusDet, float sid, float sdd);

    static RECON_AlgoOptions makeAlgo(RECON_Algorithm algo, unsigned flags);

    static RECON_IterParams makeIters(int count, float vmin, float vmax);

    static RECON_AlgoOptions defaultAlgoOptions();

    static const char* checkAlgoOptions(const RECON_AlgoOptions& a);

    static bool makeAlgoOptionsSafe(RECON_AlgoOptions& out, enum RECON_Algorithm algo, unsigned int flags, std::string& err);

    static RECON_Geometry defaultGeometry();

    static const char* checkGeometry(const RECON_Geometry& g);

    static bool makeGeometrySafe(RECON_Geometry& out, const RECON_Geometry& in, std::string& err);

    static RECON_IterParams defaultIterParams();

    static const char* checkIterParams(const RECON_IterParams& p);

    static bool makeIterParamsSafe(RECON_IterParams& out, int iter_count, float min_c, float max_c, std::string& err);

public:
    struct Callbacks
    {
        std::function<void(int)> onProgress; // 进度（0..100）
        std::function<void(RECON_LogLevel /*lv*/, const char* /*msg*/)> onLog; // 日志
    };

    CtReconClient();

    ~CtReconClient();

    // 禁止拷贝，允许移动
    CtReconClient(const CtReconClient&) = delete;
    CtReconClient& operator=(const CtReconClient&) = delete;

    bool init(int gpuIndex /*<0 表示 CPU 或默认*/);

    void deinit();

    void setCallbacks(const Callbacks& cb);

    // 外部取消：线程安全性由外部保证（如在同线程或原子封装，此处简单按 int 语义）
    void requestCancel();
    void resetCancel();

    // 从“文件路径列表”发起重建
    int reconstructFromFiles(const RECON_AlgoOptions& algo,
                             const RECON_Geometry& geom,
                             const RECON_IterParams& iter,
                             const std::vector<std::string>& filePathsUtf8,
                             int pixelBits,
                             std::vector<float>& outVolume /*输出：大小=volx*voly*volz*/,
                             unsigned alignBytes = 0);

    // 从“内存块集合”发起重建
    struct MemoryChunk
    {
        const void* data = nullptr;
        unsigned long long bytes = 0ULL;
        int pixelBits = 16; // 8/16/32
    };

    int reconstructFromMemory(const RECON_AlgoOptions& algo,
                              const RECON_Geometry& geom,
                              const RECON_IterParams& iter,
                              const std::vector<MemoryChunk>& chunks,
                              std::vector<float>& outVolume,
                              unsigned alignBytes = 0);

private:
    bool ensureReady_() const;

    static void progressTrampoline_(int percent, const RECON_UserData ud);

    static void logTrampoline_(const RECON_LogLevel lv, const char* msg, const RECON_UserData ud);

private:
    RECON_Handle handle_;
    int cancelFlag_; // 与 C 接口的 volatile int* 对接，简单整数足够（示例）
    Callbacks cb_;

    // 为 Singles 生存期保驾护航（避免临时数组过期）
    std::vector<RECON_InputSingle> singles_;

};

#endif //CX_CT_X2_RECONSTRUCTION_CLIENT_H
