#pragma once
#include "u_ribbon_handler.h"

class SegmentationHandler : public IURibbonHandler {
    Q_OBJECT
public:
    explicit SegmentationHandler(QObject* parent=nullptr);

public slots:
    void onSegThreshold();
    void onSegRegionGrow();
    void onSegAI();
};
