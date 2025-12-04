#pragma once

#include "u_ribbon_handler.h"

class ResliceHandler : public IURibbonHandler
{
Q_OBJECT
public:
    explicit ResliceHandler(QObject *parent = nullptr);

public slots:

    void onSliceMove();

    void onSliceRotate();
};
