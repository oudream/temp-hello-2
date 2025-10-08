#ifndef UI_VIEWS_U_VIEWPORT_3D_H
#define UI_VIEWS_U_VIEWPORT_3D_H

#include "u_viewports_base.h"
#include <QLabel>

class UViewport3D : public UViewportBase
{
Q_OBJECT
public:
    explicit UViewport3D(QWidget *parent = 0);
};

#endif

