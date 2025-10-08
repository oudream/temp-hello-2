#include "t_vti_mpr3_view_widget.h"

#include <QVBoxLayout>
#include <QGridLayout>
#include <QFileDialog>
#include <QSplitter>
#include <QMessageBox>

#include <vtkXMLImageDataReader.h>
#include <vtkResliceCursorRepresentation.h>
#include <vtkResliceCursorLineRepresentation.h>
#include <vtkResliceCursorThickLineRepresentation.h>
#include <vtkImageReslice.h>
#include <vtkCamera.h>
#include <vtkMath.h>

TVtiMpr3ViewWidget::TVtiMpr3ViewWidget(QWidget* parent)
        : QWidget(parent)
{
    buildUi();
    buildViewers();
    connectObservers();
}

void TVtiMpr3ViewWidget::buildUi()
{
    _toolbar = new QToolBar(this);
    _actOpen = _toolbar->addAction(QString::fromUtf8("打开 .vti"));
    _toolbar->addSeparator();

    _toolbar->addSeparator();
    _chkThick = new QCheckBox(QString::fromUtf8("Thick(厚层)"), this);
    _lblThick = new QLabel("厚度:", this);
    _sldThick = new QSlider(Qt::Horizontal, this);
    _sldThick->setRange(1, 50);
    _sldThick->setValue(10);

    _toolbar->addWidget(_chkThick);
    _toolbar->addWidget(_lblThick);
    _toolbar->addWidget(_sldThick);

    _toolbar->addSeparator();
    _lblWL = new QLabel("WL:", this);
    _sldWin = new QSlider(Qt::Horizontal, this);
    _sldLev = new QSlider(Qt::Horizontal, this);
    _sldWin->setRange(0, 1000);
    _sldLev->setRange(0, 1000);
    _sldWin->setValue(800);
    _sldLev->setValue(500);
    _toolbar->addWidget(_lblWL);
    _toolbar->addWidget(new QLabel("Win", this));
    _toolbar->addWidget(_sldWin);
    _toolbar->addWidget(new QLabel("Lev", this));
    _toolbar->addWidget(_sldLev);

    _toolbar->addSeparator();
    _lblScroll = new QLabel("滚轮因子:", this);
    _sldScroll = new QSlider(Qt::Horizontal, this);
    _sldScroll->setRange(1, 20);      // 0.1 .. 2.0
    _sldScroll->setValue(10);         // 1.0
    _toolbar->addWidget(_lblScroll);
    _toolbar->addWidget(_sldScroll);

    // 三个 QVTK 小部件：Axial | Coronal | Sagittal
    _wgtAxial    = new QVTKOpenGLNativeWidget(this);
    _wgtCoronal  = new QVTKOpenGLNativeWidget(this);
    _wgtSagittal = new QVTKOpenGLNativeWidget(this);

    auto* grid = new QGridLayout;
    grid->setContentsMargins(0,0,0,0);
    grid->setSpacing(2);
    grid->addWidget(_wgtAxial, 0, 0);
    grid->addWidget(_wgtCoronal, 0, 1);
    grid->addWidget(_wgtSagittal, 0, 2);

    auto* lay = new QVBoxLayout(this);
    lay->setContentsMargins(0,0,0,0);
    lay->addWidget(_toolbar);
    lay->addLayout(grid);
    setLayout(lay);

    // Qt 信号
    connect(_actOpen, &QAction::triggered, this, &TVtiMpr3ViewWidget::onOpenClicked);
    connect(_chkThick, &QCheckBox::checkStateChanged, this, &TVtiMpr3ViewWidget::onThickToggled);
    connect(_sldThick, &QSlider::valueChanged, this, &TVtiMpr3ViewWidget::onThickChanged);
    connect(_sldWin, &QSlider::valueChanged, this, &TVtiMpr3ViewWidget::onWLChanged);
    connect(_sldLev, &QSlider::valueChanged, this, &TVtiMpr3ViewWidget::onWLChanged);
    connect(_sldScroll, &QSlider::valueChanged, this, &TVtiMpr3ViewWidget::onScrollFactorChanged);

    updateThickUIEnabled();
}

void TVtiMpr3ViewWidget::buildViewers()
{
    // 渲染窗口
    _winAxial    = vtkSmartPointer<vtkGenericOpenGLRenderWindow>::New();
    _winCoronal  = vtkSmartPointer<vtkGenericOpenGLRenderWindow>::New();
    _winSagittal = vtkSmartPointer<vtkGenericOpenGLRenderWindow>::New();

    // 三个 ResliceImageViewer
    _viewAxial    = vtkSmartPointer<vtkResliceImageViewer>::New();
    _viewCoronal  = vtkSmartPointer<vtkResliceImageViewer>::New();
    _viewSagittal = vtkSmartPointer<vtkResliceImageViewer>::New();

    // 共享的 LUT（可替换为医学灰阶）
    _sharedLut = vtkSmartPointer<vtkLookupTable>::New();
    _sharedLut->SetRange(0.0, 1000.0);
    _sharedLut->Build();

    _viewAxial->SetLookupTable(_sharedLut);
    _viewCoronal->SetLookupTable(_sharedLut);
    _viewSagittal->SetLookupTable(_sharedLut);

    // 初始模式：AxisAligned，Thick 关闭
    _viewAxial->SetResliceModeToAxisAligned();
    _viewCoronal->SetResliceModeToAxisAligned();
    _viewSagittal->SetResliceModeToAxisAligned();

    setThickMode(false, 10);

    // 初始滚轮因子
    setSliceScrollFactor(1.0);
}

