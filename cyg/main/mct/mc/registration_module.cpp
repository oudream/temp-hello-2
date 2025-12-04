#include "registration_module.h"

#include "registration_handler.h"
#include "base/app_ids.h"
#include "u_ribbon_block.h"

#include <QAction>
#include <QIcon>


#include "u_ribbon_builder.h"

URibbonBlock RegistrationModule::createModule(QObject* parent, QMap<QString,QAction*>& actions, IURibbonHandler* handler) const {
    RibbonBuilder<RegistrationHandler> rb(parent, actions, handler);
    const QString gAlign = RB_GROUP(rb, AppIds::kGroup_Register_Align);

    RB_MAKE(rb, AppIds::kBlock_Registration, AppIds::kAction_Reg_Rigid,    AppIds::kText_Reg_Rigid,    RegistrationHandler, onRegRigid,    gAlign);
    RB_MAKE(rb, AppIds::kBlock_Registration, AppIds::kAction_Reg_NonRigid, AppIds::kText_Reg_NonRigid, RegistrationHandler, onRegNonRigid, gAlign);
    RB_MAKE(rb, AppIds::kBlock_Registration, AppIds::kAction_Reg_Icp,      AppIds::kText_Reg_Icp,      RegistrationHandler, onRegICP,      gAlign);

    return rb.buildBlock(QObject::tr(AppIds::kBlock_Registration_Cn), AppIds::ERibbonBlock::Registration);
}


IURibbonHandler *RegistrationModule::createHandler(QObject *parent) const
{
    return new RegistrationHandler(parent);
}
