#ifndef UI_DIALOGS_DIALOG_HELPER_H
#define UI_DIALOGS_DIALOG_HELPER_H

#include <QString>
#include <QMap>

class QWidget;

class DialogHelper
{
public:
    // 确认/询问
    static bool confirm(QWidget *parent, const QString &title, const QString &text);

    // 打开目录
    static QString openDirectory(QWidget *parent, const QString &title);

    // 打开文件
    static QString openFile(QWidget *parent, const QString &title,
                            const QString &filter = "所有文件 (*.*)");

    // 输入参数（传默认值，返回用户修改后的）
    static QMap<QString, QString> inputParams(QWidget *parent, const QString &title,
                                              const QMap<QString, QString> &defaults);

    // 输入单行文本
    static QString inputText(QWidget *parent, const QString &title,
                             const QString &label, const QString &defaultValue = QString());

    // 简单消息（信息提示）
    static void info(QWidget *parent, const QString &title, const QString &text);

    // 警告消息
    static void warning(QWidget *parent, const QString &title, const QString &text);

    // 错误消息
    static void error(QWidget *parent, const QString &title, const QString &text);

};

#endif
