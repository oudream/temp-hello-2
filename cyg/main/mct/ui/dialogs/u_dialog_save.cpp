// u_dialog_save.cpp
#include "u_dialog_save.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QPushButton>
#include <QFileDialog>

UDialogSave::UDialogSave(QWidget *parent) : QDialog(parent)
{
    auto *v = new QVBoxLayout(this);
    _edit = new QLineEdit(this);
    auto *h = new QHBoxLayout;
    auto *btn = new QPushButton(tr("浏览..."), this);
    h->addWidget(_edit);
    h->addWidget(btn);
    v->addLayout(h);
    auto *ok = new QPushButton(tr("保存"), this);
    v->addWidget(ok);

    connect(btn, &QPushButton::clicked, [this]
    {
        auto path = QFileDialog::getSaveFileName(this, tr("另存为"), QString(), tr("All Files (*.*)"));
        if (!path.isEmpty()) _edit->setText(path);
    });
    connect(ok, &QPushButton::clicked, [this]
    { accept(); });
}

QString UDialogSave::selectedFile() const
{ return _edit->text(); }
