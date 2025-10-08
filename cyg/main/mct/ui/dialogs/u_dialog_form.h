#ifndef UI_DIALOGS_U_DIALOG_FORM_H
#define UI_DIALOGS_U_DIALOG_FORM_H

#include <QDialog>
#include <QMap>

class QFormLayout;

class QLineEdit;

class UDialogForm : public QDialog
{
Q_OBJECT
public:
    explicit UDialogForm(QWidget *parent = nullptr);

    // 添加一个字段
    void addField(const QString &key, const QString &label, const QString &defaultValue = QString());

    // 获取结果
    QMap<QString, QString> values() const;

private:
    QFormLayout *_form = nullptr;
    QMap<QString, QLineEdit *> _fields;
};

#endif
