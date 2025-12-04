#pragma once

#include "u_ribbon_handler.h"

class ReconstructionHandler : public IURibbonHandler
{
Q_OBJECT
public:
    explicit ReconstructionHandler(QObject *parent = nullptr);

public slots:

    void onScanParams();

    void onGeometryCalib();

    void onAlgoSelect();

    void onRun();

    void onPause();

    void onResume();

    void onShowProgress();

    void onShowResults();

};
