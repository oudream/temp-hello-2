#ifndef MAIN_WINDOW_H
#define MAIN_WINDOW_H

#include <QMainWindow>
#include <QMap>
#include <QPointer>

#include "ui/pages/u_page_manager.h"
#include "ui/panels/u_panel_manager.h"

class URibbon;

class UPageBase;

class UPanelBase;

class UStatusBar;

class MctMainWindow : public QMainWindow {
Q_OBJECT
public:
    explicit MctMainWindow(QWidget *parent = 0);

    ~MctMainWindow();

protected:
    void showEvent(QShowEvent *e) override;

    void closeEvent(QCloseEvent *e) override;

private:
    // --- UI骨架 ---
    URibbon *_ribbon = nullptr;          ///< 顶部功能区（两层）
    QWidget *_central = nullptr;        ///< 中央容器（Stacked Pages）
    class QStackedWidget *_stack = nullptr;

    UStatusBar *_status = nullptr;       ///< 底部状态栏

    // --- 管理器 ---
    UPageManager *_pageMgr = nullptr;
    UPanelManager *_panelMgr = nullptr;
    QString _currentLayout = "grid_2x2"; // 记录当前四视图布局

    // --- 方法 ---
    void buildUi();

    void registerPages();

    void registerPanels();

    void wireSignals();

    void setupShortcuts();

    // --- 工具 ---
    int pageIndexOfId(const QString &id) const;   // 将 pageId 映射到 _stack 索引
    QString pageIdAtIndex(int index) const;        // 反查

private slots:

    // Ribbon 信号
    void onRequestSwitchPage(int pageIndex);

    void onRequestLayoutGrid();

    void onRequestLayout13();

    void onRequestTogglePanel(const QString &panelId, bool visible);

    // Page 转上层的命令请求与面板请求
    void onPageRequestCommand(const QString &cmd, const QVariantMap &args);

    void onPageRequestTogglePanel(const QString &panelId, bool visible);

    void onPageRequestLayoutChange(const QString &layoutId);

};


#endif // MAIN_WINDOW_H
