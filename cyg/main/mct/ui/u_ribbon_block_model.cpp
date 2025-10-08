#include "u_ribbon_block_model.h"
#include <QIcon>
#include <QKeySequence>

URibbonBlockModel::URibbonBlockModel(QObject *parent) : QObject(parent)
{}

// 统一：所有动作先在这里集中创建，再在 buildModel() 里分配到各分组
void URibbonBlockModel::createActions(QObject *parent)
{
    // --- 顶层切页（仅用于文字与路由，不一定要放到分组） ---
    _actions["switch_project"] = new QAction(QIcon(), tr("项目"), parent);
    _actions["switch_raw"] = new QAction(QIcon(), tr("原图"), parent);
    _actions["switch_recon"] = new QAction(QIcon(), tr("重建"), parent);
    _actions["switch_inspect"] = new QAction(QIcon(), tr("试检"), parent);

    // --- 项目 ---
    _actions["proj_new"] = new QAction(QIcon(), tr("新建"), parent);
    _actions["proj_open"] = new QAction(QIcon(), tr("打开"), parent);
    _actions["proj_save"] = new QAction(QIcon(), tr("保存"), parent);
    _actions["proj_save_as"] = new QAction(QIcon(), tr("另存"), parent);
    _actions["proj_recent"] = new QAction(QIcon(), tr("最近项目"), parent);
    _actions["proj_template"] = new QAction(QIcon(), tr("模板管理"), parent);
    _actions["proj_params"] = new QAction(QIcon(), tr("工程参数"), parent);
    _actions["proj_backup"] = new QAction(QIcon(), tr("备份/恢复"), parent);

    // --- 原图 ---
    _actions["raw_import"] = new QAction(QIcon(), tr("导入"), parent);
    _actions["raw_batch"] = new QAction(QIcon(), tr("批量管理"), parent);
    _actions["raw_metadata"] = new QAction(QIcon(), tr("元数据"), parent);
    _actions["raw_preproc"] = new QAction(QIcon(), tr("预处理"), parent);
    _actions["raw_presets"] = new QAction(QIcon(), tr("视图预设"), parent);

    // --- 重建 ---
    _actions["recon_scan"] = new QAction(QIcon(), tr("扫描参数"), parent);
    _actions["recon_calib"] = new QAction(QIcon(), tr("几何标定"), parent);
    _actions["recon_algo"] = new QAction(QIcon(), tr("算法选择"), parent);
    _actions["recon_run"] = new QAction(QIcon(), tr("执行"), parent);
    _actions["recon_pause"] = new QAction(QIcon(), tr("暂停"), parent);
    _actions["recon_resume"] = new QAction(QIcon(), tr("继续"), parent);
    _actions["recon_progress"] = new QAction(QIcon(), tr("日志/进度"), parent);
    _actions["recon_results"] = new QAction(QIcon(), tr("结果集"), parent);

    // --- 3D：切面/渲染/测量/分割/配准/窗体/转换 ---
    _actions["slice_move"] = new QAction(QIcon(), tr("移动"), parent);
    _actions["slice_rotate"] = new QAction(QIcon(), tr("旋转"), parent);

    _actions["render_mode"] = new QAction(QIcon(), tr("模式"), parent);
    _actions["render_lut"] = new QAction(QIcon(), tr("LUT"), parent);
    _actions["render_wl"] = new QAction(QIcon(), tr("窗宽/窗位"), parent);

    _actions["measure_line"] = new QAction(QIcon(), tr("直线"), parent);
    _actions["measure_angle"] = new QAction(QIcon(), tr("角度"), parent);
    _actions["measure_circle"] = new QAction(QIcon(), tr("圆"), parent);
    _actions["measure_area"] = new QAction(QIcon(), tr("面积"), parent);

    _actions["seg_thresh"] = new QAction(QIcon(), tr("阈值"), parent);
    _actions["seg_grow"] = new QAction(QIcon(), tr("区域生长"), parent);
    _actions["seg_ai"] = new QAction(QIcon(), tr("AI 分割"), parent);

    _actions["reg_rigid"] = new QAction(QIcon(), tr("刚性配准"), parent);
    _actions["reg_nonrigid"] = new QAction(QIcon(), tr("非刚性配准"), parent);
    _actions["reg_icp"] = new QAction(QIcon(), tr("点云对齐"), parent);

    // 布局 + 面板（窗体）
    _actions["layout_grid"] = new QAction(QIcon(), tr("2×2 布局"), parent);
    _actions["layout_1x3"] = new QAction(QIcon(), tr("1&3 布局"), parent);
    _actions["layout_equal"] = new QAction(QIcon(), tr("等分"), parent);

    _actions["panel_assets"] = new QAction(QIcon(), tr("资产"), parent);
    _actions["panel_render"] = new QAction(QIcon(), tr("渲染"), parent);
    _actions["panel_camera"] = new QAction(QIcon(), tr("相机"), parent);
    _actions["panel_knife"] = new QAction(QIcon(), tr("切面"), parent);
    _actions["panel_measure"] = new QAction(QIcon(), tr("测量"), parent);
    _actions["panel_log"] = new QAction(QIcon(), tr("日志"), parent);

    // 快捷键
    _actions["layout_grid"]->setShortcut(QKeySequence("Ctrl+G"));
    _actions["layout_1x3"]->setShortcut(QKeySequence("Ctrl+J"));
    _actions["layout_equal"]->setShortcut(QKeySequence("Ctrl+E"));

    // 面板勾选
    auto setChk = [this](const char *id)
    {
        _actions[id]->setCheckable(true);
        _actions[id]->setChecked(true);
    };
    setChk("panel_assets");
    setChk("panel_render");
    setChk("panel_camera");
    setChk("panel_knife");
    setChk("panel_measure");
    setChk("panel_log");
}

