#include "reslice_handler.h"
#include <QDebug>

ResliceHandler::ResliceHandler(QObject *p) : IURibbonHandler(p)
{}

void ResliceHandler::onSliceMove()
{ qDebug() << "[Reslice]" << "onSliceMove..."; }

void ResliceHandler::onSliceRotate()
{ qDebug() << "[Reslice]" << "onSliceRotate..."; }
