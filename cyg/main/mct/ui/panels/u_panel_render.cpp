#include "u_panel_render.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QComboBox>
#include <QSlider>
#include <QDoubleSpinBox>
#include <QVariantMap>
#include <QLabel>

UPanelRender::UPanelRender(QWidget *parent)
        : UPanelBase(tr("渲染"), parent), _root(0)
{
    buildUi();
}

UPanelRender::~UPanelRender()
{}

const char *UPanelRender::getName() const
{ return "render"; }

void UPanelRender::buildUi()
{
    _root = new QWidget(this);
    setWidget(_root);
    QVBoxLayout *v = new QVBoxLayout(_root);

    _cmbMode = new QComboBox(_root);
    _cmbMode->addItems(QStringList() << tr("体渲染") << "MIP" << tr("表面"));
    v->addWidget(new QLabel(tr("模式：")));
    v->addWidget(_cmbMode);

    _slSampling = new QSlider(Qt::Horizontal, _root);
    _slSampling->setRange(1, 100);
    _slSampling->setValue(50);
    v->addWidget(new QLabel(tr("采样率：")));
    v->addWidget(_slSampling);

    _cmbLut = new QComboBox(_root);
    _cmbLut->addItems(QStringList() << "Gray" << "Hot" << "Cool" << "Jet");
    v->addWidget(new QLabel(tr("LUT：")));
    v->addWidget(_cmbLut);

    QHBoxLayout *h1 = new QHBoxLayout();
    _spWL = new QDoubleSpinBox(_root);
    _spWL->setRange(0.0, 65535.0);
    _spWL->setValue(2000.0);
    _spWW = new QDoubleSpinBox(_root);
    _spWW->setRange(-32768.0, 32767.0);
    _spWW->setValue(500.0);
    h1->addWidget(new QLabel(tr("窗宽")));
    h1->addWidget(_spWL);
    h1->addWidget(new QLabel(tr("窗位")));
    h1->addWidget(_spWW);
    v->addLayout(h1);

    v->addStretch();

    connect(_cmbMode, SIGNAL(currentIndexChanged(int)), this, SLOT(onModeChanged(int)));
    connect(_slSampling, SIGNAL(valueChanged(int)), this, SLOT(onSamplingChanged(int)));
    connect(_cmbLut, SIGNAL(currentIndexChanged(int)), this, SLOT(onLutChanged(int)));
    connect(_spWL, SIGNAL(editingFinished()), this, SLOT(onWindowChanged()));
    connect(_spWW, SIGNAL(editingFinished()), this, SLOT(onLevelChanged()));
}

void UPanelRender::updateContext(MObject * /*activeObject*/)
{
    // 可按当前 volume 属性刷新显示
}

void UPanelRender::onModeChanged(int)
{
    QVariantMap a;
    a["mode"] = _cmbMode->currentText();
    emit requestCommand("render.set_mode", a);
}

void UPanelRender::onSamplingChanged(int v)
{
    QVariantMap a;
    a["sampling"] = v;
    emit requestCommand("render.set_sampling", a);
}

void UPanelRender::onLutChanged(int)
{
    QVariantMap a;
    a["lut"] = _cmbLut->currentText();
    emit requestCommand("render.set_lut", a);
}

void UPanelRender::onWindowChanged()
{
    QVariantMap a;
    a["window"] = _spWL->value();
    emit requestCommand("render.set_window", a);
}

void UPanelRender::onLevelChanged()
{
    QVariantMap a;
    a["level"] = _spWW->value();
    emit requestCommand("render.set_level", a);
}
