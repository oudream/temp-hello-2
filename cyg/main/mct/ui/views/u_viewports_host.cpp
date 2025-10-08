#include "u_viewports_host.h"
#include "u_viewport_axial.h"
#include "u_viewport_coronal.h"
#include "u_viewport_sagittal.h"
#include "u_viewport_3d.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QToolButton>
#include <QScrollBar>
#include <QLabel>
#include <QDebug>

UViewportsHost::UViewportsHost(QWidget *parent) : QWidget(parent)
{
    // 初始化四个视图
    _vAxial = new UViewportAxial(this);
    _vCoronal = new UViewportCoronal(this);
    _vSagittal = new UViewportSagittal(this);
    _v3D = new UViewport3D(this);

    // 创建持久 cell
    _cellAxial = makeViewportCell(_vAxial, tr("Axial"));
    _cellCoronal = makeViewportCell(_vCoronal, tr("Coronal"));
    _cellSagittal = makeViewportCell(_vSagittal, tr("Sagittal"));
    _cell3D = makeViewportCell(_v3D, tr("3D"));

    // 初始布局
    rebuild();
}

UViewportsHost::~UViewportsHost()
{}

QWidget *UViewportsHost::makeViewportCell(QWidget *view, const QString &title)
{
    QWidget *root = new QWidget(this);
    auto *grid = new QGridLayout(root);
    grid->setContentsMargins(0, 0, 0, 0);
    grid->setSpacing(0);

    // 主体视图
    grid->addWidget(view, 0, 0);

    // 右侧按钮栏
    auto *right = new QWidget(root);
    auto *rv = new QVBoxLayout(right);
    rv->setContentsMargins(2, 2, 2, 2);
    rv->setSpacing(2);

    auto *btnPlus = new QToolButton(right);
    btnPlus->setText("+");
    auto *btnMinus = new QToolButton(right);
    btnMinus->setText("-");
    auto *btnFit = new QToolButton(right);
    btnFit->setText("适应");

    rv->addWidget(btnPlus);
    rv->addWidget(btnMinus);
    rv->addWidget(btnFit);
    rv->addStretch();
    grid->addWidget(right, 0, 1);

    // TODO: 连接信号到 view 的缩放/适配槽函数
    // connect(btnPlus, &QToolButton::clicked, view, &UViewportBase::zoomIn);

    return root;
}

void UViewportsHost::setLayoutMode(const QString &mode)
{
    if (mode == _layoutMode) return;
    _layoutMode = mode;
    rebuild();
}

void UViewportsHost::equalize()
{
    if (!_grid) return;
    int rows = _grid->rowCount(), cols = _grid->columnCount();
    for (int r = 0; r < rows; ++r) _grid->setRowStretch(r, 1);
    for (int c = 0; c < cols; ++c) _grid->setColumnStretch(c, 1);
}

void UViewportsHost::resizeEvent(QResizeEvent *e)
{
    QWidget::resizeEvent(e);
    if (_layoutMode == "equalize") equalize();
}

void UViewportsHost::rebuild()
{
    if (_grid)
    {
        delete _grid;
        _grid = nullptr;
    }
    _grid = new QGridLayout(this);
    _grid->setContentsMargins(0, 0, 0, 0);
    _grid->setSpacing(2);

    if (_layoutMode == "grid_2x2")
    {
        _grid->addWidget(_cellAxial, 0, 0);
        _grid->addWidget(_cellCoronal, 0, 1);
        _grid->addWidget(_cellSagittal, 1, 0);
        _grid->addWidget(_cell3D, 1, 1);
    }
    else if (_layoutMode == "layout_1x3")
    {
        _grid->addWidget(_cell3D, 0, 0, 2, 2); // 大窗
        _grid->addWidget(_cellAxial, 0, 2);
        _grid->addWidget(_cellCoronal, 1, 2);
        _grid->addWidget(_cellSagittal, 2, 2);
    }
    else if (_layoutMode == "equalize")
    {
        _grid->addWidget(_cellAxial, 0, 0);
        _grid->addWidget(_cellCoronal, 0, 1);
        _grid->addWidget(_cellSagittal, 1, 0);
        _grid->addWidget(_cell3D, 1, 1);
        equalize();
    }
}
