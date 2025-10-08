#ifndef CX_CT_X2_RIBBON_H
#define CX_CT_X2_RIBBON_H

#include <QWidget>
#include <QMap>
#include <QAction>

class QStackedWidget;

class QButtonGroup;

class URibbonBlockModel;

class QToolButton;

class QGroupBox;

class URibbon : public QWidget
{
Q_OBJECT
public:
    explicit URibbon(QWidget *parent = nullptr);

signals:

    // 根据需求，Ribbon只负责发出请求信号
    void requestSwitchPage(int pageIndex);

    void requestLayoutGrid();

    void requestLayout13();

    void requestTogglePanel(const QString &panelId, bool visible); // 更加通用的面板控制信号

    void requestLayoutEqual(); // 等分布局（Ctrl+E）

private slots:

    void onBlockButtonClicked(int blockIndex);

private:
    URibbonBlockModel *_model = nullptr;
    QWidget *_topBlockBar = nullptr;         // 上层功能块容器
    QStackedWidget *_bottomPages = nullptr; // 下层功能名页面容器
    QButtonGroup *_blockBtnGroup = nullptr;

    void createActions();

    void buildUi();

    // 辅助函数
    static QToolButton *makeBlockButton(const QString &text);

    static QToolButton *makeActionButton(QAction *act);

    static QGroupBox *buildGroup(const QString &title, const QList<QAction *> &actions);
};

#endif //CX_CT_X2_RIBBON_H