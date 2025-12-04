#include "u_dialog_report.h"
#include <QVBoxLayout>
#include <QTextEdit>
#include <QPushButton>

UDialogReport::UDialogReport(QWidget *parent) : QDialog(parent)
{
    auto *v = new QVBoxLayout(this);
    _txt = new QTextEdit(this);
    v->addWidget(_txt);
    auto *ok = new QPushButton(tr("导出"), this);
    v->addWidget(ok);
    connect(ok, &QPushButton::clicked, [this]
    { accept(); });
}

QString UDialogReport::text() const
{ return _txt->toPlainText(); }
