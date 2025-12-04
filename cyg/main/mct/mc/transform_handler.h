#pragma once

#include "u_ribbon_handler.h"

class TransformHandler : public IURibbonHandler
{
Q_OBJECT
public:
    explicit TransformHandler(QObject *parent = nullptr);
};
