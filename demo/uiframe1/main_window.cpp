#include "main_window.h"
#include <QStatusBar>
#include <QLabel>

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent),
                                          m_ribbon(new Ribbon(this)),
                                          m_pages(new QStackedWidget(this)),
                                          m_filePage(new FilePage(this)),
                                          m_3dPage(new ThreeDPage(this)),
                                          m_panelMgr(new PanelManager(this))
{
    setWindowTitle("CT 界面框架 - 示例");
    setDockOptions(QMainWindow::AllowTabbedDocks | QMainWindow::AnimatedDocks);

    // 顶部 Ribbon（替代传统菜单栏）
    setMenuWidget(m_ribbon);

    // 中央两大页面：0 文件；1 三维
    setCentralWidget(m_pages);
    m_pages->addWidget(m_filePage);
    m_pages->addWidget(m_3dPage);

    // 右侧默认添加一个“属性”面板（可自由再加别的面板）
    PanelProperty *prop = new PanelProperty(this);
    m_panelMgr->addPanel("panel.property", prop, tr("属性"));

    // Ribbon 交互
    connect(m_ribbon, &Ribbon::requestSwitchPage, this, &MainWindow::onSwitchPage);
    connect(m_ribbon, &Ribbon::requestLayoutGrid, this, &MainWindow::onLayoutGrid);
    connect(m_ribbon, &Ribbon::requestLayout13,   this, &MainWindow::onLayout13);
    connect(m_ribbon, &Ribbon::requestToggleRightPanel, this, &MainWindow::onToggleRightPanel);

    // 状态栏
    statusBar()->addPermanentWidget(new QLabel("Ready"));
}

void MainWindow::onSwitchPage(int idx){
    m_pages->setCurrentIndex(idx);
}

void MainWindow::onLayoutGrid(){
    m_3dPage->fourView()->setLayoutMode(FourViewLayout::Grid2x2);
}

void MainWindow::onLayout13(){
    m_3dPage->fourView()->setLayoutMode(FourViewLayout::OneAndThree);
}

void MainWindow::onToggleRightPanel(){
    m_panelMgr->togglePanelVisible("panel.property");
}
