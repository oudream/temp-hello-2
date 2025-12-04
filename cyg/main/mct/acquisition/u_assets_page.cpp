#include "u_assets_page.h"

#include "base/app_ids.h"


UAssetsPage::UAssetsPage(QWidget *parent) : UPageBase(parent), _root(nullptr)
{
    buildUi(parent);
}

UAssetsPage::~UAssetsPage()
{

}

const char *UAssetsPage::getName() const
{
    return AppIds::kBlock_Acquisition;
}

QWidget *UAssetsPage::getWidget()
{
    return _root;
}

void UAssetsPage::onEnter()
{

}

void UAssetsPage::onLeave()
{

}

void UAssetsPage::buildUi(QWidget *parent)
{
    _root = new QWidget(parent);
    QVBoxLayout *v = new QVBoxLayout(_root);

    QSplitter *sp = new QSplitter(Qt::Horizontal, _root);
    _tree = new QTreeView(sp);
    _table = new QTableView(sp);
    _table->horizontalHeader()->setStretchLastSection(true);
    sp->setStretchFactor(0, 1);
    sp->setStretchFactor(1, 2);
    v->addWidget(sp);

    _lblMeta = new QLabel(tr("元数据："));
    v->addWidget(_lblMeta);

    QHBoxLayout *h = new QHBoxLayout();
    _btnImport = new QPushButton(tr("导入"));
    _btnBatch = new QPushButton(tr("批处理"));
    _btnPre = new QPushButton(tr("预处理（去条纹/归一化）"));
    h->addWidget(_btnImport);
    h->addWidget(_btnBatch);
    h->addWidget(_btnPre);
    h->addStretch();
    v->addLayout(h);

    connect(_btnImport, SIGNAL(clicked()), this, SLOT(onImport()));
    connect(_btnBatch, SIGNAL(clicked()), this, SLOT(onBatchProcess()));
    connect(_btnPre, SIGNAL(clicked()), this, SLOT(onPreprocess()));
}

void UAssetsPage::onImport()
{
    QVariantMap a; emit requestCommand("assets.import", a);
}

void UAssetsPage::onBatchProcess()
{
    QVariantMap a; emit requestCommand("assets.batch", a);
}

void UAssetsPage::onPreprocess()
{
    QVariantMap a; emit requestCommand("assets.preprocess", a);
}
