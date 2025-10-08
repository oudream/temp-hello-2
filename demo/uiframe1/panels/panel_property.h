//
// Created by Administrator on 2025/9/8.
//

#ifndef CX_CT_X2_PANEL_PROPERTY_H
#define CX_CT_X2_PANEL_PROPERTY_H


#pragma once
#include "../ipanel.h"
#include <QFormLayout>
#include <QDoubleSpinBox>
#include <QCheckBox>

class PanelProperty : public IPanel {
Q_OBJECT
public:
    explicit PanelProperty(QWidget *parent=nullptr);

signals:
    void applyProperty(double threshold, bool invert);

private:
    QDoubleSpinBox *m_spThreshold = nullptr;
    QCheckBox *m_chkInvert = nullptr;
};


#endif //CX_CT_X2_PANEL_PROPERTY_H
