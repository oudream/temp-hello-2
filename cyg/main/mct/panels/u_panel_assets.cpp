#include "u_panel_assets.h"
#include <QVBoxLayout>
#include <QListWidget>
#include <QPushButton>
#include <QVariantMap>

UPanelAssets::UPanelAssets(QWidget *parent)
        : UPanelBase(tr("体积"), parent), _root(nullptr)
{
    buildUi();
}

UPanelAssets::~UPanelAssets()
{

}

const char *UPanelAssets::getName() const
{
    return "assets";
}

void UPanelAssets::buildUi()
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

void UPanelAssets::updateContext(MObject * /*activeObject*/)
{
    // 可按上下文刷新收藏/最近
}

void UPanelAssets::onImport()
{
    QVariantMap a; emit requestCommand("assets.import", a);
}

void UPanelAssets::onOpenDir()
{
    QVariantMap a; emit requestCommand("assets.open_dir", a);
}
