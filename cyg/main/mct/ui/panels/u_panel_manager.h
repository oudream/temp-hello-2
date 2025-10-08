#ifndef CX_CT_X2_U_PANEL_MANAGER_H
#define CX_CT_X2_U_PANEL_MANAGER_H


#include <QObject>
#include <QMap>
#include <QPointer>

class QMainWindow;

class UPanelBase;

/**
 * @brief PanelManager - 统一管理 Dock 面板（注册、停靠、显隐、持久化）
 */
class UPanelManager : public QObject
{
Q_OBJECT
public:
    explicit UPanelManager(QMainWindow *mw, QObject *parent = 0);

    void registerPanel(const QString &id, UPanelBase *panel);

    UPanelBase *panel(const QString &id) const;

    void setVisible(const QString &id, bool vis);

    bool isVisible(const QString &id) const;

    QStringList allPanelIds() const;

private:
    QMainWindow *_mw = nullptr;
    QMap<QString, QPointer<UPanelBase>> _panels;

};


#endif //CX_CT_X2_U_PANEL_MANAGER_H
