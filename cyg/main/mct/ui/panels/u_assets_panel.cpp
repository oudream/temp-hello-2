#include "u_assets_panel.h"
#include <QVBoxLayout>
#include <QListWidget>
#include <QPushButton>
#include <QVariantMap>

UAssetsPanel::UAssetsPanel(QWidget *parent)
        : UPanelBase(tr("资产"), parent), _root(0)
{
    buildUi();
}

UAssetsPanel::~UAssetsPanel()
{}

const char *UAssetsPanel::getName() const
{ return "assets"; }

void UAssetsPanel::buildUi()
{
    _root = new QWidget(this);
    setWidget(_root);
    QVBoxLayout *v = new QVBoxLayout(_root);
    _listFav = new QListWidget(_root);
    v->addWidget(_listFav);
    _btnImport = new QPushButton(tr("导入..."), _root);
    _btnOpenDir = new QPushButton(tr("打开目录"), _root);
    v->addWidget(_btnImport);
    v->addWidget(_btnOpenDir);
    v->addStretch();

    connect(_btnImport, SIGNAL(clicked()), this, SLOT(onImport()));
    connect(_btnOpenDir, SIGNAL(clicked()), this, SLOT(onOpenDir()));
}

void UAssetsPanel::updateContext(MObject * /*activeObject*/)
{
    // 可按上下文刷新收藏/最近
}

void UAssetsPanel::onImport()
{
    QVariantMap a; emit requestCommand("assets.import", a);
}

void UAssetsPanel::onOpenDir()
{
    QVariantMap a; emit requestCommand("assets.open_dir", a);
}
