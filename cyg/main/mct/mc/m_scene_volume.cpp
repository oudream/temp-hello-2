#include "m_scene_volume.h"

#include <helpers/filesystem_helper.h>
#include <helpers/vtk_helper.h>
#include <helpers/itk_helper.h>
#include <helpers/log_helper.h>

#include <vtkRenderWindow.h>
#include <QGridLayout>


MSceneVolume::MSceneVolume(QVTKOpenGLNativeWidget *w3D,
                           QVTKOpenGLNativeWidget *wAx,
                           QVTKOpenGLNativeWidget *wCo,
                           QVTKOpenGLNativeWidget *wSa,
                           QWidget *parent)
        : QObject(parent), _w3D(w3D), _wAx(wAx), _wCo(wCo), _wSa(wSa)
{
    // 三窗
    _viewX = new Mpr2DView(_wAx, AppIds::EAxis::X, parent, QString::asprintf("View%s", AppIds::AxisToLabel(AppIds::EAxis::X)));
    _viewY = new Mpr2DView(_wCo, AppIds::EAxis::Y, parent, QString::asprintf("View%s", AppIds::AxisToLabel(AppIds::EAxis::Y)));
    _viewZ = new Mpr2DView(_wSa, AppIds::EAxis::Z, parent, QString::asprintf("View%s", AppIds::AxisToLabel(AppIds::EAxis::Z)));
    // 3D视图对象
    _view3D = new Mpr3DView(_w3D, parent, QString::asprintf("View%s", AppIds::AxisToLabel(AppIds::EAxis::D)));

    // 共享十字线
    _sharedCursor = vtkSmartPointer<vtkResliceCursor>::New();
    // 初始厚层设置（会被 setThickMode() 覆盖）
    _sharedCursor->SetThickMode(0);
    _sharedCursor->SetThickness(10, 10, 10);
    _viewX->setSharedCursor(_sharedCursor);
    _viewY->setSharedCursor(_sharedCursor);
    _viewZ->setSharedCursor(_sharedCursor);

    // 默认 滚轮因子
    setSliceScrollFactor(1.0);

    // WL 同步（滑条→控制器→三窗）
    connect(&_wl, &WindowLevelEnhancer::windowLevelChanged,
            [this](double w, double l)
            {
                if (_viewX) _viewX->setWL(w, l);
                if (_viewY) _viewY->setWL(w, l);
                if (_viewZ)_viewZ->setWL(w, l);
            });

    connectLinkage();
}

void MSceneVolume::connectLinkage()
{
    // 任一窗交互，刷新其他两窗（避免本窗双刷）
    connect(_viewX.get(), &Mpr2DView::sliceOrInteract, [this]
    {
        _wCo->renderWindow()->Render();
        _wSa->renderWindow()->Render();
    });
    connect(_viewY.get(), &Mpr2DView::sliceOrInteract, [this]
    {
        _wAx->renderWindow()->Render();
        _wSa->renderWindow()->Render();
    });
    connect(_viewZ.get(), &Mpr2DView::sliceOrInteract, [this]
    {
        _wAx->renderWindow()->Render();
        _wCo->renderWindow()->Render();
    });
}

bool MSceneVolume::loadVtiFile(const QString &file)
{
    if (false)
    {
        auto reader = vtkSmartPointer<vtkXMLImageDataReader>::New();
        auto errObs = vtkSmartPointer<CxVTKErrorObserver>::New();
        reader->AddObserver(vtkCommand::ErrorEvent, errObs);

        reader->SetFileName(file.toLocal8Bit().constData());
        reader->Update();

        if (errObs->GetError())
        {
            LogHelper::warning() << "VTK load error:" << errObs->GetErrorMessage();
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

void MSceneVolume::setImageData(vtkImageData *img)
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

    _viewX->setImageData(_image);
    _viewY->setImageData(_image);
    _viewZ->setImageData(_image);

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

void MSceneVolume::setViewMode3D(Mpr3DView::ViewMode m)
{
    _view3D->setViewMode(m);
}

void MSceneVolume::setWindowLevel(double win, double lev)
{
    _wl.setWindowLevel(win, lev);
}

void MSceneVolume::setThick(bool on, int px)
{
    if (_sharedCursor)
    {
        // 初始厚层设置
        _sharedCursor->SetThickMode(0);
        _sharedCursor->SetThickness(px, px, px);
        refreshAll();
    }
}

void MSceneVolume::setSliceScrollFactor(double f)
{
    if (_viewX) _viewX->setSliceScrollFactor(f);
    if (_viewY) _viewY->setSliceScrollFactor(f);
    if (_viewZ)_viewZ->setSliceScrollFactor(f);
}

Mpr2DView *MSceneVolume::viewByIndex(int which) const
{
    if (which == 0) return _viewX.get();
    if (which == 1) return _viewY.get();
    if (which == 2) return _viewZ.get();
    return nullptr;
}

bool MSceneVolume::exportSliceRendered(int which, const QString &path, int scale)
{
    auto *v = viewByIndex(which);
    if (!v) return false;
    return SliceExporter::saveRenderedWindow(v->renderWindow(), path, scale);
}

bool MSceneVolume::exportSliceHighQuality(int which, const QString &path,
                                          double spacingXY, int sizeX, int sizeY,
                                          bool linear, bool rescale16)
{
    auto *v = viewByIndex(which);
    if (!v) return false;
    return SliceExporter::exportCurrentSliceData(v->viewer(), path, spacingXY, sizeX, sizeY,
                                                 linear, rescale16);
}

QVector<double> MSceneVolume::histogram(int bins)
{
    return GrayHistogram::compute(_image, bins, nullptr);
}

void MSceneVolume::refreshAll()
{
    if (_viewX) _viewX->refresh();
    if (_viewY) _viewY->refresh();
    if (_viewZ) _viewZ->refresh();
}

void MSceneVolume::reLayout(const QString& layoutMode, QWidget *parent)
{
    if (_grid)
    {
//        _grid->deleteLater();
        delete _grid;
        _grid = nullptr;
    }
    _grid = new QGridLayout(parent);
    _grid->setContentsMargins(0, 0, 0, 0);
    _grid->setSpacing(2);

    if (layoutMode == "grid_2x2")
    {
        _grid->addWidget(_viewX->panel(), 0, 0);
        _grid->addWidget(_viewY->panel(), 0, 1);
        _grid->addWidget(_viewZ->panel(), 1, 0);
        _grid->addWidget(_view3D->panel(), 1, 1);
    }
    else if (layoutMode == "layout_1x3")
    {
        _grid->addWidget(_viewX->panel(), 0, 0, 2, 2); // 大窗
        _grid->addWidget(_viewY->panel(), 0, 2);
        _grid->addWidget(_viewZ->panel(), 1, 2);
        _grid->addWidget(_view3D->panel(), 2, 2);
    }
    else if (layoutMode == "equalize")
    {
        _grid->addWidget(_viewX->panel(), 0, 0);
        _grid->addWidget(_viewY->panel(), 0, 1);
        _grid->addWidget(_viewZ->panel(), 1, 0);
        _grid->addWidget(_view3D->panel(), 1, 1);

        int rows = _grid->rowCount(), cols = _grid->columnCount();
        for (int r = 0; r < rows; ++r) _grid->setRowStretch(r, 1);
        for (int c = 0; c < cols; ++c) _grid->setColumnStretch(c, 1);
    }
}