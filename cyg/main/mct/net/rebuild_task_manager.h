#pragma once
/**
 * @file    rebuild_task_manager.h
 * @brief   管理 CMD_REBUILD 的三段式流程（FUNC_BEGIN / FUNC_DATA / FUNC_END），
 *          负责图片落盘、任务状态跟踪，在 END 时调用 ReconstructionService::reconstructFromFiles。
 *
 * 使用方法（在 net_client_session.cpp 中的三个阶段各加一行调用即可）：
 *   - FUNC_BEGIN:
 *       RebuildTaskManager::beginTask(sectionId, sectionTime, expectImages, pixelBits, extras);
 *   - FUNC_DATA:
 *       RebuildTaskManager::appendImageBuffer(sectionId, dataPtr, dataSize, rows, cols, matType);
 *   - FUNC_END:
 *       int ret = RebuildTaskManager::endTaskAndReconstruct(sectionId);
 *
 * 说明：
 *   1) 线程安全；以 sectionId 为键管理任务。
 *   2) 图片默认保存为 PNG 无损（可改为 TIFF）。
 *   3) 几何/算法参数可通过 extras 透传（如 detU, detV, volx, voly, volz, sid, sdd, pixelSize, voxelSize, projAngles 等）。
 */

#include <string>
#include <vector>
#include <unordered_map>
#include <map>
#include <mutex>

// 前置声明（根据你的工程实际头文件名修改为正确 include）
#include "reconstruction/reconstruction_service.h"    // 提供 reconstructFromFiles / makeGeometry / makeAlgo / makeIters
#include "ccxx/datetime.h"               // 提供 cx::DateTime（如果你的头不叫这个，请改成实际的）

// 可选：日志与文件工具（若工程已有对应封装，替换为你的）
#include <helpers/log_helper.h>
#include <ccxx/cxfilesystem.h>
#include <base/app_directory_helper.h>

#include <opencv2/opencv.hpp>

struct RebuildTaskOptions
{
    // 由 BEGIN 阶段设置
    int   expectImages = 0;            // 期望图像数
    int   pixelBits    = 16;           // 8 | 16 | 32
    std::unordered_map<std::string, std::string> extras; // 透传的几何/算法参数
};

struct RebuildTaskState
{
    std::string   sectionId;
    cx::DateTime  sectionTime;              // 任务下达时间
    RebuildTaskOptions options;

    // 由 DATA 阶段累加
    std::vector<std::string> imageFiles;    // 落盘后的文件路径列表
    std::string              taskDir;       // 本任务的保存目录（例如 temp/rebuild/<sectionId>）

    int percent /*0..100*/;
    cx::DateTime  finishTime;               // 任务结束时间
};

class RebuildTaskManager
{
public:
    // 加载 服务 ReconstructionService
    static void init();

    // 卸载 服务 ReconstructionService
    static void deInit();

    // 在 FUNC_BEGIN 调用：创建/重置任务
    static bool beginTask(const std::string& sectionId,
                   const cx::DateTime& sectionTime,
                   int expectImages,
                   int pixelBits,
                   const std::unordered_map<std::string, std::string>& extras);

    // 在 FUNC_DATA 调用：将一张图像（buffer 或 Mat）落盘并记录路径
    static bool appendImageBuffer(const std::string& sectionId, const void* data, size_t size, int rows, int cols, int matType);

    static bool appendImageMat(const std::string& sectionId, const cv::Mat& img);

    // 在 FUNC_END 调用：发起重建，返回 reconstructFromFiles 的返回码（0=成功）
    static void endTaskAndReconstruct(const std::string& sectionId);

    // 取消并清理任务（例如异常中断时）
    static void cancelTask(const std::string& sectionId);

    static void finishTask(const std::string& sectionId);

    // 查询任务状态（只读拷贝）
    static bool getTaskState(const std::string& sectionId, RebuildTaskState& out);

    // 清理过期任务目录（daysAgo 之前的）
    static void purgeOldTasks(int daysAgo = 3);

    static void progressTask(int percent /*0..100*/, RECON_UserData ud);

    static std::string getTaskVolumeDir(const std::string& taskDir);

};

