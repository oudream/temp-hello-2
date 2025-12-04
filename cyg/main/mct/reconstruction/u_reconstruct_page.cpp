#include "u_reconstruct_page.h"


UReconstructPage::UReconstructPage(QWidget *parent) : UPageBase(parent), _root(nullptr)
{
    buildUi(parent);
}

UReconstructPage::~UReconstructPage()
{}

const char *UReconstructPage::getName() const
{ return "reconstruct"; }

QWidget *UReconstructPage::getWidget()
{ return _root; }

void UReconstructPage::onEnter()
{}

void UReconstructPage::onLeave()
{}

void UReconstructPage::buildUi(QWidget *parent)
{
    _root = new QWidget(parent);
    QVBoxLayout *v = new QVBoxLayout(_root);

    QHBoxLayout *h = new QHBoxLayout();
    _cmbAlgo = new QComboBox(_root);
    _cmbAlgo->addItems(QStringList() << "FDK" << "ART" << "SART" << "SIRT" << "迭代");
    h->addWidget(_cmbAlgo);
    _btnRun = new QPushButton(tr("执行"));
    _btnPause = new QPushButton(tr("暂停"));
    _btnResume = new QPushButton(tr("继续"));
    h->addWidget(_btnRun);
    h->addWidget(_btnPause);
    h->addWidget(_btnResume);
    h->addStretch();
    v->addLayout(h);

    _progress = new QProgressBar(_root);
    v->addWidget(_progress);

    _resultList = new QTableWidget(0, 2, _root);
    _resultList->setHorizontalHeaderLabels(QStringList() << tr("时间") << tr("结果集"));
    _resultList->horizontalHeader()->setStretchLastSection(true);
    _resultList->verticalHeader()->setVisible(false);
    v->addWidget(_resultList);

    connect(_cmbAlgo, SIGNAL(currentIndexChanged(int)), this, SLOT(onAlgoChanged(int)));
    connect(_btnRun, SIGNAL(clicked()), this, SLOT(onRun()));
    connect(_btnPause, SIGNAL(clicked()), this, SLOT(onPause()));
    connect(_btnResume, SIGNAL(clicked()), this, SLOT(onResume()));
}

void UReconstructPage::onAlgoChanged(int)
{
    QVariantMap a;
    a["algo"] = _cmbAlgo->currentText();
    emit requestCommand("recon.set_algo", a);
}

void UReconstructPage::onRun()
{ QVariantMap a; emit requestCommand("recon.run", a); }

void UReconstructPage::onPause()
{ QVariantMap a; emit requestCommand("recon.pause", a); }

void UReconstructPage::onResume()
{ QVariantMap a; emit requestCommand("recon.resume", a); }
