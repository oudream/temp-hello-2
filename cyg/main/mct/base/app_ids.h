#ifndef MCT_APP_IDS_H
#define MCT_APP_IDS_H

#include <QtGlobal>
#include <QString>

// 统一的 Page/Panel 名称（用于注册与路由）
namespace AppIds
{
    // App self
    inline constexpr const char *kAppName = "MCT_Studio";
//    inline constexpr const char *kAppName = QT_TR_NOOP("MCT_Studio");

    inline constexpr const char *kUnknown = "unknown";

    /// Ribbon：Block -> Group ->Action
    /// UI：Page、Panel

    // ====== RibbonBlock（功能块）======
    // 项目、原图、重建、切面、渲染、测量、分割、配准、窗体、转换、试检
    // project、acquisition、reconstruction、reslice、rendering、measurement、segmentation、registration、windowing、transform、inspection
    inline constexpr const char *kBlock_Project = "project";
    inline constexpr const char *kBlock_Acquisition = "acquisition";
    inline constexpr const char *kBlock_Reconstruction = "reconstruction";
    inline constexpr const char *kBlock_Editor = "editor";
    inline constexpr const char *kBlock_Reslice = "reslice";
    inline constexpr const char *kBlock_Rendering = "rendering";
    inline constexpr const char *kBlock_Measurement = "measurement";
    inline constexpr const char *kBlock_Segmentation = "segmentation";
    inline constexpr const char *kBlock_Registration = "registration";
    inline constexpr const char *kBlock_Windowing = "windowing";
    inline constexpr const char *kBlock_Transform = "transform";
    inline constexpr const char *kBlock_Inspection = "inspection";

    inline constexpr const char *kBlock_Project_Cn = QT_TR_NOOP("项目");
    inline constexpr const char *kBlock_Acquisition_Cn = QT_TR_NOOP("原图");
    inline constexpr const char *kBlock_Reconstruction_Cn = QT_TR_NOOP("重建");
    inline constexpr const char *kBlock_Editor_Cn = QT_TR_NOOP("编辑");
    inline constexpr const char *kBlock_Reslice_Cn = QT_TR_NOOP("切面");
    inline constexpr const char *kBlock_Rendering_Cn = QT_TR_NOOP("渲染");
    inline constexpr const char *kBlock_Measurement_Cn = QT_TR_NOOP("测量");
    inline constexpr const char *kBlock_Segmentation_Cn = QT_TR_NOOP("分割");
    inline constexpr const char *kBlock_Registration_Cn = QT_TR_NOOP("配准");
    inline constexpr const char *kBlock_Windowing_Cn = QT_TR_NOOP("窗体");
    inline constexpr const char *kBlock_Transform_Cn = QT_TR_NOOP("转换");
    inline constexpr const char *kBlock_Inspection_Cn = QT_TR_NOOP("试检");

    // Panels
    inline constexpr const char *kPanel_Assets = "assets";
    inline constexpr const char *kPanel_Camera = "camera";
    inline constexpr const char *kPanel_Knife = "knife";
    inline constexpr const char *kPanel_Rendering = "rendering";
    inline constexpr const char *kPanel_Measurement = "measurement";
    inline constexpr const char *kPanel_Log = "log";


    // ====== Group（页面分类（用于 URibbonBlock.page））======
    enum class ERibbonPage : int
    {
        Project = 0,        // 项目
        Acquisition = 1,    // 原图/采集
        Reconstruction = 2, // 重建
        Editor = 3,         // 3D编辑（切面、渲染、测量、分割、配准、窗体、转换）
        // 注意：此为最大值
        Inspection = 4      // 试检（注意：此为最大值）
    };

    // 功能块（Ribbon 上层大按钮）
    enum class ERibbonBlock : int
    {
        Project = 0,        // 项目
        Acquisition = 1,    // 原图
        Reconstruction = 2, // 重建
        Reslice = 3,        // 切面
        Rendering = 4,      // 渲染
        Measurement = 5,    // 测量
        Segmentation = 6,   // 分割
        Registration = 7,   // 配准
        Windowing = 8,      // 窗体
        Transform = 9,      // 转换
        Inspection = 10,    // 试检
    };

