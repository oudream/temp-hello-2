#include "threed_page.h"
#include <QHBoxLayout>

ThreeDPage::ThreeDPage(QWidget *parent) : QWidget(parent),
                                          m_four(new FourViewWidget(this))
{
    auto *h = new QHBoxLayout(this);
    h->setContentsMargins(4,4,4,4);
    h->addWidget(m_four, 1);
}
