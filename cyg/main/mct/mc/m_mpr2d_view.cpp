#include "m_mpr2d_view.h"

#include <vtkRenderer.h>
#include <vtkCamera.h>
#include <vtkCommand.h>


static bool D_Strace = false;


void Mpr2DView::VtkLogCallback(vtkObject *caller, unsigned long eid, void *client, void *)
{
    auto *self = reinterpret_cast<Mpr2DView *>(client);
    const char *ev = (eid == vtkCommand::RenderEvent) ? "RenderEvent" :
                     (eid == vtkCommand::ModifiedEvent) ? "ModifiedEvent" : "Event";
    qInfo().noquote() << "[" << self->_tag << "]"
                      << caller->GetClassName() << ev
                      << QString("@%1").arg(reinterpret_cast<quintptr>(caller), 0, 16);
}

Mpr2DView::Mpr2DView(QVTKOpenGLNativeWidget *host, AppIds::EAxis axis, QObject *parent, const QString &tag)
        : QObject(parent), _axis(axis), _host(host)
{
    _tag = tag;
    _bound = false;

    buildViewers();

    connectObservers();

    _cell  = new Mpr2DViewCell(axis, this);
    _panel = _cell->createHost(host, _view->GetRenderer(), QString::asprintf("%s_host", AppIds::AxisToLabel(axis)));
}

void Mpr2DView::buildViewers()
{
    // 渲染窗口
    _win = vtkSmartPointer<vtkGenericOpenGLRenderWindow>::New();

    // 三个 ResliceImageViewer
    // 十字线 vtkResliceCursor 外面传入
    _view = vtkSmartPointer<vtkResliceImageViewer>::New();
    _view->SetResliceModeToAxisAligned();

    // 共享的 LUT（可替换为医学灰阶）
    _lut = vtkSmartPointer<vtkLookupTable>::New();

    // 初始滚轮因子
    setSliceScrollFactor(1.0);

    if (D_Strace)
    {
        auto cb = vtkSmartPointer<vtkCallbackCommand>::New();
        cb->SetClientData(this);
        cb->SetCallback(&VtkLogCallback);

        // 谁在渲染
        _view->GetRenderer()->AddObserver(vtkCommand::RenderEvent, cb);

        //    // 谁被改了（Representation 经常在厚层切换/交互时改）
        //    auto* rep = vtkResliceCursorRepresentation::SafeDownCast(
        //            _view->GetResliceCursorWidget()->GetRepresentation());
        //    if (rep) rep->AddObserver(vtkCommand::ModifiedEvent, cb);
    }
}

void Mpr2DView::connectObservers()
{
    // 统一的回调：任何一个 viewer 发生 SliceChanged/Interaction，就刷新另外两个
    _evt = vtkSmartPointer<vtkCallbackCommand>::New();
    _evt->SetClientData(this);
    _evt->SetCallback(&Mpr2DView::vtkEventForwarder);

    // 监听两类事件：SliceChangedEvent(1001) 和 InteractionEvent
    const unsigned long ev1 = static_cast<unsigned long>(vtkResliceImageViewer::SliceChangedEvent);
    const unsigned long ev2 = vtkCommand::InteractionEvent;

    _view->AddObserver(ev1, _evt);
    _view->AddObserver(ev2, _evt);
}

void Mpr2DView::vtkEventForwarder(vtkObject *caller, unsigned long, void *clientData, void *)
{
    Mpr2DView *self = reinterpret_cast<Mpr2DView *>(clientData);
    if (!self) return;

    // 谁触发就刷新其它两个
    vtkResliceImageViewer *who = vtkResliceImageViewer::SafeDownCast(caller);
    if (!who) return;

    if (who != self->_view)
        self->_host->renderWindow()->Render();
}

void Mpr2DView::setImageData(vtkImageData *img)
{
    if (!img) return;
    _image = img;

    _view->SetInputData(_image);

    if (!_bound)
    {
        // 绑定窗口
        _view->SetRenderWindow(_win);

        // 绑定窗口
        _host->setRenderWindow(_win);

        // 在 buildViewers() 末尾，三个 viewer 绑定各自的 interactor：
        _view->SetupInteractor(_win->GetInteractor());

        // 初始滚轮因子
        setSliceScrollFactor(1.0);

        _bound = true;
    }

    applyAxis();

    // 像素值 → 颜色/灰度、统一 LUT 与 WL、可统一 LUT（若你有医学灰阶 LUT，可在这里设置）
    {
        double range[2];
        _image->GetScalarRange(range);
        _lut->SetRange(range[0], range[1]);
        _lut->Build();

        _view->SetLookupTable(_lut);
    }

    // 滚轮因子
    setSliceScrollFactor(_scrollFactor);

    // 自适应相机
    _view->Render();
}

void Mpr2DView::setSharedCursor(vtkResliceCursor *v)
{
    _view->SetResliceCursor(v);
}

void Mpr2DView::setAxis(AppIds::EAxis a)
{
    _axis = a;
    applyAxis();
}

void Mpr2DView::applyAxis()
{
    if (!_view) return;

    if (_view->GetInput() && _view->GetResliceCursor()->GetImage())
    {
        // 三正交方向
        switch (_axis)
        {
            case AppIds::EAxis::X:
                _view->SetSliceOrientationToXY();
                break; // Axial, 观察 Z
            case AppIds::EAxis::Y:
                _view->SetSliceOrientationToXZ();
                break; // Coronal, 观察 Y
            case AppIds::EAxis::Z:
                _view->SetSliceOrientationToYZ();
                break; // Sagittal, 观察 X
        }
        // 初始相机：平行投影（类内部也会设，这里显式一次更直观）
        _view->GetRenderer()->GetActiveCamera()->ParallelProjectionOn();
    }
}

void Mpr2DView::setWL(double win, double lev)
{
    _view->SetColorWindow(win);
    _view->SetColorLevel(lev);
    _host->renderWindow()->Render();
}

void Mpr2DView::setSliceScrollFactor(double f)
{
    if (f <= 0) f = 1.0;
    _scrollFactor = f;
    _view->SetSliceScrollFactor(f);
}

void Mpr2DView::refresh()
{
    _host->renderWindow()->Render();
}

