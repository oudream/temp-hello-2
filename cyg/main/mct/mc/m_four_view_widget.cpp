#include "m_four_view_widget.h"

#include <QGridLayout>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QVTKOpenGLNativeWidget.h>
#include <QComboBox>
#include <QCheckBox>
#include <QSlider>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QPushButton>
#include <QLabel>
#include <QFileDialog>
#include <QMessageBox>

#include "m_four_view_controller.h"   // FourViewController（装配/联动/导出等）
#include "m_mpr3d_view.h"             // Mpr3DView::ViewMode

MFourViewWidget::MFourViewWidget(QWidget *parent)
        : QWidget(parent)
{
    buildUi();
    wireUp();
}

MFourViewWidget::~MFourViewWidget() = default;

void MFourViewWidget::buildUi()
{
    auto *vbox = new QVBoxLayout(this);
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

    _w3D = new QVTKOpenGLNativeWidget(this);
    _wAx = new QVTKOpenGLNativeWidget(this);
    _wCo = new QVTKOpenGLNativeWidget(this);
    _wSa = new QVTKOpenGLNativeWidget(this);

    _rootLayout->addWidget(_wAx, 0, 0);
    _rootLayout->addWidget(_wCo, 0, 1);
    _rootLayout->addWidget(_wSa, 1, 0);
    _rootLayout->addWidget(_w3D, 1, 1);

    vbox->addLayout(_rootLayout);
    setLayout(vbox);
}

void MFourViewWidget::wireUp()
{
    // 交给 FourViewController 来装配 3D + 三窗 2D、共享光标、WL 广播、联动等
    _controller = std::make_unique<FourViewController>(_w3D, _wAx, _wCo, _wSa,
                                                       this); // :contentReference[oaicite:10]{index=10}

    // —— 3D 模式 ——（五种，与你的枚举完全一致）
    connect(_cmb3DMode, &QComboBox::currentIndexChanged, this, [this](int idx)
    {
        if (!_controller) return;
        _controller->setViewMode3D(
                static_cast<Mpr3DView::ViewMode>(idx)); // :contentReference[oaicite:11]{index=11} :contentReference[oaicite:12]{index=12}
    });

    // —— 窗宽窗位：滑条与数值框互绑，并下发到控制器（控制器会广播到三窗 2D）——
    connect(_sldWin, &QSlider::valueChanged, this, [this](int)
    { updateWlFromUi(); });
    connect(_sldLev, &QSlider::valueChanged, this, [this](int)
    { updateWlFromUi(); });
    connect(_spnWin, qOverload<double>(&QDoubleSpinBox::valueChanged), this, [this](double)
    { updateWlFromUi(); });
    connect(_spnLev, qOverload<double>(&QDoubleSpinBox::valueChanged), this, [this](double)
    { updateWlFromUi(); });

    // 预设（骨窗/软窗/金属）通过控制器内部的 WL 增强器广播到三窗 2D
    connect(_btnPresetBone, &QPushButton::clicked, this, [this]
    {
        if (_controller)
        {
            _controller->setWindowLevel(2000, 300);
            applyWlToUi(2000, 300);
        }
    }); // :contentReference[oaicite:13]{index=13}
    connect(_btnPresetSoft, &QPushButton::clicked, this, [this]
    {
        if (_controller)
        {
            _controller->setWindowLevel(400, 40);
            applyWlToUi(400, 40);
        }
    });   // :contentReference[oaicite:14]{index=14}
    connect(_btnPresetMetal, &QPushButton::clicked, this, [this]
    {
        if (_controller)
        {
            _controller->setWindowLevel(4000, 800);
            applyWlToUi(4000, 800);
        }
    }); // :contentReference[oaicite:15]{index=15}

    // —— 厚层 / 滚轮步进 ——（作用于三窗 2D）
    connect(_chkThick, &QCheckBox::toggled, this, [this](bool on)
    {
        _spnThickPx->setEnabled(on);
        if (_controller) _controller->setThick(on, _spnThickPx->value()); // :contentReference[oaicite:17]{index=17}
    });
    connect(_spnThickPx, qOverload<int>(&QSpinBox::valueChanged), this, [this](int px)
    {
        if (_controller) _controller->setThick(_chkThick->isChecked(), px); // :contentReference[oaicite:18]{index=18}
    });
    connect(_spnScroll, qOverload<double>(&QDoubleSpinBox::valueChanged), this, [this](double f)
    {
        if (_controller) _controller->setSliceScrollFactor(f); // :contentReference[oaicite:19]{index=19}
    });

    // —— 载入影像（示例：VTI）——
    connect(_btnLoad, &QPushButton::clicked, this, [this]
    {

        const QString path = QFileDialog::getOpenFileName(this, u8"打开 VTI", QString(), "VTK Image (*.vti)");
        if (path.isEmpty()) return;
        if (!loadVtiFile(path))
        {
            QMessageBox::warning(this, u8"加载失败", u8"无法读取：\n" + path);
        }
    });

    // —— 导出 Axial ——（which=0）
    connect(_btnExportAxialRendered, &QPushButton::clicked, this, [this]
    {
        const QString path = QFileDialog::getSaveFileName(this, u8"导出Axial-所见", QString(),
                                                          "PNG (*.png);;JPEG (*.jpg)");
        if (path.isEmpty()) return;
        if (!exportAxialRendered(path, 2))
        { // 示例：scale=2
            QMessageBox::warning(this, u8"导出失败", u8"所见即所得导出失败。");
        }
    });
    connect(_btnExportAxialHQ, &QPushButton::clicked, this, [this]
    {
        const QString path = QFileDialog::getSaveFileName(this, u8"导出Axial-高质", QString(),
                                                          "TIFF (*.tiff);;PNG (*.png)");
        if (path.isEmpty()) return;
        // 示例：spacingXY 自动，size 自动，线性插值，16-bit 拉伸
        if (!exportAxialHighQuality(path, -1.0, -1, -1, true, true))
        {
            QMessageBox::warning(this, u8"导出失败", u8"高质量导出失败。");
        }
    });

    // 初始化一组 WL 到 UI
    applyWlToUi(1500.0, 300.0);
}

