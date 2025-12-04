#ifndef UI_DIALOGS_U_DIALOG_CONFIRM_H
#define UI_DIALOGS_U_DIALOG_CONFIRM_H

#include <QDialog>

class QLabel;

class QPushButton;

class UDialogConfirm : public QDialog
{
Q_OBJECT
public:
    explicit UDialogConfirm(const QString &title,
                            const QString &message,
                            QWidget *parent = nullptr);

    bool result() const
    { return _result; }

private:
    QLabel *_lbl = nullptr;
    QPushButton *_btnYes = nullptr;
    QPushButton *_btnNo = nullptr;
    bool _result = false;
};

#endif
