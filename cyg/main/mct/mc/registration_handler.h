#pragma once

#include "u_ribbon_handler.h"

class RegistrationHandler : public IURibbonHandler
{
Q_OBJECT
public:
    explicit RegistrationHandler(QObject *parent = nullptr);

public slots:

    void onRegRigid();

    void onRegNonRigid();

    void onRegICP();

};
