#include "acquisition_handler.h"
#include <QDebug>

AcquisitionHandler::AcquisitionHandler(QObject *p) : IURibbonHandler(p)
{}

void AcquisitionHandler::onImport()
{ qDebug() << "[Acquisition]" << "onImport..."; }

void AcquisitionHandler::onBatchManage()
{ qDebug() << "[Acquisition]" << "onBatchManage..."; }

void AcquisitionHandler::onShowMetadata()
{ qDebug() << "[Acquisition]" << "onShowMetadata..."; }

void AcquisitionHandler::onPreprocess()
{ qDebug() << "[Acquisition]" << "onPreprocess..."; }

void AcquisitionHandler::onViewPresets()
{ qDebug() << "[Acquisition]" << "onViewPresets..."; }
