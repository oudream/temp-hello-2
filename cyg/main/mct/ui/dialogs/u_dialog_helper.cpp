#include "u_dialog_helper.h"
#include "u_dialog_open.h"
#include "u_dialog_file.h"
#include "u_dialog_form.h"
#include "u_dialog_confirm.h"

#include <QMessageBox>
#include <QInputDialog>

bool DialogHelper::confirm(QWidget *parent, const QString &title, const QString &text)
{
    UDialogConfirm dlg(title, text, parent);
    return dlg.exec() == QDialog::Accepted && dlg.result();
}

QString DialogHelper::openDirectory(QWidget *parent, const QString &title)
{
    UDialogOpen dlg(parent);
    dlg.setWindowTitle(title);
    if (dlg.exec() == QDialog::Accepted) return dlg.selectedPath();
    return QString();
}

QString DialogHelper::openFile(QWidget *parent, const QString &title, const QString &filter)
{
    UDialogFile dlg(parent);
    dlg.setWindowTitle(title);
    if (dlg.exec() == QDialog::Accepted) return dlg.selectedFile();
    return QString();
}

QMap<QString, QString> DialogHelper::inputParams(QWidget *parent, const QString &title,
                                                 const QMap<QString, QString> &defaults)
{
    UDialogForm dlg(parent);
    dlg.setWindowTitle(title);
    for (auto it = defaults.begin(); it != defaults.end(); ++it)
    {
        dlg.addField(it.key(), it.key(), it.value());
    }
    if (dlg.exec() == QDialog::Accepted) return dlg.values();
    return {};
}

QString DialogHelper::inputText(QWidget *parent, const QString &title,
                                const QString &label, const QString &defaultValue)
{
    bool ok = false;
    QString text = QInputDialog::getText(parent, title, label,
                                         QLineEdit::Normal, defaultValue, &ok);
    return ok ? text : QString();
}

void DialogHelper::info(QWidget *parent, const QString &title, const QString &text)
{
    QMessageBox::information(parent, title, text);
}

void DialogHelper::warning(QWidget *parent, const QString &title, const QString &text)
{
    QMessageBox::warning(parent, title, text);
}

void DialogHelper::error(QWidget *parent, const QString &title, const QString &text)
{
    QMessageBox::critical(parent, title, text);
}
