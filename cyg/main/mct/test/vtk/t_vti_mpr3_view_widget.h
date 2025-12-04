#ifndef CX_CT_X2_VTI_MPR3_VIEW_WIDGET_H
#define CX_CT_X2_VTI_MPR3_VIEW_WIDGET_H

/*
 * 三视图 MPR 控件（Axial/Coronal/Sagittal），基于 vtkResliceImageViewer：
 * - 三个 QVTKOpenGLNativeWidget + 三个 vtkResliceImageViewer
 * - 共享一个 vtkResliceCursor，实现 crosshair 联动与滚轮切片同步
 * - 支持 AxisAligned / Oblique、厚层（Thick）模式、厚度与滚轮因子
 * - 提供统一设置窗宽窗位的 API
 *
 * 设计要点：
 * 1) UI 与管线解耦：Qt 负责布局与简单交互；VTK 负责重采样与渲染；
 * 2) 三视图共享 ResliceCursor，任一视图互动均广播到其它视图；
 * 3) 采用 Observer 侦听 SliceChanged/InteractionEvent，触发对方渲染；
 * 4) 允许外部接入已有 vtkImageData，或直接 loadVtiFile()。
 */

#include <QWidget>
#include <QPointer>
#include <QToolBar>
#include <QLabel>
#include <QComboBox>
#include <QCheckBox>
#include <QSlider>
#include <QVTKOpenGLNativeWidget.h>

#include <vtkSmartPointer.h>
#include <vtkGenericOpenGLRenderWindow.h>
#include <vtkImageData.h>
#include <vtkRenderer.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkResliceImageViewer.h>
#include <vtkResliceCursor.h>
#include <vtkResliceCursorWidget.h>
#include <vtkCommand.h>
#include <vtkCallbackCommand.h>
#include <vtkScalarsToColors.h>
#include <vtkLookupTable.h>

class TVtiMpr3ViewWidget : public QWidget
{
Q_OBJECT
public:
    explicit TVtiMpr3ViewWidget(QWidget *parent = nullptr);

    ~TVtiMpr3ViewWidget() override = default;

    // 载入 .vti 文件；成功则返回 true，并发出 vtiLoaded 信号
    bool loadVtiFile(const QString &filename);

    // 直接注入体数据；不接管其生命周期
    void setImageData(vtkImageData *img);

    // 统一设置窗宽/窗位（三个视图一致）
    void setWindowLevel(double win, double lev);

    // 打开/关闭厚层（Thick）模式；thickness 为各向同性厚度（像素）
    void setThickMode(bool on, int thickness = 10);

    // 设置滚轮步进因子（默认 1.0，>1 表示加速滚动）
    void setSliceScrollFactor(double f);

    // 获取三个 RenderWindow（外部可做截图/相机等）
    vtkRenderWindow *renderWindowAxial() const;

    vtkRenderWindow *renderWindowCoronal() const;

    vtkRenderWindow *renderWindowSagittal() const;

signals:

    void vtiLoaded(QString file, vtkImageData *image);

private slots:

    // --- UI 桥接 ---
    void onOpenClicked();

    void onThickToggled(int on);

    void onThickChanged(int v);

    void onWLChanged();               // 简易窗宽/窗位同步
    void onScrollFactorChanged(int v);

private:
    // --- 装配 ---
    void buildUi();

    void buildViewers();              // 创建三个 viewer 与窗口/交互器
    void connectObservers();          // 建立 VTK 事件联动（切片变化即联动刷新）
    void refreshAll();                // 三个窗口一起 Render()

    // --- 工具 ---
    void updateThickUIEnabled();      // 根据 Thick 勾选与模式显示/隐藏厚度滑条
    static void vtkEventForwarder(vtkObject *caller, unsigned long eid,
                                  void *clientData, void *callData);

private:
    // Qt UI
    QPointer<QToolBar> _toolbar;
    QAction *_actOpen = nullptr;

    QPointer<QCheckBox> _chkThick;    // 厚层开关
    QPointer<QSlider> _sldThick;    // 厚度（像素）
    QPointer<QLabel> _lblThick;

    QPointer<QLabel> _lblWL;
    QPointer<QSlider> _sldWin;        // 窗宽(0..1000 -> 映射到数据范围)
    QPointer<QSlider> _sldLev;        // 窗位(0..1000)
    QPointer<QLabel> _lblScroll;
    QPointer<QSlider> _sldScroll;     // 滚轮因子(1..20 -> 0.1..2.0)

    // 三个 QVTK 控件
    QPointer<QVTKOpenGLNativeWidget> _wgtAxial;
    QPointer<QVTKOpenGLNativeWidget> _wgtCoronal;
    QPointer<QVTKOpenGLNativeWidget> _wgtSagittal;

    // 三个渲染窗口
    vtkSmartPointer<vtkGenericOpenGLRenderWindow> _winAxial;
    vtkSmartPointer<vtkGenericOpenGLRenderWindow> _winCoronal;
    vtkSmartPointer<vtkGenericOpenGLRenderWindow> _winSagittal;

    // 三个 Reslice viewers
    vtkSmartPointer<vtkResliceImageViewer> _viewAxial;
    vtkSmartPointer<vtkResliceImageViewer> _viewCoronal;
    vtkSmartPointer<vtkResliceImageViewer> _viewSagittal;

    // 共享的 ResliceCursor / LookupTable
    vtkSmartPointer<vtkResliceCursor> _sharedCursor;
    vtkSmartPointer<vtkLookupTable> _sharedLut;   // 像素值 → 颜色/灰度

    // 事件回调（把某个 viewer 的交互事件转发给另外两个刷新）
    vtkSmartPointer<vtkCallbackCommand> _evtCallback;

    // 数据
    vtkSmartPointer<vtkImageData> _imageData;

    // 当前模式与厚层状态
    bool _oblique = false;
    bool _thickOn = false;
    int _thickPx = 10;
    double _scrollFactor = 1.0;

};

#endif // CX_CT_X2_VTI_MPR3_VIEW_WIDGET_H
