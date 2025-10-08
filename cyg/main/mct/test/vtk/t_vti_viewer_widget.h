// t_vti_viewer_widget.h
#ifndef CX_CT_X2_T_VTI_VIEWER_WIDGET_H
#define CX_CT_X2_T_VTI_VIEWER_WIDGET_H

/*
 * 本文件声明一个可嵌入 QWidget 的 VTI 影像查看控件：
 * - 左侧 3D 体绘制：体素 volume rendering（颜色+不透明度传输函数）
 * - 右侧 2D 切片：可切换 X/Y/Z 三向并拖动层号
 *
 * 设计要点：
 * 1) UI 与 VTK 解耦：Qt 负责布局/交互，VTK 负责渲染管线。
 * 2) 3D 与 2D 各自维护独立的 Renderer/RenderWindow。
 * 3) 方向/层号与数据范围（Extent/Dimensions）一致性检查，避免越界。
 * 4) setImageData() 是“重建管线”的唯一入口，变更数据时统一刷新两条管线。
 */

#include <QWidget>
#include <QPointer>
#include <QComboBox>
#include <QSlider>
#include <QLabel>
#include <QToolBar>
#include <QVTKOpenGLNativeWidget.h> // Qt 与 VTK 的桥接控件（OpenGL 原生上下文）

// VTK 智能指针与数据/渲染相关头
#include <vtkSmartPointer.h>
#include <vtkImageData.h>
#include <vtkRenderer.h>
#include <vtkGenericOpenGLRenderWindow.h>
// #include <vtkGPUVolumeRayCastMapper.h> // 可切 GPU 体绘制
#include <vtkSmartVolumeMapper.h>         // 智能选择 CPU/GPU 的体映射器
#include <vtkVolumeProperty.h>
#include <vtkColorTransferFunction.h>
#include <vtkPiecewiseFunction.h>
#include <vtkVolume.h>
#include <vtkImageSliceMapper.h>
#include <vtkImageSlice.h>
#include <vtkXMLImageDataReader.h>        // .vti 读取

// ==== 等值面所需 ====
#include <vtkContourFilter.h>
#include <vtkPolyDataMapper.h>
#include <vtkActor.h>
#include <vtkProperty.h>

class TVtiViewerWidget : public QWidget
{
Q_OBJECT
public:
    explicit TVtiViewerWidget(QWidget *parent = nullptr);

    ~TVtiViewerWidget() override = default;

    // ---------------- 对外 API（最小可用接口） ----------------

    // 1) 载入 .vti 文件（内部完成 reader->Update()、setImageData()）
    bool loadVtiFile(const QString &filename);

    // 2) 设置体数据（外部已有 vtkImageData 时走此入口）
    //    统一刷新 3D 体绘制与 2D 切片，且根据数据范围重置 UI 状态。
    void setImageData(vtkImageData *img);

    // 3) 设置切片方向：0=X(Sagittal)、1=Y(Coronal)、2=Z(Axial)
    //    该值直接对应 vtkImageSliceMapper::SetOrientation() 的 I/J/K。
    void setSliceOrientation(int orientation);

    // 4) 设置切片层号：与当前方向匹配（范围由数据 Extent 计算）
    void setSliceIndex(int slice);

    // 5) 暴露底层渲染窗口，便于外部做相机/交互等高级定制
    vtkRenderWindow *renderWindow3D() const;

    vtkRenderWindow *renderWindow2D() const;

signals:

    // 成功加载 .vti 后发出（便于外部联动，如统计/直方图/病历等）
    void vtiLoaded(QString file, vtkImageData *image);

    // 切片变化事件：orientation ∈ {0,1,2}，slice 为层号
    void sliceChanged(int orientation, int slice);

private slots:

    // --- 内部槽：UI 事件到渲染状态的桥接 ---
    void onOpenClicked();

    void onOriChanged(int idx);

    void onSliceChanged(int v);

    // ==== 新增：模式/等值面 ====
    void onModeChanged(int idx);

