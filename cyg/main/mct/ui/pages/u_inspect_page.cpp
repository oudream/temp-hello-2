#include "u_inspect_page.h"

#include <QVBoxLayout>
#include <QTableWidget>
#include <QHeaderView>
#include <QPushButton>
#include <QVariantMap>

#include <test/vtk/t_vti_viewer_widget.h>
#include <test/vtk/t_vti_mpr3_view_widget.h>
#include <test/vtk/t_mpr_four_view_widget.h>
#include <test/vtk/t_smart_pointer.h>

#include <mc/m_four_view_widget.h>

UInspectPage::UInspectPage(QObject *parent) : UPageBase(parent), _root(0)
{
    buildUi();
}

UInspectPage::~UInspectPage()
{}

const char *UInspectPage::getName() const
{ return "inspect"; }

QWidget *UInspectPage::getWidget()
{ return _root; }

void UInspectPage::onEnter()
{}

void UInspectPage::onLeave()
{}

void UInspectPage::buildUi()
{
//    TSmartPointer::Destruction();
    _root = new QWidget();
    QVBoxLayout *v = new QVBoxLayout(_root);
//    auto *viewer = new TVtiViewerWidget(_root);
//    auto *viewer = new TVtiMpr3ViewWidget(_root);
//    auto *viewer = new TMprFourViewWidget(_root);
    auto *viewer = new MFourViewWidget(_root);

    v->setContentsMargins(0, 0, 0, 0);
    v->addWidget(viewer);

//    connect(_btnRun, SIGNAL(clicked()), this, SLOT(onRunBatch()));
//    connect(_btnReport, SIGNAL(clicked()), this, SLOT(onExportReport()));
}