QAction *URibbonBlockModel::action(const QString &id) const
{
    return _actions.value(id, nullptr);
}

const QList<URibbonBlock> &URibbonBlockModel::getBlocks()
{
    if (!_isBuilt)
    {
        buildModel();
        _isBuilt = true;
    }
    return _blocks;
}

// 组织所有功能块与分组（严格按《需求规格》3.1/3.2）
void URibbonBlockModel::buildModel()
{
    _blocks.clear();
    // 页面索引：0=项目, 1=原图, 2=重建, 3=3D, 4=试检  （与主窗路由一致）:contentReference[oaicite:6]{index=6}

    // 1) 项目
    _blocks.append({tr("项目"), 0, {
            {tr("工程"), {action("proj_new"), action("proj_open"), action("proj_save"), action("proj_save_as")}},
            {tr("管理"), {action("proj_recent"), action("proj_template"), action("proj_params"), action("proj_backup")}}
    }});

    // 2) 原图
    _blocks.append({tr("原图"), 1, {
            {tr("数据"), {action("raw_import"), action("raw_batch"), action("raw_metadata")}},
            {tr("预处理"), {action("raw_preproc"), action("raw_presets")}}
    }});

    // 3) 重建
    _blocks.append({tr("重建"), 2, {
            {tr("参数"), {action("recon_scan"), action("recon_calib"), action("recon_algo")}},
            {tr("执行"), {action("recon_run"), action("recon_pause"), action("recon_resume")}},
            {tr("结果"), {action("recon_progress"), action("recon_results")}}
    }});

    // 4) 切面（路由到 3D）
    _blocks.append({tr("切面"), 3, {
            {tr("操作"), {action("slice_move"), action("slice_rotate")}}
    }});

    // 5) 渲染（路由到 3D）
    _blocks.append({tr("渲染"), 3, {
            {tr("显示"), {action("render_mode"), action("render_lut"), action("render_wl")}}
    }});

    // 6) 测量（路由到 3D）
    _blocks.append({tr("测量"), 3, {
            {tr("工具"),
             {action("measure_line"), action("measure_angle"), action("measure_circle"), action("measure_area")}}
    }});

    // 7) 分割（路由到 3D）
    _blocks.append({tr("分割"), 3, {
            {tr("方法"), {action("seg_thresh"), action("seg_grow"), action("seg_ai")}}
    }});

    // 8) 配准（路由到 3D）
    _blocks.append({tr("配准"), 3, {
            {tr("对齐"), {action("reg_rigid"), action("reg_nonrigid"), action("reg_icp")}}
    }});

    // 9) 窗体（路由到 3D）：布局 + 面板勾选
    _blocks.append({tr("窗体"), 3, {
            {tr("布局"), {action("layout_grid"), action("layout_1x3"), action("layout_equal")}},
            {tr("面板"), {action("panel_assets"), action("panel_render"), action("panel_camera"),
                          action("panel_knife"), action("panel_measure"), action("panel_log")}}
    }});

    // 10) 转换（路由到 3D）
    _blocks.append({tr("转换"), 3, {
            {tr("导出/格式"), { /* 预留：conv_export/conv_format 若后续补齐 */ }}
    }});

    // 11) 试检
    _blocks.append({tr("试检"), 4, {
            {tr("流程"), { /* 预留：insp_policy/insp_roi/insp_batch/insp_stats/insp_export */ }}
    }});
}
