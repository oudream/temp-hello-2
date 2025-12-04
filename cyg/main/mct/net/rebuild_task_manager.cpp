#include "rebuild_task_manager.h"
#include "helpers/opencv_helper.h"


#include <ccxx/cxstring.h>
#include <ccxx/cxcontainer.h>


static std::mutex mtx_;
static std::map<std::string, RebuildTaskState> tasks_; // key = sectionId
// 当前正在执行重建的任务 id（用于防止被淘汰）
static std::string s_activeSectionId;

// 仅允许一个后台重建线程在运行
static std::thread* s_worker_ = nullptr;
static std::atomic<bool> s_running{false};
// 固定 2GB 输出体素缓冲区（约 536,870,912 个 float）
static std::vector<float> s_outVolume(2ull * 1024 * 1024 * 1024 / sizeof(float));

// ========== 内部：extras → 几何 ========== //
RECON_Geometry makeGeometryFromExtras_(const std::unordered_map<std::string, std::string> &extras, size_t numProjections)
{
    RECON_Geometry g;

    g.vol_dim_x = CxContainer::valueTo(extras, "vol_dim_x", 512u);
    g.vol_dim_y = CxContainer::valueTo(extras, "vol_dim_y", 512u);
    g.vol_dim_z = CxContainer::valueTo(extras, "vol_dim_z", 512u);

    g.proj_angles = (extras.count("proj_angles") ?
                     CxContainer::valueTo(extras, "proj_angles", (unsigned int)numProjections)
                                                 : (unsigned int)numProjections);
    g.det_u = CxContainer::valueTo(extras, "det_u", 1024u);
    g.det_v = CxContainer::valueTo(extras, "det_v", 1024u);

    g.pixel_size      = CxContainer::valueTo(extras, "pixel_size", 0.1f);
    g.voxel_size      = CxContainer::valueTo(extras, "voxel_size", 0.1f);
    g.radius_source   = CxContainer::valueTo(extras, "radius_source", 0.0f);
    g.radius_detector = CxContainer::valueTo(extras, "radius_detector", 0.0f);
    g.sid             = CxContainer::valueTo(extras, "sid", 0.0f);
    g.sdd             = CxContainer::valueTo(extras, "sdd", 0.0f);

    g.offset_u        = CxContainer::valueTo(extras, "offset_u", 0);
    g.offset_v        = CxContainer::valueTo(extras, "offset_v", 0);
    g.tilt_u_deg      = CxContainer::valueTo(extras, "tilt_u_deg", 0.0f);
    g.tilt_v_deg      = CxContainer::valueTo(extras, "tilt_v_deg", 0.0f);
    g.tilt_inplane_deg= CxContainer::valueTo(extras, "tilt_inplane_deg", 0.0f);
    g.type            = CxContainer::valueTo(extras, "type", 1);

    return g;
}

// ========== 内部：extras → 算法选项 ========== //
RECON_AlgoOptions makeAlgoFromExtras_(const std::unordered_map<std::string, std::string> &extras)
{
    RECON_AlgoOptions a;
    a.algorithm = (RECON_Algorithm)CxContainer::valueTo(extras, "algorithm", (int)RECON_ALGO_FDK);
    a.flags     = CxContainer::valueTo(extras, "flags", 0u);
    a.reserved0 = 0;
    return a;
}

// ========== 内部：extras → 迭代参数 ========== //
RECON_IterParams makeIterFromExtras_(const std::unordered_map<std::string, std::string> &extras)
{
    RECON_IterParams it;
    it.iter_count     = CxContainer::valueTo(extras, "iter_count", 0);
    it.max_constraint = CxContainer::valueTo(extras, "max_constraint", +1e30f);
    it.min_constraint = CxContainer::valueTo(extras, "min_constraint", -1e30f);
    return it;
}


