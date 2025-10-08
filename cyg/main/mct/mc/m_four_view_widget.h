#ifndef CX_CT_X2_M_FOUR_VIEW_WIDGET_H
#define CX_CT_X2_M_FOUR_VIEW_WIDGET_H

#include <QWidget>
#include <memory>

class QGridLayout;

class QHBoxLayout;

class QVTKOpenGLNativeWidget;

class QComboBox;

class QCheckBox;

class QSlider;

class QDoubleSpinBox;

class QSpinBox;

class QPushButton;

class QLabel;

class FourViewController; // m_four_view_controller.h
class vtkImageData;

class MFourViewWidget : public QWidget
{
Q_OBJECT
public:
    explicit MFourViewWidget(QWidget *parent = nullptr);

    ~MFourViewWidget();

    // 数据入口（两种任选其一）
    bool loadVtiFile(const QString &file);         // 示例：加载 VTI 文件
    void setImageData(vtkImageData *img);          // 或外部直接塞入影像

    // 便捷：让外部也能切换 3D 模式 / 设置 WL（可选）
    enum class ViewMode
    {
        Composite = 0, MIP = 1, MinIP = 2, AIP = 3, Isosurface = 4
    };

    void setViewMode3D(ViewMode m);

    void setWindowLevel(double win, double lev);

    // 导出 Axial（which=0）
    bool exportAxialRendered(const QString &path, int scale = 1);

    bool exportAxialHighQuality(const QString &path,
                                double spacingXY = -1.0, int sizeX = -1, int sizeY = -1,
                                bool linear = true, bool rescale16 = true);

    // 拿到底层控制器（如需更高级定制）
    FourViewController *controller() const
    { return _controller.get(); }

private:
    void buildUi();     // 创建 2×2 视窗 + 顶部工具条
    void wireUp();      // 创建控制器并与 UI 事件对接

    // UI <-> 控制器 同步辅助
    void applyWlToUi(double w, double l);

    void updateWlFromUi();

private: // 视图区
    QGridLayout *_rootLayout = nullptr;
    QVTKOpenGLNativeWidget *_w3D = nullptr;
    QVTKOpenGLNativeWidget *_wAx = nullptr;
    QVTKOpenGLNativeWidget *_wCo = nullptr;
    QVTKOpenGLNativeWidget *_wSa = nullptr;

private: // 工具条
    QHBoxLayout *_bar = nullptr;
    QPushButton *_btnLoad = nullptr;
    QComboBox *_cmb3DMode = nullptr;
    QLabel *_lblWin = nullptr;
    QSlider *_sldWin = nullptr;
    QDoubleSpinBox *_spnWin = nullptr;
    QLabel *_lblLev = nullptr;
    QSlider *_sldLev = nullptr;
    QDoubleSpinBox *_spnLev = nullptr;
    QPushButton *_btnPresetBone = nullptr;
    QPushButton *_btnPresetSoft = nullptr;
    QPushButton *_btnPresetMetal = nullptr;
    QCheckBox *_chkThick = nullptr;
    QSpinBox *_spnThickPx = nullptr;
    QLabel *_lblScroll = nullptr;
    QDoubleSpinBox *_spnScroll = nullptr;
    QPushButton *_btnExportAxialRendered = nullptr;
    QPushButton *_btnExportAxialHQ = nullptr;

private:
    std::unique_ptr<FourViewController> _controller;

};

#endif // CX_CT_X2_M_FOUR_VIEW_WIDGET_H
