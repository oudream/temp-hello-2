#ifndef CX_CT_X2_MAIN_WINDOW_H
#define CX_CT_X2_MAIN_WINDOW_H


#include <QMainWindow>
#include <QStackedWidget>
#include "Ribbon.h"
#include "file_page.h"
#include "threed_page.h"
#include "panel_manager.h"
#include "panels/panel_property.h"
#include "util.h"

class MainWindow : public QMainWindow {
Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = nullptr);

private slots:
    void onSwitchPage(int idx);
    void onLayoutGrid();
    void onLayout13();
    void onToggleRightPanel();

private:
    Ribbon *m_ribbon = nullptr;
    QStackedWidget *m_pages = nullptr;
    FilePage *m_filePage = nullptr;
    ThreeDPage *m_3dPage = nullptr;
    PanelManager *m_panelMgr = nullptr;
};

#endif //CX_CT_X2_MAIN_WINDOW_H
