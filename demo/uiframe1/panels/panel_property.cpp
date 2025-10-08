//
// Created by Administrator on 2025/9/8.
//

#include "panel_property.h"
#include <QPushButton>

PanelProperty::PanelProperty(QWidget *parent) : IPanel(parent) {
    auto *f = new QFormLayout(this);
    f->setContentsMargins(8,8,8,8);
    f->addRow(tr("属性"), new QWidget);

    m_spThreshold = new QDoubleSpinBox(this);
    m_spThreshold->setRange(0.0, 100.0);
    m_spThreshold->setValue(50.0);
    f->addRow(tr("阈值:"), m_spThreshold);

    m_chkInvert = new QCheckBox(tr("反相"), this);
    f->addRow(QString(), m_chkInvert);

    auto *btn = new QPushButton(tr("应用"));
    connect(btn, &QPushButton::clicked, this, [this]{
        emit applyProperty(m_spThreshold->value(), m_chkInvert->isChecked());
        emit panelEvent("apply-property", QVariant::fromValue(m_spThreshold->value()));
    });
    f->addRow(QString(), btn);
}
