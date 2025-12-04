// u_dialog_progress.h
#ifndef UI_DIALOGS_U_DIALOG_PROGRESS_H
#define UI_DIALOGS_U_DIALOG_PROGRESS_H

#include <QDialog>

class QProgressBar;

class QTextEdit;

class UDialogProgress : public QDialog
{
Q_OBJECT
public:
    explicit UDialogProgress(QWidget *parent = nullptr);

    void setProgress(int value);

    void appendLog(const QString &line);

private:
    QProgressBar *_bar = nullptr;
    QTextEdit *_log = nullptr;
};

#endif
