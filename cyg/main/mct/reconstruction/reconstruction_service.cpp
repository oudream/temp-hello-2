#include "reconstruction_service.h"
#include "reconstruction_client.h"


#include <helpers/log_helper.h>
#include <opencv2/opencv.hpp>


#include <ccxx/cxfilesystem.h>

static RECON_Handle handle_ = nullptr;
static int cancelFlag_ = 0; // 与 C 接口的 volatile int* 对接，简单整数足够（示例）
static RECON_ProgressCallback progressCallback_ = nullptr;

// 为 Singles 生存期保驾护航（避免临时数组过期）
static std::vector<RECON_InputSingle> singles_;


bool ReconstructionService::init(int gpuIndex)
{
    if (handle_) return true;
    RECON_InitOptions opt;
    std::memset(&opt, 0, sizeof(opt));
    opt.gpu_index = gpuIndex;
    return (ct_recon_init(&opt, &handle_) == RECON_OK);
}

void ReconstructionService::deInit()
{
    if (handle_)
    {
        ct_recon_deinit(handle_);
        handle_ = nullptr;
    }
}

void ReconstructionService::requestCancel()
{
    cancelFlag_ = 1;
}

void ReconstructionService::resetCancel()
{
    cancelFlag_ = 0;
}

int ReconstructionService::reconstructFromFiles(const RECON_AlgoOptions &algo, const RECON_Geometry &geom, const RECON_IterParams &iter
                                                , const std::vector<std::string> &filePathsUtf8, int pixelBits
                                                , std::vector<float> &outVolume
                                                , RECON_UserData userData
                                                , unsigned int alignBytes)
{
    if (!ensureReady_()) return RECON_ERR_NOT_INITIALIZED;

    // 准备输出缓冲
    const unsigned long long voxels = static_cast<unsigned long long>(geom.vol_dim_x) * geom.vol_dim_y * geom.vol_dim_z;
    if (voxels > outVolume.size())
    {
        outVolume.resize((size_t) voxels);
    }
    float* out_volume = outVolume.data();

    RECON_Response resp = {};
    resp.out_volume = out_volume;
    resp.out_bytes = voxels * sizeof(float);
    resp.dtype = RECON_DTYPE_F32;
    resp.alignment = alignBytes;
    resp.reserved_flags = (alignBytes ? 1u : 0u); // 示例：bit0 表“按 alignment 对齐”

    // 构造输入视图
    std::vector<const char *> cstrs;
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
//    {
//        const unsigned volx = 400, voly = 400, volz = 200;
//        const unsigned detU = 588, detV = 461, projAngles = 720;
//
//        req.geom = CtReconClient::makeGeometry(
//                volx, voly, volz,
//                projAngles, detU, detV,
//                /*pixelSize*/ 0.0495f*5, /*voxelSize*/ 0.1f,
//                /*radiusSrc*/ 46.735f, /*radiusDet*/ 184.602f,
//                /*sid*/ 68.755f, /*sdd*/ 206.745f);
//
//        req.algo = CtReconClient::makeAlgo(RECON_ALGO_FDK, 1 /*flags*/ /*0x3 *//*保存切片+前处理(示例)*/);//RECON_ALGO_FDK
//        req.iter= CtReconClient::makeIters(50, 0.0f, 1.0f);
//    }
    req.input = view;
    req.progress_cb = &progressTrampoline_;
    req.log_cb = &logTrampoline_;
    req.user_data = userData; /* 透传指针 */
    req.cancel_flag = static_cast<volatile int *>(&cancelFlag_);

    resetCancel();

    int rr = ct_recon_reconstruct(handle_, &req, &resp);
    return rr;
}

int ReconstructionService::reconstructFromMemory(const RECON_AlgoOptions &algo, const RECON_Geometry &geom, const RECON_IterParams &iter, const std::vector<MemoryChunk> &chunks, std::vector<float> &outVolume, unsigned int alignBytes)
{
    if (!ensureReady_()) return RECON_ERR_NOT_INITIALIZED;

    const unsigned long long voxels = static_cast<unsigned long long>(geom.vol_dim_x) * geom.vol_dim_y * geom.vol_dim_z;
    outVolume.resize((size_t) voxels);

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
    req.progress_cb = &progressTrampoline_;
    req.log_cb = &logTrampoline_;
    req.user_data = nullptr;
    req.cancel_flag = static_cast<volatile int *>(&cancelFlag_);

    resetCancel();
    return ct_recon_reconstruct(handle_, &req, &resp);
}

bool ReconstructionService::ensureReady_()
{
    return handle_ != nullptr;
}

void ReconstructionService::progressTrampoline_(int percent, RECON_UserData ud)
{
    if (progressCallback_ != nullptr)
    {
        progressCallback_(percent, ud);
    }
}

void ReconstructionService::logTrampoline_(RECON_LogLevel lv, const char *msg, RECON_UserData ud)
{
    auto level = (LogStream::LogLevel) lv;
    LogHelper::log(level, msg);
}

void ReconstructionService::setProgressCallback(RECON_ProgressCallback progressCallback)
{
    progressCallback_ = progressCallback;
}



