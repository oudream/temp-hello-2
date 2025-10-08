#ifndef CX_CT_X2_VIEW_FRAME_H
#define CX_CT_X2_VIEW_FRAME_H


#include <QWidget>
#include <QGraphicsView>
#include <QGraphicsScene>
#include <QScrollBar>
#include <QToolButton>
#include <QLabel>
#include <QPainter>     // 需要显式包含
#include "IView.h"

class CTGraphicsView : public QGraphicsView {
Q_OBJECT
public:
    explicit CTGraphicsView(QWidget *parent=nullptr) : QGraphicsView(parent) {
        setScene(new QGraphicsScene(this));
        setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
        setDragMode(QGraphicsView::ScrollHandDrag);
        setBackgroundBrush(QColor(45,45,48));
        // 放一张棋盘格做占位
        const int s = 20, n = 20;
        QImage img(s*n, s*n, QImage::Format_ARGB32_Premultiplied);
        img.fill(Qt::transparent);
        QPainter p(&img);
        for(int y=0;y<n;y++)
            for(int x=0;x<n;x++)
                p.fillRect(x*s, y*s, s, s, ((x+y)%2==0) ? QColor(70,70,70) : QColor(55,55,55));
        p.end();
        scene()->addPixmap(QPixmap::fromImage(img));
        setSceneRect(0,0,img.width(), img.height());
    }
public slots:
    void zoomIn(){ scale(1.2, 1.2); }
    void zoomOut(){ scale(1/1.2, 1/1.2); }
    void fitToView() { fitInView(sceneRect(), Qt::KeepAspectRatio); }
};

// 一个视图容器：右边（竖向工具+竖滚动条），底部（工具+横滚动条）+ 中央画布
class ViewFrame : public IView {
Q_OBJECT
public:
    explicit ViewFrame(const QString &title, QWidget *parent=nullptr);
    CTGraphicsView* graphicsView() const { return m_view; }
signals:
    void requestFitAll();
private:
    CTGraphicsView *m_view = nullptr;
    QScrollBar *m_hbar = nullptr;
    QScrollBar *m_vbar = nullptr;
    static QToolButton* makeBtn(const QString &text);
};


#endif //CX_CT_X2_VIEW_FRAME_H
