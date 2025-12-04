#include "rendering_handler.h"
#include <QDebug>

RenderingHandler::RenderingHandler(QObject *p) : IURibbonHandler(p)
{}

void RenderingHandler::onRenderMode()
{ qDebug() << "[Rendering]" << "onRenderMode..."; }

void RenderingHandler::onRenderLut()
{ qDebug() << "[Rendering]" << "onRenderLut..."; }

void RenderingHandler::onRenderWindowLevel()
{ qDebug() << "[Rendering]" << "onRenderWindowLevel..."; }
