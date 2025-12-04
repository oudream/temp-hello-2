#include "windowing_module.h"
#include "windowing_handler.h"
#include "base/app_ids.h"
#include "base/icon_manager.h"
#include "u_ribbon_block.h"
#include <QAction>
#include <QIcon>


#include "u_ribbon_builder.h"

URibbonBlock WindowingModule::createModule(QObject* parent, QMap<QString,QAction*>& actions, IURibbonHandler* handler) const {
    RibbonBuilder<WindowingHandler> rb(parent, actions, handler);
    const QString gLayout = RB_GROUP(rb, AppIds::kGroup_Window_Layout);
    const QString gPanels = RB_GROUP(rb, AppIds::kGroup_Window_Panels);

    RB_MAKE(rb, AppIds::kBlock_Windowing, AppIds::kAction_Layout_Grid,   AppIds::kText_Layout_Grid,   WindowingHandler, onLayoutGrid,   gLayout);
    RB_MAKE(rb, AppIds::kBlock_Windowing, AppIds::kAction_Layout_1x3,    AppIds::kText_Layout_1x3,    WindowingHandler, onLayout1x3,    gLayout);
    RB_MAKE(rb, AppIds::kBlock_Windowing, AppIds::kAction_Layout_Equal,  AppIds::kText_Layout_Equal,  WindowingHandler, onLayoutEqual,  gLayout);

    RB_MAKE(rb, AppIds::kBlock_Windowing, AppIds::kAction_Panel_Assets,  AppIds::kText_Panel_Assets,  WindowingHandler, onPanelAssets,  gPanels);
    RB_MAKE(rb, AppIds::kBlock_Windowing, AppIds::kAction_Panel_Render,  AppIds::kText_Panel_Render,  WindowingHandler, onPanelRender,  gPanels);
    RB_MAKE(rb, AppIds::kBlock_Windowing, AppIds::kAction_Panel_Camera,  AppIds::kText_Panel_Camera,  WindowingHandler, onPanelCamera,  gPanels);
    RB_MAKE(rb, AppIds::kBlock_Windowing, AppIds::kAction_Panel_Knife,   AppIds::kText_Panel_Knife,   WindowingHandler, onPanelKnife,   gPanels);
    RB_MAKE(rb, AppIds::kBlock_Windowing, AppIds::kAction_Panel_Measure, AppIds::kText_Panel_Measure, WindowingHandler, onPanelMeasure, gPanels);
    RB_MAKE(rb, AppIds::kBlock_Windowing, AppIds::kAction_Panel_Log,     AppIds::kText_Panel_Log,     WindowingHandler, onPanelLog,     gPanels);

    return rb.buildBlock(QObject::tr(AppIds::kBlock_Windowing_Cn), AppIds::ERibbonBlock::Windowing);
}

IURibbonHandler *WindowingModule::createHandler(QObject *parent) const
{
    return new WindowingHandler(parent);
}
