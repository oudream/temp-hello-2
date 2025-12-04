#include "u_scene_page.h"

#include "dialogs/u_dialog_helper.h"
#include "base/app_ids.h"

#include <QScrollBar>
#include <QToolButton>


MScenePage::MScenePage(QWidget *parent) : UPageBase(parent), _root(nullptr)
{
    buildUi(parent);
    wireUp();
}

MScenePage::~MScenePage()
{

}

const char *MScenePage::getName() const
{
    return AppIds::kBlock_Editor;
}

QWidget *MScenePage::getWidget()
{
    return _root;
}

void MScenePage::onEnter()
{

}

void MScenePage::onLeave()
{

}

void MScenePage::buildUi(QWidget *parent)
{
    _root = new QWidget(parent);

    auto *vbox = new QVBoxLayout(_root);
    vbox->setContentsMargins(0, 0, 0, 0);
    vbox->setSpacing(4);

    // ========== 顶部工具条 ==========
    _bar = new QHBoxLayout();
    _bar->setSpacing(8);

    _btnLoad = new QPushButton(u8"载入VTI");
    _cmb3DMode = new QComboBox();
    _cmb3DMode->addItems({u8"Composite", u8"MIP", u8"MinIP", u8"AIP", u8"Isosurface"});

    _lblWin = new QLabel(u8"W:");
    _sldWin = new QSlider(Qt::Horizontal);
    _sldWin->setRange(1, 10000);
    _sldWin->setValue(1500);
    _spnWin = new QDoubleSpinBox();
    _spnWin->setRange(1, 100000);
    _spnWin->setDecimals(1);
    _spnWin->setValue(1500);

    _lblLev = new QLabel(u8"L:");
    _sldLev = new QSlider(Qt::Horizontal);
    _sldLev->setRange(-5000, 5000);
    _sldLev->setValue(300);
    _spnLev = new QDoubleSpinBox();
    _spnLev->setRange(-100000, 100000);
    _spnLev->setDecimals(1);
    _spnLev->setValue(300);

    _btnPresetBone = new QPushButton(u8"骨窗");
    _btnPresetSoft = new QPushButton(u8"软窗");
    _btnPresetMetal = new QPushButton(u8"金属");

    _chkThick = new QCheckBox(u8"厚层");
    _spnThickPx = new QSpinBox();
    _spnThickPx->setRange(1, 999);
    _spnThickPx->setValue(10);
    _spnThickPx->setEnabled(false);

    _lblScroll = new QLabel(u8"滚轮×");
    _spnScroll = new QDoubleSpinBox();
    _spnScroll->setRange(0.1, 10.0);
    _spnScroll->setSingleStep(0.1);
    _spnScroll->setValue(1.0);

    _btnExportAxialRendered = new QPushButton(u8"导出Axial-所见");
    _btnExportAxialHQ = new QPushButton(u8"导出Axial-高质");

    // 布置
    _bar->addWidget(_btnLoad);
    _bar->addSpacing(8);
    _bar->addWidget(new QLabel(u8"3D模式:"));
    _bar->addWidget(_cmb3DMode);
    _bar->addSpacing(12);
    _bar->addWidget(_lblWin);
    _bar->addWidget(_sldWin);
    _bar->addWidget(_spnWin);
    _bar->addSpacing(6);
    _bar->addWidget(_lblLev);
    _bar->addWidget(_sldLev);
    _bar->addWidget(_spnLev);
    _bar->addWidget(_btnPresetBone);
    _bar->addWidget(_btnPresetSoft);
    _bar->addWidget(_btnPresetMetal);
    _bar->addSpacing(12);
    _bar->addWidget(_chkThick);
    _bar->addWidget(_spnThickPx);
    _bar->addSpacing(12);
    _bar->addWidget(_lblScroll);
    _bar->addWidget(_spnScroll);
    _bar->addStretch();
    _bar->addWidget(_btnExportAxialRendered);
    _bar->addWidget(_btnExportAxialHQ);

    vbox->addLayout(_bar);

    // ========== 2×2 视窗 ==========
    _rootLayout = new QGridLayout();
    _rootLayout->setContentsMargins(0, 0, 0, 0);
    _rootLayout->setSpacing(2);
    _host = new QWidget(_root);
    _rootLayout->addWidget(_host, 0, 0);

    // 交给 FourViewController 来装配 3D + 三窗 2D、共享光标、WL 广播、联动等
    _w3D = new QVTKOpenGLNativeWidget(_root);
    _wAx = new QVTKOpenGLNativeWidget(_root);
    _wCo = new QVTKOpenGLNativeWidget(_root);
    _wSa = new QVTKOpenGLNativeWidget(_root);
    _controller = new MSceneVolume(_w3D, _wAx, _wCo, _wSa, _host);
    _controller->reLayout("grid_2x2", _host);

    vbox->addLayout(_rootLayout);
}

