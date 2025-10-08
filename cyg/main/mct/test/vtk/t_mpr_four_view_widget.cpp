#include "t_mpr_four_view_widget.h"

#include <QVBoxLayout>
#include <QGridLayout>
#include <QSplitter>
#include <QFileDialog>
#include <QMessageBox>

#include <vtkCamera.h>
#include <vtkMath.h>

/* 构造：先 UI 再管线，避免互相依赖时的空指针 */
TMprFourViewWidget::TMprFourViewWidget(QWidget *parent)
        : QWidget(parent)
{
    buildUi();          // 工具条 + 四块 QVTK
    build3D();          // 左 3D 体渲染管线（不喂数据）
    buildMprViewers();  // 右 3×MPR 管线（不喂数据）
    connectObservers(); // 建立三视图事件联动
}

/* ---------------- UI 组装 ---------------- */

void TMprFourViewWidget::buildUi()
{
    mToolbar = new QToolBar(this);
    mActOpen = mToolbar->addAction(QString::fromUtf8("打开 .vti"));
    mToolbar->addSeparator();

    // Reslice 模式：AxisAligned / Oblique
    mToolbar->addWidget(new QLabel("Reslice:", this));
    mCmbMode = new QComboBox(this);
    mCmbMode->addItem("AxisAligned");
    mCmbMode->addItem("Oblique");
    mCmbMode->setCurrentIndex(1); // 默认 Oblique
    mToolbar->addWidget(mCmbMode);

    // 厚层 Thick
    mToolbar->addSeparator();
    mChkThick = new QCheckBox(QString::fromUtf8("Thick(厚层)"), this);
    mLblThick = new QLabel(QString::fromUtf8("厚度:"), this);
    mSldThick = new QSlider(Qt::Horizontal, this);
    mSldThick->setRange(1, 50);
    mSldThick->setValue(mThickPx);
    mToolbar->addWidget(mChkThick);
    mToolbar->addWidget(mLblThick);
    mToolbar->addWidget(mSldThick);

    // 窗宽窗位
    mToolbar->addSeparator();
    mLblWL = new QLabel("WL:", this);
    mSldWin = new QSlider(Qt::Horizontal, this);
    mSldLev = new QSlider(Qt::Horizontal, this);
    mSldWin->setRange(0, 1000);
    mSldLev->setRange(0, 1000);
    mSldWin->setValue(800);
    mSldLev->setValue(500);
    mToolbar->addWidget(mLblWL);
    mToolbar->addWidget(new QLabel("Win", this));
    mToolbar->addWidget(mSldWin);
    mToolbar->addWidget(new QLabel("Lev", this));
    mToolbar->addWidget(mSldLev);

    // 滚轮因子
    mToolbar->addSeparator();
    mLblScroll = new QLabel(QString::fromUtf8("滚轮因子:"), this);
    mSldScroll = new QSlider(Qt::Horizontal, this);
    mSldScroll->setRange(1, 20);  // 0.1 .. 2.0
    mSldScroll->setValue(10);     // 1.0
    mToolbar->addWidget(mLblScroll);
    mToolbar->addWidget(mSldScroll);

    // 四个 QVTK 控件：左 3D；右三视图
    mWgt3D = new QVTKOpenGLNativeWidget(this);
    mWgtAxial = new QVTKOpenGLNativeWidget(this);
    mWgtCoronal = new QVTKOpenGLNativeWidget(this);
    mWgtSagittal = new QVTKOpenGLNativeWidget(this);

    // 右侧三视图使用网格 1×3；整体再用 QSplitter 左右分栏
    auto *rightGrid = new QWidget(this);
    auto *grid = new QGridLayout(rightGrid);
    grid->setContentsMargins(0, 0, 0, 0);
    grid->setSpacing(2);
    grid->addWidget(mWgtAxial, 0, 0);
    grid->addWidget(mWgtCoronal, 0, 1);
    grid->addWidget(mWgtSagittal, 0, 2);

    auto *split = new QSplitter(Qt::Horizontal, this);
    split->addWidget(mWgt3D);
    split->addWidget(rightGrid);
    split->setStretchFactor(0, 2);
    split->setStretchFactor(1, 3);

    auto *lay = new QVBoxLayout(this);
    lay->setContentsMargins(0, 0, 0, 0);
    lay->addWidget(mToolbar);
    lay->addWidget(split);
    setLayout(lay);

    // Qt 信号
    connect(mActOpen, &QAction::triggered, this, &TMprFourViewWidget::onOpenClicked);
    connect(mCmbMode, qOverload<int>(&QComboBox::currentIndexChanged), this, &TMprFourViewWidget::onModeChanged);
    connect(mChkThick, &QCheckBox::stateChanged, this, &TMprFourViewWidget::onThickToggled);
    connect(mSldThick, &QSlider::valueChanged, this, &TMprFourViewWidget::onThickChanged);
    connect(mSldWin, &QSlider::valueChanged, this, &TMprFourViewWidget::onWLChanged);
    connect(mSldLev, &QSlider::valueChanged, this, &TMprFourViewWidget::onWLChanged);
    connect(mSldScroll, &QSlider::valueChanged, this, &TMprFourViewWidget::onScrollFactorChanged);
}

