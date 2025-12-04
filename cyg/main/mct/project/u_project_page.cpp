#include "u_project_page.h"

#include "base/app_ids.h"


UProjectPage::UProjectPage(QWidget *parent) : UPageBase(parent), _root(nullptr)
{
    buildUi(parent);
}

UProjectPage::~UProjectPage()
{

}

const char *UProjectPage::getName() const
{
    return AppIds::kBlock_Project;
}

QWidget *UProjectPage::getWidget()
{
    return _root;
}

void UProjectPage::onEnter()
{
    fillRecent();
}

void UProjectPage::onLeave()
{

}

void UProjectPage::buildUi(QWidget *parent)
{
    _root = new QWidget();
    QVBoxLayout *v = new QVBoxLayout(_root);

    _lblPath = new QLabel(tr("当前工程：<无>"));
    v->addWidget(_lblPath);

    _tbl = new QTableWidget(0, 3, _root);
    _tbl->setHorizontalHeaderLabels(QStringList() << tr("时间") << tr("名称") << tr("路径"));
    _tbl->horizontalHeader()->setStretchLastSection(true);
    _tbl->verticalHeader()->setVisible(false);
    _tbl->setSelectionBehavior(QAbstractItemView::SelectRows);
    _tbl->setEditTriggers(QAbstractItemView::NoEditTriggers);
    v->addWidget(_tbl);

    QHBoxLayout *h = new QHBoxLayout();
    _btnNew = new QPushButton(tr("新建"));
    _btnOpen = new QPushButton(tr("打开"));
    _btnSave = new QPushButton(tr("保存"));
    _btnExport = new QPushButton(tr("导出配置"));
    _btnImport = new QPushButton(tr("导入配置"));
    _btnExit = new QPushButton(tr("退出"));
    h->addWidget(_btnNew);
    h->addWidget(_btnOpen);
    h->addWidget(_btnSave);
    h->addStretch();
    h->addWidget(_btnExport);
    h->addWidget(_btnImport);
    h->addWidget(_btnExit);
    v->addLayout(h);

    connect(_btnNew, SIGNAL(clicked()), this, SLOT(onNewProject()));
    connect(_btnOpen, SIGNAL(clicked()), this, SLOT(onOpenProject()));
    connect(_btnSave, SIGNAL(clicked()), this, SLOT(onSaveProject()));
    connect(_btnExport, SIGNAL(clicked()), this, SLOT(onExportConfig()));
    connect(_btnImport, SIGNAL(clicked()), this, SLOT(onImportConfig()));
    connect(_btnExit, SIGNAL(clicked()), this, SLOT(onExitApp()));
}

void UProjectPage::fillRecent()
{
    // 仅示例：实际应通过适配层拉取最近工程列表（不在UI层做IO） 参见“解耦/非功能性要求”
    // 清空
    _tbl->setRowCount(0);
}

void UProjectPage::onNewProject()
{
    QVariantMap a; emit requestCommand("project.new", a);
}

void UProjectPage::onOpenProject()
{
    QVariantMap a; emit requestCommand("project.open", a);
}

void UProjectPage::onSaveProject()
{
    QVariantMap a; emit requestCommand("project.save", a);
}

void UProjectPage::onExportConfig()
{
    QVariantMap a; emit requestCommand("project.export_config", a);
}

void UProjectPage::onImportConfig()
{
    QVariantMap a; emit requestCommand("project.import_config", a);
}

void UProjectPage::onExitApp()
{
    // 统一通过上行命令，由主窗口决定是否关闭（主窗会弹确认）
    QVariantMap a; emit requestCommand("app.exit", a);
}