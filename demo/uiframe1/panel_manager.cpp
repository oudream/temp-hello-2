#include "panel_manager.h"

PanelManager::PanelManager(QMainWindow *mw) : QObject(mw), m_mw(mw) {}

QDockWidget* PanelManager::addPanel(const QString &id, QWidget *widget, const QString &title){
    if (m_panels.contains(id)) return m_panels.value(id);
    QDockWidget *dock = new QDockWidget(title, m_mw);
    dock->setObjectName(id);
    dock->setWidget(widget);
    dock->setFeatures(QDockWidget::DockWidgetClosable | QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetFloatable);
    m_mw->addDockWidget(Qt::RightDockWidgetArea, dock);

    QAction *act = new QAction(title, this);
    act->setCheckable(true);
    act->setChecked(true);
    QObject::connect(act, &QAction::toggled, dock, &QDockWidget::setVisible);
    QObject::connect(dock, &QDockWidget::visibilityChanged, act, &QAction::setChecked);

    m_panels.insert(id, dock);
    m_actions.insert(id, act);
    return dock;
}

void PanelManager::togglePanelVisible(const QString &id){
    auto *dock = m_panels.value(id, nullptr);
    if (!dock) return;
    dock->setVisible(!dock->isVisible());
}

bool PanelManager::isPanelVisible(const QString &id) const{
    auto *dock = m_panels.value(id, nullptr);
    return dock ? dock->isVisible() : false;
}
