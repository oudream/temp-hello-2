#include "u_panel_camera.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QDoubleSpinBox>
#include <QPushButton>
#include <QVariantMap>

UPanelCamera::UPanelCamera(QWidget *parent)
        : UPanelBase(tr("相机"), parent), _root(nullptr)
{
    buildUi();
}

UPanelCamera::~UPanelCamera()
{}

const char *UPanelCamera::getName() const
{ return "camera"; }

void UPanelCamera::buildUi()
{
    _root = new QWidget(this);
    setWidget(_root);
    QVBoxLayout *v = new QVBoxLayout(_root);

    QHBoxLayout *h1 = new QHBoxLayout();
    _spDolly = new QDoubleSpinBox(_root);
    _spDolly->setRange(-1000.0, 1000.0);
    _spDolly->setValue(50.0);
    _btnDolly = new QPushButton(tr("Dolly"), _root);
    h1->addWidget(_spDolly);
    h1->addWidget(_btnDolly);
    v->addLayout(h1);

    QHBoxLayout *h2 = new QHBoxLayout();
    _spAz = new QDoubleSpinBox(_root);
    _spAz->setRange(-180.0, 180.0);
    _spAz->setValue(15.0);
    _spEl = new QDoubleSpinBox(_root);
    _spEl->setRange(-180.0, 180.0);
    _spEl->setValue(10.0);
    _btnOrbit = new QPushButton(tr("Orbit"), _root);
    h2->addWidget(_spAz);
    h2->addWidget(_spEl);
    h2->addWidget(_btnOrbit);
    v->addLayout(h2);

    _btnReset = new QPushButton(tr("重置相机"), _root);
    v->addWidget(_btnReset);
    v->addStretch();

    connect(_btnDolly, SIGNAL(clicked()), this, SLOT(onDolly()));
    connect(_btnOrbit, SIGNAL(clicked()), this, SLOT(onOrbit()));
    connect(_btnReset, SIGNAL(clicked()), this, SLOT(onReset()));
}

void UPanelCamera::updateContext(MObject * /*activeObject*/)
{
    // 可根据 active camera 刷新参数显示
}

void UPanelCamera::onDolly()
{
    QVariantMap a;
    a["delta"] = _spDolly->value();
    emit requestCommand("camera.dolly", a);
}

void UPanelCamera::onOrbit()
{
    QVariantMap a;
    a["azimuth"] = _spAz->value();
    a["elevation"] = _spEl->value();
    emit requestCommand("camera.orbit", a);
}

void UPanelCamera::onReset()
{
    QVariantMap a; emit requestCommand("camera.reset", a);
}