/* ---------------- 左 3D 体渲染 ---------------- */

void TMprFourViewWidget::build3D()
{
    mWin3D = vtkSmartPointer<vtkGenericOpenGLRenderWindow>::New();
    mRen3D = vtkSmartPointer<vtkRenderer>::New();
    mWin3D->AddRenderer(mRen3D);
    mWgt3D->setRenderWindow(mWin3D);

    mRen3D->SetBackground(0.06, 0.06, 0.08);

    // 最小体渲染管线（可按你旧工程再扩展传输函数/光照等）
    mVolMapper = vtkSmartPointer<vtkSmartVolumeMapper>::New();
    mCtf = vtkSmartPointer<vtkColorTransferFunction>::New();
    mPwf = vtkSmartPointer<vtkPiecewiseFunction>::New();
    mVolProp = vtkSmartPointer<vtkVolumeProperty>::New();
    mVolume = vtkSmartPointer<vtkVolume>::New();

    mVolProp->SetInterpolationTypeToLinear();
    mVolProp->ShadeOff();
    mVolProp->SetColor(mCtf);
    mVolProp->SetScalarOpacity(mPwf);
    mVolume->SetMapper(mVolMapper);
    mVolume->SetProperty(mVolProp);
}

/* ---------------- 右 3×MPR（ResliceImageViewer） ---------------- */

void TMprFourViewWidget::buildMprViewers()
{
    // RenderWindow
    mWinAxial = vtkSmartPointer<vtkGenericOpenGLRenderWindow>::New();
    mWinCoronal = vtkSmartPointer<vtkGenericOpenGLRenderWindow>::New();
    mWinSagittal = vtkSmartPointer<vtkGenericOpenGLRenderWindow>::New();

    // 三个 viewer
    mViewAxial = vtkSmartPointer<vtkResliceImageViewer>::New();
    mViewCoronal = vtkSmartPointer<vtkResliceImageViewer>::New();
    mViewSagittal = vtkSmartPointer<vtkResliceImageViewer>::New();

    // 绑定 RenderWindow
    mViewAxial->SetRenderWindow(mWinAxial);
    mViewCoronal->SetRenderWindow(mWinCoronal);
    mViewSagittal->SetRenderWindow(mWinSagittal);

    mWgtAxial->setRenderWindow(mWinAxial);
    mWgtCoronal->setRenderWindow(mWinCoronal);
    mWgtSagittal->setRenderWindow(mWinSagittal);

    // 共享 LUT（灰阶；范围会在 setImageData() 时重设）
    mSharedLut = vtkSmartPointer<vtkLookupTable>::New();
    mSharedLut->SetRange(0.0, 1000.0);
    mSharedLut->Build();
    mViewAxial->SetLookupTable(mSharedLut);
    mViewCoronal->SetLookupTable(mSharedLut);
    mViewSagittal->SetLookupTable(mSharedLut);

    // 初始为 Oblique（可交互十字线与厚层）——工具条可切换回 AxisAligned
    setObliqueMode(mOblique);

    // 关键：把 interactor 塞进去，否则滚轮/十字线无效（官方实现依赖它）【见源码】。
    mViewAxial->SetupInteractor(mWinAxial->GetInteractor());
    mViewCoronal->SetupInteractor(mWinCoronal->GetInteractor());
    mViewSagittal->SetupInteractor(mWinSagittal->GetInteractor());
}

/* ---------------- 图像输入与基本配置 ---------------- */

bool TMprFourViewWidget::loadVtiFile(const QString &filename)
{
    auto reader = vtkSmartPointer<vtkXMLImageDataReader>::New();
    reader->SetFileName(filename.toLocal8Bit().constData());
    try
    { reader->Update(); }
    catch (...)
    {
        QMessageBox::critical(this, "错误", "读取 .vti 失败");
        return false;
    }
    setImageData(reader->GetOutput());
    emit vtiLoaded(filename, mImage);
    return true;
}