void TVtiMpr3ViewWidget::connectObservers()
{
    // 统一的回调：任何一个 viewer 发生 SliceChanged/Interaction，就刷新另外两个
    _evtCallback = vtkSmartPointer<vtkCallbackCommand>::New();
    _evtCallback->SetClientData(this);
    _evtCallback->SetCallback(&TVtiMpr3ViewWidget::vtkEventForwarder);

    // 监听两类事件：SliceChangedEvent(1001) 和 InteractionEvent
    const unsigned long ev1 = static_cast<unsigned long>(vtkResliceImageViewer::SliceChangedEvent);
    const unsigned long ev2 = vtkCommand::InteractionEvent;

    _viewAxial->AddObserver(ev1, _evtCallback);
    _viewAxial->AddObserver(ev2, _evtCallback);

    _viewCoronal->AddObserver(ev1, _evtCallback);
    _viewCoronal->AddObserver(ev2, _evtCallback);

    _viewSagittal->AddObserver(ev1, _evtCallback);
    _viewSagittal->AddObserver(ev2, _evtCallback);
}

void TVtiMpr3ViewWidget::vtkEventForwarder(vtkObject* caller, unsigned long,
                                           void* clientData, void*)
{
    TVtiMpr3ViewWidget* self = reinterpret_cast<TVtiMpr3ViewWidget*>(clientData);
    if (!self) return;

    // 谁触发就刷新其它两个
    vtkResliceImageViewer* who = vtkResliceImageViewer::SafeDownCast(caller);
    if (!who) return;

    if (who != self->_viewAxial)
        self->_wgtAxial->renderWindow()->Render();
    if (who != self->_viewCoronal)
        self->_wgtCoronal->renderWindow()->Render();
    if (who != self->_viewSagittal)
        self->_wgtSagittal->renderWindow()->Render();
}

void TVtiMpr3ViewWidget::refreshAll()
{
    if (_wgtAxial)    _wgtAxial->renderWindow()->Render();
    if (_wgtCoronal)  _wgtCoronal->renderWindow()->Render();
    if (_wgtSagittal) _wgtSagittal->renderWindow()->Render();
}

bool TVtiMpr3ViewWidget::loadVtiFile(const QString& filename)
{
    vtkSmartPointer<vtkXMLImageDataReader> reader = vtkSmartPointer<vtkXMLImageDataReader>::New();
    reader->SetFileName(filename.toLocal8Bit().constData());
    try { reader->Update(); }
    catch (...) {
        QMessageBox::critical(this, "错误", "读取 .vti 失败");
        return false;
    }
    setImageData(reader->GetOutput());
    emit vtiLoaded(filename, _imageData);
    return true;
}

void TVtiMpr3ViewWidget::setImageData(vtkImageData* img)
{
    if (!img) return;
    _imageData = img;

    _viewAxial->SetInputData(_imageData);
    _viewCoronal->SetInputData(_imageData);
    _viewSagittal->SetInputData(_imageData);

    // 绑定窗口
    _viewAxial->SetRenderWindow(_winAxial);
    _viewCoronal->SetRenderWindow(_winCoronal);
    _viewSagittal->SetRenderWindow(_winSagittal);

    _wgtAxial->setRenderWindow(_winAxial);
    _wgtCoronal->setRenderWindow(_winCoronal);
    _wgtSagittal->setRenderWindow(_winSagittal);

    // 在 buildViewers() 末尾，三个 viewer 绑定各自的 interactor：
    _viewAxial->SetupInteractor(_winAxial->GetInteractor());
    _viewCoronal->SetupInteractor(_winCoronal->GetInteractor());
    _viewSagittal->SetupInteractor(_winSagittal->GetInteractor());

    // 共享 cursor 并把图像塞给三个 viewer
    {
        // 以 Axial 的 cursor 为主，三个视图共享它
        _sharedCursor = vtkSmartPointer<vtkResliceCursor>::New();
        // 初始厚层设置（会被 setThickMode() 覆盖）
        _sharedCursor->SetThickMode(0);
        _sharedCursor->SetThickness(10, 10, 10);

        // 给三个 viewer 都塞入同一个 cursor
        _viewAxial->SetResliceCursor(_sharedCursor);
        _viewCoronal->SetResliceCursor(_sharedCursor);
        _viewSagittal->SetResliceCursor(_sharedCursor);
    }

    // 让 cursor 知道图像与中心
    _sharedCursor->SetImage(_imageData);
    _sharedCursor->SetCenter(_imageData->GetCenter());

    // 初始三正交
    {
        // 三正交方向（与 vtkImageViewer2 的取值一致）
        _viewAxial->SetSliceOrientationToXY();   // Axial(Z)
        _viewCoronal->SetSliceOrientationToXZ(); // Coronal(Y)
        _viewSagittal->SetSliceOrientationToYZ();// Sagittal(X)

        // 初始相机：平行投影（类内部也会设，这里显式一次更直观）
        _viewAxial->GetRenderer()->GetActiveCamera()->ParallelProjectionOn();
        _viewCoronal->GetRenderer()->GetActiveCamera()->ParallelProjectionOn();
        _viewSagittal->GetRenderer()->GetActiveCamera()->ParallelProjectionOn();
    }

    // 统一 LUT 与 WL
    {
        double range[2]; _imageData->GetScalarRange(range);
        _sharedLut->SetRange(range[0], range[1]);
        _sharedLut->Build();

        _viewAxial->SetLookupTable(_sharedLut);
        _viewCoronal->SetLookupTable(_sharedLut);
        _viewSagittal->SetLookupTable(_sharedLut);

        // 用当前滑条值更新 WL
        onWLChanged();
    }

    // 同步厚层/模式/滚轮因子
    setThickMode(_thickOn, _thickPx);
    setSliceScrollFactor(_scrollFactor);

    // 自适应相机
    _viewAxial->Render();
    _viewCoronal->Render();
    _viewSagittal->Render();
}

