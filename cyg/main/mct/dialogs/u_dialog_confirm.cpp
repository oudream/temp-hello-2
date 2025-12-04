#include "u_dialog_confirm.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>

UDialogConfirm::UDialogConfirm(const QString &title, const QString &message, QWidget *parent)
        : QDialog(parent)
{
    setWindowTitle(title);

    auto *v = new QVBoxLayout(this);
    _lbl = new QLabel(message, this);
    v->addWidget(_lbl);

    auto *h = new QHBoxLayout;
    _btnYes = new QPushButton(tr("是"), this);
    _btnNo = new QPushButton(tr("否"), this);
    h->addStretch();
    h->addWidget(_btnYes);
    h->addWidget(_btnNo);
    v->addLayout(h);

    connect(_btnYes, &QPushButton::clicked, [this]
    {
        _result = true;
        accept();
    });
    connect(_btnNo, &QPushButton::clicked, [this]
    {
        _result = false;
        reject();
    });
}
