#include "colored_svg_icon.h"

#include <QPainter>
#include <QSvgRenderer>
#include <QGuiApplication>
#include <QScreen>
#include <QApplication>

ColoredSvgIconEngine::ColoredSvgIconEngine(QByteArray svgData)
        : svg_(std::move(svgData)) {}

QIconEngine *ColoredSvgIconEngine::clone() const {
    auto *e = new ColoredSvgIconEngine(svg_);
    e->defaultColor_ = defaultColor_;
    e->colors_ = colors_;
    return e;
}

void ColoredSvgIconEngine::setDefaultColor(const QColor &c) { defaultColor_ = c; }

void ColoredSvgIconEngine::setColor(QIcon::Mode mode, QIcon::State state, const QColor &c) {
    colors_.insert(keyMS(mode, state), c);
}

void ColoredSvgIconEngine::paint(QPainter *p, const QRect &r, QIcon::Mode m, QIcon::State s) {
    const QColor c = colors_.value(keyMS(m, s), defaultColor_);
    const QPixmap pm = render(r.size(), c);
    p->drawPixmap(r.topLeft(), pm);
}

QPixmap ColoredSvgIconEngine::pixmap(const QSize &size, QIcon::Mode m, QIcon::State s) {
    const QColor c = colors_.value(keyMS(m, s), defaultColor_);
    return render(size, c);
}

QPixmap ColoredSvgIconEngine::render(const QSize &size, const QColor &c) const {
    QByteArray replaced = svg_;
    replaced.replace("currentColor", c.name(QColor::HexRgb).toUtf8());

    QSvgRenderer renderer(replaced);
    const qreal dpr = qApp ? qApp->devicePixelRatio() : 1.0;

    QPixmap pm(size * dpr);
    pm.fill(Qt::transparent);
    pm.setDevicePixelRatio(dpr);

    QPainter pt(&pm);
    pt.setRenderHint(QPainter::Antialiasing, true);
    pt.setRenderHint(QPainter::SmoothPixmapTransform, true);

    // 关键：把 SVG 渲染进目标矩形（按按钮/图标尺寸缩放）
    const QRectF target(QPointF(0,0), QSizeF(size));
    renderer.render(&pt, target);
    return pm;
}

quint64 ColoredSvgIconEngine::keyMS(QIcon::Mode m, QIcon::State s)
{
    return (static_cast<quint64>(m) << 8) | static_cast<quint64>(s);
}
