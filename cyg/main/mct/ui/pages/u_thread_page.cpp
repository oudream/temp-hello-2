#include "u_thread_page.h"
#include <QVBoxLayout>
#include <QTableWidget>
#include <QHeaderView>
#include <QPushButton>
#include <QVariantMap>

UThreadPage::UThreadPage(QObject *parent) : UPageBase(parent), _root(0)
{ buildUi(); }

UThreadPage::~UThreadPage()
{}

const char *UThreadPage::getName() const
{ return "thread"; }

QWidget *UThreadPage::getWidget()
{ return _root; }

void UThreadPage::onEnter()
{}

void UThreadPage::onLeave()
{}

void UThreadPage::buildUi()
{
    _root = new QWidget();
    QVBoxLayout *v = new QVBoxLayout(_root);
    _tbl = new QTableWidget(0, 4, _root);
    _tbl->setHorizontalHeaderLabels(QStringList() << tr("任务") << tr("状态") << tr("进度") << tr("耗时"));
    _tbl->horizontalHeader()->setStretchLastSection(true);
    _tbl->verticalHeader()->setVisible(false);
    v->addWidget(_tbl);

    _btnRefresh = new QPushButton(tr("刷新"));
    v->addWidget(_btnRefresh);
    connect(_btnRefresh, SIGNAL(clicked()), this, SLOT(onRefresh()));
}

void UThreadPage::onRefresh()
{
    QVariantMap a; emit requestCommand("thread.refresh", a);
}
