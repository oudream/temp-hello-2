#pragma once

#include "u_ribbon_handler.h"

class RenderingHandler : public IURibbonHandler
{
Q_OBJECT
public:
    explicit RenderingHandler(QObject *parent = nullptr);

public slots:

    void onRenderMode();

    void onRenderLut();

    void onRenderWindowLevel();
};
