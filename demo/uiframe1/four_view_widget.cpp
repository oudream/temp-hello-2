#include "four_view_widget.h"
#include <QGridLayout>
#include <QHBoxLayout>
#include <QVBoxLayout>

FourViewWidget::FourViewWidget(QWidget *parent) : QWidget(parent),
                                                  m_mode(FourViewLayout::Grid2x2),
                                                  m_axial(new ViewFrame("Axial", this)),
                                                  m_sagittal(new ViewFrame("Sagittal", this)),
                                                  m_coronal(new ViewFrame("Coronal", this)),
                                                  m_view3D(new ViewFrame("3D", this))
{
    rebuild();
}

void FourViewWidget::setLayoutMode(FourViewLayout mode){
    if (m_mode == mode) return;
    m_mode = mode;
    rebuild();
}

void FourViewWidget::rebuild(){
    if (m_currentLayout) {
        QLayoutItem *child;
        while((child = m_currentLayout->takeAt(0))){
            if (child->widget()) child->widget()->setParent(nullptr);
            delete child;
        }
        delete m_currentLayout;
        m_currentLayout = nullptr;
    }

    if (m_mode == FourViewLayout::Grid2x2) {
        QGridLayout *g = new QGridLayout;
        g->setContentsMargins(2,2,2,2);
        g->setSpacing(2);
        // 2x2: 左上Axial 右上Sagittal 左下3D 右下Coronal
        g->addWidget(m_axial,     0, 0);
        g->addWidget(m_sagittal,  0, 1);
        g->addWidget(m_view3D,    1, 0);
        g->addWidget(m_coronal,   1, 1);
        setLayout(g);
        m_currentLayout = g;
    } else {
        // 1&3: 左边3D(7)，右边三正交(3)
        QHBoxLayout *h = new QHBoxLayout;
        h->setContentsMargins(2,2,2,2);
        h->setSpacing(2);

        h->addWidget(m_view3D, 7);

        QVBoxLayout *v = new QVBoxLayout;
        v->setSpacing(2);
        v->addWidget(m_axial);
        v->addWidget(m_sagittal);
        v->addWidget(m_coronal);

        QWidget *rightCol = new QWidget;
        rightCol->setLayout(v);
        h->addWidget(rightCol, 3);

        setLayout(h);
        m_currentLayout = h;
    }
}
