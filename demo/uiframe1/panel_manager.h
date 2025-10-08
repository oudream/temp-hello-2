#ifndef CX_CT_X2_PANEL_MANAGER_H
#define CX_CT_X2_PANEL_MANAGER_H


#include <QObject>
#include <QMap>
#include <QDockWidget>
#include <QAction>
#include <QMainWindow>

class PanelManager : public QObject {
Q_OBJECT
public:
    explicit PanelManager(QMainWindow *mw);
    QDockWidget* addPanel(const QString &id, QWidget *widget, const QString &title);
    void togglePanelVisible(const QString &id);
    bool isPanelVisible(const QString &id) const;
    QAction* visibilityAction(const QString &id) const { return m_actions.value(id, nullptr); }

private:
    QMainWindow *m_mw = nullptr;
    QMap<QString, QDockWidget*> m_panels;
    QMap<QString, QAction*> m_actions;
};


#endif //CX_CT_X2_PANEL_MANAGER_H
