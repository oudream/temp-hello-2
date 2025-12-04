#include "measurement_handler.h"
#include <QDebug>

MeasurementHandler::MeasurementHandler(QObject *p) : IURibbonHandler(p)
{}

void MeasurementHandler::onMeasureLine()
{ qDebug() << "[Measurement]" << "onMeasureLine..."; }

void MeasurementHandler::onMeasureAngle()
{ qDebug() << "[Measurement]" << "onMeasureAngle..."; }

void MeasurementHandler::onMeasureCircle()
{ qDebug() << "[Measurement]" << "onMeasureCircle..."; }

void MeasurementHandler::onMeasureArea()
{ qDebug() << "[Measurement]" << "onMeasureArea..."; }
