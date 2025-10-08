#ifndef UI_DIALOGS_U_DIALOG_OPEN_H
#define UI_DIALOGS_U_DIALOG_OPEN_H

#include <QDialog>

class QLineEdit;

class QPushButton;

class UDialogOpen : public QDialog
{
Q_OBJECT
public:
    explicit UDialogOpen(QWidget *parent = 0);

    QString selectedPath() const;

private:
    QLineEdit *_edit = nullptr;
};

#endif
