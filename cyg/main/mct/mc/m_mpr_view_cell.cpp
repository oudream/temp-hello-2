#include "m_mpr_view_cell.h"

#include <QPushButton>
#include <QMouseEvent>
#include <vtkAbstractVolumeMapper.h>

#include <vtkImageSlice.h>
#include <vtkImageData.h>
#include <vtkImageMapper3D.h>
#include <vtkMapper.h>


static bool propHasValidInput(vtkProp* p)
{
    if (!p || !p->GetVisibility()) return false;

    // ImageSlice
    if (auto* is = vtkImageSlice::SafeDownCast(p)) {
        if (auto* m = is->GetMapper()) {
            // 有连接或直接的输入对象即视为有效
            if (m->GetInput() || m->GetNumberOfInputConnections(0) > 0) return true;
        }
        return false;
    }

    // PolyActor
    if (auto* a = vtkActor::SafeDownCast(p)) {
        if (auto* m = a->GetMapper()) {
            if (m->GetInput() || m->GetNumberOfInputConnections(0) > 0) return true;
        }
        return false;
    }

    // Volume
    if (auto* v = vtkVolume::SafeDownCast(p)) {
        if (auto* m = v->GetMapper()) {
            if (const auto* input = m->GetInputDataObject(0, 0); input || m->GetNumberOfInputConnections(0) > 0)
                return true;
        }
        return false;
    }

    // 其它类型：用 Bounds 粗判；无输入通常 bounds 会是无效或极值
    const double *b = p->GetBounds();
    const bool boundsOk = !(b[0] >= b[1] || b[2] >= b[3] || b[4] >= b[5]);
    return boundsOk;
}


// ====== InteractorStyleMPR 实现 ======
vtkStandardNewMacro(MprViewCell::InteractorStyleMPR);

void MprViewCell::InteractorStyleMPR::OnMouseWheelForward()
{
    if (this->Interactor && this->Interactor->GetControlKey() && this->ren_)
    {
        auto cam = this->ren_->GetActiveCamera();
        if (cam->GetParallelProjection()) cam->SetParallelScale(cam->GetParallelScale() * 0.9);
        else cam->Zoom(1.0 / 0.9);
        this->ren_->ResetCameraClippingRange();
        this->Interactor->Render();
        return; // 不滚动切片
    }
    this->Superclass::OnMouseWheelForward();
}

void MprViewCell::InteractorStyleMPR::OnMouseWheelBackward()
{
    if (this->Interactor && this->Interactor->GetControlKey() && this->ren_)
    {
        auto cam = this->ren_->GetActiveCamera();
        if (cam->GetParallelProjection()) cam->SetParallelScale(cam->GetParallelScale() * 1.1);
        else cam->Zoom(1.0 / 1.1);
        this->ren_->ResetCameraClippingRange();
        this->Interactor->Render();
        return;
    }
    this->Superclass::OnMouseWheelBackward();
}

void MprViewCell::InteractorStyleMPR::OnChar()
{
    if (this->Interactor && this->ren_)
    {
        const auto kc = this->Interactor->GetKeyCode();
        if (kc == 'r' || kc == 'R')
        {
            this->ren_->ResetCamera();
            this->ren_->ResetCameraClippingRange();
            this->Interactor->Render();
            return;
        }
    }
    this->Superclass::OnChar();
}

// ====== CtrlWheelFilter 实现 ======
MprViewCell::CtrlWheelFilter::CtrlWheelFilter(MprViewCell *owner)
        : QObject(owner), owner_(owner)
{}

bool MprViewCell::CtrlWheelFilter::eventFilter(QObject *obj, QEvent *e)
{
    if (e->type() != QEvent::Wheel) return QObject::eventFilter(obj, e);
    auto *we = dynamic_cast<QWheelEvent *>(e);
    if (!(we->modifiers() & Qt::ControlModifier)) return QObject::eventFilter(obj, e);

    const QPoint delta = we->angleDelta();
    if (delta.y() == 0) return true;

    const double steps = static_cast<double>(delta.y()) / 120.0;
    const double factorPerStep = (steps > 0 ? 0.9 : 1.1);
    const double factor = std::pow(factorPerStep, std::abs(steps));
    owner_->zoomRenderer(factor);

    if (auto *cam = owner_->_renderer->GetActiveCamera())
    {
        if (owner_->state_.baseParallelScale <= 0.0)
            owner_->state_.baseParallelScale = cam->GetParallelScale();
        owner_->state_.zoom = owner_->state_.baseParallelScale / cam->GetParallelScale();
        owner_->updateScrollBars();
    }
    we->accept();
    return true; // 吃掉，避免 VTK 再处理
}

// ====== MprViewCell 主体 ======
MprViewCell::MprViewCell(EAxis axis, QObject *parent)
    : QObject(parent), _axis(axis), _view(nullptr)
{
    _title = AppIds::AxisToLabel(axis);
}

