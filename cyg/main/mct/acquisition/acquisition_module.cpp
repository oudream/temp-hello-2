#include "acquisition_module.h"

#include "acquisition_handler.h"
#include "base/app_ids.h"
#include "u_ribbon_block.h"

#include <QAction>
#include <QIcon>


#include "u_ribbon_builder.h"

URibbonBlock AcquisitionModule::createModule(QObject* parent, QMap<QString,QAction*>& actions, IURibbonHandler* handler) const {
    RibbonBuilder<AcquisitionHandler> rb(parent, actions, handler);
    const QString gData   = RB_GROUP(rb, AppIds::kGroup_Raw_Data);
    const QString gPreproc= RB_GROUP(rb, AppIds::kGroup_Raw_Preproc);

    RB_MAKE(rb, AppIds::kBlock_Acquisition,   AppIds::kAction_Raw_Import,   AppIds::kText_Raw_Import,   AcquisitionHandler, onImport,       gData);
    RB_MAKE(rb, AppIds::kBlock_Acquisition,   AppIds::kAction_Raw_Batch,    AppIds::kText_Raw_Batch,    AcquisitionHandler, onBatchManage,  gData);
    RB_MAKE(rb, AppIds::kBlock_Acquisition,   AppIds::kAction_Raw_Metadata, AppIds::kText_Raw_Metadata, AcquisitionHandler, onShowMetadata, gData);

    RB_MAKE(rb, AppIds::kBlock_Acquisition,   AppIds::kAction_Raw_Preproc,  AppIds::kText_Raw_Preproc,  AcquisitionHandler, onPreprocess,   gPreproc);
    RB_MAKE(rb, AppIds::kBlock_Acquisition,   AppIds::kAction_Raw_Presets,  AppIds::kText_Raw_Presets,  AcquisitionHandler, onViewPresets,  gPreproc);

    return rb.buildBlock(QObject::tr(AppIds::kBlock_Acquisition_Cn), AppIds::ERibbonBlock::Acquisition);
}

IURibbonHandler *AcquisitionModule::createHandler(QObject *parent) const
{
    return new AcquisitionHandler(parent);
}