void TMprFourViewWidget::setImageData(vtkImageData *img)
{
    if (!img) return;
    mImage = img;

    // 共享一个 ResliceCursor（十字线联动的核心）
    shareResliceCursor();

    // ★ 先喂数据，再渲染（避免 WindowLevel 无输入报错）
    mViewAxial->SetInputData(mImage);
    mViewCoronal->SetInputData(mImage);
    mViewSagittal->SetInputData(mImage);

    // 让 cursor 知道图像与中心
    mSharedCursor->SetImage(mImage);
    mSharedCursor->SetCenter(mImage->GetCenter());

    // 三正交
    applyInitialOrientations();

    // WL 的 LUT 范围与滑条默认值
    double r[2];
    mImage->GetScalarRange(r);
    mSharedLut->SetRange(r[0], r[1]);
    mSharedLut->Build();
    onWLChanged(); // 用滑条映射一次

    // 同步 Thick/Oblique/滚轮因子
    setThickMode(mThickOn, mThickPx);
    setObliqueMode(mOblique);
    setSliceScrollFactor(mScrollFactor);

    // 3D 体渲染挂接该数据
    setup3DVolume();

    refreshAll(); // 首次渲染所有视图
}

void TMprFourViewWidget::setup3DVolume()
{
    if (!mImage) return;

    mVolMapper->RemoveAllInputs();
    mVolMapper->SetInputData(mImage);
    mVolMapper->SetRequestedRenderModeToRayCast(); // 稳定保守；可改默认/硬件

    // 简单传输函数（按你旧工程可随时替换/细化）【沿用旧控件风格】
    double r[2];
    mImage->GetScalarRange(r);
    const double lo = r[0], hi = r[1], mid = (lo + hi) * 0.5;

    mCtf->RemoveAllPoints();
    mCtf->AddRGBPoint(lo, 0.0, 0.0, 0.0);
    mCtf->AddRGBPoint(mid, 0.9, 0.7, 0.6);
    mCtf->AddRGBPoint(hi, 1.0, 1.0, 1.0);

    mPwf->RemoveAllPoints();
    mPwf->AddPoint(lo, 0.00);
    mPwf->AddPoint(mid, 0.10);
    mPwf->AddPoint(hi, 0.90);

    mRen3D->RemoveAllViewProps();
    mRen3D->AddViewProp(mVolume);
    mRen3D->ResetCamera();
}

/* ---------------- 共享 Cursor / 事件联动 / 方向 ---------------- */

void TMprFourViewWidget::shareResliceCursor()
{
    if (!mSharedCursor)
        mSharedCursor = vtkSmartPointer<vtkResliceCursor>::New();

    // 初始厚层设置（会被 setThickMode() 覆盖）
    mSharedCursor->SetThickMode(0);
    mSharedCursor->SetThickness(10, 10, 10);

    // 三个 viewer 共享同一个 cursor（官方推荐：同一个 Cursor，不同的表示层）
    mViewAxial->SetResliceCursor(mSharedCursor);
    mViewCoronal->SetResliceCursor(mSharedCursor);
    mViewSagittal->SetResliceCursor(mSharedCursor);
}

void TMprFourViewWidget::connectObservers()
{
    // 统一回调：谁动就刷新其它两个（并不改变数据，只触发重绘）
    mEvtCallback = vtkSmartPointer<vtkCallbackCommand>::New();
    mEvtCallback->SetClientData(this);
    mEvtCallback->SetCallback(&TMprFourViewWidget::vtkEventForwarder);

    const unsigned long ev1 = static_cast<unsigned long>(vtkResliceImageViewer::SliceChangedEvent);
    const unsigned long ev2 = vtkCommand::InteractionEvent;

    mViewAxial->AddObserver(ev1, mEvtCallback);
    mViewAxial->AddObserver(ev2, mEvtCallback);

    mViewCoronal->AddObserver(ev1, mEvtCallback);
    mViewCoronal->AddObserver(ev2, mEvtCallback);

    mViewSagittal->AddObserver(ev1, mEvtCallback);
    mViewSagittal->AddObserver(ev2, mEvtCallback);
}

void TMprFourViewWidget::vtkEventForwarder(vtkObject *caller, unsigned long, void *clientData, void *)
{
    auto *self = reinterpret_cast<TMprFourViewWidget *>(clientData);
    if (!self) return;
    auto *who = vtkResliceImageViewer::SafeDownCast(caller);
    if (!who) return;

    // 刷新另外两个 2D；3D 是否刷新可按需（十字线移动通常不必重绘 3D）
    if (who != self->mViewAxial) self->mWgtAxial->renderWindow()->Render();
    if (who != self->mViewCoronal) self->mWgtCoronal->renderWindow()->Render();
    if (who != self->mViewSagittal) self->mWgtSagittal->renderWindow()->Render();
}

