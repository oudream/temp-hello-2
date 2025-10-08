#include "t_vti_viewer_widget.h"

#include <QHBoxLayout>
#include <QFileDialog>
#include <QMessageBox>
#include <QSplitter>
#include <algorithm>

/*
 * 构造：按“先 UI、后管线”的顺序搭建，避免互相依赖时的空指针。
 * build3DPipeline / build2DPipeline 仅构造管线对象，不绑定数据。
 */

/*
 * 关键变化：
 * 1) 工具条新增“显示模式”下拉框：Composite(体绘)、MIP、MinIP、AIP、Isosurface(等值面)。
 * 2) 选择 Isosurface 时，显示并启用“等值值”滑条；其它模式隐藏/禁用它。
 * 3) 左侧 3D 视图在五种模式之间切换：三种投影/体绘复用 _volume；等值面使用 _surfActor。
 */

TVtiViewerWidget::TVtiViewerWidget(QWidget *parent)
        : QWidget(parent)
{
    buildUi();           // 搭 UI（工具条 + 两个 QVTK 控件 + 分割布局）
    build3DPipeline();   // 体绘制链（Renderer/Mapper/Volume/Property）
    build2DPipeline();   // 切片链（SliceMapper/ImageSlice）
}

/* ---------------- UI 搭建与信号连接 ---------------- */

void TVtiViewerWidget::buildUi()
{
    _toolbar = new QToolBar(this);

    // 打开 .vti 的动作（使用 VTK XMLImageDataReader）
    _actOpen = _toolbar->addAction(QString::fromUtf8("打开.vti"));
    _toolbar->addSeparator();

    // ==== 新增：显示模式 ====
    _lblMode = new QLabel(QString::fromUtf8("显示模式:"), this);
    _toolbar->addWidget(_lblMode);

    _cmbMode = new QComboBox(this);
    _cmbMode->addItem("Composite(体绘)"); // 0
    _cmbMode->addItem("MIP");             // 1
    _cmbMode->addItem("MinIP");           // 2
    _cmbMode->addItem("AIP");             // 3
    _cmbMode->addItem("Isosurface");      // 4
    _cmbMode->setEnabled(false);
    _toolbar->addWidget(_cmbMode);

    _toolbar->addSeparator();

    // 切片方向
    _cmbOri = new QComboBox(this);
    _cmbOri->addItem("Axial(Z)");    // index 0 -> VTK 2
    _cmbOri->addItem("Coronal(Y)");  // index 1 -> VTK 1
    _cmbOri->addItem("Sagittal(X)"); // index 2 -> VTK 0
    _toolbar->addWidget(_cmbOri);

    _toolbar->addSeparator();

    // 层号
    _lbl = new QLabel("Slice: -", this);
    _toolbar->addWidget(_lbl);

    _sld = new QSlider(Qt::Horizontal, this);
    _sld->setEnabled(false);
    _sld->setMinimum(0);
    _sld->setMaximum(0);
    _sld->setFixedWidth(220);
    _toolbar->addWidget(_sld);

    // ==== 新增：等值面滑条（默认隐藏，选择 Isosurface 才显示）====
    _toolbar->addSeparator();
    _lblIso = new QLabel("Iso: -", this);
    _sldIso = new QSlider(Qt::Horizontal, this);
    _sldIso->setEnabled(false);
    _sldIso->setMinimum(0);
    _sldIso->setMaximum(1000);
    _sldIso->setFixedWidth(220);

    _toolbar->addWidget(_lblIso);
    _toolbar->addWidget(_sldIso);
    _lblIso->setVisible(false);
    _sldIso->setVisible(false);

    // 两个 VTK 视口
    _vtk3D = new QVTKOpenGLNativeWidget(this);
    _vtk2D = new QVTKOpenGLNativeWidget(this);

    auto *split = new QSplitter(this);
    split->addWidget(_vtk3D);
    split->addWidget(_vtk2D);
    split->setStretchFactor(0, 2);
    split->setStretchFactor(1, 1);

    auto *lay = new QVBoxLayout(this);
    lay->setContentsMargins(0, 0, 0, 0);
    lay->addWidget(_toolbar);
    lay->addWidget(split);
    setLayout(lay);

    // 信号连接
    connect(_actOpen, &QAction::triggered, this, &TVtiViewerWidget::onOpenClicked);
    connect(_cmbOri, qOverload<int>(&QComboBox::currentIndexChanged), this, &TVtiViewerWidget::onOriChanged);
    connect(_sld, &QSlider::valueChanged, this, &TVtiViewerWidget::onSliceChanged);

    connect(_cmbMode, qOverload<int>(&QComboBox::currentIndexChanged), this, &TVtiViewerWidget::onModeChanged);
    connect(_sldIso, &QSlider::valueChanged, this, &TVtiViewerWidget::onIsoChanged);
}