    void onIsoChanged(int v);

private:
    // --- 内部装配：职责单一，便于定位问题 ---
    void buildUi();              // 组装工具条 + 两个 QVTK 小部件 + 布局
    void build3DPipeline();      // 搭建 3D 体绘制管线（渲染器/映射器/体对象/属性）
    void build2DPipeline();      // 搭建 2D 切片管线（SliceMapper + ImageSlice）
    void setupVolumeRendering(); // 将 _imageData 送入体绘制链、配置传输函数并显示
    void setup2DSlice();         // 将 _imageData 送入切片链、添加到渲染器并显示
    void updateSliceRange();     // 根据当前方向刷新滑条范围（防止越界）
    void refresh2DCamera();      // 2D 视口相机重置（让切片铺满并保持像素感）

    // ==== 新增：根据模式刷新左侧 3D 视图 ====
    enum class ViewMode
    {
        Composite = 0, MIP = 1, MinIP = 2, AIP = 3, Isosurface = 4
    };

    void applyViewMode();        // 根据 _viewMode 切换 3D 内容
    void ensureSurfacePipeline(); // 首次使用等值面时搭建表面管线
    void updateIsoBySlider();     // 用滑条值刷新等值面阈值
    void updateIsoSliderRange();  // 用数据范围设置等值面滑条

private:
    // --- Qt UI 元件 ---
    QPointer<QToolBar> _toolbar;
    QAction *_actOpen = nullptr;
    QPointer<QComboBox> _cmbOri;  // Axial/Coronal/Sagittal
    QPointer<QSlider> _sld;     // 层号滑条
    QPointer<QLabel> _lbl;     // 层号显示

    // ==== 新增：显示模式 + 等值面滑条 ====
    QPointer<QComboBox> _cmbMode;        // Composite/MIP/MinIP/AIP/Isosurface
    QPointer<QLabel> _lblMode;
    QPointer<QSlider> _sldIso;         // 等值面阈值滑条
    QPointer<QLabel> _lblIso;         // 显示当前 iso 值
    ViewMode _viewMode = ViewMode::Composite;

    // 两个 VTK 视图
    QPointer<QVTKOpenGLNativeWidget> _vtk3D;   // 左：体绘制
    QPointer<QVTKOpenGLNativeWidget> _vtk2D;   // 右：切片

    // --- VTK 数据与管线对象 ---
    vtkSmartPointer<vtkImageData> _image;   // 规则体素网格

    // 3D 体绘制链
    vtkSmartPointer<vtkGenericOpenGLRenderWindow> _win3D;
    vtkSmartPointer<vtkRenderer> _ren3D;
//  vtkSmartPointer<vtkGPUVolumeRayCastMapper>    _volMapper; // 若切 GPU 可改用此行
    vtkSmartPointer<vtkSmartVolumeMapper> _volMapper; // 智能/CPU/GPU 体映射器
    vtkSmartPointer<vtkColorTransferFunction> _ctf;       // 颜色传输（标量->RGB）
    vtkSmartPointer<vtkPiecewiseFunction> _pwf;       // 不透明度传输（标量->α）
    vtkSmartPointer<vtkVolumeProperty> _volProp;   // 体外观（插值/光照/传输函数）
    vtkSmartPointer<vtkVolume> _volume;    // 体对象（类似 3D Actor）

    // ==== 新增：3D（等值面表面管线，按需创建）
    vtkSmartPointer<vtkContourFilter> _contour;
    vtkSmartPointer<vtkPolyDataMapper> _surfMapper;
    vtkSmartPointer<vtkActor> _surfActor;

    // 2D 切片链
    vtkSmartPointer<vtkGenericOpenGLRenderWindow> _win2D;
    vtkSmartPointer<vtkRenderer> _ren2D;
    vtkSmartPointer<vtkImageSliceMapper> _sliceMapper; // 负责按 I/J/K+层号取切片
    vtkSmartPointer<vtkImageSlice> _sliceActor;  // 2D “演员”呈现切片

    int _orientation = 2; // 当前切片方向：0=X,1=Y,2=Z（默认 Axial）

};

#endif //CX_CT_X2_T_VTI_VIEWER_WIDGET_H
