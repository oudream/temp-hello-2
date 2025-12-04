#include "u_panel_manager.h"
#include <QMainWindow>
#include "panels/u_panel_base.h"

UPanelManager::UPanelManager(QMainWindow *mw, QObject *parent)
        : QObject(parent), _mw(mw)
{}

void UPanelManager::registerPanel(const QString &id, UPanelBase *panel)
{
    _panels.insert(id, panel);
    _mw->addDockWidget(Qt::RightDockWidgetArea, panel);
}

UPanelBase *UPanelManager::panel(const QString &id) const
{
    return _panels.value(id, nullptr);
}

void UPanelManager::setVisible(const QString &id, bool vis)
{
    if (auto *p = panel(id)) p->setVisible(vis);
}

bool UPanelManager::isVisible(const QString &id) const
{
    if (auto *p = panel(id)) return p->isVisible();
    return false;
}

QStringList UPanelManager::allPanelIds() const
{ return _panels.keys(); }
