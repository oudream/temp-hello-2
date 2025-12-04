#include "windowing_handler.h"
#include <QDebug>

WindowingHandler::WindowingHandler(QObject *p) : IURibbonHandler(p)
{}

void WindowingHandler::onLayoutGrid()
{ qDebug() << "[Windowing]" << "onLayoutGrid..."; }

void WindowingHandler::onLayout1x3()
{ qDebug() << "[Windowing]" << "onLayout1x3..."; }

void WindowingHandler::onLayoutEqual()
{ qDebug() << "[Windowing]" << "onLayoutEqual..."; }

void WindowingHandler::onPanelAssets()
{ qDebug() << "[Windowing]" << "onPanelAssets..."; }

void WindowingHandler::onPanelRender()
{ qDebug() << "[Windowing]" << "onPanelRender..."; }

void WindowingHandler::onPanelCamera()
{ qDebug() << "[Windowing]" << "onPanelCamera..."; }

void WindowingHandler::onPanelKnife()
{ qDebug() << "[Windowing]" << "onPanelKnife..."; }

void WindowingHandler::onPanelMeasure()
{ qDebug() << "[Windowing]" << "onPanelMeasure..."; }

void WindowingHandler::onPanelLog()
{ qDebug() << "[Windowing]" << "onPanelLog..."; }