void TVtiViewerWidget::build3DPipeline()
{
    _win3D = vtkSmartPointer<vtkGenericOpenGLRenderWindow>::New();
    _ren3D = vtkSmartPointer<vtkRenderer>::New();
    _win3D->AddRenderer(_ren3D);
    _vtk3D->setRenderWindow(_win3D);

    _ren3D->SetBackground(0.06, 0.06, 0.08);

    // 体绘/投影通用的体渲染对象
    _volMapper = vtkSmartPointer<vtkSmartVolumeMapper>::New();
    _ctf = vtkSmartPointer<vtkColorTransferFunction>::New();
    _pwf = vtkSmartPointer<vtkPiecewiseFunction>::New();
    _volProp = vtkSmartPointer<vtkVolumeProperty>::New();
    _volume = vtkSmartPointer<vtkVolume>::New();

    _volProp->SetInterpolationTypeToLinear();
    _volProp->ShadeOff();
    _volProp->SetColor(_ctf);
    _volProp->SetScalarOpacity(_pwf);
    _volume->SetMapper(_volMapper);
    _volume->SetProperty(_volProp);

    // 等值面表面管线（按需在 ensureSurfacePipeline() 中创建与挂接）
}

void TVtiViewerWidget::build2DPipeline()
{
    _win2D = vtkSmartPointer<vtkGenericOpenGLRenderWindow>::New();
    _ren2D = vtkSmartPointer<vtkRenderer>::New();
    _win2D->AddRenderer(_ren2D);
    _vtk2D->setRenderWindow(_win2D);

    _ren2D->SetBackground(0.1, 0.1, 0.1);

    _sliceMapper = vtkSmartPointer<vtkImageSliceMapper>::New();
    _sliceMapper->SliceAtFocalPointOff();
    _sliceActor = vtkSmartPointer<vtkImageSlice>::New();
    _sliceActor->SetMapper(_sliceMapper);
}

bool TVtiViewerWidget::loadVtiFile(const QString &filename)
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
    emit vtiLoaded(filename, _image);
    return true;
}

void TVtiViewerWidget::setImageData(vtkImageData *img)
{
    if (!img) return;
    _image = img;

    // ★ 保险：数据一到手就给 mapper
    _volMapper->RemoveAllInputs();
    _volMapper->SetInputData(_image);

    setupVolumeRendering(); // 默认先装配体渲染（传输函数/范围）
    setup2DSlice();

    // 默认 Axial(Z)
    _cmbOri->setCurrentIndex(0);
    setSliceOrientation(2);
    updateSliceRange();
    setSliceIndex((_sld->minimum() + _sld->maximum()) / 2);

    // 初始化等值面滑条范围
    updateIsoSliderRange();

    // 根据当前模式实际应用到左侧 3D 视图
    applyViewMode();

    _cmbMode->setEnabled(true);
}

