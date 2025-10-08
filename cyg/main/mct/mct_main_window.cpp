#include "mct_main_window.h"
#include "base/app_ids.h"

// Ribbon 与需求规格/模型
// 信号接口：requestSwitchPage / requestLayoutGrid ...  :contentReference[oaicite:8]{index=8}
#include "ui/u_ribbon.h"
// 功能块/功能名结构（行动作集中注册）
// :contentReference[oaicite:9]{index=9} :contentReference[oaicite:10]{index=10}
#include "ui/u_ribbon_block_model.h"

#include <QStackedWidget>
#include <QDockWidget>
#include <QApplication>
#include <QSettings>
#include <QCloseEvent>
#include <QMessageBox>
#include <QDebug>
#include <QVBoxLayout>
#include <QSettings>

// Pages
#include "ui/pages/u_page_base.h"
#include "ui/pages/u_project_page.h"
#include "ui/pages/u_assets_page.h"
#include "ui/pages/u_reconstruct_page.h"
#include "ui/pages/u_threed_proxy_page.h"
#include "ui/pages/u_thread_page.h"
#include "ui/pages/u_inspect_page.h"

// Panels
#include "ui/panels/u_panel_base.h"
#include "ui/panels/u_assets_panel.h"
#include "ui/panels/u_panel_camera.h"
#include "ui/panels/u_panel_knife.h"
#include "ui/panels/u_panel_render.h"
#include "ui/panels/u_panel_measure.h"
#include "ui/panels/u_panel_log.h"

// Views（四视图宿主）
#include "ui/views/u_viewports_host.h"

// 状态栏
#include "ui/u_status_bar.h"

static const char* K_LAST_PAGE   = "ui/last_page";
static const char* K_LAYOUT_MODE = "ui/layout_mode";
static const char* K_PANEL_VIS   = "ui/panel/%1/visible";


MctMainWindow::MctMainWindow(QWidget *parent) : QMainWindow(parent) {
    // 创建 界面控件
    buildUi();

    // 通过 PageManager 注册
    registerPages();

    // 通过 PanelManager 注册
    registerPanels();

    // 只连 Ribbon → MainWindow 的请求
    wireSignals();

    // Ctrl+1..5 / Ctrl+G/J/E（可在 Ribbon 上也设）
    setupShortcuts();

    // 默认进入 3D 页（需求规格 2. 顶层布局） :contentReference[oaicite:11]{index=11}
    // switchToPageByIndex(pageIndexOfId(AppIds::kPage_ThreeD));

    // === 取消系统标题栏（最小化/最大化/关闭顶栏） ===
//    setWindowFlag(Qt::FramelessWindowHint, true);
    setWindowFlags(Qt::FramelessWindowHint | Qt::Window);

    showMaximized();
}

MctMainWindow::~MctMainWindow() {}

// 从上到下分三层：URibbon（两层）、UPageManager（一层）
void MctMainWindow::buildUi() {
    // Ribbon 功能块 -> 功能点
    _ribbon = new URibbon(this);
    // 工作区
    _stack = new QStackedWidget(this);
    _central = new QWidget(this);
    auto *v = new QVBoxLayout(_central);
    v->setContentsMargins(0, 0, 0, 0);
    v->setSpacing(0);
    v->addWidget(_ribbon, 0);  // 不伸展
    v->addWidget(_stack,  1);  // 伸展填充
    setCentralWidget(_central);

    // 状态栏
    _status = new UStatusBar(this);

    // ✅ 创建 Managers
    _pageMgr = new UPageManager(_stack, this);
    _panelMgr = new UPanelManager(this, this);
}

