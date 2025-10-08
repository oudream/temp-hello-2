#include "u_panel_log.h"
#include <QVBoxLayout>
#include <QTextEdit>
#include <QComboBox>
#include <QPushButton>
#include <QVariantMap>

UPanelLog::UPanelLog(QWidget *parent)
        : UPanelBase(tr("日志"), parent), _root(0)
{
    buildUi();
}

UPanelLog::~UPanelLog()
{}

const char *UPanelLog::getName() const
{ return "log"; }

void UPanelLog::buildUi()
{
    _root = new QWidget(this);
    setWidget(_root);
    QVBoxLayout *v = new QVBoxLayout(_root);

    _cmbLevel = new QComboBox(_root);
    _cmbLevel->addItems(QStringList() << "TRACE" << "DEBUG" << "INFO" << "WARN" << "ERROR");
    v->addWidget(_cmbLevel);

    _txt = new QTextEdit(_root);
    _txt->setReadOnly(true);
    v->addWidget(_txt);

    _btnExport = new QPushButton(tr("导出"), _root);
    v->addWidget(_btnExport);
    v->addStretch();

    connect(_cmbLevel, SIGNAL(currentIndexChanged(int)), this, SLOT(onLevelChanged(int)));
    connect(_btnExport, SIGNAL(clicked()), this, SLOT(onExport()));
}

void UPanelLog::updateContext(MObject * /*activeObject*/)
{}

void UPanelLog::onLevelChanged(int)
{
    QVariantMap a;
    a["level"] = _cmbLevel->currentText();
    emit requestCommand("log.set_level", a);
}

void UPanelLog::onExport()
{
    QVariantMap a; emit requestCommand("log.export", a);
}

void UPanelLog::appendLog(const QString &line)
{
    _txt->append(line);
}
