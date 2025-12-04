#ifndef CX_CT_X2_RIBBON_H
#define CX_CT_X2_RIBBON_H

#include <QWidget>
#include <QMap>
#include <QAction>

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QToolButton>
#include <QStackedWidget>
#include <QButtonGroup>

#include "u_ribbon_handler.h"
#include "u_ribbon_block.h"


class URibbon : public QWidget
{
Q_OBJECT
public:
    explicit URibbon(QWidget *parent = nullptr);

signals:

    // 根据需求，Ribbon只负责发出请求信号
    void requestSwitchPage(int pageIndex);

    // 更加通用的面板控制信号
    void requestTogglePanel(const QString &panelId, bool visible);


private slots:

    void onBlockButtonClicked(int blockIndex);

private:
    // 成员变量：按 ERibbonPage 顺序保存 5 个页面的 layout 指针
    QMap<QString, QAction *> _actions; ///< 全局动作表（按 ID 检索）
    QList<IURibbonHandler *> _handlers;
    QList<URibbonBlock> _blocks;       ///< 功能块列表（上层）
    bool _isBuilt = false;

    QWidget *_topBlockBar = nullptr;            // 上层功能块容器，顶部块按钮
    QStackedWidget *_bottomPages = nullptr;     // 下层功能名页面容器
    QButtonGroup *_blockBtnGroup = nullptr;

    void createActions();

    void buildUi();

    /// 辅助函数

    static QToolButton *makeBlockButton(const QString &text);

    static QToolButton *makeActionButton(QAction *act);

    static QFrame *buildGroup(const QString &title, const QList<QAction *> &actions);

};

#endif //CX_CT_X2_RIBBON_H