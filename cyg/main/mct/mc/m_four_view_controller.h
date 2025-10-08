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

class FourViewController : public QObject
{
Q_OBJECT
public:
    FourViewController(QVTKOpenGLNativeWidget *w3D,
                       QVTKOpenGLNativeWidget *wAx,
                       QVTKOpenGLNativeWidget *wCo,
                       QVTKOpenGLNativeWidget *wSa,
                       QObject *parent = nullptr);

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

    // 直方图
    QVector<double> histogram(int bins = 512);

    vtkImageData *image() const
    { return _image; }

    void refreshAll();

signals:

    void vtiLoaded(QString file, vtkImageData *image);

private:
    void connectLinkage(); // 三窗联动（事件互刷）
    Mpr2DView *viewByIndex(int which) const;

private:
    QPointer<QVTKOpenGLNativeWidget> _w3D, _wAx, _wCo, _wSa;

    std::unique_ptr<Mpr3DView> _view3D;
    std::unique_ptr<Mpr2DView> _axial, _coronal, _sagittal;

    vtkSmartPointer<vtkResliceCursor> _sharedCursor;
    WindowLevelEnhancer _wl;
    vtkSmartPointer<vtkImageData> _image;
};


#endif //CX_CT_X2_M_FOUR_VIEW_CONTROLLER_H
