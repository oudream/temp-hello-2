#pragma once

#include "u_ribbon_handler.h"

class InspectionHandler : public IURibbonHandler
{
Q_OBJECT
public:
    explicit InspectionHandler(QObject *parent = nullptr);
};
