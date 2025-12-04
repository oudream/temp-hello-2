#include "segmentation_handler.h"
#include <QDebug>

SegmentationHandler::SegmentationHandler(QObject* p): IURibbonHandler(p) {}

void SegmentationHandler::onSegThreshold(){ qDebug() << "[Segmentation]" << "onSegThreshold..."; }
void SegmentationHandler::onSegRegionGrow(){ qDebug() << "[Segmentation]" << "onSegRegionGrow..."; }
void SegmentationHandler::onSegAI(){ qDebug() << "[Segmentation]" << "onSegAI..."; }
