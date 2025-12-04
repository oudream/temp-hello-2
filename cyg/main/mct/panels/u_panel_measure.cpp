#include "u_panel_measure.h"
#include <QVBoxLayout>
#include <QListWidget>
#include <QPushButton>
#include <QVariantMap>

UPanelMeasure::UPanelMeasure(QWidget *parent)
        : UPanelBase(tr("测量"), parent), _root(nullptr)
{
    buildUi();
}

UPanelMeasure::~UPanelMeasure()
{}

const char *UPanelMeasure::getName() const
{ return "measure"; }

void UPanelMeasure::buildUi()
{
    _root = new QWidget(this);
    setWidget(_root);
    QVBoxLayout *v = new QVBoxLayout(_root);
    _list = new QListWidget(_root);
    v->addWidget(_list);

    _btnNewLine = new QPushButton(tr("新增直线"), _root);
    _btnNewAngle = new QPushButton(tr("新增角度"), _root);
    _btnDelete = new QPushButton(tr("删除所选"), _root);
    v->addWidget(_btnNewLine);
    v->addWidget(_btnNewAngle);
    v->addWidget(_btnDelete);
    v->addStretch();

    connect(_btnNewLine, SIGNAL(clicked()), this, SLOT(onNewLine()));
    connect(_btnNewAngle, SIGNAL(clicked()), this, SLOT(onNewAngle()));
    connect(_btnDelete, SIGNAL(clicked()), this, SLOT(onDelete()));
}

void UPanelMeasure::updateContext(MObject * /*activeObject*/)
{
    // 可刷新列表数据
}

void UPanelMeasure::onNewLine()
{
    QVariantMap a; emit requestCommand("measure.create_line", a);
}

void UPanelMeasure::onNewAngle()
{
    QVariantMap a; emit requestCommand("measure.create_angle", a);
}

void UPanelMeasure::onDelete()
{
    QVariantMap a; emit requestCommand("measure.delete_selected", a);
}
