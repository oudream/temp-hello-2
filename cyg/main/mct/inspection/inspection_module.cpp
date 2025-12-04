#include "inspection_module.h"

#include "inspection_handler.h"
#include "base/app_ids.h"
#include "u_ribbon_block.h"

#include <QAction>
#include <QIcon>


#include "u_ribbon_builder.h"

URibbonBlock InspectionModule::createModule(QObject* parent, QMap<QString,QAction*>& actions, IURibbonHandler* handler) const {
    RibbonBuilder<InspectionHandler> rb(parent, actions, handler);
    const QString gFlow = RB_GROUP(rb, AppIds::kGroup_Inspect_Flow);
    Q_UNUSED(gFlow);
    return rb.buildBlock(QObject::tr(AppIds::kBlock_Inspection_Cn), AppIds::ERibbonBlock::Inspection);
}

IURibbonHandler *InspectionModule::createHandler(QObject *parent) const
{
    return new InspectionHandler(parent);
}
