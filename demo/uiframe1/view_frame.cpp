#include "view_frame.h"
#include <QVBoxLayout>
#include <QHBoxLayout>

QToolButton* ViewFrame::makeBtn(const QString &text){
    QToolButton *b = new QToolButton;
    b->setText(text);
    b->setToolButtonStyle(Qt::ToolButtonTextOnly);
    b->setFixedHeight(22);
    return b;
}

ViewFrame::ViewFrame(const QString &title, QWidget *parent) : IView(parent) {
    auto *outer = new QVBoxLayout(this);
    outer->setContentsMargins(2,2,2,2);
    outer->setSpacing(2);

    auto *centerRow = new QHBoxLayout;
    centerRow->setContentsMargins(0,0,0,0);
    centerRow->setSpacing(2);

    // 中央画布
    m_view = new CTGraphicsView(this);
    m_view->setObjectName("viewCanvas");
    centerRow->addWidget(m_view, 1);

    // 右边工具栏 + 竖滚动条
    QWidget *rightBar = new QWidget;
    auto *rv = new QVBoxLayout(rightBar);
    rv->setContentsMargins(0,0,0,0);
    rv->setSpacing(2);

    auto *btnZoomInR  = makeBtn("+");
    auto *btnZoomOutR = makeBtn("-");
    auto *btnFitR     = makeBtn("适应");
    rv->addWidget(btnZoomInR);
    rv->addWidget(btnZoomOutR);
    rv->addWidget(btnFitR);
    rv->addSpacing(6);

    m_vbar = new QScrollBar(Qt::Vertical);
    rv->addWidget(m_vbar, 1);

    centerRow->addWidget(rightBar);
    outer->addLayout(centerRow, 1);

    // 底部工具栏 + 横滚动条
    QWidget *bottomBar = new QWidget;
    auto *bh = new QHBoxLayout(bottomBar);
    bh->setContentsMargins(0,0,0,0);
    bh->setSpacing(4);
    auto *btnZoomOut = makeBtn("缩小");
    auto *btnZoomIn  = makeBtn("放大");
    auto *btnFit     = makeBtn("合适");
    bh->addWidget(btnZoomOut);
    bh->addWidget(btnZoomIn);
    bh->addWidget(btnFit);
    bh->addSpacing(4);
    m_hbar = new QScrollBar(Qt::Horizontal);
    bh->addWidget(m_hbar, 1);
    outer->addWidget(bottomBar);

    // 滚动条联动
    connect(m_hbar, &QScrollBar::valueChanged, this, [this](int v){
        m_view->horizontalScrollBar()->setValue(v);
    });
    connect(m_view->horizontalScrollBar(), &QScrollBar::rangeChanged, this, [this](int min,int max){
        m_hbar->setRange(min, max);
    });
    connect(m_view->horizontalScrollBar(), &QScrollBar::valueChanged, m_hbar, &QScrollBar::setValue);

    connect(m_vbar, &QScrollBar::valueChanged, this, [this](int v){
        m_view->verticalScrollBar()->setValue(v);
    });
    connect(m_view->verticalScrollBar(), &QScrollBar::rangeChanged, this, [this](int min,int max){
        m_vbar->setRange(min, max);
    });
    connect(m_view->verticalScrollBar(), &QScrollBar::valueChanged, m_vbar, &QScrollBar::setValue);

    // 按钮
    connect(btnZoomIn,  &QToolButton::clicked, m_view, &CTGraphicsView::zoomIn);
    connect(btnZoomOut, &QToolButton::clicked, m_view, &CTGraphicsView::zoomOut);
    connect(btnFit,     &QToolButton::clicked, m_view, &CTGraphicsView::fitToView);

    connect(btnZoomInR,  &QToolButton::clicked, m_view, &CTGraphicsView::zoomIn);
    connect(btnZoomOutR, &QToolButton::clicked, m_view, &CTGraphicsView::zoomOut);
    connect(btnFitR,     &QToolButton::clicked, m_view, &CTGraphicsView::fitToView);

    // 标题
    QLabel *lbl = new QLabel(title);
    lbl->setObjectName("viewTitle");
    lbl->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
    lbl->setFixedHeight(18);
    outer->insertWidget(0, lbl);
}
