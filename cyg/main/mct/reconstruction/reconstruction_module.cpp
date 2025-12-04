#include "reconstruction_module.h"
#include "reconstruction_handler.h"
#include "base/app_ids.h"
#include "u_ribbon_block.h"
#include <QAction>
#include <QIcon>


#include "u_ribbon_builder.h"

URibbonBlock ReconstructionModule::createModule(QObject* parent, QMap<QString,QAction*>& actions, IURibbonHandler* handler) const {
    RibbonBuilder<ReconstructionHandler> rb(parent, actions, handler);
    const QString gParams = RB_GROUP(rb, AppIds::kGroup_Recon_Params);
    const QString gExec   = RB_GROUP(rb, AppIds::kGroup_Recon_Exec);
    const QString gResult = RB_GROUP(rb, AppIds::kGroup_Recon_Result);

    RB_MAKE(rb, AppIds::kBlock_Reconstruction, AppIds::kAction_Recon_Scan,     AppIds::kText_Recon_Scan,     ReconstructionHandler, onScanParams,   gParams);
    RB_MAKE(rb, AppIds::kBlock_Reconstruction, AppIds::kAction_Recon_Calib,    AppIds::kText_Recon_Calib,    ReconstructionHandler, onGeometryCalib,gParams);
    RB_MAKE(rb, AppIds::kBlock_Reconstruction, AppIds::kAction_Recon_Algo,     AppIds::kText_Recon_Algo,     ReconstructionHandler, onAlgoSelect,   gParams);

    RB_MAKE(rb, AppIds::kBlock_Reconstruction, AppIds::kAction_Recon_Run,      AppIds::kText_Recon_Run,      ReconstructionHandler, onRun,          gExec);
    RB_MAKE(rb, AppIds::kBlock_Reconstruction, AppIds::kAction_Recon_Pause,    AppIds::kText_Recon_Pause,    ReconstructionHandler, onPause,        gExec);
    RB_MAKE(rb, AppIds::kBlock_Reconstruction, AppIds::kAction_Recon_Resume,   AppIds::kText_Recon_Resume,   ReconstructionHandler, onResume,       gExec);

    RB_MAKE(rb, AppIds::kBlock_Reconstruction, AppIds::kAction_Recon_Progress, AppIds::kText_Recon_Progress, ReconstructionHandler, onShowProgress, gResult);
    RB_MAKE(rb, AppIds::kBlock_Reconstruction, AppIds::kAction_Recon_Results,  AppIds::kText_Recon_Results,  ReconstructionHandler, onShowResults,  gResult);

    return rb.buildBlock(QObject::tr(AppIds::kBlock_Reconstruction_Cn), AppIds::ERibbonBlock::Reconstruction);
}

IURibbonHandler *ReconstructionModule::createHandler(QObject *parent) const
{
    return new ReconstructionHandler(parent);
}
