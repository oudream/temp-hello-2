#include "u_panel_knife.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QDoubleSpinBox>
#include <QComboBox>
#include <QPushButton>
#include <QVariantMap>

UPanelKnife::UPanelKnife(QWidget *parent)
        : UPanelBase(tr("切面"), parent), _root(nullptr)
{
    buildUi();
}

UPanelKnife::~UPanelKnife()
{}

const char *UPanelKnife::getName() const
{ return "knife"; }

void UPanelKnife::buildUi()
{
    _root = new QWidget(this);
    setWidget(_root);
    QVBoxLayout *v = new QVBoxLayout(_root);

    _cmbKnife = new QComboBox(_root);
    _cmbKnife->addItems(QStringList() << "AXIAL" << "SAGITTAL" << "CORONAL");
    v->addWidget(_cmbKnife);

    QHBoxLayout *h1 = new QHBoxLayout();
    _spMove = new QDoubleSpinBox(_root);
    _spMove->setRange(-1000.0, 1000.0);
    _spMove->setValue(1.0);
    _btnMove = new QPushButton(tr("移动(mm)"), _root);
    h1->addWidget(_spMove);
    h1->addWidget(_btnMove);
    v->addLayout(h1);

    QHBoxLayout *h2 = new QHBoxLayout();
    _spRotate = new QDoubleSpinBox(_root);
    _spRotate->setRange(-180.0, 180.0);
    _spRotate->setValue(1.0);
    _btnRotate = new QPushButton(tr("旋转(°)"), _root);
    h2->addWidget(_spRotate);
    h2->addWidget(_btnRotate);
    v->addLayout(h2);

    QHBoxLayout *h3 = new QHBoxLayout();
    _spThick = new QDoubleSpinBox(_root);
    _spThick->setRange(0.0, 50.0);
    _spThick->setValue(1.0);
    _btnThick = new QPushButton(tr("厚度(mm)"), _root);
    h3->addWidget(_spThick);
    h3->addWidget(_btnThick);
    v->addLayout(h3);

    _cmbLock = new QComboBox(_root);
    _cmbLock->addItems(QStringList() << "None" << "X" << "Y" << "Z");
    _btnLock = new QPushButton(tr("锁轴"), _root);
    QHBoxLayout *h4 = new QHBoxLayout();
    h4->addWidget(_cmbLock);
    h4->addWidget(_btnLock);
    v->addLayout(h4);
    v->addStretch();

    connect(_btnMove, SIGNAL(clicked()), this, SLOT(onMove()));
    connect(_btnRotate, SIGNAL(clicked()), this, SLOT(onRotate()));
    connect(_btnThick, SIGNAL(clicked()), this, SLOT(onThickness()));
    connect(_btnLock, SIGNAL(clicked()), this, SLOT(onLockAxis()));
}

void UPanelKnife::updateContext(MObject * /*activeObject*/)
{}

void UPanelKnife::onMove()
{
    QVariantMap a;
    a["knife"] = _cmbKnife->currentText();
    a["delta_mm"] = _spMove->value();
    emit requestCommand("knife.move", a);
}

void UPanelKnife::onRotate()
{
    QVariantMap a;
    a["knife"] = _cmbKnife->currentText();
    a["delta_deg"] = _spRotate->value();
    emit requestCommand("knife.rotate", a);
}

void UPanelKnife::onThickness()
{
    QVariantMap a;
    a["knife"] = _cmbKnife->currentText();
    a["thickness_mm"] = _spThick->value();
    emit requestCommand("knife.set_thickness", a);
}

void UPanelKnife::onLockAxis()
{
    QVariantMap a;
    a["axis"] = _cmbLock->currentText();
    emit requestCommand("knife.set_lock_axis", a);
}
