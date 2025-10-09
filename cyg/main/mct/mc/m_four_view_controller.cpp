#include "m_four_view_controller.h"

#include <helpers/filesystem_helper.h>
#include <helpers/vtk_helper.h>
#include <helpers/itk_helper.h>
#include <helpers/log_helper.h>

#include <vtkRenderWindow.h>

FourViewController::FourViewController(QVTKOpenGLNativeWidget *w3D,
                                       QVTKOpenGLNativeWidget *wAx,
                                       QVTKOpenGLNativeWidget *wCo,
                                       QVTKOpenGLNativeWidget *wSa,
                                       QObject *parent)
        : QObject(parent), _w3D(w3D), _wAx(wAx), _wCo(wCo), _wSa(wSa)
{
    // 3D视图对象
    _view3D = std::make_unique<Mpr3DView>(_w3D, this, "_view3D");
    // 三窗
    _axial = std::make_unique<Mpr2DView>(_wAx, Mpr2DView::AX, this, "_axial");
    _coronal = std::make_unique<Mpr2DView>(_wCo, Mpr2DView::AY, this, "_coronal");
    _sagittal = std::make_unique<Mpr2DView>(_wSa, Mpr2DView::AZ, this, "_sagittal");

    // 共享十字线
    _sharedCursor = vtkSmartPointer<vtkResliceCursor>::New();
    // 初始厚层设置（会被 setThickMode() 覆盖）
    _sharedCursor->SetThickMode(0);
    _sharedCursor->SetThickness(10, 10, 10);
    _axial->setSharedCursor(_sharedCursor);
    _coronal->setSharedCursor(_sharedCursor);
    _sagittal->setSharedCursor(_sharedCursor);

    // 默认 滚轮因子
    setSliceScrollFactor(1.0);

    // WL 同步（滑条→控制器→三窗）
    connect(&_wl, &WindowLevelEnhancer::windowLevelChanged,
            [this](double w, double l)
            {
                if (_axial) _axial->setWL(w, l);
                if (_coronal) _coronal->setWL(w, l);
                if (_sagittal)_sagittal->setWL(w, l);
            });

    connectLinkage();
}

void FourViewController::connectLinkage()
{
    // 任一窗交互，刷新其他两窗（避免本窗双刷）
    connect(_axial.get(), &Mpr2DView::sliceOrInteract, [this]
    {
        _wCo->renderWindow()->Render();
        _wSa->renderWindow()->Render();
    });
    connect(_coronal.get(), &Mpr2DView::sliceOrInteract, [this]
    {
        _wAx->renderWindow()->Render();
        _wSa->renderWindow()->Render();
    });
    connect(_sagittal.get(), &Mpr2DView::sliceOrInteract, [this]
    {
        _wAx->renderWindow()->Render();
        _wCo->renderWindow()->Render();
    });
}

bool FourViewController::loadVtiFile(const QString &file)
{
    if (false)
    {
        auto reader = vtkSmartPointer<vtkXMLImageDataReader>::New();
        auto errObs = vtkSmartPointer<CxVTKErrorObserver>::New();
        reader->AddObserver(vtkCommand::ErrorEvent, errObs);

        reader->SetFileName(file.toLocal8Bit().constData());
        reader->Update();

        if (errObs->GetError()) {
//            LogHelper::Warning() << "VTK load error:" << errObs->GetErrorMessage();
            return false;
        }

        setImageData(reader->GetOutput());
    }
    {
        std::string dirPath = "D:\\ct\\data-volumes\\3Res\\sirt";
//        auto files = FileSystemHelper::ScanFiles(dirPath);
//        if (files.empty())
//        {
//            std::cerr << "目录下没有找到 bmp 文件: " << dirPath << std::endl;
//            return EXIT_FAILURE;
//        }
//
//        std::cout << "共找到 " << files.size() << " 张 bmp 切片" << std::endl;

        // 2. 加载堆叠
        auto volumeData = ITKHelper::LoadBMPStack(dirPath, 0, 199);
        setImageData(volumeData);
    }

    emit vtiLoaded(file, _image);
    return true;
}

void FourViewController::setImageData(vtkImageData *img)
{
    _image = img;
    if (!_image) return;

    // 切片游标 cursor
    // 让 cursor 知道图像与中心
    _sharedCursor->SetImage(_image);
    // 初始化 cursor 的切片模式、厚度
    _sharedCursor->SetThickMode(0);
    _sharedCursor->SetThickness(10, 10, 10);
    // 让 cursor 知道图像与中心
    _sharedCursor->SetCenter(_image->GetCenter());

    _axial->setImageData(_image);
    _coronal->setImageData(_image);
    _sagittal->setImageData(_image);

    _view3D->setImageData(_image);

    // 给 WL 一组初值（约等于“软窗”）
    {
        double r[2];
        _image->GetScalarRange(r);
//        const double win = std::max(1.0, lerp((r[1] - r[0]) * 0.05, (r[1] - r[0]) * 1.5,
//                                              _sldWin->value() / 1000.0));
//        const double lev = lerp(r[0], r[1], _sldLev->value() / 1000.0);
//        _wl.setWindowLevel(win, lev);
        // 粗设
        _wl.setWindowLevel(0.8 * (r[1] - r[0]), 0.5 * (r[0] + r[1]));
    }
}

void FourViewController::setViewMode3D(Mpr3DView::ViewMode m)
{
    _view3D->setViewMode(m);
}

void FourViewController::setWindowLevel(double win, double lev)
{
    _wl.setWindowLevel(win, lev);
}

void FourViewController::setThick(bool on, int px)
{
    if (_sharedCursor)
    {
        // 初始厚层设置
        _sharedCursor->SetThickMode(0);
        _sharedCursor->SetThickness(px, px, px);
        refreshAll();
    }
}

void FourViewController::setSliceScrollFactor(double f)
{
    if (_axial) _axial->setSliceScrollFactor(f);
    if (_coronal) _coronal->setSliceScrollFactor(f);
    if (_sagittal)_sagittal->setSliceScrollFactor(f);
}

Mpr2DView *FourViewController::viewByIndex(int which) const
{
    if (which == 0) return _axial.get();
    if (which == 1) return _coronal.get();
    if (which == 2) return _sagittal.get();
    return nullptr;
}

bool FourViewController::exportSliceRendered(int which, const QString &path, int scale)
{
    auto *v = viewByIndex(which);
    if (!v) return false;
    return SliceExporter::saveRenderedWindow(v->renderWindow(), path, scale);
}

bool FourViewController::exportSliceHighQuality(int which, const QString &path,
                                                double spacingXY, int sizeX, int sizeY,
                                                bool linear, bool rescale16)
{
    auto *v = viewByIndex(which);
    if (!v) return false;
    return SliceExporter::exportCurrentSliceData(v->viewer(), path, spacingXY, sizeX, sizeY,
                                                 linear, rescale16);
}

QVector<double> FourViewController::histogram(int bins)
{
    return GrayHistogram::compute(_image, bins, nullptr);
}

void FourViewController::refreshAll()
{
    if (_axial) _axial->refresh();
    if (_coronal) _coronal->refresh();
    if (_sagittal) _sagittal->refresh();
}
