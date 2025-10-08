#ifndef UI_U_THREAD_PAGE_H
#define UI_U_THREAD_PAGE_H

#include "u_page_base.h"
#include <QWidget>
#include <QPointer>

class QTableWidget;

class QPushButton;

/**
 * @brief ThreadPage - 线程/任务页（按清单保留）
 * 作用：展示后台工作任务（示例壳），便于后续迁移到更完善的任务中心
 */
class UThreadPage : public UPageBase
{
Q_OBJECT
public:
    explicit UThreadPage(QObject *parent = 0);

    virtual ~UThreadPage();

    virtual const char *getName() const;

    virtual QWidget *getWidget();

    virtual void onEnter();

    virtual void onLeave();

private slots:

    void onRefresh();

private:
    QWidget *_root;
    QPointer<QTableWidget> _tbl;
    QPointer<QPushButton> _btnRefresh;

    void buildUi();
};

#endif // UI_U_THREAD_PAGE_H
