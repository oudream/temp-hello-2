#include "ribbon.h"
#include <QHBoxLayout>
#include <QToolButton>

static QToolButton* makeBtn(QAction *act) {
    QToolButton *b = new QToolButton;
    b->setDefaultAction(act);
    b->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    b->setFixedSize(QSize(80, 64));
    return b;
}

QGroupBox* Ribbon::buildGroup(const QString &title, const QList<QAction*> &actions) {
    QGroupBox *g = new QGroupBox(title);
    auto *layout = new QHBoxLayout(g);
    layout->setContentsMargins(8, 6, 8, 6);
    layout->setSpacing(6);
    for (auto *a : actions) layout->addWidget(makeBtn(a));
    layout->addStretch();
    return g;
}

Ribbon::Ribbon(QWidget *parent) : QWidget(parent), m_tabs(new QTabWidget(this)) {
    // 动作
    m_actSwitchFile = new QAction(QIcon(), tr("文件"));
    m_actSwitch3D   = new QAction(QIcon(), tr("三维"));
    m_actLayoutGrid = new QAction(QIcon(), tr("2×2布局"));
    m_actLayout13   = new QAction(QIcon(), tr("1&3布局"));
    m_actToggleRightPanel = new QAction(QIcon(), tr("显示/隐藏右侧面板"));

    connect(m_actSwitchFile, &QAction::triggered, this, [this]{ emit requestSwitchPage(0); });
    connect(m_actSwitch3D,   &QAction::triggered, this, [this]{ emit requestSwitchPage(1); });
    connect(m_actLayoutGrid, &QAction::triggered, this, [this]{ emit requestLayoutGrid(); });
    connect(m_actLayout13,   &QAction::triggered, this, [this]{ emit requestLayout13(); });
    connect(m_actToggleRightPanel, &QAction::triggered, this, [this]{ emit requestToggleRightPanel(); });

    m_tabs->addTab(buildHomePage(), tr("开始"));
    m_tabs->addTab(buildViewPage(), tr("视图"));
    m_tabs->addTab(buildPanelPage(), tr("面板"));

    auto *outer = new QHBoxLayout(this);
    outer->setContentsMargins(4, 0, 4, 0);
    outer->addWidget(m_tabs);
}

QWidget* Ribbon::buildHomePage() {
    QWidget *w = new QWidget;
    auto *h = new QHBoxLayout(w);
    h->setContentsMargins(8, 4, 8, 4);
    h->setSpacing(12);

    h->addWidget(buildGroup(tr("切换页面"), { m_actSwitchFile, m_actSwitch3D }));
    h->addWidget(buildGroup(tr("布局"), { m_actLayoutGrid, m_actLayout13 }));
    h->addWidget(buildGroup(tr("面板"), { m_actToggleRightPanel }));
    h->addStretch();
    return w;
}

QWidget* Ribbon::buildViewPage() {
    QWidget *w = new QWidget;
    auto *h = new QHBoxLayout(w);
    h->setContentsMargins(8,4,8,4);
    h->addWidget(buildGroup(tr("布局"), { m_actLayoutGrid, m_actLayout13 }));
    h->addStretch();
    return w;
}

QWidget* Ribbon::buildPanelPage() {
    QWidget *w = new QWidget;
    auto *h = new QHBoxLayout(w);
    h->setContentsMargins(8,4,8,4);
    h->addWidget(buildGroup(tr("面板"), { m_actToggleRightPanel }));
    h->addStretch();
    return w;
}
