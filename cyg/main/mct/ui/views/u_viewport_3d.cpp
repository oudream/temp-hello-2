// u_viewport_3d.cpp
#include "u_viewport_3d.h"
#include <QVBoxLayout>

UViewport3D::UViewport3D(QWidget *parent) : UViewportBase(parent)
{
    auto *v = new QVBoxLayout(this);
    v->addWidget(new QLabel(tr("3D 视图（占位，可挂VTK）")));
}
