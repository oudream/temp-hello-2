#ifndef CX_CT_X2_M_FOUR_VIEW_CONTROLLER_H
#define CX_CT_X2_M_FOUR_VIEW_CONTROLLER_H


#include <QObject>
#include <QPointer>
#include <QVTKOpenGLNativeWidget.h>
#include "m_mpr3d_view.h"
#include "m_mpr2d_view.h"
#include "m_slice_exporter.h"
#include "m_window_level_enhancer.h"
#include "m_gray_histogram.h"
#include <vtkResliceCursor.h>
#include <vtkXMLImageDataReader.h>


class MSceneVolume : public QObject
{
Q_OBJECT
public:
    MSceneVolume(QVTKOpenGLNativeWidget *w3D,
                 QVTKOpenGLNativeWidget *wAx,
                 QVTKOpenGLNativeWidget *wCo,
                 QVTKOpenGLNativeWidget *wSa,
                 QWidget *parent);

    // 数据入口
    bool loadVtiFile(const QString &file);

    void setImageData(vtkImageData *img);

    // 3D 模式
    void setViewMode3D(Mpr3DView::ViewMode m);

    // WL/Oblique/Thick/滚轮步进（都会同步到三窗）
    void setWindowLevel(double win, double lev);

    void setThick(bool on, int px);

    void setSliceScrollFactor(double f);

    // 切面导出：which=0/1/2 -> Axial/Coronal/Sagittal
    bool exportSliceRendered(int which, const QString &path, int scale = 1);

    bool exportSliceHighQuality(int which, const QString &path,
                                double spacingXY = -1.0, int sizeX = -1, int sizeY = -1,
                                bool linear = true, bool rescale16 = true);

    void refreshAll();

    void reLayout(const QString &layoutMode, QWidget *parent);

    // 直方图
    QVector<double> histogram(int bins = 512);

    inline vtkImageData *image() const
    { return _image; }

    // 1) 四个 Qt-VTK 窗口
    inline QVTKOpenGLNativeWidget *w3D() const
    { return _w3D; }

    inline QVTKOpenGLNativeWidget *wAxial() const
    { return _wAx; }

    inline QVTKOpenGLNativeWidget *wCoronal() const
    { return _wCo; }

    inline QVTKOpenGLNativeWidget *wSagittal() const
    { return _wSa; }

    // 2) 四个视图对象
    inline Mpr3DView *view3D() const
    { return _view3D.get(); }

    inline Mpr2DView *axialView() const
    { return _viewX.get(); }

    inline Mpr2DView *coronalView() const
    { return _viewY.get(); }

    inline Mpr2DView *sagittalView() const
    { return _viewZ.get(); }

    // 3) 共享 ResliceCursor 与 WL 管理器
    inline vtkResliceCursor *cursor() const
    { return _sharedCursor; }

    inline WindowLevelEnhancer &windowLevel()
    { return _wl; }

    inline const WindowLevelEnhancer &windowLevel() const
    { return _wl; }

    // 4) 直接拿四个 vtkRenderWindow（用于渲染/截图/强制刷新）
    inline vtkRenderWindow *renderWindow3D() const
    { return _w3D ? _w3D->renderWindow() : nullptr; }

    inline vtkRenderWindow *renderWindowAxial() const
    { return _wAx ? _wAx->renderWindow() : nullptr; }

    inline vtkRenderWindow *renderWindowCoronal() const
    { return _wCo ? _wCo->renderWindow() : nullptr; }

    inline vtkRenderWindow *renderWindowSagittal() const
    { return _wSa ? _wSa->renderWindow() : nullptr; }

    // 5) 按索引获取 2D 视图（0/1/2 -> Axial/Coronal/Sagittal）
    Mpr2DView *viewByIndex(int which) const;

signals:

    void vtiLoaded(QString file, vtkImageData *image);

private:
    void connectLinkage(); // 三窗联动（事件互刷）

private:
    QPointer<QVTKOpenGLNativeWidget> _w3D, _wAx, _wCo, _wSa;

    QPointer<Mpr3DView> _view3D;
    QPointer<Mpr2DView> _viewX, _viewY, _viewZ;

    vtkSmartPointer<vtkResliceCursor> _sharedCursor;
    WindowLevelEnhancer _wl;
    vtkSmartPointer<vtkImageData> _image;

    QPointer<QGridLayout> _grid;

};


#endif //CX_CT_X2_M_FOUR_VIEW_CONTROLLER_H
