#pragma once

#include "u_ribbon_handler.h"

class AcquisitionHandler : public IURibbonHandler
{
Q_OBJECT
public:
    explicit AcquisitionHandler(QObject *parent = nullptr);

public slots:

    void onImport();

    void onBatchManage();

    void onShowMetadata();

    void onPreprocess();

    void onViewPresets();
};
