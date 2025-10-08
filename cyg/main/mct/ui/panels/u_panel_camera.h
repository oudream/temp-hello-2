#ifndef UI_U_PANEL_CAMERA_H
#define UI_U_PANEL_CAMERA_H

#include "u_panel_base.h"
#include <QPointer>

class QDoubleSpinBox;

class QPushButton;

/**
 * @brief PanelCamera - 相机控制面板（3D页）
 * 作用：相机 Dolly/Orbit/Reset 等；仅发命令，不直接改 VTK。
 */
class UPanelCamera : public UPanelBase
{
Q_OBJECT
public:
    explicit UPanelCamera(QWidget *parent = 0);

    virtual ~UPanelCamera();

    virtual const char *getName() const;

public slots:

    virtual void updateContext(MObject *activeObject);

private slots:

    void onDolly();

    void onOrbit();

    void onReset();

private:
    QWidget *_root;
    QPointer<QDoubleSpinBox> _spDolly;
    QPointer<QDoubleSpinBox> _spAz;
    QPointer<QDoubleSpinBox> _spEl;
    QPointer<QPushButton> _btnDolly;
    QPointer<QPushButton> _btnOrbit;
    QPointer<QPushButton> _btnReset;

    void buildUi();
};

#endif // UI_U_PANEL_CAMERA_H