    // 映射函数：枚举 -> 中文名
    inline const char *RibbonPageToLabel(ERibbonPage p)
    {
        switch (p)
        {
            case ERibbonPage::Project:
                return kBlock_Project_Cn;
            case ERibbonPage::Acquisition:
                return kBlock_Acquisition_Cn;
            case ERibbonPage::Reconstruction:
                return kBlock_Reconstruction_Cn;
            case ERibbonPage::Inspection:
                return kBlock_Inspection_Cn;
            default:
                return kBlock_Editor_Cn;
        }
    }

    inline ERibbonPage RibbonPageToEnum(const char *label)
    {
        if (strcmp(label, kBlock_Project_Cn) == 0)
            return ERibbonPage::Project;
        else if (strcmp(label, kBlock_Acquisition_Cn) == 0)
            return ERibbonPage::Acquisition;
        else if (strcmp(label, kBlock_Reconstruction_Cn) == 0)
            return ERibbonPage::Reconstruction;
        else if (strcmp(label, kBlock_Inspection_Cn) == 0)
            return ERibbonPage::Inspection;
        else
            return ERibbonPage::Editor;
    }

    inline int RibbonPageToEnum(const QString& title)
    {
        QByteArray bytes = title.toUtf8();
        const char* label = bytes.constData();
        auto page = RibbonPageToEnum(label);
        return static_cast<int>(page);
    }

    inline ERibbonPage RibbonBlockToPage(ERibbonBlock p)
    {
        switch (p)
        {
            case ERibbonBlock::Project:
                return ERibbonPage::Project;
            case ERibbonBlock::Acquisition:
                return ERibbonPage::Acquisition;
            case ERibbonBlock::Reconstruction:
                return ERibbonPage::Reconstruction;
            case ERibbonBlock::Inspection:
                return ERibbonPage::Inspection;
            default:
                return ERibbonPage::Editor;
        }
    }

    inline const std::unordered_map<std::string, std::string> kBlockEnMap = {
            {kBlock_Project,        kBlock_Project_Cn},
            {kBlock_Acquisition,    kBlock_Acquisition_Cn},
            {kBlock_Reconstruction, kBlock_Reconstruction_Cn},
            {kBlock_Reslice,        kBlock_Reslice_Cn},
            {kBlock_Rendering,      kBlock_Rendering_Cn},
            {kBlock_Measurement,    kBlock_Measurement_Cn},
            {kBlock_Segmentation,   kBlock_Segmentation_Cn},
            {kBlock_Registration,   kBlock_Registration_Cn},
            {kBlock_Windowing,      kBlock_Windowing_Cn},
            {kBlock_Transform,      kBlock_Transform_Cn},
            {kBlock_Inspection,     kBlock_Inspection_Cn},
    };

    inline const std::unordered_map<std::string, std::string> kBlockCnMap = {
            {kBlock_Project_Cn,        kBlock_Project},
            {kBlock_Acquisition_Cn,    kBlock_Acquisition},
            {kBlock_Reconstruction_Cn, kBlock_Reconstruction},
            {kBlock_Reslice_Cn,        kBlock_Reslice},
            {kBlock_Rendering_Cn,      kBlock_Rendering},
            {kBlock_Measurement_Cn,    kBlock_Measurement},
            {kBlock_Segmentation_Cn,   kBlock_Segmentation},
            {kBlock_Registration_Cn,   kBlock_Registration},
            {kBlock_Windowing_Cn,      kBlock_Windowing},
            {kBlock_Transform_Cn,      kBlock_Transform},
            {kBlock_Inspection_Cn,     kBlock_Inspection},
    };

    inline const char *RibbonBlockToChinese(const std::string &en)
    {
        auto it = kBlockEnMap.find(en);
        return (it != kBlockEnMap.end()) ? it->second.c_str() : en.c_str();
    }

    inline const char *RibbonBlockToEnglish(const std::string &cn)
    {
        auto it = kBlockCnMap.find(cn);
        return (it != kBlockCnMap.end()) ? it->second.c_str() : cn.c_str();
    }


