#include "u_ribbon.h"

#include "u_ribbon_registry.h"
#include "base/icon_manager.h"


URibbon::URibbon(QWidget *parent) : QWidget(parent),
                                    _topBlockBar(new QWidget(this)),
                                    _bottomPages(new QStackedWidget(this)),
                                    _blockBtnGroup(new QButtonGroup(this))
{
    _topBlockBar->setObjectName("ribbonTopBar");
    _bottomPages->setObjectName("ribbonBottomBar");

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
    // 确保 URibbon 中有：
    // QMap<QString, QAction*> _actions;
    // QList<URibbonBlock> _blocks;

    _actions.clear();
    _blocks.clear();
    _handlers.clear();

    // 1) 汇总注册的模块，逐个产出 UI Block，并在 _actions 中登记动作
    const auto &modules = URibbonRegistry::getModules();
    for (const auto *module: modules)
    {
        if (!module) continue;

        // 模块负责：创建所需 QAction 并写入 actions map；返回自身的 Block（含分组、指向这些 QAction 的指针）
        auto handler = module->createHandler(this);
        URibbonBlock block = module->createModule(this, _actions, handler);
        _blocks.append(block);
        _handlers.append(handler);
    }

    // 2)（可选）在这里补充全局/公共动作的连接（如果这些动作不是由模块托管）
    // 例如：布局与面板的切换事件，如果这些动作是由某个模块创建的：
    auto connectTogglePanel = [this](const char *id)
    {
        if (auto *a = _actions.value(QString::fromUtf8(id), nullptr))
        {
            QObject::connect(a, &QAction::toggled, this,
                             [this, id](bool on)
                             { emit requestTogglePanel(QString::fromUtf8(id), on); });
        }
    };
    // 仅当对应动作确实已由某模块创建时，这些连接才会生效（可按需保留/删除）
    connectTogglePanel(AppIds::kAction_Panel_Assets);
    connectTogglePanel(AppIds::kAction_Panel_Render);
    connectTogglePanel(AppIds::kAction_Panel_Camera);
    connectTogglePanel(AppIds::kAction_Panel_Knife);
    connectTogglePanel(AppIds::kAction_Panel_Measure);
    connectTogglePanel(AppIds::kAction_Panel_Log);
}

void URibbon::buildUi()
{
    // 清理旧 UI
    if (_bottomPages)
    {
        while (_bottomPages->count() > 0)
        {
            auto *w = _bottomPages->widget(0);
            _bottomPages->removeWidget(w);
            w->deleteLater();
        }
    }

    // 1. 创建上层功能块按钮
    auto *oldLayout = _topBlockBar->layout();
    auto *topLayout = new QHBoxLayout(_topBlockBar);
    delete oldLayout;
    topLayout->setContentsMargins(2, 0, 2, 0); // 左右12px，上下6px
    topLayout->setSpacing(1); // 按钮之间留白
    for (int i = 0; i < _blocks.size(); ++i)
    {
        const auto &block = _blocks[i];
        QToolButton *blockButton = makeBlockButton(block.title);
        blockButton->setCheckable(true);
        topLayout->addWidget(blockButton);
        _blockBtnGroup->addButton(blockButton, i); // 只管理，不作UI显示

        // 2. 为每个功能块创建一个对应的下层放功能块的区域
        auto *blockPanel = new QWidget;
        auto *blockTabLayout = new QHBoxLayout(blockPanel);
        blockTabLayout->setContentsMargins(2, 0, 2, 0);
        blockTabLayout->setSpacing(2);
        for (const auto &groupData: block.groups)
        {
            blockTabLayout->addWidget(buildGroup(groupData.title, groupData.actions));
        }
        blockTabLayout->addStretch();
        _bottomPages->addWidget(blockPanel);
    }
    topLayout->addStretch();
    connect(_blockBtnGroup, QOverload<int>::of(&QButtonGroup::idClicked), this, &URibbon::onBlockButtonClicked);

    // 3. 整体布局
    auto *outerLayout = new QVBoxLayout(this);
    outerLayout->setContentsMargins(4, 0, 4, 0); // Ribbon 与窗口边界有缓冲
    outerLayout->setSpacing(0);
    outerLayout->addWidget(_topBlockBar);
    outerLayout->addWidget(_bottomPages);
}

void URibbon::onBlockButtonClicked(int blockIndex)
{
    // 当上层按钮被点击，切换下层页面，并发出切换主窗口页面的信号
    auto page = AppIds::RibbonBlockToPage(static_cast<AppIds::ERibbonBlock>(blockIndex));
    int pageIndex = static_cast<int>(page);
    _bottomPages->setCurrentIndex(blockIndex);
    emit requestSwitchPage(pageIndex);
}

// 模块，创建分块按钮
QToolButton *URibbon::makeBlockButton(const QString &text)
{
    auto *b = new QToolButton;
    b->setText(text);
    b->setToolButtonStyle(Qt::ToolButtonTextOnly);
    b->setFixedHeight(32);      // 高度 32
    b->setMinimumWidth(72);     // 加宽，保证文字不挤
    b->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    // 可在QSS中设置样式，如选中效果
    return b;
}

// 功能点，创建功能点按钮
QToolButton *URibbon::makeActionButton(QAction *act)
{
    auto *b = new QToolButton;
    b->setDefaultAction(act);

    // 改为图标在左、文字在右（更扁平）
    b->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);

    // 统一小图标尺寸（可根据DPI再调）
    const auto sz = IconManager::i().sizes();
    b->setIconSize(sz.actionIcon);
    b->setFixedSize(sz.buttonSize);

    // 目标高度 28~32，这里给 28
    // b->setMinimumHeight(28);
    // b->setMaximumHeight(28);

    // 宽度不用死给，交给布局；如需固定可设 setFixedSize(...)
    b->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    b->setAutoRaise(false);
    return b;
}

// 功能组 -> 功能点，创建功能组窗口
QFrame *URibbon::buildGroup(const QString &title, const QList<QAction *> &actions)
{
    // 使用 QFrame，无标题，无额外 top margin
    auto* g = new QFrame;
    g->setObjectName("ribbonGroup");
    g->setFrameShape(QFrame::NoFrame);   // 无默认边框
    g->setFixedHeight(84);               // 可按需要调节

    // 自定义外观（圆角 + 边框 + 背景）
    g->setStyleSheet(R"(
        #ribbonGroup {
            border: 1px solid #2a2a2a;
            border-radius: 6px;
            background-color: #2d2d2d;
        }
    )");

    // 水平布局（按钮在一行）
    auto* layout = new QHBoxLayout(g);
    layout->setContentsMargins(0, 2, 2, 2);  // 左上右下边距
    layout->setSpacing(2);

    for (auto* a : actions)
    {
        if (a)
            layout->addWidget(makeActionButton(a));
    }

    layout->addStretch();
    return g;
}
