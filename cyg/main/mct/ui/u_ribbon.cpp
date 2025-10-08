#include "u_ribbon.h"
#include "u_ribbon_block_model.h"
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QToolButton>
#include <QGroupBox>
#include <QStackedWidget>
#include <QButtonGroup>

URibbon::URibbon(QWidget *parent) : QWidget(parent),
                                    _model(new URibbonBlockModel(this)),
                                    _topBlockBar(new QWidget(this)),
                                    _bottomPages(new QStackedWidget(this)),
                                    _blockBtnGroup(new QButtonGroup(this))
{
    createActions();
    buildUi();

    // 默认选中第一个功能块
    if (_blockBtnGroup->buttons().count() > 0)
    {
        _blockBtnGroup->buttons().first()->click();
    }
}

void URibbon::createActions()
{
    _model->createActions(this);

    // 布局三件套
    connect(_model->action("layout_grid"), &QAction::triggered, this, &URibbon::requestLayoutGrid);
    connect(_model->action("layout_1x3"), &QAction::triggered, this, &URibbon::requestLayout13);
    connect(_model->action("layout_equal"), &QAction::triggered, this, &URibbon::requestLayoutEqual);

    // 面板勾选（toggled 带 bool）
    auto connPanel = [this](const char *id)
    {
        auto *a = _model->action(id);
        if (a)
        {
            QObject::connect(a, &QAction::toggled, this,
                             [this, id](bool on)
                             { emit requestTogglePanel(QString::fromUtf8(id), on); });
        }
    };
    connPanel("panel_assets");
    connPanel("panel_render");
    connPanel("panel_camera");
    connPanel("panel_knife");
    connPanel("panel_measure");
    connPanel("panel_log");
}

void URibbon::buildUi()
{
    // 1. 创建上层功能块按钮
    auto *topLayout = new QHBoxLayout(_topBlockBar);
    topLayout->setContentsMargins(12, 6, 12, 6); // 左右12px，上下6px
    topLayout->setSpacing(1); // 按钮之间留白

    const auto &blocks = _model->getBlocks();
    for (int i = 0; i < blocks.size(); ++i)
    {
        const auto &block = blocks[i];
        QToolButton *btn = makeBlockButton(block.title);
        btn->setCheckable(true);
        topLayout->addWidget(btn);
        _blockBtnGroup->addButton(btn, i);

        // 2. 为每个功能块创建一个对应的下层页面
        QWidget *page = new QWidget;
        auto *pageLayout = new QHBoxLayout(page);
        pageLayout->setContentsMargins(8, 4, 8, 4);
        pageLayout->setSpacing(12);
        for (const auto &groupData: block.groups)
        {
            pageLayout->addWidget(buildGroup(groupData.title, groupData.actions));
        }
        pageLayout->addStretch();
        _bottomPages->addWidget(page);
    }
    topLayout->addStretch();
    connect(_blockBtnGroup, QOverload<int>::of(&QButtonGroup::idClicked), this, &URibbon::onBlockButtonClicked);

    // 3. 整体布局
    auto *outerLayout = new QVBoxLayout(this);
    outerLayout->setContentsMargins(4, 2, 4, 2); // Ribbon 与窗口边界有缓冲
    outerLayout->setSpacing(0);
    outerLayout->addWidget(_topBlockBar);
    outerLayout->addWidget(_bottomPages);

    _topBlockBar->setObjectName("ribbonTopBar");
    _bottomPages->setObjectName("ribbonBottomBar");
}

void URibbon::onBlockButtonClicked(int blockIndex)
{
    // 当上层按钮被点击，切换下层页面，并发出切换主窗口页面的信号
    _bottomPages->setCurrentIndex(blockIndex);
    const auto &block = _model->getBlocks().at(blockIndex);
    emit requestSwitchPage(block.pageIndex);
}


// --- 辅助函数 ---
QToolButton *URibbon::makeBlockButton(const QString &text)
{
    QToolButton *b = new QToolButton;
    b->setText(text);
    b->setToolButtonStyle(Qt::ToolButtonTextOnly);
    b->setFixedHeight(32);      // 高度 32
    b->setMinimumWidth(72);     // 加宽，保证文字不挤
    b->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    // 可在QSS中设置样式，如选中效果
    return b;
}

// 功能点
QToolButton *URibbon::makeActionButton(QAction *act)
{
    QToolButton *b = new QToolButton;
    b->setDefaultAction(act);

    // 改为图标在左、文字在右（更扁平）
    b->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);

    // 统一小图标尺寸（可根据DPI再调）
    b->setIconSize(QSize(18, 18));

    // 目标高度 28~32，这里给 28
    b->setMinimumHeight(28);
    b->setMaximumHeight(28);

    // 宽度不用死给，交给布局；如需固定可设 setFixedSize(...)
    b->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    b->setAutoRaise(true);
    return b;
}

// 功能块（功能组） -> 功能点
QGroupBox *URibbon::buildGroup(const QString &title, const QList<QAction *> &actions)
{
    QGroupBox *g = new QGroupBox(title);
    g->setTitle(QString());
    g->setFlat(true);
    g->setFixedHeight(84);
    auto *layout = new QHBoxLayout(g);
    layout->setContentsMargins(8, 6, 8, 6);
    layout->setSpacing(6);
    for (auto *a: actions)
    {
        if (a) layout->addWidget(makeActionButton(a));
    }
    layout->addStretch();
    return g;
}