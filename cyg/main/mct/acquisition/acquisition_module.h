#pragma once

#include "u_ribbon_module.h"

class AcquisitionModule : public IURibbonModule
{
public:
    URibbonBlock createModule(QObject *parent, QMap<QString, QAction *> &actions, IURibbonHandler *handler) const override;

    IURibbonHandler *createHandler(QObject *parent) const override;

};
