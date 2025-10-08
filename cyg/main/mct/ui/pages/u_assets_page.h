#ifndef UI_U_ASSETS_PAGE_H
#define UI_U_ASSETS_PAGE_H

#include "u_page_base.h"
#include <QWidget>
#include <QPointer>

class QTreeView;

class QTableView;

class QPushButton;

class QLabel;

class QFileSystemModel;

/**
 * @brief AssetsPage - 原图资产页
 * 作用：影像资产浏览与管理（文件树/缩略图、批量导入、元数据、基础预处理）
 * 职责：只发“导入/批处理/预处理”等命令请求，不做IO。
 */
class UAssetsPage : public UPageBase
{
Q_OBJECT
public:
    explicit UAssetsPage(QObject *parent = 0);

    virtual ~UAssetsPage();

    virtual const char *getName() const;

    virtual QWidget *getWidget();

    virtual void onEnter();

    virtual void onLeave();

private slots:

    void onImport();

    void onBatchProcess();

    void onPreprocess();

private:
    QWidget *_root;
    QPointer<QTreeView> _tree;
    QPointer<QTableView> _table;
    QPointer<QPushButton> _btnImport;
    QPointer<QPushButton> _btnBatch;
    QPointer<QPushButton> _btnPre;
    QPointer<QLabel> _lblMeta;

    void buildUi();
};

#endif // UI_U_ASSETS_PAGE_H