    // ====== Group（显示名）======
    inline constexpr const char *kGroup_Project_Engine = QT_TR_NOOP("工程");
    inline constexpr const char *kGroup_Project_Manage = QT_TR_NOOP("管理");

    inline constexpr const char *kGroup_Raw_Data = QT_TR_NOOP("数据");
    inline constexpr const char *kGroup_Raw_Preproc = QT_TR_NOOP("预处理");

    inline constexpr const char *kGroup_Recon_Params = QT_TR_NOOP("参数");
    inline constexpr const char *kGroup_Recon_Exec = QT_TR_NOOP("执行");
    inline constexpr const char *kGroup_Recon_Result = QT_TR_NOOP("结果");

    inline constexpr const char *kGroup_Slice_Op = QT_TR_NOOP("操作");
    inline constexpr const char *kGroup_Render_Display = QT_TR_NOOP("显示");
    inline constexpr const char *kGroup_Measure_Tools = QT_TR_NOOP("工具");
    inline constexpr const char *kGroup_Segment_Method = QT_TR_NOOP("方法");
    inline constexpr const char *kGroup_Register_Align = QT_TR_NOOP("对齐");
    inline constexpr const char *kGroup_Window_Layout = QT_TR_NOOP("布局");
    inline constexpr const char *kGroup_Window_Panels = QT_TR_NOOP("面板");
    inline constexpr const char *kGroup_Convert_IO = QT_TR_NOOP("导出/格式");
    inline constexpr const char *kGroup_Inspect_Flow = QT_TR_NOOP("流程");


    // ====== Action（字符串ID）======
    // Tab 切换
    inline constexpr const char *kAction_Switch_Project = "switch_project";
    inline constexpr const char *kAction_Switch_Raw = "switch_raw";
    inline constexpr const char *kAction_Switch_Recon = "switch_recon";
    inline constexpr const char *kAction_Switch_Inspect = "switch_inspect";

    // 项目
    inline constexpr const char *kAction_Proj_New = "proj_new";
    inline constexpr const char *kAction_Proj_Open = "proj_open";
    inline constexpr const char *kAction_Proj_Save = "proj_save";
    inline constexpr const char *kAction_Proj_SaveAs = "proj_save_as";
    inline constexpr const char *kAction_Proj_Recent = "proj_recent";
    inline constexpr const char *kAction_Proj_Template = "proj_template";
    inline constexpr const char *kAction_Proj_Params = "proj_params";
    inline constexpr const char *kAction_Proj_Backup = "proj_backup";

    // 原图
    inline constexpr const char *kAction_Raw_Import = "raw_import";
    inline constexpr const char *kAction_Raw_Batch = "raw_batch";
    inline constexpr const char *kAction_Raw_Metadata = "raw_metadata";
    inline constexpr const char *kAction_Raw_Preproc = "raw_preproc";
    inline constexpr const char *kAction_Raw_Presets = "raw_presets";

    // 重建
    inline constexpr const char *kAction_Recon_Scan = "recon_scan";
    inline constexpr const char *kAction_Recon_Calib = "recon_calib";
    inline constexpr const char *kAction_Recon_Algo = "recon_algo";
    inline constexpr const char *kAction_Recon_Run = "recon_run";
    inline constexpr const char *kAction_Recon_Pause = "recon_pause";
    inline constexpr const char *kAction_Recon_Resume = "recon_resume";
    inline constexpr const char *kAction_Recon_Progress = "recon_progress";
    inline constexpr const char *kAction_Recon_Results = "recon_results";

    // 3D 与杂项
    inline constexpr const char *kAction_Slice_Move = "slice_move";
    inline constexpr const char *kAction_Slice_Rotate = "slice_rotate";

    inline constexpr const char *kAction_Render_Mode = "render_mode";
    inline constexpr const char *kAction_Render_Lut = "render_lut";
    inline constexpr const char *kAction_Render_WL = "render_wl";

    inline constexpr const char *kAction_Measure_Line = "measure_line";
    inline constexpr const char *kAction_Measure_Angle = "measure_angle";
    inline constexpr const char *kAction_Measure_Circle = "measure_circle";
    inline constexpr const char *kAction_Measure_Area = "measure_area";

