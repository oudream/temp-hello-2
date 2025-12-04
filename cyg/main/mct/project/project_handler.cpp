#include "project_handler.h"


#include <QDebug>


ProjectHandler::ProjectHandler(QObject *parent) : IURibbonHandler(parent)
{
}

// ========= 工程（Engine） =========
void ProjectHandler::onNewProject()
{
    qDebug() << "[Project] New project...";
    // TODO: 弹新建对话框 -> 建目录/工程文件 -> 初始化状态
}

void ProjectHandler::onOpenProject()
{
    qDebug() << "[Project] Open project...";
    // TODO: 文件对话框选择工程文件 -> 加载 -> emit projectOpened(path)
}

void ProjectHandler::onSaveProject()
{
    qDebug() << "[Project] Save project...";
    // TODO: 若已有路径则覆盖保存；否则转到 onSaveAs()
    // emit projectSaved(path);
}

void ProjectHandler::onSaveAs()
{
    qDebug() << "[Project] Save project as...";
    // TODO: 文件对话框选择新路径 -> 保存 -> emit projectSaved(path)
}

// ========= 管理（Manage） =========
void ProjectHandler::onOpenRecent()
{
    qDebug() << "[Project] Open recent...";
    // TODO: 打开“最近工程”列表（菜单/面板），并执行打开
}

void ProjectHandler::onManageTemplates()
{
    qDebug() << "[Project] Manage templates...";
    // TODO: 模板管理（增删改查、从模板新建等）
}

void ProjectHandler::onProjectParams()
{
    qDebug() << "[Project] Project parameters...";
    // TODO: 弹出“工程参数”窗口，保存后刷新相关状态
}

void ProjectHandler::onBackupRestore()
{
    qDebug() << "[Project] Backup / Restore...";
    // TODO: 备份当前工程到压缩包；或从备份恢复
}
