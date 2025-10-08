#ifndef UI_VIEWS_U_VIEWPORT_CORONAL_H
#define UI_VIEWS_U_VIEWPORT_CORONAL_H

#include "u_viewports_base.h"
#include <QLabel>

class UViewportCoronal : public UViewportBase
{
Q_OBJECT
public:
    explicit UViewportCoronal(QWidget *parent = 0);
};

#endif

