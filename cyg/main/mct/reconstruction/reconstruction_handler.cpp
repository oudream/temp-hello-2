#include "reconstruction_handler.h"
#include <QDebug>

ReconstructionHandler::ReconstructionHandler(QObject *p) : IURibbonHandler(p)
{}

void ReconstructionHandler::onScanParams()
{ qDebug() << "[Reconstruction]" << "onScanParams..."; }

void ReconstructionHandler::onGeometryCalib()
{ qDebug() << "[Reconstruction]" << "onGeometryCalib..."; }

void ReconstructionHandler::onAlgoSelect()
{ qDebug() << "[Reconstruction]" << "onAlgoSelect..."; }

void ReconstructionHandler::onRun()
{ qDebug() << "[Reconstruction]" << "onRun..."; }

void ReconstructionHandler::onPause()
{ qDebug() << "[Reconstruction]" << "onPause..."; }

void ReconstructionHandler::onResume()
{ qDebug() << "[Reconstruction]" << "onResume..."; }

void ReconstructionHandler::onShowProgress()
{ qDebug() << "[Reconstruction]" << "onShowProgress..."; }

void ReconstructionHandler::onShowResults()
{ qDebug() << "[Reconstruction]" << "onShowResults..."; }
