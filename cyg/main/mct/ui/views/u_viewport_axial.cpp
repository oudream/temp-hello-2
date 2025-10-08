#include "u_viewport_axial.h"
#include <QVBoxLayout>

UViewportAxial::UViewportAxial(QWidget *parent) : UViewportBase(parent)
{
    auto *v = new QVBoxLayout(this);
    v->addWidget(new QLabel(tr("Axial 视图（占位，可挂VTK）")));
}