void MScenePage::wireUp()
{
    // —— 3D 模式 ——（五种，与你的枚举完全一致）
    connect(_cmb3DMode, &QComboBox::currentIndexChanged, this, [this](int idx)
    {
        if (!_controller) return;
        _controller->setViewMode3D(static_cast<Mpr3DView::ViewMode>(idx));
    });

    // —— 窗宽窗位：滑条与数值框互绑，并下发到控制器（控制器会广播到三窗 2D）——
    connect(_sldWin, &QSlider::valueChanged, this, [this](int)
    {
        updateWlFromUi();
    });

    connect(_sldLev, &QSlider::valueChanged, this, [this](int)
    {
        updateWlFromUi();
    });

    connect(_spnWin, qOverload<double>(&QDoubleSpinBox::valueChanged), this, [this](double)
    {
        updateWlFromUi();
    });

    connect(_spnLev, qOverload<double>(&QDoubleSpinBox::valueChanged), this, [this](double)
    {
        updateWlFromUi();
    });

    // 预设（骨窗/软窗/金属）通过控制器内部的 WL 增强器广播到三窗 2D
    connect(_btnPresetBone, &QPushButton::clicked, this, [this]
    {
        if (_controller)
        {
            _controller->setWindowLevel(2000, 300);
            applyWlToUi(2000, 300);
        }
    });

    connect(_btnPresetSoft, &QPushButton::clicked, this, [this]
    {
        if (_controller)
        {
            _controller->setWindowLevel(400, 40);
            applyWlToUi(400, 40);
        }
    });

    connect(_btnPresetMetal, &QPushButton::clicked, this, [this]
    {
        if (_controller)
        {
            _controller->setWindowLevel(4000, 800);
            applyWlToUi(4000, 800);
        }
    });

    // —— 厚层 / 滚轮步进 ——（作用于三窗 2D）
    connect(_chkThick, &QCheckBox::toggled, this, [this](bool on)
    {
        _spnThickPx->setEnabled(on);
        if (_controller) _controller->setThick(on, _spnThickPx->value());
    });

    connect(_spnThickPx, qOverload<int>(&QSpinBox::valueChanged), this, [this](int px)
    {
        if (_controller) _controller->setThick(_chkThick->isChecked(), px);
    });

    connect(_spnScroll, qOverload<double>(&QDoubleSpinBox::valueChanged), this, [this](double f)
    {
        if (_controller) _controller->setSliceScrollFactor(f);
    });

    // —— 载入影像（示例：VTI）——
    connect(_btnLoad, &QPushButton::clicked, this, [this]
    {
        const QString path = QFileDialog::getOpenFileName(_root, u8"打开 VTI", QString(), "VTK Image (*.vti)");
        if (path.isEmpty()) return;
        if (!loadVtiFile(path))
        {
            DialogHelper::warning(_root, u8"加载失败", u8"无法读取：\n" + path);
        }
    });

    // —— 导出 Axial ——（which=0）
    connect(_btnExportAxialRendered, &QPushButton::clicked, this, [this]
    {
        const QString path = QFileDialog::getSaveFileName(_root, u8"导出Axial-所见", QString(),
                                                          "PNG (*.png);;JPEG (*.jpg)");
        if (path.isEmpty()) return;
        if (!exportAxialRendered(path, 2))
        {
            // 示例：scale=2
            DialogHelper::warning(_root, u8"导出失败", u8"所见即所得导出失败。");
        }
    });

    connect(_btnExportAxialHQ, &QPushButton::clicked, this, [this]
    {
        const QString path = QFileDialog::getSaveFileName(_root, u8"导出Axial-高质", QString(),
                                                          "TIFF (*.tiff);;PNG (*.png)");
        if (path.isEmpty()) return;
        // 示例：spacingXY 自动，size 自动，线性插值，16-bit 拉伸
        if (!exportAxialHighQuality(path, -1.0, -1, -1, true, true))
        {
            DialogHelper::warning(_root, u8"导出失败", u8"高质量导出失败。");
        }
    });

    // 初始化一组 WL 到 UI
    applyWlToUi(1500.0, 300.0);
}

bool MScenePage::loadVtiFile(const QString &file)
{
    return _controller && _controller->loadVtiFile(file);
}

void MScenePage::setImageData(vtkImageData *img)
{
    if (_controller)
    {

    }
    _controller->setImageData(img);
}

void MScenePage::set3DViewMode(Mpr3DView::ViewMode m)
{
    if (!_controller) return;
    {
        _controller->setViewMode3D(m);
    }
}

void MScenePage::setWindowLevel(double win, double lev)
{
    if (_controller)
    {
        _controller->setWindowLevel(win, lev);
    }
    applyWlToUi(win, lev);
}

bool MScenePage::exportAxialRendered(const QString &path, int scale)
{
    return _controller && _controller->exportSliceRendered(0, path, scale);
}

bool MScenePage::exportAxialHighQuality(const QString &path,
                                        double spacingXY, int sizeX, int sizeY,
                                        bool linear, bool rescale16)
{
    return _controller && _controller->exportSliceHighQuality(0, path, spacingXY, sizeX, sizeY,
                                                              linear, rescale16);
}

void MScenePage::applyWlToUi(double w, double l)
{
    const QSignalBlocker b1(_sldWin), b2(_sldLev), b3(_spnWin), b4(_spnLev);
    _sldWin->setValue(int(std::round(w)));
    _sldLev->setValue(int(std::round(l)));
    _spnWin->setValue(w);
    _spnLev->setValue(l);
}

void MScenePage::updateWlFromUi()
{
    const double w = _spnWin->value();
    const double l = _spnLev->value();
    {
        const QSignalBlocker b1(_sldWin), b2(_sldLev);
        _sldWin->setValue(int(std::round(w)));
        _sldLev->setValue(int(std::round(l)));
    }
    if (_controller) _controller->setWindowLevel(w, l);
}

