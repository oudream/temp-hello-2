#include "reslice_module.h"
#include "reslice_handler.h"
#include "base/app_ids.h"
#include "u_ribbon_block.h"
#include <QAction>
#include <QIcon>


#include "u_ribbon_builder.h"

URibbonBlock ResliceModule::createModule(QObject* parent, QMap<QString,QAction*>& actions, IURibbonHandler* handler) const {
    RibbonBuilder<ResliceHandler> rb(parent, actions, handler);
    const QString gOp = RB_GROUP(rb, AppIds::kGroup_Slice_Op);

    RB_MAKE(rb, AppIds::kBlock_Reslice, AppIds::kAction_Slice_Move,   AppIds::kText_Slice_Move,   ResliceHandler, onSliceMove,   gOp);
    RB_MAKE(rb, AppIds::kBlock_Reslice, AppIds::kAction_Slice_Rotate, AppIds::kText_Slice_Rotate, ResliceHandler, onSliceRotate, gOp);

    return rb.buildBlock(QObject::tr(AppIds::kBlock_Reslice_Cn), AppIds::ERibbonBlock::Reslice);
}


IURibbonHandler *ResliceModule::createHandler(QObject *parent) const
{
    return new ResliceHandler(parent);
}