void MctMainWindow::registerPages() {
    // 常规三页
    auto *pProject = new UProjectPage(this);
    auto *pAssets = new UAssetsPage(this);
    auto *pRecon = new UReconstructPage(this);

    // 3D 宿主
    QWidget *host = new UViewportsHost(this);

    auto *pThreeD = new UThreeDProxyPage(host, this);
    auto *pInspect = new UInspectPage(this);

    _pageMgr->registerPage("project", pProject);
    _pageMgr->registerPage("assets", pAssets);
    _pageMgr->registerPage("reconstruct", pRecon);
    _pageMgr->registerPage("three_d", pThreeD);
    _pageMgr->registerPage("inspect", pInspect);

    // 连接 Page 的三类标准信号（命令/面板/布局）
    for (auto *page: findChildren<UPageBase *>()) {
        connect(page, SIGNAL(requestCommand(QString, QVariantMap)),
                this, SLOT(onPageRequestCommand(QString, QVariantMap)));
        connect(page, SIGNAL(requestTogglePanel(QString, bool)),
                this, SLOT(onPageRequestTogglePanel(QString, bool)));
        connect(page, SIGNAL(requestLayoutChange(QString)),
                this, SLOT(onPageRequestLayoutChange(QString)));
    }

    // 默认先切到 3D（showEvent 再加载持久化）
    _pageMgr->switchTo(_pageMgr->indexOf("three_d"));
}

void MctMainWindow::registerPanels() {
    // 右侧 Dock 面板区，按需求规格注册属性/渲染/日志/相机/切面/测量等                     :contentReference[oaicite:13]{index=13}
    auto *pnlAssets = new UAssetsPanel(this);
    auto *pnlCamera = new UPanelCamera(this);
    auto *pnlKnife = new UPanelKnife(this);
    auto *pnlRender = new UPanelRender(this);
    auto *pnlMeasure = new UPanelMeasure(this);
    auto *pnlLog = new UPanelLog(this);

    _panelMgr->registerPanel("assets", pnlAssets);
    _panelMgr->registerPanel("camera", pnlCamera);
    _panelMgr->registerPanel("knife", pnlKnife);
    _panelMgr->registerPanel("render", pnlRender);
    _panelMgr->registerPanel("measure", pnlMeasure);
    _panelMgr->registerPanel("log", pnlLog);

    // 分组 tabify（不变）
    tabifyDockWidget(pnlAssets, pnlRender);
    tabifyDockWidget(pnlCamera, pnlKnife);
    tabifyDockWidget(pnlMeasure, pnlLog);

    // 面板命令上行
    for (auto *pnl: findChildren<UPanelBase *>()) {
        connect(pnl, &UPanelBase::requestCommand,
                this, &MctMainWindow::onPageRequestCommand);
    }
}

void MctMainWindow::wireSignals() {
    // Ribbon 只发请求信号 → 主窗路由（与需求文档一致）                                 :contentReference[oaicite:14]{index=14}
    connect(_ribbon, SIGNAL(requestSwitchPage(int)),
            this, SLOT(onRequestSwitchPage(int)));
    connect(_ribbon, SIGNAL(requestLayoutGrid()),
            this, SLOT(onRequestLayoutGrid()));
    connect(_ribbon, SIGNAL(requestLayout13()),
            this, SLOT(onRequestLayout13()));
    connect(_ribbon, SIGNAL(requestTogglePanel(QString, bool)),
            this, SLOT(onRequestTogglePanel(QString, bool)));
}

void MctMainWindow::setupShortcuts() {
    auto mk = [this](const QKeySequence &ks, auto slot) {
        auto *a = new QAction(this);
        a->setShortcut(ks);
        addAction(a);
        connect(a, &QAction::triggered, this, slot);
    };
    mk(QKeySequence("Ctrl+1"), [this] { onRequestSwitchPage(0); });
    mk(QKeySequence("Ctrl+2"), [this] { onRequestSwitchPage(1); });
    mk(QKeySequence("Ctrl+3"), [this] { onRequestSwitchPage(2); });
    mk(QKeySequence("Ctrl+4"), [this] { onRequestSwitchPage(3); });
    mk(QKeySequence("Ctrl+5"), [this] { onRequestSwitchPage(4); });
    mk(QKeySequence("Ctrl+G"), [this] { onRequestLayoutGrid(); });
    mk(QKeySequence("Ctrl+J"), [this] { onRequestLayout13(); });
    mk(QKeySequence("Ctrl+E"), [this] { onPageRequestLayoutChange("equalize"); });
}

