#include "registration_handler.h"
#include <QDebug>

RegistrationHandler::RegistrationHandler(QObject *p) : IURibbonHandler(p)
{}

void RegistrationHandler::onRegRigid()
{ qDebug() << "[Registration]" << "onRegRigid..."; }

void RegistrationHandler::onRegNonRigid()
{ qDebug() << "[Registration]" << "onRegNonRigid..."; }

void RegistrationHandler::onRegICP()
{ qDebug() << "[Registration]" << "onRegICP..."; }