QWidget* MprViewCell::createHost(QVTKOpenGLNativeWidget *view, vtkRenderer *renderer, const QString &hostObjectName)
{
    _root = view ? view->parentWidget() : nullptr;
    _view = view;
    _renderer = renderer;
    _hostPanel = new QWidget(_root);

    if (!_hostPanel || !_view || !_renderer) return nullptr;

    if (!hostObjectName.isEmpty()) _hostPanel->setObjectName(hostObjectName);

    auto *hostLayout = new QVBoxLayout(_hostPanel);
    hostLayout->setContentsMargins(0, 0, 0, 0);
    hostLayout->setSpacing(4);

    // 1) 视图行：左 QVTK 视图 + 右 竖滚动条（与视图等高）
    auto *vRow = new QWidget(_hostPanel);
    auto *vRowLayout = new QHBoxLayout(vRow);
    vRowLayout->setContentsMargins(0, 0, 0, 0);
    vRowLayout->setSpacing(0);

    vRowLayout->addWidget(_view, /*stretch*/1);

    _vScroll = new QScrollBar(Qt::Vertical, vRow);
    _vScroll->setObjectName("vScroll");                     // 供 attachControls 查找
    _vScroll->setRange(-100, 100);
    _vScroll->setValue(0);
    _vScroll->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);
    _vScroll->setFixedWidth(_vScroll->sizeHint().width());   // 避免被挤没
    vRowLayout->addWidget(_vScroll, /*stretch*/0);

    // 视图行
    hostLayout->addWidget(vRow, /*stretch*/1);

    // 2) 底部控件行（标题 + 水平滚动条 + 放大/缩小/适应）
    auto *hRow = buildControlRow();
    hostLayout->addWidget(hRow, /*stretch*/1);

    // 3) 事件连接：竖滚联动
    connect(_vScroll, &QScrollBar::valueChanged, this, &MprViewCell::onPanV);

    // 4) Ctrl+滚轮缩放
    installCtrlWheelZoom();

    // 5) 如果需要，装 MPR 交互风格（按你之前的开关/策略调用）
    // setMprInteractorStyleFor(view_, renderer_);

    return _hostPanel;
}

void MprViewCell::installCtrlWheelZoom()
{
    if (!_view) return;
    if (!_view->property("ctrlWheelInstalled").toBool())
    {
        _view->installEventFilter(new CtrlWheelFilter(this));
        _view->setProperty("ctrlWheelInstalled", true);
    }
}

void MprViewCell::setMprInteractorStyleFor(const QVTKOpenGLNativeWidget *view, vtkRenderer *renderer)
{
    if (!view || !renderer) return;
    const auto style = vtkSmartPointer<InteractorStyleMPR>::New();
    style->SetRenderer(renderer);
    view->interactor()->SetInteractorStyle(style);
}

QWidget* MprViewCell::buildControlRow()
{
    auto* row = new QWidget(_hostPanel);
    auto* hLay = new QHBoxLayout(row);
    hLay->setContentsMargins(0, 0, 0, 0);
    hLay->setSpacing(6);

    auto* lbl = new QLabel(_title, row);
    lbl->setMinimumWidth(48);
    hLay->addWidget(lbl);

    _hScroll = new QScrollBar(Qt::Horizontal, row);
    _hScroll->setRange(-100, 100);
    _hScroll->setValue(0);
    _hScroll->setToolTip(tr("左右平移"));
    hLay->addWidget(_hScroll, 1);

    auto* btnZoomIn  = new QPushButton(tr("放大"), row);
    auto* btnZoomOut = new QPushButton(tr("缩小"), row);
    auto* btnFit     = new QPushButton(tr("适应"), row);
    hLay->addWidget(btnZoomIn);
    hLay->addWidget(btnZoomOut);
    hLay->addWidget(btnFit);

    // 行高与水平滚动条一致，避免“过高”
    row->setFixedHeight(_hScroll->sizeHint().height());

    // 信号槽
    connect(_hScroll, &QScrollBar::valueChanged, this, &MprViewCell::onPanH);
    connect(btnZoomIn,  &QPushButton::clicked, this, &MprViewCell::onZoomIn);
    connect(btnZoomOut, &QPushButton::clicked, this, &MprViewCell::onZoomOut);
    connect(btnFit,     &QPushButton::clicked, this, &MprViewCell::onFit);

    return row;
}

void MprViewCell::updateScrollBars() const
{
    if (!_hostPanel) return;
    const double z = qMax(1.0, state_.zoom);
    const int maxVal = static_cast<int>((z - 1.0) * 100.0);
    auto apply = [maxVal](QScrollBar *sb)
    {
        if (!sb) return;
        sb->blockSignals(true);
        sb->setRange(-maxVal, maxVal);
        sb->setSingleStep(qMax(1, maxVal / 10));
        sb->setPageStep(qMax(1, maxVal / 3));
        if (maxVal == 0) sb->setValue(0);
        sb->setEnabled(maxVal > 0);
        sb->blockSignals(false);
    };
    apply(_hScroll);
    apply(_vScroll);
}

