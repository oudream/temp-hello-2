#include "u_dialog_form.h"
#include <QFormLayout>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>
#include <QDialogButtonBox>

UDialogForm::UDialogForm(QWidget *parent) : QDialog(parent)
{
    auto *v = new QVBoxLayout(this);
    _form = new QFormLayout;
    v->addLayout(_form);

    auto *buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    connect(buttons, &QDialogButtonBox::accepted, this, &UDialogForm::accept);
    connect(buttons, &QDialogButtonBox::rejected, this, &UDialogForm::reject);
    v->addWidget(buttons);
}

void UDialogForm::addField(const QString &key, const QString &label, const QString &defaultValue)
{
    auto *edit = new QLineEdit(defaultValue, this);
    _form->addRow(label, edit);
    _fields[key] = edit;
}

QMap<QString, QString> UDialogForm::values() const
{
    QMap<QString, QString> result;
    for (auto it = _fields.begin(); it != _fields.end(); ++it)
    {
        result[it.key()] = it.value()->text();
    }
    return result;
}
