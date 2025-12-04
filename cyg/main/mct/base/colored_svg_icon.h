#ifndef CX_CT_X2_COLORED_SVG_ICON_H
#define CX_CT_X2_COLORED_SVG_ICON_H


#include <QIconEngine>
#include <QColor>
#include <QByteArray>
#include <QHash>

class ColoredSvgIconEngine : public QIconEngine {
public:
    explicit ColoredSvgIconEngine(QByteArray svgData);

    QIconEngine *clone() const override;

    // 配置颜色：优先匹配 (mode,state)，其次用 defaultColor_
    void setDefaultColor(const QColor &c);

    void setColor(QIcon::Mode mode, QIcon::State state, const QColor &c);

    // QIconEngine 接口
    void paint(QPainter *p, const QRect &r, QIcon::Mode mode, QIcon::State state) override;

    QPixmap pixmap(const QSize &size, QIcon::Mode mode, QIcon::State state) override;

private:
    QPixmap render(const QSize &size, const QColor &c) const;

    static quint64 keyMS(QIcon::Mode m, QIcon::State s);

private:
    QByteArray svg_;                 // 原始 SVG 文本
    QColor defaultColor_ = Qt::white;
    QHash<quint64, QColor> colors_; // (mode,state) -> color

};



#endif //CX_CT_X2_COLORED_SVG_ICON_H