void TVtiMpr3ViewWidget::setWindowLevel(double win, double lev)
{
    _viewAxial->SetColorWindow(win);
    _viewAxial->SetColorLevel(lev);
    _viewCoronal->SetColorWindow(win);
    _viewCoronal->SetColorLevel(lev);
    _viewSagittal->SetColorWindow(win);
    _viewSagittal->SetColorLevel(lev);
    refreshAll();
}

void TVtiMpr3ViewWidget::setThickMode(bool on, int thickness)
{
    _thickOn = on;
    _thickPx = thickness < 1 ? 1 : thickness;

    // 设置 ThickMode：切换 representation（Thin/Thick）
    _viewAxial->SetThickMode(on ? 1 : 0);
    _viewCoronal->SetThickMode(on ? 1 : 0);
    _viewSagittal->SetThickMode(on ? 1 : 0);

    // 厚度（像素） -> 三方向 thickness
    if (_sharedCursor)
    {
        _sharedCursor->SetThickness(_thickPx, _thickPx, _thickPx);
    }

    updateThickUIEnabled();
    refreshAll();
}

void TVtiMpr3ViewWidget::setSliceScrollFactor(double f)
{
    if (f <= 0) f = 1.0;
    _scrollFactor = f;
    _viewAxial->SetSliceScrollFactor(f);
    _viewCoronal->SetSliceScrollFactor(f);
    _viewSagittal->SetSliceScrollFactor(f);
}

vtkRenderWindow* TVtiMpr3ViewWidget::renderWindowAxial() const
{
    return _wgtAxial ? _wgtAxial->renderWindow() : nullptr;
}
vtkRenderWindow* TVtiMpr3ViewWidget::renderWindowCoronal() const
{
    return _wgtCoronal ? _wgtCoronal->renderWindow() : nullptr;
}
vtkRenderWindow* TVtiMpr3ViewWidget::renderWindowSagittal() const
{
    return _wgtSagittal ? _wgtSagittal->renderWindow() : nullptr;
}

/* ---------------- UI 槽 ---------------- */

void TVtiMpr3ViewWidget::onOpenClicked()
{
    const QString file = QFileDialog::getOpenFileName(this, "选择 .vti", {}, "VTK Image (*.vti)");
    if (!file.isEmpty()) loadVtiFile(file);
}

void TVtiMpr3ViewWidget::onThickToggled(int on)
{
    setThickMode(on != 0, _sldThick->value());
}

void TVtiMpr3ViewWidget::onThickChanged(int v)
{
    if (_thickOn) setThickMode(true, v);
}

static inline double lerp(double a, double b, double t)
{
    return a*(1.0 - t) + b*t;
}

void TVtiMpr3ViewWidget::onWLChanged()
{
    if (!_imageData) return;
    double r[2]; _imageData->GetScalarRange(r);
    const double win = std::max(1.0, lerp( (r[1]-r[0])*0.05, (r[1]-r[0])*1.5,
                                           _sldWin->value() / 1000.0 ));
    const double lev = lerp(r[0], r[1], _sldLev->value() / 1000.0);
    setWindowLevel(win, lev);
}

void TVtiMpr3ViewWidget::onScrollFactorChanged(int v)
{
    // 1..20 -> 0.1 .. 2.0
    const double f = 0.1 + (v-1) * ( (2.0-0.1) / 19.0 );
    setSliceScrollFactor(f);
}

void TVtiMpr3ViewWidget::updateThickUIEnabled()
{
    const bool on = _chkThick->isChecked();
    _lblThick->setVisible(on);
    _sldThick->setVisible(on);
}
