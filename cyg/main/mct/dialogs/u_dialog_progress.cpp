// u_dialog_progress.cpp
#include "u_dialog_progress.h"
#include <QVBoxLayout>
#include <QProgressBar>
#include <QTextEdit>
#include <QPushButton>

UDialogProgress::UDialogProgress(QWidget *parent) : QDialog(parent)
{
    auto *v = new QVBoxLayout(this);
    _bar = new QProgressBar(this);
    _log = new QTextEdit(this);
    _log->setReadOnly(true);
    auto *btn = new QPushButton(tr("关闭"), this);

    v->addWidget(_bar);
    v->addWidget(_log);
    v->addWidget(btn);

    connect(btn, &QPushButton::clicked, this, &UDialogProgress::accept);
}

void UDialogProgress::setProgress(int value)
{
    _bar->setValue(value);
}

void UDialogProgress::appendLog(const QString &line)
{
    _log->append(line);
}
