#include "imagecanvas.h"
#include "utils_cvqt.h"

#include <QPainter>
#include <QMouseEvent>
#include <QFileInfo>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>


ImageCanvas::ImageCanvas(QWidget *parent)
        : QWidget(parent)
{
    setObjectName("imageCanvas"); // 方便 MainWindow::findChild
    setMouseTracking(true);
    setAttribute(Qt::WA_OpaquePaintEvent, true);

    m_scale = 1.0;
}

bool ImageCanvas::loadImage(const QString &filePath)
{
    auto m_imgBgr1 = cv::imread(filePath.toStdString(), cv::IMREAD_COLOR);
    // 使用 OpenCV 读取（保持 BGR）
    m_imgBgr = m_imgBgr1;
    if (m_imgBgr.empty()) return false;

    // 转 QImage 以便在 Qt 中显示
    m_imgQt = cvMatToQImage(m_imgBgr);

    m_shapes.clear();
    updateFit();
    update();
    return true;
}

bool ImageCanvas::saveComposited(const QString &filePath) const
{
    if (m_imgBgr.empty()) return false;

    cv::Mat composed = composeToMat();
    if (composed.empty()) return false;

    // 用 OpenCV 写文件
    std::vector<int> params;
    // 可根据后缀选择编码参数（示例略）
    return cv::imwrite(filePath.toStdString(), composed);
}

void ImageCanvas::clearShapes()
{
    m_shapes.clear();
    update();
}

void ImageCanvas::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    QPainter p(this);
    p.fillRect(rect(), Qt::black);

    if (!m_imgQt.isNull()) {
        // 绘制图像（已等比适配）
        p.drawImage(m_destRect, m_imgQt);

        // 绘制历史形状
        p.setRenderHint(QPainter::Antialiasing, true);
        for (const auto &s : m_shapes) {
            QPen pen(s.color, s.thickness);
            p.setPen(pen);
            if (s.type == Shape::Rect) {
                // 窗口坐标绘制
                QRect rWin = QRect(imageToWindow(s.rect.topLeft()).toPoint(),
                                   imageToWindow(s.rect.bottomRight()).toPoint()).normalized();
                p.drawRect(rWin);
            } else if (s.type == Shape::Circle) {
                QPoint cWin = imageToWindow(s.center).toPoint();
                int rWin = int(s.radius * m_scale);
                p.drawEllipse(cWin, rWin, rWin);
            }
        }

        // 绘制进行中的形状（橡皮筋）
        if (m_isDrawing && m_tool != Tool::Select) {
            QPen pen(Qt::yellow, 2, Qt::DashLine);
            p.setPen(pen);
            if (m_tool == Tool::Rect) {
                QRect r = QRect(m_startWin, m_currWin).normalized();
                p.drawRect(r);
            } else if (m_tool == Tool::Circle) {
                // 半径 = 起点到当前点的距离（窗口坐标）
                int dx = m_currWin.x() - m_startWin.x();
                int dy = m_currWin.y() - m_startWin.y();
                int r = int(std::sqrt(double(dx*dx + dy*dy)));
                p.drawEllipse(m_startWin, r, r);
            }
        }
    }
}

void ImageCanvas::mousePressEvent(QMouseEvent *event)
{
    if (event->button() != Qt::LeftButton) return;
    if (m_imgQt.isNull()) return;

    if (m_tool == Tool::Rect || m_tool == Tool::Circle) {
        if (!m_destRect.contains(event->pos())) return; // 仅在图像区域内开始
        m_isDrawing = true;
        m_startWin = event->pos();
        m_currWin  = m_startWin;
        update();
    }
}

void ImageCanvas::mouseMoveEvent(QMouseEvent *event)
{
    if (!m_isDrawing) return;
    m_currWin = event->pos();
    update();
}

void ImageCanvas::mouseReleaseEvent(QMouseEvent *event)
{
    if (!m_isDrawing || event->button()!=Qt::LeftButton) return;

    if (m_tool == Tool::Rect) {
        QRect rWin = QRect(m_startWin, m_currWin).normalized();
        // 转到图像坐标
        QPoint tlImg = windowToImage(rWin.topLeft()).toPoint();
        QPoint brImg = windowToImage(rWin.bottomRight()).toPoint();
        QRect  rImg  = QRect(tlImg, brImg).normalized();
        if (rImg.width()>1 && rImg.height()>1) {
            Shape s; s.type = Shape::Rect; s.rect = rImg; s.color = Qt::red; s.thickness = 2;
            m_shapes.push_back(s);
        }
    } else if (m_tool == Tool::Circle) {
        int dx = m_currWin.x() - m_startWin.x();
        int dy = m_currWin.y() - m_startWin.y();
        int rWin = int(std::sqrt(double(dx*dx + dy*dy)));
        if (rWin > 1) {
            // 圆心/半径转换到图像坐标
            QPoint cImg = windowToImage(m_startWin).toPoint();
            int rImg = int(rWin / m_scale);
            Shape s; s.type = Shape::Circle; s.center = cImg; s.radius = rImg; s.color = Qt::green; s.thickness = 2;
            m_shapes.push_back(s);
        }
    }

    m_isDrawing = false;
    update();
}

void ImageCanvas::resizeEvent(QResizeEvent *event)
{
    Q_UNUSED(event);
    updateFit();
}

QPointF ImageCanvas::windowToImage(const QPointF &p) const
{
    // 先减去偏移，再除以缩放
    QPointF pt = p - m_destRect.topLeft();
    return QPointF(pt.x() / m_scale, pt.y() / m_scale);
}

QPointF ImageCanvas::imageToWindow(const QPointF &p) const
{
    QPointF pt(p.x() * m_scale, p.y() * m_scale);
    return pt + m_destRect.topLeft();
}

void ImageCanvas::updateFit()
{
    if (m_imgQt.isNull()) { m_destRect = QRect(); m_scale = 1.0; return; }

    const int vw = width();
    const int vh = height();
    const int iw = m_imgQt.width();
    const int ih = m_imgQt.height();

    if (iw == 0 || ih == 0) { m_destRect = QRect(); m_scale = 1.0; return; }

    double sx = double(vw) / iw;
    double sy = double(vh) / ih;
    m_scale = std::min(sx, sy);

    int dw = int(iw * m_scale);
    int dh = int(ih * m_scale);
    int dx = (vw - dw) / 2;
    int dy = (vh - dh) / 2;
    m_destRect = QRect(dx, dy, dw, dh);
}

cv::Mat ImageCanvas::composeToMat() const
{
    if (m_imgQt.isNull()) return {};

    // 以原始图像尺寸创建画布（避免缩放带来的精度损失）
    QImage canvas = cvMatToQImage(m_imgBgr); // 原始尺寸

    QPainter p(&canvas);
    p.setRenderHint(QPainter::Antialiasing, true);

    // 在原始图像坐标上绘制保存用的形状
    for (const auto &s : m_shapes) {
        QPen pen(s.color, s.thickness);
        p.setPen(pen);
        if (s.type == Shape::Rect) {
            p.drawRect(s.rect);
        } else if (s.type == Shape::Circle) {
            p.drawEllipse(s.center, s.radius, s.radius);
        }
    }
    p.end();

    // 转回 cv::Mat（BGR）以便用 OpenCV 保存
    return qImageToCvMat(canvas);
}