// ========== BEGIN 阶段 ========== //
bool RebuildTaskManager::beginTask(const std::string &sectionId, const cx::DateTime &sectionTime, int expectImages, int pixelBits,
                                   const std::unordered_map<std::string, std::string> &extras)
{
    std::lock_guard<std::mutex> lk(mtx_);
    if (tasks_.find(sectionId) != tasks_.end())
    {
        return false;
    }

    // ------ 容量控制：最多保留 8 个任务（在创建新任务之前） ------
    // 若当前已有 >= 8 个任务，则先删除 sectionTime 最小的旧任务，
    // 删除时跳过当前正在重建的任务 s_activeSectionId。
    while (tasks_.size() >= 8)
    {
        auto oldestIt = tasks_.end();
        for (auto it = tasks_.begin(); it != tasks_.end(); ++it)
        {
            // 跳过正在重建的任务
            if (!s_activeSectionId.empty() && it->first == s_activeSectionId)
                continue;

            if (oldestIt == tasks_.end() || it->second.sectionTime < oldestIt->second.sectionTime)
            {
                oldestIt = it;
            }
        }

        if (oldestIt == tasks_.end())
        {
            // 理论上不会发生：除正在运行的一个之外，其它都可以被删除
            break;
        }

        LogHelper::debug() << "[Rebuild-Begin] purge old task, id=" << oldestIt->first;
        tasks_.erase(oldestIt);
    }

    // 准备输出缓冲
    RECON_Geometry geom = makeGeometryFromExtras_(extras, expectImages);

    RebuildTaskState &st = tasks_[sectionId];
    st.sectionId = sectionId;
    st.sectionTime = sectionTime;
    st.options.expectImages = expectImages;
    st.options.pixelBits = pixelBits;
    st.options.extras = extras;
    st.imageFiles.clear();
    st.taskDir.clear();
    st.percent = 0;

    const unsigned long long voxels = static_cast<unsigned long long>(geom.vol_dim_x) * geom.vol_dim_y * geom.vol_dim_z;
    if (voxels > s_outVolume.size())
    {
        s_outVolume.resize((size_t)voxels); // 由上层提供大体积输出缓冲区时，传其指针进来
    }

    LogHelper::debug() << "[Rebuild-Begin] id=" << sectionId << " expect=" << expectImages << " pixelBits=" << pixelBits;
    return true;
}

// ========== DATA 阶段（buffer 版本） ========== //
bool RebuildTaskManager::appendImageBuffer(const std::string &sectionId, const void *data, size_t size, int rows, int cols, int matType)
{
    if (!data || size == 0 || rows <= 0 || cols <= 0)
    {
        LogHelper::error() << "[Rebuild-Data] invalid input.";
        return false;
    }

    cv::Mat img(rows, cols, matType, const_cast<void *>(data));
    return appendImageMat(sectionId, img);
}

// ========== DATA 阶段（Mat 版本） ========== //
// sectionId: xxx.0.png
bool RebuildTaskManager::appendImageMat(const std::string &sectionId, const cv::Mat &img)
{
    std::lock_guard<std::mutex> lk(mtx_);
    auto si = sectionId;
    auto sf = CxString::tokenLeft(si, cx::CHAR_DOT);
    auto it = tasks_.find(sf);
    if (it == tasks_.end())
    {
        LogHelper::warning() << "[Rebuild-Data] no task for id=" << sectionId;
        return false;
    }
    RebuildTaskState &st = it->second;

    if (st.taskDir.empty())
    {
        auto dir = AppDirectoryHelper::tempJoin(sf);
        CxFilesystem::createDirs(dir);
        st.taskDir = dir;
    }

    auto fp = CxFilesystem::join(st.taskDir, si);
    auto rst = cvh::IOHelper::save(fp, img);
    if (!rst)
    {
        LogHelper::error() << "[Rebuild-Data] save image failed.";
        return false;
    }
    st.imageFiles.push_back(fp);
    LogHelper::debug() << "[Rebuild-Data] saved: " << fp;
    return true;
}