    inline constexpr const char *kAction_Seg_Thresh = "seg_thresh";
    inline constexpr const char *kAction_Seg_Grow = "seg_grow";
    inline constexpr const char *kAction_Seg_AI = "seg_ai";

    inline constexpr const char *kAction_Reg_Rigid = "reg_rigid";
    inline constexpr const char *kAction_Reg_NonRigid = "reg_nonrigid";
    inline constexpr const char *kAction_Reg_Icp = "reg_icp";

    inline constexpr const char *kAction_Layout_Grid = "layout_grid";
    inline constexpr const char *kAction_Layout_1x3 = "layout_1x3";
    inline constexpr const char *kAction_Layout_Equal = "layout_equal";

    inline constexpr const char *kAction_Panel_Assets = "panel_assets";
    inline constexpr const char *kAction_Panel_Render = "panel_render";
    inline constexpr const char *kAction_Panel_Camera = "panel_camera";
    inline constexpr const char *kAction_Panel_Knife = "panel_knife";
    inline constexpr const char *kAction_Panel_Measure = "panel_measure";
    inline constexpr const char *kAction_Panel_Log = "panel_log";

    // ====== 快捷键 ======
    inline constexpr const char *kShortcut_Layout_Grid = "Ctrl+G";
    inline constexpr const char *kShortcut_Layout_1x3 = "Ctrl+J";
    inline constexpr const char *kShortcut_Layout_Equal = "Ctrl+E";

    // ====== 文本：项目（供 QAction 文案）======
    inline constexpr const char *kText_Proj_New = QT_TR_NOOP("新建");
    inline constexpr const char *kText_Proj_Open = QT_TR_NOOP("打开");
    inline constexpr const char *kText_Proj_Save = QT_TR_NOOP("保存");
    inline constexpr const char *kText_Proj_SaveAs = QT_TR_NOOP("另存");
    inline constexpr const char *kText_Proj_Recent = QT_TR_NOOP("最近项目");
    inline constexpr const char *kText_Proj_Template = QT_TR_NOOP("模板管理");
    inline constexpr const char *kText_Proj_Params = QT_TR_NOOP("工程参数");
    inline constexpr const char *kText_Proj_Backup = QT_TR_NOOP("备份/恢复");

    // --- Acquisition / 原图 ---
    inline constexpr const char *kText_Raw_Import = QT_TR_NOOP("导入");
    inline constexpr const char *kText_Raw_Batch = QT_TR_NOOP("批量管理");
    inline constexpr const char *kText_Raw_Metadata = QT_TR_NOOP("元数据");
    inline constexpr const char *kText_Raw_Preproc = QT_TR_NOOP("预处理");
    inline constexpr const char *kText_Raw_Presets = QT_TR_NOOP("视图预设");

    // --- Reconstruction / 重建 ---
    inline constexpr const char *kText_Recon_Scan = QT_TR_NOOP("扫描参数");
    inline constexpr const char *kText_Recon_Calib = QT_TR_NOOP("几何标定");
    inline constexpr const char *kText_Recon_Algo = QT_TR_NOOP("算法选择");
    inline constexpr const char *kText_Recon_Run = QT_TR_NOOP("执行");
    inline constexpr const char *kText_Recon_Pause = QT_TR_NOOP("暂停");
    inline constexpr const char *kText_Recon_Resume = QT_TR_NOOP("继续");
    inline constexpr const char *kText_Recon_Progress = QT_TR_NOOP("日志/进度");
    inline constexpr const char *kText_Recon_Results = QT_TR_NOOP("结果集");

    // --- Reslice / 切面 ---
    inline constexpr const char *kText_Slice_Move = QT_TR_NOOP("移动");
    inline constexpr const char *kText_Slice_Rotate = QT_TR_NOOP("旋转");

    // --- Rendering / 渲染 ---
    inline constexpr const char *kText_Render_Mode = QT_TR_NOOP("模式");
    inline constexpr const char *kText_Render_Lut = QT_TR_NOOP("LUT");
    inline constexpr const char *kText_Render_WL = QT_TR_NOOP("窗宽/窗位");

