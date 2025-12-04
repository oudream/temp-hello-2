#include "reconstruction_client.h"


// 构造 RECON_Geometry / RECON_AlgoOptions / RECON_IterParams 的辅助
RECON_Geometry CtReconClient::makeGeometry(unsigned volx, unsigned voly, unsigned volz,
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

RECON_AlgoOptions CtReconClient::makeAlgo(RECON_Algorithm algo, unsigned flags)
{
    RECON_AlgoOptions a = {};
    a.algorithm = algo;
    a.flags = flags;
    return a;
}

RECON_IterParams CtReconClient::makeIters(int count, float vmin, float vmax)
{
    RECON_IterParams it;
    std::memset(&it, 0, sizeof(it));
    it.iter_count = count;
    it.min_constraint = vmin;
    it.max_constraint = vmax;
    return it;
}

CtReconClient::CtReconClient()
    : handle_(nullptr), cancelFlag_(0)
{

}

CtReconClient::~CtReconClient()
{
    deinit();
}


bool CtReconClient::init(int gpuIndex /*<0 表示 CPU 或默认*/)
{
    if (handle_) return true;
    RECON_InitOptions opt;
    std::memset(&opt, 0, sizeof(opt));
    opt.gpu_index = gpuIndex;
    return (ct_recon_init(&opt, &handle_) == RECON_OK);
}

void CtReconClient::deinit()
{
    if (handle_)
    {
        ct_recon_deinit(handle_);
        handle_ = nullptr;
    }
}

void CtReconClient::setCallbacks(const Callbacks& cb) { cb_ = cb; }

// 外部取消：线程安全性由外部保证（如在同线程或原子封装，此处简单按 int 语义）
void CtReconClient::requestCancel() { cancelFlag_ = 1; }
void CtReconClient::resetCancel() { cancelFlag_ = 0; }


// 从“文件路径列表”发起重建
int CtReconClient::reconstructFromFiles(const RECON_AlgoOptions& algo,
                         const RECON_Geometry& geom,
                         const RECON_IterParams& iter,
                         const std::vector<std::string>& filePathsUtf8,
                         int pixelBits,
                         std::vector<float>& outVolume /*输出：大小=volx*voly*volz*/,
                         unsigned alignBytes)
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

int CtReconClient::reconstructFromMemory(const RECON_AlgoOptions& algo,
                          const RECON_Geometry& geom,
                          const RECON_IterParams& iter,
                          const std::vector<MemoryChunk>& chunks,
                          std::vector<float>& outVolume,
                          unsigned alignBytes)
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

bool CtReconClient::ensureReady_() const { return handle_ != nullptr; }

void CtReconClient::progressTrampoline_(int percent, const RECON_UserData ud)
{
    if (const auto* self = static_cast<CtReconClient*>(ud); self && self->cb_.onProgress) self->cb_.
                onProgress(percent);
}

void CtReconClient::logTrampoline_(const RECON_LogLevel lv, const char* msg, const RECON_UserData ud)
{
    if (const auto* self = static_cast<CtReconClient*>(ud); self && self->cb_.onLog) self->cb_.onLog(
                lv, msg ? msg : "");
}

RECON_Geometry CtReconClient::defaultGeometry()
{
    RECON_Geometry g{};

    /* ---- 重建体素尺寸 ---- */
    g.vol_dim_x = 400;
    g.vol_dim_y = 400;
    g.vol_dim_z = 200;

    /* ---- 投影几何 ---- */
    g.proj_angles = 720;
    g.det_u       = 588;
    g.det_v       = 461;

    /* ---- 物理参数 ---- */
    g.pixel_size      = 0.0495f * 5;   // mm/pixel，示例
    g.voxel_size      = 0.1f;          // mm/voxel
    g.radius_source   = 46.735f;       // mm
    g.radius_detector = 184.602f;      // mm
    g.sid             = 68.755f;       // mm（Source → Isocenter）
    g.sdd             = 206.745f;      // mm（Source → Detector）

    /* ---- 探测器偏移/倾斜 ---- */
    g.offset_u = 0;
    g.offset_v = 0;

    g.tilt_u_deg      = 0.0f;
    g.tilt_v_deg      = 0.0f;
    g.tilt_inplane_deg= 0.0f;

    /* ---- 机型 ---- */
    g.type = 1;

    return g;
}

const char *CtReconClient::checkGeometry(const RECON_Geometry &g)
{
    /* --- 体素尺寸 --- */
    if (g.vol_dim_x == 0 || g.vol_dim_y == 0 || g.vol_dim_z == 0)
        return "vol_dim_x/y/z 不能为 0";

    if (g.vol_dim_x > 8192 || g.vol_dim_y > 8192 || g.vol_dim_z > 8192)
        return "vol_dim_x/y/z 太大（可能导致 GPU 内存溢出）";

    /* --- 投影几何 --- */
    if (g.proj_angles < 10)
        return "proj_angles 太少（至少需要 ≥10 张投影）";

    if (g.proj_angles < 1024)
        return "proj_angles 太多（至多需要 <1024 张投影）";

    if (g.det_u == 0 || g.det_v == 0)
        return "det_u 或 det_v 不能为 0";

    if (g.det_u > 10000 || g.det_v > 10000)
        return "det_u 或 det_v 太大（超过常规探测器范围）";

    /* --- 物理参数 --- */
    if (g.pixel_size <= 0.0f || g.pixel_size > 5.0f)
        return "pixel_size 不合理（单位 mm）";

    if (g.voxel_size <= 0.0f || g.voxel_size > 10.0f)
        return "voxel_size 不合理（单位 mm）";

    if (g.sid <= 0.0f || g.sdd <= 0.0f)
        return "SID 或 SDD 必须为正值";

    if (g.sdd <= g.sid)
        return "SDD 必须大于 SID（源 → 探测器距离 > 源 → 等中心距离）";

    /* --- 半径 --- */
    if (g.radius_source <= 0 || g.radius_detector <= 0)
        return "radius_source 或 radius_detector 必须为正";

    if (g.radius_detector <= g.radius_source)
        return "radius_detector 必须 > radius_source";

    /* --- 偏移与倾斜 --- */
    if (std::abs(g.tilt_u_deg) > 30.0f || std::abs(g.tilt_v_deg) > 30.0f)
        return "探测器倾斜角超过 ±30°（非常不正常）";

    if (std::abs(g.tilt_inplane_deg) > 45.0f)
        return "探测器 in-plane 倾斜超过 ±45°（极不合理）";

    /* --- 机型 --- */
    if (g.type != 0 && g.type != 1)
        return "type 必须为 0 或 1";

    /* --- 都通过 --- */
    return nullptr; // null 表示合法
}

bool CtReconClient::makeGeometrySafe(RECON_Geometry &out, const RECON_Geometry &in, std::string &err)
{
    const char* e = checkGeometry(in);
    if (e)
    {
        err = e;
        return false;
    }

    out = in;
    return true;
}

/* ---------- 算法选项默认值 ---------- */
RECON_AlgoOptions CtReconClient::defaultAlgoOptions()
{
    RECON_AlgoOptions a;
    a.algorithm = RECON_ALGO_FDK; // 默认用 FDK，当作基线算法
    a.flags     = 0;              // 默认不保存切片、不做前处理
    a.reserved0 = 0;
    return a;
}

/* ---------- 算法选项合法性检测 ---------- */
/* 返回 NULL 表示合法；否则返回错误描述字符串 */
const char *CtReconClient::checkAlgoOptions(const RECON_AlgoOptions &a)
{
    /* 算法枚举是否合法 */
    switch (a.algorithm)
    {
        case RECON_ALGO_FDK:
        case RECON_ALGO_CGLS:
        case RECON_ALGO_SIRT:
            break;
        default:
            return "algorithm 非法（不支持的重建算法）";
    }

    /* flags 暂时不做严格限制，留给后续扩展：
       bit0 = 保存切片，bit1 = 启用前处理，其他位保留
       这里只要求 reserved0 为 0，避免误传垃圾
    */
    if (a.reserved0 != 0)
        return "reserved0 必须为 0（预留字段禁止使用）";

    return nullptr;
}

/* 可选：安全构造封装 */
bool CtReconClient::makeAlgoOptionsSafe(RECON_AlgoOptions &out, enum RECON_Algorithm algo, unsigned int flags, std::string &err)
{
    RECON_AlgoOptions tmp;
    tmp.algorithm = algo;
    tmp.flags     = flags;
    tmp.reserved0 = 0;

    if (const char* e = checkAlgoOptions(tmp))
    {
        err = e;
        return false;
    }

    out = tmp;
    return true;
}

/* ---------- 迭代参数默认值 ---------- */
RECON_IterParams CtReconClient::defaultIterParams()
{
    RECON_IterParams p;
    p.iter_count     = 50;   // 默认 50 次，对 CGLS/SIRT 比较常见
    p.min_constraint = 0.0f; // 默认 [0,1] 归一化区间
    p.max_constraint = 1.0f;
    return p;
}

/* ---------- 迭代参数合法性检测 ---------- */
/* 返回 NULL 表示合法；否则返回错误描述字符串 */
const char *CtReconClient::checkIterParams(const RECON_IterParams &p)
{
    /* 迭代次数 */
    if (p.iter_count <= 0)
        return "iter_count 必须 >= 1";

    if (p.iter_count > 2000)
        return "iter_count 过大，可能导致重建时间过长";

    /* NaN 检查 */
    if (std::isnan(p.min_constraint) || std::isnan(p.max_constraint))
        return "min_constraint / max_constraint 不能为 NaN";

    /* 约束区间：
       - 两个都为 0 表示“关闭约束”，直接放行
       - 否则要求 max >= min，且区间不要离谱大
    */
    if (!(p.min_constraint == 0.0f && p.max_constraint == 0.0f))
    {
        if (p.max_constraint < p.min_constraint)
            return "max_constraint 不能小于 min_constraint";

        if (std::fabs(p.max_constraint - p.min_constraint) > 1e6f)
            return "约束区间过大，疑似参数设置错误";
    }

    return nullptr;
}

bool CtReconClient::makeIterParamsSafe(RECON_IterParams &out, int iter_count, float min_c, float max_c, std::string &err)
{
    RECON_IterParams tmp;
    tmp.iter_count     = iter_count;
    tmp.min_constraint = min_c;
    tmp.max_constraint = max_c;

    if (const char* e = checkIterParams(tmp))
    {
        err = e;
        return false;
    }

    out = tmp;
    return true;
}
