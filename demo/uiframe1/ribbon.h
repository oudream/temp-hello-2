#ifndef CX_CT_X2_RIBBON_H
#define CX_CT_X2_RIBBON_H


#include <QWidget>
#include <QTabWidget>
#include <QAction>
#include <QGroupBox>

class Ribbon : public QWidget {
Q_OBJECT
public:
    explicit Ribbon(QWidget *parent=nullptr);

    QAction *actSwitchToFile() const { return m_actSwitchFile; }
    QAction *actSwitchTo3D() const   { return m_actSwitch3D; }
    QAction *actLayoutGrid()  const  { return m_actLayoutGrid; }
    QAction *actLayout13()    const  { return m_actLayout13; }
    QAction *actToggleRightPanel() const { return m_actToggleRightPanel; }

signals:
    void requestSwitchPage(int pageIndex);   // 0-file, 1-3d
    void requestLayoutGrid();
    void requestLayout13();
    void requestToggleRightPanel();

private:
    QTabWidget *m_tabs = nullptr;
    QAction *m_actSwitchFile = nullptr;
    QAction *m_actSwitch3D   = nullptr;
    QAction *m_actLayoutGrid  = nullptr;
    QAction *m_actLayout13    = nullptr;
    QAction *m_actToggleRightPanel = nullptr;

    QWidget* buildHomePage();
    QWidget* buildViewPage();
    QWidget* buildPanelPage();
    static QGroupBox* buildGroup(const QString &title, const QList<QAction*> &actions);
};


#endif //CX_CT_X2_RIBBON_H