bool MFourViewWidget::loadVtiFile(const QString &file)
{
    return _controller && _controller->loadVtiFile(file);            // :contentReference[oaicite:20]{index=20}
}

void MFourViewWidget::setImageData(vtkImageData *img)
{
    if (_controller) _controller->setImageData(img);                         // :contentReference[oaicite:21]{index=21}
}

void MFourViewWidget::setViewMode3D(ViewMode m)
{
    if (!_controller) return;
    _controller->setViewMode3D(static_cast<Mpr3DView::ViewMode>(m));         // :contentReference[oaicite:22]{index=22}
}

void MFourViewWidget::setWindowLevel(double win, double lev)
{
    if (_controller)
        _controller->setWindowLevel(win, lev);                  // 广播到三窗 2D :contentReference[oaicite:23]{index=23}
    applyWlToUi(win, lev);
}

bool MFourViewWidget::exportAxialRendered(const QString &path, int scale)
{
    return _controller && _controller->exportSliceRendered(0, path, scale); // Axial=0 :contentReference[oaicite:24]{index=24}
}

bool MFourViewWidget::exportAxialHighQuality(const QString &path,
                                             double spacingXY, int sizeX, int sizeY,
                                             bool linear, bool rescale16)
{
    return _controller && _controller->exportSliceHighQuality(0, path, spacingXY, sizeX, sizeY,
                                                              linear, rescale16);      // Axial=0 :contentReference[oaicite:25]{index=25}
}

void MFourViewWidget::applyWlToUi(double w, double l)
{
    const QSignalBlocker b1(_sldWin), b2(_sldLev), b3(_spnWin), b4(_spnLev);
    _sldWin->setValue(int(std::round(w)));
    _sldLev->setValue(int(std::round(l)));
    _spnWin->setValue(w);
    _spnLev->setValue(l);
}

void MFourViewWidget::updateWlFromUi()
{
    const double w = _spnWin->value();
    const double l = _spnLev->value();
    {
        const QSignalBlocker b1(_sldWin), b2(_sldLev);
        _sldWin->setValue(int(std::round(w)));
        _sldLev->setValue(int(std::round(l)));
    }
    if (_controller) _controller->setWindowLevel(w, l);                       // :contentReference[oaicite:26]{index=26}
}