int MctMainWindow::pageIndexOfId(const QString &id) const {
    int idx = 0;
    for (int i = 0; i < _stack->count(); ++i) {
        // 由于我们是按注册顺序 push 的，这里做一个简单映射（保持注册顺序）
        if (id == AppIds::kPage_Project && i == 0) return i;
        if (id == AppIds::kPage_Assets && i == 1) return i;
        if (id == AppIds::kPage_Reconstruct && i == 2) return i;
        if (id == AppIds::kPage_ThreeD && i == 3) return i;
        if (id == AppIds::kPage_Inspect && i == 4) return i;
    }
    return 0;
}

QString MctMainWindow::pageIdAtIndex(int index) const {
    switch (index) {
        case 0:
            return AppIds::kPage_Project;
        case 1:
            return AppIds::kPage_Assets;
        case 2:
            return AppIds::kPage_Reconstruct;
        case 3:
            return AppIds::kPage_ThreeD;
        case 4:
            return AppIds::kPage_Inspect;
        default:
            return AppIds::kPage_Project;
    }
}

// --- Ribbon 请求 ---

void MctMainWindow::onRequestSwitchPage(int pageIndex) { _pageMgr->switchTo(pageIndex); }

void MctMainWindow::onRequestLayoutGrid() {
    // 转发到 3D 宿主
    if (auto *host = findChild<UViewportsHost *>()) {
        // 2×2 布局（需求规格 6）           :contentReference[oaicite:15]{index=15}
        host->setLayoutMode("grid_2x2");
        _currentLayout = "grid_2x2";
    }
}

void MctMainWindow::onRequestLayout13() {
    if (auto *host = findChild<UViewportsHost *>()) {
        // 1&3 布局（需求规格 6）            :contentReference[oaicite:16]{index=16}
        host->setLayoutMode("layout_1x3");
        _currentLayout = "layout_1x3";
    }
}

void MctMainWindow::onRequestTogglePanel(const QString &panelId, bool vis) {
    _panelMgr->setVisible(panelId, vis);
}

// --- Page 请求 ---

void MctMainWindow::onPageRequestCommand(const QString &cmd, const QVariantMap &args) {
    // 这里对接 CommandBus（UI不直接改Model；命令化，符合统一规范 4.1）
    // :contentReference[oaicite:17]{index=17}

    if (cmd == "app.exit") {
        // 触发 closeEvent → 弹确认 → 持久化 → 退出
        this->close();
        return;
    }

    qDebug() << "[UI] requestCommand:" << cmd << args;
    _status->setMessage(tr("命令：%1").arg(cmd));
}

void MctMainWindow::onPageRequestTogglePanel(const QString &panelId, bool visible) {
    onRequestTogglePanel(panelId, visible);
}

void MctMainWindow::onPageRequestLayoutChange(const QString &layoutId) {
    if (auto *host = findChild<UViewportsHost *>()) {
        host->setLayoutMode(layoutId);
        _currentLayout = layoutId;
    }
}

void MctMainWindow::showEvent(QShowEvent *e) {
    QMainWindow::showEvent(e);
    static bool once = false;
    if (once) return;
    once = true;

    QSettings s;
    const int lastIdx = s.value(K_LAST_PAGE, _pageMgr->indexOf("three_d")).toInt();
    _pageMgr->switchTo(lastIdx);

    _currentLayout = s.value(K_LAYOUT_MODE, "grid_2x2").toString();
    if (auto *host = findChild<UViewportsHost *>()) host->setLayoutMode(_currentLayout);

    for (const auto &id: _panelMgr->allPanelIds()) {
        const bool vis = s.value(QString(K_PANEL_VIS).arg(id), true).toBool();
        _panelMgr->setVisible(id, vis);
    }
}

void MctMainWindow::closeEvent(QCloseEvent *e) {
    // 先二次确认
    const auto ret = QMessageBox::question(
            this, tr("退出"),
            tr("确定要退出 MCTstudio 吗？"),
            QMessageBox::Yes | QMessageBox::No, QMessageBox::No
    );
    if (ret != QMessageBox::Yes) { e->ignore(); return; }

    // 持久化用户界面状态
    QSettings s;
    s.setValue(K_LAST_PAGE, _stack->currentIndex());
    s.setValue(K_LAYOUT_MODE, _currentLayout);
    for (const auto &id: _panelMgr->allPanelIds()) {
        s.setValue(QString(K_PANEL_VIS).arg(id), _panelMgr->isVisible(id));
    }
    QMainWindow::closeEvent(e);
}
