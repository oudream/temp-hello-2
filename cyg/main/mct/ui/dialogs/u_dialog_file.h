#ifndef UI_DIALOGS_U_DIALOG_FILE_H
#define UI_DIALOGS_U_DIALOG_FILE_H

#include <QDialog>

class QLineEdit;

class UDialogFile : public QDialog
{
Q_OBJECT
public:
    explicit UDialogFile(QWidget *parent = nullptr);

    QString selectedFile() const;

private:
    QLineEdit *_edit = nullptr;
};

#endif
