#ifndef CX_CT_X2_U_RIBBON_BLOCK_H
#define CX_CT_X2_U_RIBBON_BLOCK_H


#include <QString>
#include <QList>
#include <QAction>

#include "base/app_ids.h"

/**
 * @brief RibbonActionGroup - “功能名分组”
 * 例：布局组包含 2×2 / 1&3 / 等分；面板组包含 相机/切面/渲染/测量/日志/资产 显隐勾选
 */
struct URibbonActionGroup
{
    QString title;
    QList<QAction *> actions;
};

/**
 * @brief RibbonBlock - “功能块”
 * 上层功能块（项目/原图/重建/体编/体选/切面/渲染/测量/分割/配准/窗体/转换/试检）
 * 绑定到某个主窗口页面（0~4），Ribbon 顶栏点击后，Ribbon 会据此发出切页请求。
 */
struct URibbonBlock
{
    QString title;
    AppIds::ERibbonBlock blockIndex;
    QList<URibbonActionGroup> groups;
};


#endif //CX_CT_X2_U_RIBBON_BLOCK_H
