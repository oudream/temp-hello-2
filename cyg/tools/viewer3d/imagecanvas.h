#ifndef CX_CT_X2_IMAGECANVAS_H
#define CX_CT_X2_IMAGECANVAS_H


#include <QWidget>
#include <QImage>
#include <QVector>
#include <QPoint>
#include <QRect>
#include <QColor>
#include <opencv2/core.hpp>

// ImageCanvas: 负责显示图像 + 鼠标交互画矩形/圆
// - 内部保存原始图像 (cv::Mat)
// - 绘制覆盖在 Qt 的 paintEvent 中完成（不直接写入原图，保存时再合成）

class ImageCanvas : public QWidget
{
Q_OBJECT
public:
    enum class Tool { Select, Rect, Circle };

    explicit ImageCanvas(QWidget *parent = nullptr);

    // 加载/保存
    bool loadImage(const QString &filePath); // 使用 OpenCV imread
    bool saveComposited(const QString &filePath) const; // 原图 + 叠加形状 一并保存（OpenCV imwrite）

    void setTool(Tool t) { m_tool = t; }
    Tool tool() const { return m_tool; }

    void clearShapes();

    // 便于 .ui 里通过 objectName 查找
    QSize sizeHint() const override { return QSize(960, 720); }

protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;

private:
    // 形状数据结构
    struct Shape {
        enum Type { Rect, Circle } type;
        QRect rect;          // 对于 Rect
        QPoint center;       // 对于 Circle
        int radius = 0;      // 对于 Circle
        QColor color = Qt::red;
        int thickness = 2;   // 绘制线宽（显示与保存都使用）
    };

    // 坐标换算：窗口像素 <-> 图像像素（保持等比适配到窗口）
    QPointF windowToImage(const QPointF &p) const;
    QPointF imageToWindow(const QPointF &p) const;
    void   updateFit();

    // 合成：把当前 QImage 图像 + 形状绘制在一个 QImage 上并转换为 cv::Mat
    cv::Mat composeToMat() const;

private:
    cv::Mat m_imgBgr;       // 原始图像（BGR，OpenCV 风格）
    QImage  m_imgQt;        // 与 m_imgBgr 同步的显示图（RGB888 或 GRAY）

    QVector<Shape> m_shapes;

    Tool m_tool = Tool::Select;
    bool m_isDrawing = false;
    QPoint m_startWin;      // 鼠标起点（窗口坐标）
    QPoint m_currWin;       // 鼠标当前（窗口坐标）

    // 适配相关
    QRect  m_destRect;      // 图像在窗口中的显示区域（等比缩放后的位置）
    double m_scale = 1.0;   // 图像像素 -> 窗口像素 的缩放系数
};

#endif //CX_CT_X2_IMAGECANVAS_H
