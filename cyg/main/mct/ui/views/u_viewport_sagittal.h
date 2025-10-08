#ifndef UI_VIEWS_U_VIEWPORT_SAGITTAL_H
#define UI_VIEWS_U_VIEWPORT_SAGITTAL_H

#include "u_viewports_base.h"
#include <QLabel>

class UViewportSagittal : public UViewportBase
{
Q_OBJECT
public:
    explicit UViewportSagittal(QWidget *parent = 0);
};

#endif

