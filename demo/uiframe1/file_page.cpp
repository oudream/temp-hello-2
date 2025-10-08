#include "file_page.h"
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QToolButton>
#include <QLabel>
#include <QListWidget>
#include <QStringList>

static QToolButton* bigBtn(const QString &text){
    QToolButton *b = new QToolButton;
    b->setText(text);
    b->setToolButtonStyle(Qt::ToolButtonTextOnly);
    b->setFixedSize(120, 48);
    return b;
}

FilePage::FilePage(QWidget *parent) : QWidget(parent) {
    auto *h = new QHBoxLayout(this);
    h->setContentsMargins(6,6,6,6);
    h->setSpacing(8);
    h->addWidget(buildLeftButtons());
    h->addWidget(buildRightArea(), 1);
}

QWidget* FilePage::buildLeftButtons(){
    QWidget *left = new QWidget;
    auto *v = new QVBoxLayout(left);
    v->setContentsMargins(0,0,0,0);
    v->setSpacing(6);
    QStringList items = { tr("打开"), tr("新建"), tr("最近"), tr("导入"), tr("导出"), tr("设置"), tr("退出") };
    for (const auto &s : items) v->addWidget(bigBtn(s));
    v->addStretch(1);
    left->setFixedWidth(140);
    return left;
}

QWidget* FilePage::buildRightArea(){
    QWidget *right = new QWidget;
    auto *v = new QVBoxLayout(right);
    v->setContentsMargins(6,6,6,6);
    v->setSpacing(6);

    QLabel *title = new QLabel(tr("最近项目"));
    title->setObjectName("sectionTitle");
    v->addWidget(title);

    QListWidget *list = new QListWidget;
    for (int i=0;i<12;i++) list->addItem(QString("demo_project_%1.vgl").arg(100000+i));
    v->addWidget(list, 1);

    return right;
}