void MprViewCell::onPanH(int val)
{
    const int dv = val - state_.lastH;
    state_.lastH = val;
    const double ndx = dv / 100.0;
    if (ndx != 0.0) panRenderer(ndx, 0.0);
}

void MprViewCell::onPanV(int val)
{
    const int dv = val - state_.lastV;
    state_.lastV = val;
    const double ndy = -dv / 100.0;
    if (ndy != 0.0) panRenderer(0.0, ndy);
}

void MprViewCell::onZoomIn()
{
    zoomRenderer(0.9);
    if (auto *cam = _renderer->GetActiveCamera())
    {
        if (state_.baseParallelScale <= 0.0) state_.baseParallelScale = cam->GetParallelScale();
        state_.zoom = state_.baseParallelScale / cam->GetParallelScale();
        updateScrollBars();
    }
}

void MprViewCell::onZoomOut()
{
    zoomRenderer(1.1);
    if (auto *cam = _renderer->GetActiveCamera())
    {
        if (state_.baseParallelScale <= 0.0) state_.baseParallelScale = cam->GetParallelScale();
        state_.zoom = state_.baseParallelScale / cam->GetParallelScale();
        updateScrollBars();
    }
}

void MprViewCell::onFit()
{
    if (_hScroll) _hScroll->setValue(0);
    if (_vScroll) _vScroll->setValue(0);
    state_.lastH = state_.lastV = 0;

    fitRenderer();
    if (auto *cam = _renderer->GetActiveCamera())
    {
        state_.baseParallelScale = cam->GetParallelScale();
    }
    state_.zoom = 1.0;
    updateScrollBars();
}

// —— 渲染器数学（与你现有实现保持一致的核心逻辑） —— //
void MprViewCell::panRenderer(double ndx, double ndy) const
{
    if (!_renderer) return;
    auto *cam = _renderer->GetActiveCamera();
    if (!cam) return;

    if (!hasRenderableContent()) return; // ★ 没有内容就不平移也不渲染

    int *pSz = _renderer->GetRenderWindow()->GetSize();
    const double w = std::max(1, pSz[0]), h = std::max(1, pSz[1]);
    const double aspect = w / h;
    const double scale = cam->GetParallelProjection() ? cam->GetParallelScale() : 1.0;

    double fp[3], pos[3], vu[3];
    cam->GetFocalPoint(fp);
    cam->GetPosition(pos);
    cam->GetViewUp(vu);

    double vd[3] = {fp[0] - pos[0], fp[1] - pos[1], fp[2] - pos[2]};
    double right[3] = {vd[1] * vu[2] - vd[2] * vu[1],
                       vd[2] * vu[0] - vd[0] * vu[2],
                       vd[0] * vu[1] - vd[1] * vu[0]};

    auto norm = [](double v[3])
    {
        const double L = std::sqrt(v[0] * v[0] + v[1] * v[1] + v[2] * v[2]);
        if (L < 1e-12) return;
        v[0] /= L;
        v[1] /= L;
        v[2] /= L;
    };
    norm(vu);
    norm(right);

    const double dx = ndx * 2.0 * scale * aspect;
    const double dy = ndy * 2.0 * scale;
    const double delta[3] = {
            right[0] * dx + vu[0] * dy,
            right[1] * dx + vu[1] * dy,
            right[2] * dx + vu[2] * dy
    };

    for (int i = 0; i < 3; ++i)
    {
        fp[i] += delta[i];
        pos[i] += delta[i];
    }
    cam->SetFocalPoint(fp);
    cam->SetPosition(pos);
    _renderer->ResetCameraClippingRange();
    _renderer->GetRenderWindow()->Render();
}

void MprViewCell::zoomRenderer(double factor) const
{
    if (!_renderer) return;
    auto *cam = _renderer->GetActiveCamera();
    if (!cam) return;

    if (cam->GetParallelProjection())
        cam->SetParallelScale(cam->GetParallelScale() * factor);
    else
        cam->Zoom(1.0 / factor);

    _renderer->ResetCameraClippingRange();
    _renderer->GetRenderWindow()->Render();
}

void MprViewCell::fitRenderer() const
{
    if (!_renderer) return;
    _renderer->ResetCamera();
    _renderer->ResetCameraClippingRange();
    _renderer->GetRenderWindow()->Render();
}

bool MprViewCell::hasRenderableContent() const
{
    if (!_renderer) return false;
    auto* rw = _renderer->GetRenderWindow();
    if (!rw) return false;

    // 窗口尺寸有效
    if (const int *sz = rw->GetSize(); sz[0] <= 1 || sz[1] <= 1) return false;

    // 有可见并且 mapper 接好输入的 Prop
    vtkPropCollection* props = _renderer->GetViewProps();
    if (!props) return false;

    // props->InitTraversal();
    // while (vtkProp* p = props->GetNextProp()) {
    //     if (propHasValidInput(p)) return true;
    // }
    return true;
}
