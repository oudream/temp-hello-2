#ifndef UI_U_PANEL_RENDER_H
#define UI_U_PANEL_RENDER_H

#include "u_panel_base.h"
#include <QPointer>

class QComboBox;

class QSlider;

class QDoubleSpinBox;

class QPushButton;

/**
 * @brief PanelRender - 渲染设置（3D页-渲染）
 * 作用：体渲染/MIP/表面、TF/采样率、LUT、窗宽窗位
 */
class UPanelRender : public UPanelBase
{
Q_OBJECT
public:
    explicit UPanelRender(QWidget *parent = 0);

    virtual ~UPanelRender();

    virtual const char *getName() const;

public slots:

    virtual void updateContext(MObject *activeObject);

private slots:

    void onModeChanged(int);

    void onSamplingChanged(int);

    void onLutChanged(int);

    void onWindowChanged();

    void onLevelChanged();

private:
    QWidget *_root;
    QPointer<QComboBox> _cmbMode;   ///< 渲染模式
    QPointer<QSlider> _slSampling;  ///< 采样率
    QPointer<QComboBox> _cmbLut;    ///< LUT
    QPointer<QDoubleSpinBox> _spWL; ///< 窗宽
    QPointer<QDoubleSpinBox> _spWW; ///< 窗位

    void buildUi();
};

#endif // UI_U_PANEL_RENDER_H
