#include "project_module.h"

#include "project_handler.h"

#include "u_ribbon_builder.h"

URibbonBlock ProjectModule::createModule(QObject* parent, QMap<QString,QAction*>& actions, IURibbonHandler* handler) const {
    RibbonBuilder<ProjectHandler> rb(parent, actions, handler);
    const QString gEngine = RB_GROUP(rb, AppIds::kGroup_Project_Engine);
    const QString gManage = RB_GROUP(rb, AppIds::kGroup_Project_Manage);

    RB_MAKE(rb, AppIds::kBlock_Project, AppIds::kAction_Proj_New,      AppIds::kText_Proj_New,      ProjectHandler, onNewProject,     gEngine);
    RB_MAKE(rb, AppIds::kBlock_Project, AppIds::kAction_Proj_Open,     AppIds::kText_Proj_Open,     ProjectHandler, onOpenProject,    gEngine);
    RB_MAKE(rb, AppIds::kBlock_Project, AppIds::kAction_Proj_Save,     AppIds::kText_Proj_Save,     ProjectHandler, onSaveProject,    gEngine);
    RB_MAKE(rb, AppIds::kBlock_Project, AppIds::kAction_Proj_SaveAs,   AppIds::kText_Proj_SaveAs,   ProjectHandler, onSaveAs,         gEngine);

    RB_MAKE(rb, AppIds::kBlock_Project, AppIds::kAction_Proj_Recent,   AppIds::kText_Proj_Recent,   ProjectHandler, onOpenRecent,     gManage);
    RB_MAKE(rb, AppIds::kBlock_Project, AppIds::kAction_Proj_Template, AppIds::kText_Proj_Template, ProjectHandler, onManageTemplates,gManage);
    RB_MAKE(rb, AppIds::kBlock_Project, AppIds::kAction_Proj_Params,   AppIds::kText_Proj_Params,   ProjectHandler, onProjectParams,  gManage);
    RB_MAKE(rb, AppIds::kBlock_Project, AppIds::kAction_Proj_Backup,   AppIds::kText_Proj_Backup,   ProjectHandler, onBackupRestore,  gManage);

    return rb.buildBlock(QObject::tr(AppIds::kBlock_Project_Cn), AppIds::ERibbonBlock::Project);
}

// createHandler 的职责：创建逻辑处理器
IURibbonHandler* ProjectModule::createHandler(QObject* parent) const
{
    return new ProjectHandler(parent);
}
