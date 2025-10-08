#include "u_viewport_sagittal.h"
#include <QVBoxLayout>

UViewportSagittal::UViewportSagittal(QWidget *parent) : UViewportBase(parent)
{
    auto *v = new QVBoxLayout(this);
    v->addWidget(new QLabel(tr("Sagittal 视图（占位，可挂VTK）")));
}
