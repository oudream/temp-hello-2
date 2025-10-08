#ifndef UI_U_PANEL_LOG_H
#define UI_U_PANEL_LOG_H

#include "u_panel_base.h"
#include <QPointer>

class QTextEdit;

class QComboBox;

class QPushButton;

/**
 * @brief PanelLog - 日志面板
 * 作用：显示系统/任务日志，支持级别过滤与导出
 */
class UPanelLog : public UPanelBase
{
Q_OBJECT
public:
    explicit UPanelLog(QWidget *parent = 0);

    virtual ~UPanelLog();

    virtual const char *getName() const;

public slots:

    virtual void updateContext(MObject *activeObject);

private slots:

    void onLevelChanged(int);

    void onExport();

public slots:

    /// 供上层把日志文本推送到面板
    void appendLog(const QString &line);

private:
    QWidget *_root;
    QPointer<QTextEdit> _txt;
    QPointer<QComboBox> _cmbLevel;
    QPointer<QPushButton> _btnExport;

    void buildUi();
};

#endif // UI_U_PANEL_LOG_H
