#ifndef UI_U_PANEL_KNIFE_H
#define UI_U_PANEL_KNIFE_H

#include "u_panel_base.h"
#include <QPointer>

class QDoubleSpinBox;

class QComboBox;

class QPushButton;

/**
 * @brief PanelKnife - MPR 三刀控制面板（3D页-切面）
 * 作用：移动/旋转三刀、厚度/锁轴设置；命令化发出
 */
class UPanelKnife : public UPanelBase
{
Q_OBJECT
public:
    explicit UPanelKnife(QWidget *parent = 0);

    virtual ~UPanelKnife();

    virtual const char *getName() const;

public slots:

    virtual void updateContext(MObject *activeObject);

private slots:

    void onMove();

    void onRotate();

    void onThickness();

    void onLockAxis();

private:
    QWidget *_root;
    QPointer<QComboBox> _cmbKnife;     ///< AXIAL/SAGITTAL/CORONAL
    QPointer<QDoubleSpinBox> _spMove;  ///< mm
    QPointer<QDoubleSpinBox> _spRotate;///< degree
    QPointer<QDoubleSpinBox> _spThick; ///< mm
    QPointer<QComboBox> _cmbLock;      ///< X/Y/Z/None
    QPointer<QPushButton> _btnMove;
    QPointer<QPushButton> _btnRotate;
    QPointer<QPushButton> _btnThick;
    QPointer<QPushButton> _btnLock;

    void buildUi();
};

#endif // UI_U_PANEL_KNIFE_H