void TVtiViewerWidget::setupVolumeRendering()
{
    if (!_image) return;

    // ★ 关键：先把数据接到体映射器上
    _volMapper->RemoveAllInputs();        // 防止残留
    _volMapper->SetInputData(_image);     // 或者 SetInputConnection(reader->GetOutputPort())

    // 默认按复合(Composite)来配置传输函数，投影模式时依然复用此颜色/不透明度
    _volMapper->SetRequestedRenderModeToRayCast(); // 保守稳定；如需 GPU 可换 Default

    double r[2];
    _image->GetScalarRange(r);
    const double lo = r[0], hi = r[1], mid = (lo + hi) * 0.5;

    _ctf->RemoveAllPoints();
    _ctf->AddRGBPoint(lo, 0.0, 0.0, 0.0);
    _ctf->AddRGBPoint(mid, 0.9, 0.7, 0.6);
    _ctf->AddRGBPoint(hi, 1.0, 1.0, 1.0);

    _pwf->RemoveAllPoints();
    _pwf->AddPoint(lo, 0.00);
    _pwf->AddPoint(mid, 0.10);
    _pwf->AddPoint(hi, 0.90);

    // 左视图先挂体对象（不同模式时 applyViewMode() 会切换）
    _ren3D->RemoveAllViewProps();
    _ren3D->AddViewProp(_volume);
    _ren3D->ResetCamera();
    _vtk3D->renderWindow()->Render();
}

void TVtiViewerWidget::setup2DSlice()
{
    if (!_image) return;

    _sliceMapper->SetInputData(_image);
    _ren2D->RemoveAllViewProps();
    _ren2D->AddViewProp(_sliceActor);
    refresh2DCamera();
    _vtk2D->renderWindow()->Render();
}

void TVtiViewerWidget::updateSliceRange()
{
    if (!_image) return;

    int e[6];
    _image->GetExtent(e);
    const int countX = e[1] - e[0] + 1;
    const int countY = e[3] - e[2] + 1;
    const int countZ = e[5] - e[4] + 1;

    int maxSlice = 0;
    if (_orientation == 0) maxSlice = countX - 1;
    else if (_orientation == 1) maxSlice = countY - 1;
    else maxSlice = countZ - 1;

    _sld->setEnabled(true);
    _sld->setMinimum(0);
    _sld->setMaximum(std::max(0, maxSlice));
}

void TVtiViewerWidget::refresh2DCamera()
{
    _ren2D->ResetCamera();
    _vtk2D->renderWindow()->Render();
}

/* ---------------- UI 槽：文件/方向/层号 ---------------- */

void TVtiViewerWidget::onOpenClicked()
{
    const QString file = QFileDialog::getOpenFileName(this, "选择 .vti", {}, "VTK Image (*.vti)");
    if (!file.isEmpty()) loadVtiFile(file);
}

void TVtiViewerWidget::onOriChanged(int idxUi)
{
    static const int mapUiToVtk[3] = {2, 1, 0}; // UI: Axial/Coronal/Sagittal -> VTK 2/1/0
    setSliceOrientation(mapUiToVtk[idxUi]);
    updateSliceRange();
    setSliceIndex((_sld->maximum() - _sld->minimum()) / 2);
}

void TVtiViewerWidget::onSliceChanged(int v)
{
    setSliceIndex(v);
}

/* ---------------- 新增：模式/等值面 ---------------- */

void TVtiViewerWidget::onModeChanged(int idx)
{
    _viewMode = static_cast<ViewMode>(idx);

    // 等值面时显示 iso 滑条；其它模式隐藏
    const bool isoOn = (_viewMode == ViewMode::Isosurface);
    _lblIso->setVisible(isoOn);
    _sldIso->setVisible(isoOn);
    _sldIso->setEnabled(isoOn);

    applyViewMode();
}

void TVtiViewerWidget::onIsoChanged(int /*v*/)
{
    if (_viewMode != ViewMode::Isosurface) return;
    updateIsoBySlider();
    applyViewMode(); // 仅更新表面几何即可
}

/* ---------------- 方向与层号核心 ---------------- */

void TVtiViewerWidget::setSliceOrientation(int orientation)
{
    _orientation = std::clamp(orientation, 0, 2);
    _sliceMapper->SetOrientation(_orientation);
    refresh2DCamera();
}

