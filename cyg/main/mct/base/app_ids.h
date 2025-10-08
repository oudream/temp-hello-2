#ifndef MCT_APP_IDS_H
#define MCT_APP_IDS_H

// 统一的 Page/Panel 名称（用于注册与路由）
namespace AppIds
{
    // Pages
    static const char *kPage_Project = "project";
    static const char *kPage_Assets = "assets";
    static const char *kPage_Reconstruct = "reconstruct";
    static const char *kPage_ThreeD = "three_d";
    static const char *kPage_Inspect = "inspect";

    // Panels
    static const char *kPanel_Assets = "assets";
    static const char *kPanel_Camera = "camera";
    static const char *kPanel_Knife = "knife";
    static const char *kPanel_Render = "render";
    static const char *kPanel_Measure = "measure";
    static const char *kPanel_Log = "log";


// 页（中央 Stacked 页）
    enum class PageId
    {
        Project,   // 项目
        Assets,    // 原图
        Reconstruct, // 重建
        ThreeD,    // 3D
        Inspect    // 试检
    };

// 功能块（Ribbon 上层大按钮）
    enum class BlockId
    {
        Project,
        Assets,
        Reconstruct,
        ThreeD,
        Inspect,
        // 以下功能块点击后均切到 3D 页
        VoxelEdit,     // 体编
        VoxelSelect,   // 体选
        Slice,         // 切面
        Render,        // 渲染
        Measure,       // 测量
        Segment,       // 分割
        Register,      // 配准
        Windowing,     // 窗体（布局/面板）
        Convert        // 转换
    };

}

#endif // MCT_APP_IDS_H
