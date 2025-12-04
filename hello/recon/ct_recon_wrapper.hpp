#ifndef CX_CT_X2_RECON_WRAPPER_HPP
#define CX_CT_X2_RECON_WRAPPER_HPP

// src/ct_recon_wrapper.hpp
// 简洁易用的 C++ 门面；依赖 C 接口，不要求 C++17 以上
#include <vector>
#include <string>
#include <functional>
#include <cstring>

extern "C" {
#include "ct_recon_api.h"
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
    struct Callbacks
    {
        std::function<void(int)> onProgress; // 进度（0..100）
        std::function<void(RECON_LogLevel /*lv*/, const char* /*msg*/)> onLog; // 日志
    };

    CtReconClient() : handle_(nullptr), cancelFlag_(0)
    {
    }

    ~CtReconClient() { deinit(); }

    // 禁止拷贝，允许移动
    CtReconClient(const CtReconClient&) = delete;
    CtReconClient& operator=(const CtReconClient&) = delete;

    bool init(int gpuIndex /*<0 表示 CPU 或默认*/)
    {
        if (handle_) return true;
        RECON_InitOptions opt;
        std::memset(&opt, 0, sizeof(opt));
        opt.gpu_index = gpuIndex;
        return (ct_recon_init(&opt, &handle_) == RECON_OK);
    }

    void deinit()
    {
        if (handle_)
        {
            ct_recon_deinit(handle_);
            handle_ = nullptr;
        }
    }

    void setCallbacks(const Callbacks& cb) { cb_ = cb; }

    // 外部取消：线程安全性由外部保证（如在同线程或原子封装，此处简单按 int 语义）
    void requestCancel() { cancelFlag_ = 1; }
    void resetCancel() { cancelFlag_ = 0; }

    // 构造 RECON_Geometry / RECON_AlgoOptions / RECON_IterParams 的辅助
    static RECON_Geometry makeGeometry(unsigned volx, unsigned voly, unsigned volz,
                                    unsigned projAngles, unsigned detU, unsigned detV,
                                    float pixelSize, float voxelSize,
                                    float radiusSrc, float radiusDet, float sid, float sdd)
    {
        RECON_Geometry g = {};
        g.vol_dim_x = volx;
        g.vol_dim_y = voly;
        g.vol_dim_z = volz;
        g.proj_angles = projAngles;
        g.det_u = detU;
        g.det_v = detV;
        g.pixel_size = pixelSize;
        g.voxel_size = voxelSize;
        g.radius_source = radiusSrc;
        g.radius_detector = radiusDet;
        g.sid = sid;
        g.sdd = sdd;
        g.type = 0; // 默认：倾斜圆锥束 等
        return g;
    }

    static RECON_AlgoOptions makeAlgo(RECON_Algorithm algo, unsigned flags)
    {
        RECON_AlgoOptions a = {};
        a.algorithm = algo;
        a.flags = flags;
        return a;
    }

    static RECON_IterParams makeIters(int count, float vmin, float vmax)
    {
        RECON_IterParams it;
        std::memset(&it, 0, sizeof(it));
        it.iter_count = count;
        it.min_constraint = vmin;
        it.max_constraint = vmax;
        return it;
    }

    // 从“文件路径列表”发起重建
    int reconstructFromFiles(const RECON_AlgoOptions& algo,
                             const RECON_Geometry& geom,
                             const RECON_IterParams& iter,
                             const std::vector<std::string>& filePathsUtf8,
                             int pixelBits,
                             std::vector<float>& outVolume /*输出：大小=volx*voly*volz*/,
                             unsigned alignBytes = 0)
    {
        if (!ensureReady_()) return RECON_ERR_NOT_INITIALIZED;

        // 准备输出缓冲
        const unsigned long long voxels = static_cast<unsigned long long>(geom.vol_dim_x) * geom.vol_dim_y * geom.
            vol_dim_z;
        outVolume.resize((size_t)voxels);

        RECON_Response resp = {};
        resp.out_volume = outVolume.data();
        resp.out_bytes = voxels * sizeof(float);
        resp.dtype = RECON_DTYPE_F32;
        resp.alignment = alignBytes;
        resp.reserved_flags = (alignBytes ? 1u : 0u); // 示例：bit0 表“按 alignment 对齐”

        // 构造输入视图
        std::vector<const char*> cstrs;
        cstrs.reserve(filePathsUtf8.size());
        for (size_t i = 0; i < filePathsUtf8.size(); ++i) cstrs.push_back(filePathsUtf8[i].c_str());

        RECON_InputFiles files = {};
        files.paths_utf8 = (filePathsUtf8.empty() ? nullptr : &cstrs[0]);
        files.count = static_cast<int>(filePathsUtf8.size());
        files.pixel_bits = pixelBits;

        RECON_InputView view = {};
        view.kind = RECON_INPUT_FILES;
        view.u.files = files;

        // 组装请求
        RECON_Request req = {};
        req.algo = algo;
        req.geom = geom;
        req.iter = iter;
        req.input = view;
        req.progress_cb = &CtReconClient::progressTrampoline_;
        req.log_cb = &CtReconClient::logTrampoline_;
        req.user_data = this; /* 透传 this */
        req.cancel_flag = static_cast<volatile int*>(&cancelFlag_);

        resetCancel();
        return ct_recon_reconstruct(handle_, &req, &resp);
    }

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
                              unsigned alignBytes = 0)
    {
        if (!ensureReady_()) return RECON_ERR_NOT_INITIALIZED;

        const unsigned long long voxels = static_cast<unsigned long long>(geom.vol_dim_x) * geom.vol_dim_y * geom.vol_dim_z;
        outVolume.resize((size_t)voxels);

        RECON_Response resp = {};
        resp.out_volume = outVolume.data();
        resp.out_bytes = voxels * sizeof(float);
        resp.dtype = RECON_DTYPE_F32;
        resp.alignment = alignBytes;
        resp.reserved_flags = (alignBytes ? 1u : 0u);

        // Singles
        singles_.clear();
        singles_.reserve(chunks.size());
        for (size_t i = 0; i < chunks.size(); ++i)
        {
            RECON_InputSingle si = {};
            si.data = chunks[i].data;
            si.bytes = chunks[i].bytes;
            si.pixel_bits = chunks[i].pixelBits;
            singles_.push_back(si);
        }
        RECON_InputSingles ss = {};
        ss.singles = singles_.empty() ? nullptr : &singles_[0];
        ss.count = static_cast<int>(singles_.size());

        RECON_InputView view = {};
        view.kind = RECON_INPUT_SINGLES;
        view.u.singles = ss;

        RECON_Request req = {};
        req.algo = algo;
        req.geom = geom;
        req.iter = iter;
        req.input = view;
        req.progress_cb = &CtReconClient::progressTrampoline_;
        req.log_cb = &CtReconClient::logTrampoline_;
        req.user_data = this;
        req.cancel_flag = static_cast<volatile int*>(&cancelFlag_);

        resetCancel();
        return ct_recon_reconstruct(handle_, &req, &resp);
    }

private:
    bool ensureReady_() const { return handle_ != nullptr; }

    static void progressTrampoline_(int percent, const RECON_UserData ud)
    {
        if (const auto* self = static_cast<CtReconClient*>(ud); self && self->cb_.onProgress) self->cb_.
            onProgress(percent);
    }

    static void logTrampoline_(const RECON_LogLevel lv, const char* msg, const RECON_UserData ud)
    {
        if (const auto* self = static_cast<CtReconClient*>(ud); self && self->cb_.onLog) self->cb_.onLog(
            lv, msg ? msg : "");
    }

private:
    RECON_Handle handle_;
    int cancelFlag_; // 与 C 接口的 volatile int* 对接，简单整数足够（示例）
    Callbacks cb_;

    // 为 Singles 生存期保驾护航（避免临时数组过期）
    std::vector<RECON_InputSingle> singles_;
};

#endif //CX_CT_X2_RECON_WRAPPER_HPP
