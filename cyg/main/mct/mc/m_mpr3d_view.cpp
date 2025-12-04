#include "m_mpr3d_view.h"

#include <vtkCamera.h>
#include <vtkProperty.h>


void Mpr3DView::VtkLogCallback(vtkObject *caller, unsigned long eid, void *client, void *)
{
    auto *self = reinterpret_cast<Mpr3DView *>(client);
    const char *ev = (eid == vtkCommand::RenderEvent) ? "RenderEvent" :
                     (eid == vtkCommand::ModifiedEvent) ? "ModifiedEvent" : "Event";
    qInfo().noquote() << "[" << self->_tag << "]"
                      << caller->GetClassName() << ev
                      << QString("@%1").arg(reinterpret_cast<quintptr>(caller), 0, 16);
}

Mpr3DView::Mpr3DView(QVTKOpenGLNativeWidget *host, QObject *parent, const QString &tag)
        : QObject(parent), _host(host)
{
    _win = vtkSmartPointer<vtkGenericOpenGLRenderWindow>::New();
    _ren = vtkSmartPointer<vtkRenderer>::New();
    _ren->SetObjectName(tag.toStdString());
    _win->AddRenderer(_ren);
    _host->setRenderWindow(_win);
    _ren->SetBackground(0.06, 0.06, 0.08);

    // 体渲染最小管线
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

    _tag = tag;
    auto cb = vtkSmartPointer<vtkCallbackCommand>::New();
    cb->SetClientData(this);
    cb->SetCallback(&VtkLogCallback);

    // 谁在渲染
    _ren->AddObserver(vtkCommand::RenderEvent, cb);

    _cell  = new Mpr3DViewCell(AppIds::EAxis::D, this);
    _panel = _cell->createHost(host, _ren, QString::asprintf("%s_host", AppIds::AxisToLabel(AppIds::EAxis::D)));
}

void Mpr3DView::setImageData(vtkImageData *img)
{
    _image = img;
    if (!_image) return;
    _volMapper->SetInputData(_image);
    _volMapper->SetRequestedRenderModeToRayCast();

    double r[2];
    _image->GetScalarRange(r);
    const double lo = r[0], hi = r[1], mid = 0.5 * (lo + hi);
    _ctf->RemoveAllPoints();
    _ctf->AddRGBPoint(lo, 0.0, 0.0, 0.0);
    _ctf->AddRGBPoint(mid, 0.9, 0.7, 0.6);
    _ctf->AddRGBPoint(hi, 1.0, 1.0, 1.0);
    _pwf->RemoveAllPoints();
    _pwf->AddPoint(lo, 0.00);
    _pwf->AddPoint(mid, 0.10);
    _pwf->AddPoint(hi, 0.90);

    apply();
}

void Mpr3DView::setTransferFunctions(vtkColorTransferFunction *ctf, vtkPiecewiseFunction *pwf)
{
    if (ctf) _ctf->DeepCopy(ctf);
    if (pwf) _pwf->DeepCopy(pwf);
    apply();
}

void Mpr3DView::setViewMode(ViewMode m)
{
    _mode = m;
    apply();
}

void Mpr3DView::ensureIsoPipeline()
{
    if (_contour) return;
    _contour = vtkSmartPointer<vtkContourFilter>::New();
    _surfMapper = vtkSmartPointer<vtkPolyDataMapper>::New();
    _surfActor = vtkSmartPointer<vtkActor>::New();
    _surfMapper->SetInputConnection(_contour->GetOutputPort());
    _surfActor->SetMapper(_surfMapper);
    _surfActor->GetProperty()->SetColor(0.92, 0.92, 0.98);
}

void Mpr3DView::apply()
{
    if (!_image) return;
    _ren->RemoveAllViewProps();

    switch (_mode)
    {
        case ViewMode::Composite:
            _volMapper->SetBlendModeToComposite();
            _ren->AddViewProp(_volume);
            break;
        case ViewMode::MIP:
            _volMapper->SetBlendModeToMaximumIntensity();
            _ren->AddViewProp(_volume);
            break;
        case ViewMode::MinIP:
            _volMapper->SetBlendModeToMinimumIntensity();
            _ren->AddViewProp(_volume);
            break;
        case ViewMode::AIP:
            _volMapper->SetBlendModeToAverageIntensity();
            _ren->AddViewProp(_volume);
            break;
        case ViewMode::Isosurface:
            ensureIsoPipeline();
            _contour->SetInputData(_image);
            // 默认阈值设在中位附近；外部可以提供 UI 来调
            double r[2];
            _image->GetScalarRange(r);
            _contour->SetValue(0, 0.5 * (r[0] + r[1]));
            _ren->AddActor(_surfActor);
            break;
    }
    _ren->ResetCamera();
    _host->renderWindow()->Render();
}

void Mpr3DView::resetCamera()
{
    _ren->ResetCamera();
    _host->renderWindow()->Render();
}

