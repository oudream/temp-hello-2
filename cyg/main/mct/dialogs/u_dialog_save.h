// u_dialog_save.h
#ifndef UI_DIALOGS_U_DIALOG_SAVE_H
#define UI_DIALOGS_U_DIALOG_SAVE_H

#include <QDialog>

class QLineEdit;

class UDialogSave : public QDialog
{
Q_OBJECT
public:
    explicit UDialogSave(QWidget *parent = nullptr);

    QString selectedFile() const;

private:
    QLineEdit *_edit = nullptr;
};

#endif
