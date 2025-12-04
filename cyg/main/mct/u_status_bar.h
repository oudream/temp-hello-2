#ifndef VIEWS_STATUS_BAR_H
#define VIEWS_STATUS_BAR_H

#include <QStatusBar>

class UStatusBar : public QStatusBar
{
Q_OBJECT
public:
    explicit UStatusBar(QWidget *parent = 0) : QStatusBar(parent)
    { setSizeGripEnabled(true); }

public slots:

    /// 设置提示文本（UI线程）
    void setMessage(const QString &text)
    { showMessage(text, 3000); }
};

#endif // VIEWS_STATUS_BAR_H