    // --- Measurement / 测量 ---
    inline constexpr const char *kText_Measure_Line = QT_TR_NOOP("直线");
    inline constexpr const char *kText_Measure_Angle = QT_TR_NOOP("角度");
    inline constexpr const char *kText_Measure_Circle = QT_TR_NOOP("圆");
    inline constexpr const char *kText_Measure_Area = QT_TR_NOOP("面积");

    // --- Segmentation / 分割 ---
    inline constexpr const char *kText_Seg_Thresh = QT_TR_NOOP("阈值");
    inline constexpr const char *kText_Seg_Grow = QT_TR_NOOP("区域生长");
    inline constexpr const char *kText_Seg_AI = QT_TR_NOOP("AI 分割");

    // --- Registration / 配准 ---
    inline constexpr const char *kText_Reg_Rigid = QT_TR_NOOP("刚性配准");
    inline constexpr const char *kText_Reg_NonRigid = QT_TR_NOOP("非刚性配准");
    inline constexpr const char *kText_Reg_Icp = QT_TR_NOOP("点云对齐");

    // --- Windowing / 窗体（布局 + 面板） ---
    inline constexpr const char *kText_Layout_Grid = QT_TR_NOOP("2×2 布局");
    inline constexpr const char *kText_Layout_1x3 = QT_TR_NOOP("1&3 布局");
    inline constexpr const char *kText_Layout_Equal = QT_TR_NOOP("等分");

    inline constexpr const char *kText_Panel_Assets = QT_TR_NOOP("资产");
    inline constexpr const char *kText_Panel_Render = QT_TR_NOOP("渲染");
    inline constexpr const char *kText_Panel_Camera = QT_TR_NOOP("相机");
    inline constexpr const char *kText_Panel_Knife = QT_TR_NOOP("切面");
    inline constexpr const char *kText_Panel_Measure = QT_TR_NOOP("测量");
    inline constexpr const char *kText_Panel_Log = QT_TR_NOOP("日志");

    // --- Transform / 转换（占位） ---
    inline constexpr const char *kText_Convert_IO = QT_TR_NOOP("导出/格式");

    // --- Inspection / 试检（占位） ---
    inline constexpr const char *kText_Inspect_Flow = QT_TR_NOOP("流程");


    // ====== 轴向 Axis ======
    inline constexpr const char *kAxis_X = "X";
    inline constexpr const char *kAxis_Y = "Y";
    inline constexpr const char *kAxis_Z = "Z";
    inline constexpr const char *kAxis_D = "D";

    inline constexpr const char *kAxis_X_Cn = QT_TR_NOOP("X轴");
    inline constexpr const char *kAxis_Y_Cn = QT_TR_NOOP("Y轴");
    inline constexpr const char *kAxis_Z_Cn = QT_TR_NOOP("Z轴");
    inline constexpr const char *kAxis_D_Cn = QT_TR_NOOP("D轴");

    // 三轴枚举（用于切面、刀具、相机旋转等统一标识）
    enum class EAxis : int
    {
        X = 0,
        Y = 1,
        Z = 2,
        D = 3,
    };

    inline const char* AxisToLabel(EAxis a)
    {
        switch (a)
        {
            case EAxis::X: return kAxis_X;
            case EAxis::Y: return kAxis_Y;
            case EAxis::Z: return kAxis_Z;
            case EAxis::D: return kAxis_D;
            default: return kUnknown;
        }
    }

    inline const char* AxisToChinese(EAxis a)
    {
        switch (a)
        {
            case EAxis::X: return kAxis_X_Cn;
            case EAxis::Y: return kAxis_Y_Cn;
            case EAxis::Z: return kAxis_Z_Cn;
            case EAxis::D: return kAxis_D_Cn;
            default: return kUnknown;
        }
    }

    // QString → std::string（UTF-8 编码）
    inline std::string ToStdString(const QString& qstr)
    {
        // toUtf8() 返回 QByteArray，内部是 UTF-8 编码
        QByteArray bytes = qstr.toUtf8();
        return std::string(bytes.constBegin(), bytes.constEnd());
    }

    // std::string（UTF-8 编码）→ QString
    inline QString ToQString(const std::string& str)
    {
        return QString::fromUtf8(str.c_str());
    }
}

#endif // MCT_APP_IDS_H
