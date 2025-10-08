#include <QApplication>
#include <QMainWindow>
#include <QToolBar>
#include <QAction>
#include <QLabel>
#include <QVBoxLayout>
#include <QStyle>
#include <QTimer>

#include <common/docking/DockManager.h>
#include <common/docking/DockWidget.h>

#include <QGraphicsView>

using namespace ads;

// 一个占位控件：中间显示标题文字，方便区分视图
static QWidget* makePlaceholder(const QString& title, QWidget* parent = nullptr)
{
    auto* w = new QWidget(parent);
    auto* lay = new QVBoxLayout(w);
    lay->setContentsMargins(0,0,0,0);
    lay->addStretch();
    auto* lbl = new QLabel(title, w);
    lbl->setAlignment(Qt::AlignCenter);
    lbl->setStyleSheet("font: 700 20px 'Microsoft YaHei'; color: #DDD;");
    lay->addWidget(lbl);
    lay->addStretch();
    w->setAutoFillBackground(true);
    QPalette pal = w->palette();
    pal.setColor(QPalette::Window, QColor(32,32,32));
    w->setPalette(pal);
    return w;
}

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    QMainWindow win;

    // -------- 1) 配置 ADS（注意：配置要在 DockManager 构造之前） --------
    CDockManager::setConfigFlags(
            CDockManager::DefaultOpaqueConfig
            | CDockManager::FocusHighlighting
            | CDockManager::OpaqueSplitterResize
    );
    // 启用 Auto-Hide（可鼠标掠过展开）
    CDockManager::setAutoHideConfigFlags(static_cast<CDockManager::AutoHideFlags>(CDockManager::AutoHideFeatureEnabled |
                                                                                  CDockManager::AutoHideShowOnMouseOver));

    auto* dockManager = new CDockManager(&win);

    // -------- 2) 准备 4 个视图 DockWidget（实际项目换成 QVTKOpenGLNativeWidget） --------
    auto* wAxial    = new CDockWidget("Axial");
    wAxial->setWidget(makePlaceholder("Axial"));

    auto* wCoronal  = new CDockWidget("Coronal");
    wCoronal->setWidget(makePlaceholder("Coronal"));

    auto* wSagittal = new CDockWidget("Sagittal");
    wSagittal->setWidget(makePlaceholder("Sagittal"));

    auto* wView3D   = new CDockWidget("3D View");
    wView3D->setWidget(makePlaceholder("3D View"));

    // 右侧属性与直方图面板
    auto* wProps = new CDockWidget("属性");
    wProps->setWidget(makePlaceholder("属性面板"));

    auto* wHist = new CDockWidget("直方图");
    wHist->setWidget(makePlaceholder("直方图"));

    // -------- 3) 默认布局：2x2 + 右侧两面板（直方图演示 Auto-Hide） --------
    // 中心先放 Axial，得到“中心区域”指针
    CDockAreaWidget* centerArea = dockManager->setCentralWidget(wAxial);

    // 在中心区域右侧新开区域放 Coronal；返回“右侧区域”指针
    CDockAreaWidget* rightArea = dockManager->addDockWidget(RightDockWidgetArea, wCoronal, centerArea);

    // 在“中心区域”下方再开区域放 Sagittal（左下）
    dockManager->addDockWidget(BottomDockWidgetArea, wSagittal, centerArea);

    // 在“右侧区域”下方再开区域放 3D（右下）
    dockManager->addDockWidget(BottomDockWidgetArea, wView3D, rightArea);

    // 右侧属性与直方图（直方图默认置于属性下方）
    CDockAreaWidget* propArea = dockManager->addDockWidget(RightDockWidgetArea, wProps, centerArea);
    dockManager->addDockWidget(BottomDockWidgetArea, wHist, propArea);

    // 把直方图 Pin 成 Auto-Hide（右侧抽屉）
    dockManager->addAutoHideDockWidget(ads::SideBarLocation::SideBarRight, wHist);

    // -------- 4) 工具栏：切换 2×2 / 1+3、保存与恢复布局 --------
    QByteArray savedState; // 内存里保存一份 XML（实际可落盘）

    auto* tb = new QToolBar(&win);
    tb->setMovable(false);
    win.addToolBar(Qt::TopToolBarArea, tb);

    auto* actLayout2x2 = tb->addAction("布局：2×2");
    auto* actLayout13  = tb->addAction("布局：1+3");
    tb->addSeparator();
    auto* actSave   = tb->addAction("保存布局");
    auto* actLoad   = tb->addAction("恢复布局");
    auto* actReset  = tb->addAction("重置");

    QObject::connect(actLayout2x2, &QAction::triggered, [&](){
        // 直接重建：以 Axial 为中心，然后右侧 Coronal，下方 Sagittal/3D
        dockManager->closeOtherAreas(centerArea); // 清理中心以外的树形（保险，可选）
        centerArea = dockManager->setCentralWidget(wAxial);
        rightArea  = dockManager->addDockWidget(RightDockWidgetArea,  wCoronal,  centerArea);
        dockManager->addDockWidget(BottomDockWidgetArea, wSagittal, centerArea);
        dockManager->addDockWidget(BottomDockWidgetArea, wView3D,  rightArea);
        propArea   = dockManager->addDockWidget(RightDockWidgetArea,  wProps, centerArea);
        dockManager->addDockWidget(BottomDockWidgetArea, wHist,   propArea);
        dockManager->addAutoHideDockWidget(ads::SideBarLocation::SideBarRight, wHist);
    });

    QObject::connect(actLayout13, &QAction::triggered, [&](){
        // 以 3D 为“大图”，其余三个围在 上/左/下
        dockManager->closeOtherAreas(centerArea);
        centerArea = dockManager->setCentralWidget(wView3D);
        dockManager->addDockWidget(TopDockWidgetArea,    wAxial,    centerArea);
        dockManager->addDockWidget(LeftDockWidgetArea,   wCoronal,  centerArea);
        dockManager->addDockWidget(BottomDockWidgetArea, wSagittal, centerArea);
        propArea   = dockManager->addDockWidget(RightDockWidgetArea, wProps, centerArea);
        dockManager->addDockWidget(BottomDockWidgetArea, wHist,     propArea);
        dockManager->addAutoHideDockWidget(ads::SideBarLocation::SideBarRight, wHist);
    });

    QObject::connect(actSave, &QAction::triggered, [&](){
        savedState = dockManager->saveState();
    });
    QObject::connect(actLoad, &QAction::triggered, [&](){
        if(!savedState.isEmpty())
            dockManager->restoreState(savedState, 0);
    });
    QObject::connect(actReset, &QAction::triggered, [&](){
        // 相当于恢复到程序刚启动时的默认布局
        qApp->exit(EXIT_SUCCESS);
        // 或者也可以自行重建布局，这里做最简单的“重启式重置”
    });

    // -------- 5) 主窗体 --------
    win.setWindowTitle("MCT Docking Demo (ADS) — 2x2 & 1+3 + Auto-Hide");
    win.setCentralWidget(dockManager);
    win.resize(1400, 900);
    win.show();

    return app.exec();
}
