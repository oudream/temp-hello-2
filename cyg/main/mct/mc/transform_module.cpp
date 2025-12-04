#include "transform_module.h"
#include "transform_handler.h"
#include "base/app_ids.h"
#include "u_ribbon_block.h"
#include <QAction>
#include <QIcon>


#include "u_ribbon_builder.h"

URibbonBlock TransformModule::createModule(QObject* parent, QMap<QString,QAction*>& actions, IURibbonHandler* handler) const {
    RibbonBuilder<TransformHandler> rb(parent, actions, handler);
    const QString gIO = RB_GROUP(rb, AppIds::kGroup_Convert_IO);
    Q_UNUSED(gIO); // 目前无具体动作
    return rb.buildBlock(QObject::tr(AppIds::kBlock_Transform_Cn), AppIds::ERibbonBlock::Transform);
}

IURibbonHandler *TransformModule::createHandler(QObject *parent) const
{
    return new TransformHandler(parent);
}
