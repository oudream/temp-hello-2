#ifndef UI_VIEWS_U_VIEWPORT_AXIAL_H
#define UI_VIEWS_U_VIEWPORT_AXIAL_H

#include "u_viewports_base.h"
#include <QLabel>

class UViewportAxial : public UViewportBase
{
Q_OBJECT
public:
    explicit UViewportAxial(QWidget *parent = 0);
};

#endif