// ========== END 阶段：发起重建 ========== //
void RebuildTaskManager::endTaskAndReconstruct(const std::string &sectionId)
{
    RebuildTaskState stCopy;
    {
        std::lock_guard<std::mutex> lk(mtx_);
        auto it = tasks_.find(sectionId);
        if (it == tasks_.end())
        {
            LogHelper::warning() << "[Rebuild-End] no task for id=" << sectionId;
            return; // 未找到任务
        }
        // 正在有后台线程运行？
        if (s_running.load(std::memory_order_acquire) || s_worker_ != nullptr)
        {
            LogHelper::warning() << "[Rebuild-End] busy, another reconstruction is running.";
            return; // 忙
        }
        stCopy = it->second; // 拷贝一份，释放锁后做重建

        // 记录当前正在重建的任务 id，避免被 beginTask 淘汰
        s_activeSectionId = sectionId;
    }

    size_t n = stCopy.imageFiles.size();
    LogHelper::debug() << "[Rebuild-End] id=" << sectionId << " images=" << n << " / expect=" << stCopy.options.expectImages;

    // —— 生成重建参数 —— //
//    RECON_AlgoOptions algo = makeAlgoFromExtras_(stCopy.options.extras);
//    RECON_Geometry geom = makeGeometryFromExtras_(stCopy.options.extras, n);
//    RECON_IterParams iter = makeIterFromExtras_(stCopy.options.extras);

    // 准备输出缓冲
//    const unsigned long long voxels = static_cast<unsigned long long>(geom.vol_dim_x) * geom.vol_dim_y * geom.vol_dim_z;
//    s_outVolume.resize((size_t)voxels); // 由上层提供大体积输出缓冲区时，传其指针进来

    // 启动后台线程，不阻塞当前调用
    {
        std::lock_guard<std::mutex> lk(mtx_);
        s_running.store(true, std::memory_order_release);
        s_worker_ = new std::thread([stCopy, sectionId]()
                                    {
                                        RECON_UserData userData; // void *，记录下来，让她
                                        std::string taskDir;
                                        std::vector<std::string> *imageFiles = nullptr;
                                        {
                                            std::lock_guard<std::mutex> lk(mtx_);
                                            auto it = tasks_.find(sectionId);
                                            if (it != tasks_.end())
                                            {
                                                userData = &it->second;
                                                imageFiles = &it->second.imageFiles;
                                                taskDir = it->second.taskDir;
                                            }
                                        }

                                        if (imageFiles == nullptr)
                                        {
                                            LogHelper::error() << "[Rebuild-Worker] imageFiles is null, sectionId: " << sectionId;
                                            return;
                                        }

                                        RECON_AlgoOptions algo = stCopy.options.extras.empty() ? RECON_AlgoOptions{} : makeAlgoFromExtras_(stCopy.options.extras);
                                        RECON_Geometry geom = makeGeometryFromExtras_(stCopy.options.extras, stCopy.imageFiles.size());
                                        RECON_IterParams iter = makeIterFromExtras_(stCopy.options.extras);
                                        int pixelBits = stCopy.options.pixelBits;

                                        // 注意：输出体素缓冲区应当由调用方提供并管理内存。
                                        // 如果你的 ReconstructionService::reconstructFromFiles 需要外部缓冲区指针，
                                        // 在此处把外部传入的 buffer 指针透传进去；当前示例使用 nullptr。
                                        int rr = ReconstructionService::reconstructFromFiles(
                                                /*algo*/    algo,
                                                /*geom*/    geom,
                                                /*iter*/    iter,
                                                /*files*/   *imageFiles,
                                                /*bits */   pixelBits,
                                                /*out  */   s_outVolume,
                                                /*u data*/  userData,
                                                /*align*/   0
                                        );

                                        // 保存重建完的体素
                                        if (rr == RECON_OK)
                                        {
                                            int row = static_cast<int>(geom.vol_dim_y);
                                            int col = static_cast<int>(geom.vol_dim_x);
                                            int area = row * col;
                                            float* out_volume = s_outVolume.data();
                                            LogHelper::debug() << "[Rebuild-Worker] slice-count=" << geom.vol_dim_z << ":";
                                            std::string restDir = getTaskVolumeDir(taskDir);
                                            CxFilesystem::ensureDirs(restDir);
                                            for (int i = 0; i < geom.vol_dim_z; ++i)
                                            {
                                                cv::Mat mat(row, col, CV_32FC1);
                                                memcpy(mat.data, out_volume + area * i, area * sizeof(float));
                                                cv::normalize(mat, mat, 0, 255, cv::NORM_MINMAX);
                                                mat.convertTo(mat, CV_8UC1);
                                                std::string fp = CxFilesystem::join(restDir, std::to_string(i) + ".bmp");
                                                bool rw = cv::imwrite(fp, mat);
                                                LogHelper::debug() << "[Rebuild-Worker] slice=" << i << " saved " << (rw ? "OK" : "FAILED") << " :file=" << fp;
                                            }
                                        }

                                        LogHelper::debug() << "[Rebuild-Worker] reconstruct complete, result=" << rr << " (0 means success).";

                                        // —— 清理任务（含内存任务表；若还需删除磁盘图片，可扩展） —— //
                                        RebuildTaskManager::finishTask(sectionId);

                                        // 线程收尾与状态复位
                                        {
                                            std::lock_guard<std::mutex> lk2(mtx_);
                                            s_running.store(false, std::memory_order_release);

                                            // 当前重建任务结束，允许被淘汰
                                            if (s_activeSectionId == sectionId)
                                            {
                                                s_activeSectionId.clear();
                                            }

                                            std::thread* th = s_worker_;
                                            s_worker_ = nullptr;

                                            // 线程是分离态，无需 join，但要在退出前释放对象
                                            // 为安全，只有在确认线程已执行到此处时才 delete
                                            // 这里就是线程函数末尾，安全 delete
                                            delete th;
                                        }
                                    });

        // 分离线程：不阻塞调用方
        s_worker_->detach();
    }

    // 已成功启动后台重建
    LogHelper::debug() << "[Rebuild-End] reconstruct success detach.";
}

