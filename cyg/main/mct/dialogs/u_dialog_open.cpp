#include "u_dialog_open.h"
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLineEdit>
#include <QPushButton>
#include <QFileDialog>

UDialogOpen::UDialogOpen(QWidget *parent) : QDialog(parent)
{
    auto *v = new QVBoxLayout(this);
    _edit = new QLineEdit(this);
    auto *h = new QHBoxLayout;
    auto *btn = new QPushButton(tr("浏览..."), this);
    h->addWidget(_edit);
    h->addWidget(btn);
    v->addLayout(h);
    auto *ok = new QPushButton(tr("确定"), this);
    v->addWidget(ok);
    connect(btn, &QPushButton::clicked, [this]
    {
        auto path = QFileDialog::getExistingDirectory(this, tr("选择目录"));
        if (!path.isEmpty()) _edit->setText(path);
    });
    connect(ok, &QPushButton::clicked, [this]
    { accept(); });
}

QString UDialogOpen::selectedPath() const
{ return _edit->text(); }