void TMprFourViewWidget::applyInitialOrientations()
{
    // 对齐三个正交方向（与 vtkImageViewer2 相同的取值）
    mViewAxial->SetSliceOrientationToXY();    // 观察 Z（Axial）
    mViewCoronal->SetSliceOrientationToXZ();  // 观察 Y（Coronal）
    mViewSagittal->SetSliceOrientationToYZ(); // 观察 X（Sagittal）

    // 显式指定平行投影（类内部也会设置，这里清晰表达 2D 语义）
    mViewAxial->GetRenderer()->GetActiveCamera()->ParallelProjectionOn();
    mViewCoronal->GetRenderer()->GetActiveCamera()->ParallelProjectionOn();
    mViewSagittal->GetRenderer()->GetActiveCamera()->ParallelProjectionOn();
}

/* ---------------- 模式/厚层/WL/滚轮 因子 ---------------- */

void TMprFourViewWidget::setObliqueMode(bool on)
{
    mOblique = on;
    if (on)
    {
        // Oblique：启用 ResliceCursorWidget（可交互十字线/厚层）【见源码】。
        mViewAxial->SetResliceModeToOblique();
        mViewCoronal->SetResliceModeToOblique();
        mViewSagittal->SetResliceModeToOblique();
    }
    else
    {
        // AxisAligned：退化为 ImageViewer2 行为（不开十字线）【见源码】。
        mViewAxial->SetResliceModeToAxisAligned();
        mViewCoronal->SetResliceModeToAxisAligned();
        mViewSagittal->SetResliceModeToAxisAligned();
    }
    refreshAll2D();
}

void TMprFourViewWidget::setThickMode(bool on, int thicknessPx)
{
    mThickOn = on;
    mThickPx = thicknessPx < 1 ? 1 : thicknessPx;

    mViewAxial->SetThickMode(on ? 1 : 0);
    mViewCoronal->SetThickMode(on ? 1 : 0);
    mViewSagittal->SetThickMode(on ? 1 : 0);

    if (mSharedCursor)
        mSharedCursor->SetThickness(mThickPx, mThickPx, mThickPx);

    refreshAll2D();
}

void TMprFourViewWidget::setSliceScrollFactor(double f)
{
    if (f <= 0) f = 1.0;
    mScrollFactor = f;
    mViewAxial->SetSliceScrollFactor(f);
    mViewCoronal->SetSliceScrollFactor(f);
    mViewSagittal->SetSliceScrollFactor(f);
}

static inline double lerp(double a, double b, double t)
{ return a * (1.0 - t) + b * t; }

void TMprFourViewWidget::setWindowLevel(double win, double lev)
{
    // 同步三视图的 WL
    mViewAxial->SetColorWindow(win);
    mViewAxial->SetColorLevel(lev);
    mViewCoronal->SetColorWindow(win);
    mViewCoronal->SetColorLevel(lev);
    mViewSagittal->SetColorWindow(win);
    mViewSagittal->SetColorLevel(lev);
    refreshAll2D();
}

void TMprFourViewWidget::onWLChanged()
{
    if (!mImage) return;
    double r[2];
    mImage->GetScalarRange(r);
    const double win = std::max(1.0, lerp((r[1] - r[0]) * 0.05, (r[1] - r[0]) * 1.5,
                                          mSldWin->value() / 1000.0));
    const double lev = lerp(r[0], r[1], mSldLev->value() / 1000.0);
    setWindowLevel(win, lev);
}

void TMprFourViewWidget::onModeChanged(int idx)
{
    setObliqueMode(idx == 1);
}

void TMprFourViewWidget::onThickToggled(int on)
{
    setThickMode(on != 0, mSldThick->value());
}

void TMprFourViewWidget::onThickChanged(int v)
{
    if (mThickOn) setThickMode(true, v);
}

void TMprFourViewWidget::onScrollFactorChanged(int v)
{
    // 1..20 -> 0.1 .. 2.0
    const double f = 0.1 + (v - 1) * ((2.0 - 0.1) / 19.0);
    setSliceScrollFactor(f);
}

void TMprFourViewWidget::refreshAll2D()
{
    if (mWgtAxial) mWgtAxial->renderWindow()->Render();
    if (mWgtCoronal) mWgtCoronal->renderWindow()->Render();
    if (mWgtSagittal) mWgtSagittal->renderWindow()->Render();
}

void TMprFourViewWidget::refreshAll()
{
    refreshAll2D();
    if (mWgt3D) mWgt3D->renderWindow()->Render();
}

/* ---------------- 打开文件 ---------------- */

void TMprFourViewWidget::onOpenClicked()
{
    const QString file = QFileDialog::getOpenFileName(this, "选择 .vti", {}, "VTK Image (*.vti)");
    if (!file.isEmpty()) loadVtiFile(file);
}
