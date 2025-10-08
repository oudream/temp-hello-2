#ifndef CX_CT_X2_MPR_FOUR_VIEW_WIDGET_H
#define CX_CT_X2_MPR_FOUR_VIEW_WIDGET_H

/*
 * 四视图控件（完整注释版）
 * ------------------------------------------------------------
 * 左侧：3D 体渲染（Volume Rendering）
 * 右侧：三正交 MPR（Axial/Coronal/Sagittal），使用 vtkResliceImageViewer
 *       - 三个视图共享同一个 vtkResliceCursor（十字线联动）
 *       - AxisAligned / Oblique 模式切换（Oblique 才有可交互十字线/厚层）
 *       - Thick 模式 + 厚度（像素）
 *       - 统一窗宽窗位（WL）
 *       - 滚轮步进因子（SliceScrollFactor）
 *
 * 设计原则：
 * 1) UI 与管线解耦：Qt 负责布局/工具条；VTK 负责渲染与交互。
 * 2) 保障时序：先 SetInputData，再渲染（避免 WindowLevel 无输入报错）。
 * 3) 共享 ResliceCursor，并在任一视图交互触发时，刷新其它两个视图。
 * 4) 对外只暴露最小 API：loadVtiFile / setImageData / setWindowLevel 等。
 */

#include <QWidget>
#include <QPointer>
#include <QToolBar>
#include <QComboBox>
#include <QCheckBox>
#include <QSlider>
#include <QLabel>
#include <QVTKOpenGLNativeWidget.h>

#include <vtkSmartPointer.h>
#include <vtkGenericOpenGLRenderWindow.h>
#include <vtkRenderer.h>
#include <vtkImageData.h>
#include <vtkXMLImageDataReader.h>

#include <vtkSmartVolumeMapper.h>
#include <vtkVolume.h>
#include <vtkVolumeProperty.h>
#include <vtkColorTransferFunction.h>
#include <vtkPiecewiseFunction.h>

#include <vtkResliceImageViewer.h>
#include <vtkResliceCursor.h>
#include <vtkResliceCursorWidget.h>
#include <vtkResliceCursorRepresentation.h>
#include <vtkCallbackCommand.h>
#include <vtkLookupTable.h>

class TMprFourViewWidget : public QWidget
{
Q_OBJECT
public:
    explicit TMprFourViewWidget(QWidget *parent = nullptr);

    ~TMprFourViewWidget() override = default;

    // 载入 .vti 文件（内部用 vtkXMLImageDataReader）
    bool loadVtiFile(const QString &filename);

    // 直接注入体数据（不接管生命周期）
    void setImageData(vtkImageData *img);

    // 统一设置窗宽窗位（三个 MPR 同时生效）
    void setWindowLevel(double win, double lev);

    // Thick（厚层）与厚度
    void setThickMode(bool on, int thicknessPx);

    // Oblique（斜切）或 AxisAligned（轴对齐）
    void setObliqueMode(bool on);

    // 滚轮步进因子（默认 1.0；>1 更“快”，<1 更“细”）
    void setSliceScrollFactor(double f);

signals:

    void vtiLoaded(QString file, vtkImageData *image);

private slots:

    // —— 工具条交互 —— //
    void onOpenClicked();

    void onModeChanged(int idx);     // AxisAligned / Oblique
    void onThickToggled(int on);

    void onThickChanged(int v);

    void onWLChanged();

    void onScrollFactorChanged(int v);

private:
    // —— 装配与联动 —— //
    void buildUi();                  // 工具条 + 四个 QVTK 控件 + 布局
    void build3D();                  // 左侧 3D 体渲染管线
    void buildMprViewers();          // 右侧三视图（ResliceImageViewer）
    void shareResliceCursor();       // 三视图共享同一个 Cursor
    void connectObservers();         // 任一视图交互后，刷新其他视图
    void applyInitialOrientations(); // Axial/Coronal/Sagittal
    void refreshAll2D();             // 刷新三个 2D 视图
    void refreshAll();               // 刷新全部（含 3D）

    // —— 3D 体渲染（最小管线：VolumeMapper + Volume） —— //
    void setup3DVolume();            // 将 _imageData 输入至体渲染并设默认传输函数

    // —— VTK 事件回调（静态函数 + this 指针） —— //
    static void vtkEventForwarder(vtkObject *caller, unsigned long eid,
                                  void *clientData, void *callData);

private:
    // Qt —— 工具条与控件 —— //
    QPointer<QToolBar> mToolbar;
    QAction *mActOpen = nullptr;

    QPointer<QComboBox> mCmbMode;     // 0 AxisAligned, 1 Oblique
    QPointer<QCheckBox> mChkThick;    // Thick on/off
    QPointer<QSlider> mSldThick;    // 厚度（像素）
    QPointer<QLabel> mLblThick;

    QPointer<QLabel> mLblWL;
    QPointer<QSlider> mSldWin;      // 窗宽
    QPointer<QSlider> mSldLev;      // 窗位

    QPointer<QLabel> mLblScroll;
    QPointer<QSlider> mSldScroll;   // 滚轮因子（1..20 -> 0.1..2.0）

    // 四个 QVTK 小部件：左 3D + 右三视图
    QPointer<QVTKOpenGLNativeWidget> mWgt3D;
    QPointer<QVTKOpenGLNativeWidget> mWgtAxial;
    QPointer<QVTKOpenGLNativeWidget> mWgtCoronal;
    QPointer<QVTKOpenGLNativeWidget> mWgtSagittal;

    // 对应的 RenderWindow
    vtkSmartPointer<vtkGenericOpenGLRenderWindow> mWin3D;
    vtkSmartPointer<vtkGenericOpenGLRenderWindow> mWinAxial;
    vtkSmartPointer<vtkGenericOpenGLRenderWindow> mWinCoronal;
    vtkSmartPointer<vtkGenericOpenGLRenderWindow> mWinSagittal;

    // 3D 渲染链
    vtkSmartPointer<vtkRenderer> mRen3D;
    vtkSmartPointer<vtkSmartVolumeMapper> mVolMapper;
    vtkSmartPointer<vtkColorTransferFunction> mCtf;
    vtkSmartPointer<vtkPiecewiseFunction> mPwf;
    vtkSmartPointer<vtkVolumeProperty> mVolProp;
    vtkSmartPointer<vtkVolume> mVolume;

    // 三个 ResliceImageViewer
    vtkSmartPointer<vtkResliceImageViewer> mViewAxial;
    vtkSmartPointer<vtkResliceImageViewer> mViewCoronal;
    vtkSmartPointer<vtkResliceImageViewer> mViewSagittal;

    // 共享对象
    vtkSmartPointer<vtkResliceCursor> mSharedCursor;
    vtkSmartPointer<vtkLookupTable> mSharedLut;
    vtkSmartPointer<vtkCallbackCommand> mEvtCallback;

    // 数据
    vtkSmartPointer<vtkImageData> mImage;

    // 当前状态
    bool mOblique = true;  // 默认开启可交互十字线
    bool mThickOn = false;
    int mThickPx = 10;
    double mScrollFactor = 1.0;
};

#endif // CX_CT_X2_MPR_FOUR_VIEW_WIDGET_H