void TVtiViewerWidget::setSliceIndex(int slice)
{
    if (!_image) return;
    _sliceMapper->SetSliceNumber(slice);
    _lbl->setText(QString("Slice: %1").arg(slice));
    _vtk2D->renderWindow()->Render();
    emit sliceChanged(_orientation, slice);
}

vtkRenderWindow *TVtiViewerWidget::renderWindow3D() const
{
    return _vtk3D ? _vtk3D->renderWindow() : nullptr;
}

vtkRenderWindow *TVtiViewerWidget::renderWindow2D() const
{
    return _vtk2D ? _vtk2D->renderWindow() : nullptr;
}

/* ---------------- 新增：3D 左视图模式切换实现 ---------------- */

void TVtiViewerWidget::applyViewMode()
{
    if (!_image) return;

    // 先清空左视图的 ViewProps
    _ren3D->RemoveAllViewProps();

    switch (_viewMode)
    {
        case ViewMode::Composite:
            _volMapper->SetBlendModeToComposite();
            // 体绘合成：标准体绘
            _volMapper->SetInputData(_image);
            _ren3D->AddViewProp(_volume);
            break;

        case ViewMode::MIP:
            // 最大强度投影（可用于突出高密度/金属/血管）
            _volMapper->SetBlendModeToMaximumIntensity();
            _volMapper->SetInputData(_image);
            _ren3D->AddViewProp(_volume);
            break;

        case ViewMode::MinIP:
            // 最小强度投影（突出低密度/气腔）
            _volMapper->SetBlendModeToMinimumIntensity();
            _volMapper->SetInputData(_image);
            _ren3D->AddViewProp(_volume);
            break;

        case ViewMode::AIP:
            // 平均投影（厚层平均）
            _volMapper->SetBlendModeToAverageIntensity();
            _volMapper->SetInputData(_image);
            _ren3D->AddViewProp(_volume);
            break;

        case ViewMode::Isosurface:
            // 等值面：确保表面管线已搭建，并用当前 iso 值更新几何
            ensureSurfacePipeline();
            _contour->SetInputData(_image);
            _ren3D->AddActor(_surfActor);
            break;
    }

    _ren3D->ResetCamera();
    _vtk3D->renderWindow()->Render();
}

void TVtiViewerWidget::ensureSurfacePipeline()
{
    if (!_contour)
    {
        _contour = vtkSmartPointer<vtkContourFilter>::New();
        _surfMapper = vtkSmartPointer<vtkPolyDataMapper>::New();
        _surfActor = vtkSmartPointer<vtkActor>::New();

        _surfMapper->SetInputConnection(_contour->GetOutputPort());
        _surfActor->SetMapper(_surfMapper);

        // 简单灰白材质
        _surfActor->GetProperty()->SetColor(0.9, 0.9, 0.95);
        _surfActor->GetProperty()->SetOpacity(1.0);

        // 初始化 iso 值
        updateIsoBySlider();
    }
}

void TVtiViewerWidget::updateIsoBySlider()
{
    if (!_image || !_sldIso) return;

    // 滑条线性映射到数据范围
    double r[2];
    _image->GetScalarRange(r);
    const double lo = r[0], hi = r[1];
    const double t = (_sldIso->value() - _sldIso->minimum()) /
                     double(std::max(1, _sldIso->maximum() - _sldIso->minimum()));
    const double iso = lo * (1.0 - t) + hi * t;

    if (!_contour) return;
    _contour->SetValue(0, iso);
    _lblIso->setText(QString("Iso: %1").arg(iso, 0, 'f', 2));
}

void TVtiViewerWidget::updateIsoSliderRange()
{
    if (!_image)
    {
        _sldIso->setEnabled(false);
        return;
    }

    // 用标量范围设滑条区间（这里用 0~1000 均匀映射；也可以根据直方图更智能地分配）
    _sldIso->setMinimum(0);
    _sldIso->setMaximum(1000);
    _sldIso->setValue(500); // 初始中位
    _lblIso->setText("Iso: -");
}
