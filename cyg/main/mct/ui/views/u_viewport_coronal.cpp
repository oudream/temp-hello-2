#include "u_viewport_coronal.h"
#include <QVBoxLayout>

UViewportCoronal::UViewportCoronal(QWidget *parent) : UViewportBase(parent)
{
    auto *v = new QVBoxLayout(this);
    v->addWidget(new QLabel(tr("Coronal 视图（占位，可挂VTK）")));
}
