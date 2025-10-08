#ifndef UI_U_ASSETS_PANEL_H
#define UI_U_ASSETS_PANEL_H

#include "u_panel_base.h"
#include <QPointer>

class QListWidget;

class QPushButton;

/**
 * @brief AssetsPanel - 资产侧栏（与 AssetsPage 配合）
 * 作用：快速筛选/收藏/最近使用；触发导入与管理命令
 */
class UAssetsPanel : public UPanelBase
{
Q_OBJECT
public:
    explicit UAssetsPanel(QWidget *parent = 0);

    virtual ~UAssetsPanel();

    virtual const char *getName() const;

public slots:

    virtual void updateContext(MObject *activeObject);

private slots:

    void onImport();

    void onOpenDir();

private:
    QWidget *_root;
    QPointer<QListWidget> _listFav;
    QPointer<QPushButton> _btnImport;
    QPointer<QPushButton> _btnOpenDir;

    void buildUi();
};

#endif // UI_U_ASSETS_PANEL_H
