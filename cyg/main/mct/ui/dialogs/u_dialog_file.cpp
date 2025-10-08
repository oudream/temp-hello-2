#include "u_dialog_file.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QPushButton>
#include <QFileDialog>

UDialogFile::UDialogFile(QWidget *parent) : QDialog(parent)
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
        auto path = QFileDialog::getOpenFileName(this, tr("选择文件"), QString(), tr("所有文件 (*.*)"));
        if (!path.isEmpty()) _edit->setText(path);
    });
    connect(ok, &QPushButton::clicked, this, &UDialogFile::accept);
}

QString UDialogFile::selectedFile() const
{ return _edit->text(); }
