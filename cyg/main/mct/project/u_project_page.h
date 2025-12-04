#ifndef UI_U_PROJECT_PAGE_H
#define UI_U_PROJECT_PAGE_H

#include "pages/u_page_base.h"
#include <QWidget>
#include <QPointer>

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QTableWidget>
#include <QHeaderView>
#include <QPushButton>
#include <QVariantMap>

/**
 * @brief ProjectPage - 项目页
 * 作用：工程级信息总览（路径、批次、模板、最近记录、配置导入导出）
 * 职责：提供工程操作的入口按钮与概览表；点击后发出 requestCommand 交由上层转换为 Cmd*。
 * 原理：纯UI壳，不持久化；实际读写放到上层/适配层。
 */
class UProjectPage : public UPageBase
{
Q_OBJECT
public:
    explicit UProjectPage(QWidget *parent);

    ~UProjectPage() override;

    const char *getName() const override;

    QWidget *getWidget() override;

    void onEnter() override;

    void onLeave() override;

private slots:

    void onNewProject();

    void onOpenProject();

    void onSaveProject();

    void onExportConfig();

    void onImportConfig();

    void onExitApp();

private:
    QWidget *_root;                 ///< 根Widget（UI线程使用）
    QPointer<QLabel> _lblPath;      ///< 当前工程路径显示
    QPointer<QTableWidget> _tbl;    ///< 最近工程/批次列表
    QPointer<QPushButton> _btnNew;
    QPointer<QPushButton> _btnOpen;
    QPointer<QPushButton> _btnSave;
    QPointer<QPushButton> _btnExport;
    QPointer<QPushButton> _btnImport;
    QPointer<QPushButton> _btnExit;

    void buildUi(QWidget *parent);

    void fillRecent();
};

#endif // UI_U_PROJECT_PAGE_H
