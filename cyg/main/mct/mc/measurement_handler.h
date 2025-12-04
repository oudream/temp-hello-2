#pragma once

#include "u_ribbon_handler.h"

class MeasurementHandler : public IURibbonHandler
{
Q_OBJECT
public:
    explicit MeasurementHandler(QObject *parent = nullptr);

public slots:

    void onMeasureLine();

    void onMeasureAngle();

    void onMeasureCircle();

    void onMeasureArea();

};
