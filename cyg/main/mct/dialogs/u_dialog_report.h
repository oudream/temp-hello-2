#ifndef UI_DIALOGS_U_DIALOG_REPORT_H
#define UI_DIALOGS_U_DIALOG_REPORT_H

#include <QDialog>

class QTextEdit;

class QPushButton;

class UDialogReport : public QDialog
{
Q_OBJECT
public:
    explicit UDialogReport(QWidget *parent = 0);

    QString text() const;

private:
    QTextEdit *_txt = nullptr;
};

#endif