// ========== 取消并清理 ========== //
void RebuildTaskManager::cancelTask(const std::string &sectionId)
{
    std::lock_guard<std::mutex> lk(mtx_);
    auto it = tasks_.find(sectionId);
    if (it != tasks_.end())
    {
        ReconstructionService::requestCancel();
        // 这里只清理内存任务表；若需要删除磁盘图片，可在此调用删除目录
        tasks_.erase(it);
    }
}

void RebuildTaskManager::finishTask(const std::string &sectionId)
{
    std::lock_guard<std::mutex> lk(mtx_);
    auto it = tasks_.find(sectionId);
    if (it != tasks_.end())
    {
        it->second.finishTime = cx::DateTime::now();
        // 这里只清理内存任务表；若需要删除磁盘图片，可在此调用删除目录
    }
}

// ========== 查询状态 ========== //
bool RebuildTaskManager::getTaskState(const std::string &sectionId, RebuildTaskState &out)
{
    std::lock_guard<std::mutex> lk(mtx_);
    auto it = tasks_.find(sectionId);
    if (it == tasks_.end())
    {
        return false;
    }
    out = it->second;
    return true;
}

// ========== 清理过期任务（按需实现目录遍历与删除） ========== //
void RebuildTaskManager::purgeOldTasks(int /*daysAgo*/)
{
    // 预留：若你的 CxFilesystem 有按时间删除接口，可在此实现。
}

void RebuildTaskManager::init()
{
    ReconstructionService::setProgressCallback(progressTask);
    // 加载 服务 ReconstructionService
    ReconstructionService::init();
}

void RebuildTaskManager::deInit()
{
    // 加载 服务 ReconstructionService
    ReconstructionService::deInit();
    ReconstructionService::setProgressCallback(nullptr);
}

void RebuildTaskManager::progressTask(int percent, RECON_UserData ud)
{
    const auto* ptr = reinterpret_cast<const RebuildTaskState*>(ud);
    if (!ptr) return;

    {
        std::lock_guard<std::mutex> lk(mtx_);
        for (auto& kv : tasks_)
        {
            if (&kv.second == ptr)
            {
                kv.second.percent = percent;
                break;
            }
        }
    }
}

std::string RebuildTaskManager::getTaskVolumeDir(const std::string &taskDir)
{
    return CxFilesystem::normalizeGeneric(taskDir) + "-volume";
}
