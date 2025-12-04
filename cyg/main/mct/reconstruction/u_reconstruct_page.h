#ifndef UI_U_RECONSTRUCT_PAGE_H
#define UI_U_RECONSTRUCT_PAGE_H

#include "pages/u_page_base.h"

#include <QWidget>
#include <QPointer>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QComboBox>
#include <QPushButton>
#include <QProgressBar>
#include <QTableWidget>
#include <QHeaderView>
#include <QVariantMap>


/**
 * @brief ReconstructPage - 重建页
 * 作用：控制重建流程（参数、几何、算法、执行/暂停/恢复、进度、结果版本）
 * 职责：UI控件改动发出命令；进度通过事件适配回显（此处不做耗时操作）。
 */
class UReconstructPage : public UPageBase
{
Q_OBJECT
public:
    explicit UReconstructPage(QWidget *parent);

    ~UReconstructPage() override;

    const char *getName() const override;

    QWidget *getWidget() override;

    void onEnter() override;

    void onLeave() override;

private slots:

    void onAlgoChanged(int);

    void onRun();

    void onPause();

    void onResume();

private:
    QWidget *_root;
    QPointer<QComboBox> _cmbAlgo;
    QPointer<QPushButton> _btnRun;
    QPointer<QPushButton> _btnPause;
    QPointer<QPushButton> _btnResume;
    QPointer<QProgressBar> _progress;
    QPointer<QTableWidget> _resultList;

    void buildUi(QWidget *parent);
};

#endif // UI_U_RECONSTRUCT_PAGE_H
