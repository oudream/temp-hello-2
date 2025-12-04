#include "rendering_module.h"
#include "rendering_handler.h"
#include "base/app_ids.h"
#include "u_ribbon_block.h"
#include <QAction>
#include <QIcon>


#include "u_ribbon_builder.h"

URibbonBlock RenderingModule::createModule(QObject* parent, QMap<QString,QAction*>& actions, IURibbonHandler* handler) const {
    RibbonBuilder<RenderingHandler> rb(parent, actions, handler);
    const QString gDisp = RB_GROUP(rb, AppIds::kGroup_Render_Display);

    RB_MAKE(rb, AppIds::kBlock_Rendering, AppIds::kAction_Render_Mode, AppIds::kText_Render_Mode, RenderingHandler, onRenderMode,       gDisp);
    RB_MAKE(rb, AppIds::kBlock_Rendering, AppIds::kAction_Render_Lut,  AppIds::kText_Render_Lut,  RenderingHandler, onRenderLut,        gDisp);
    RB_MAKE(rb, AppIds::kBlock_Rendering, AppIds::kAction_Render_WL,   AppIds::kText_Render_WL,   RenderingHandler, onRenderWindowLevel, gDisp);

    return rb.buildBlock(QObject::tr(AppIds::kBlock_Rendering_Cn), AppIds::ERibbonBlock::Rendering);
}

IURibbonHandler *RenderingModule::createHandler(QObject *parent) const
{
    return new RenderingHandler(parent);
}
