#pragma once

#include "u_ribbon_handler.h"

class WindowingHandler : public IURibbonHandler
{
Q_OBJECT
public:
    explicit WindowingHandler(QObject *parent = nullptr);

public slots:

    void onLayoutGrid();

    void onLayout1x3();

    void onLayoutEqual();

    void onPanelAssets();

    void onPanelRender();

    void onPanelCamera();

    void onPanelKnife();

    void onPanelMeasure();

    void onPanelLog();
};
