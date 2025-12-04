#ifndef CX_CT_X2_PROJECT_HANDLER_H
#define CX_CT_X2_PROJECT_HANDLER_H


#include "u_ribbon_handler.h"


// 继承 QObject 以便使用信号和槽机制
class ProjectHandler : public IURibbonHandler
{
Q_OBJECT

public:
    explicit ProjectHandler(QObject *parent = nullptr);

public slots:

    // 定义槽函数，对应各个 Action 的具体操作
    void onNewProject();

    void onOpenProject();

    void onSaveProject();

    void onSaveAs();

    void onOpenRecent();

    void onManageTemplates();

    void onProjectParams();

    void onBackupRestore();

};

#endif //CX_CT_X2_PROJECT_HANDLER_H
