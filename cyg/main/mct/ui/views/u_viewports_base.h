#ifndef UI_VIEWS_U_VIEWPORTS_BASE_H
#define UI_VIEWS_U_VIEWPORTS_BASE_H

#include <QWidget>

/**
 * @brief UViewportBase - 单个视口基类（此处先做占位UI；后续可挂 VTK）
 * 作用：对齐统一接口；提供 zoomIn/zoomOut/fitToView 占位实现
 */
class UViewportBase : public QWidget
{
Q_OBJECT
public:
    explicit UViewportBase(QWidget *parent = 0) : QWidget(parent)
    { setAutoFillBackground(true); }

public slots:

    virtual void zoomIn()
    {}

    virtual void zoomOut()
    {}

    virtual void fitToView()
    {}
};

#endif // UI_VIEWS_U_VIEWPORTS_BASE_H
