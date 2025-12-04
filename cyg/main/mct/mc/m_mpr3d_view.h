#ifndef CX_CT_X2_M_MPR3D_VIEW_H
#define CX_CT_X2_M_MPR3D_VIEW_H


#include <QVTKOpenGLNativeWidget.h>
#include <vtkSmartPointer.h>
#include <vtkImageData.h>
#include <vtkGenericOpenGLRenderWindow.h>
#include <vtkRenderer.h>
#include <vtkSmartVolumeMapper.h>
#include <vtkVolumeProperty.h>
#include <vtkColorTransferFunction.h>
#include <vtkPiecewiseFunction.h>
#include <vtkVolume.h>
#include <vtkContourFilter.h>
#include <vtkPolyDataMapper.h>
#include <vtkActor.h>
#include <vtkCallbackCommand.h>

#include "m_mpr3d_view_cell.h"
#include "base/app_ids.h"


class Mpr3DView : public QObject
{
Q_OBJECT
public:
    enum class ViewMode // 切换体渲染模式
    {
        Composite = 0, MIP = 1, MinIP = 2, AIP = 3, Isosurface = 4
    };

    explicit Mpr3DView(QVTKOpenGLNativeWidget *host, QObject *parent = nullptr, const QString &tag = QString());

    void setImageData(vtkImageData *img);

    void setViewMode(ViewMode m); // 切换体渲染模式

    void setTransferFunctions(vtkColorTransferFunction *ctf, vtkPiecewiseFunction *pwf); // 可覆盖默认 TF

    void resetCamera();

    inline QWidget* panel() const
    { return _panel; }

    vtkRenderWindow *renderWindow() const
    { return _win; }

    inline vtkImageData *image() const
    { return _image; }

    inline AppIds::EAxis axis() const
    { return AppIds::EAxis::D; }

private:
    void ensureIsoPipeline(); // 初始化等值面管线（懒加载）

    void apply();             // 根据当前模式刷新渲染

private:
    QVTKOpenGLNativeWidget *_host = nullptr;
    vtkSmartPointer<vtkGenericOpenGLRenderWindow> _win;
    vtkSmartPointer<vtkRenderer> _ren;

    // 体渲染
    vtkSmartPointer<vtkSmartVolumeMapper> _volMapper;
    vtkSmartPointer<vtkColorTransferFunction> _ctf;
    vtkSmartPointer<vtkPiecewiseFunction> _pwf;
    vtkSmartPointer<vtkVolumeProperty> _volProp;
    vtkSmartPointer<vtkVolume> _volume;

    // 等值面
    vtkSmartPointer<vtkContourFilter> _contour;
    vtkSmartPointer<vtkPolyDataMapper> _surfMapper;
    vtkSmartPointer<vtkActor> _surfActor;

    vtkSmartPointer<vtkImageData> _image;
    ViewMode _mode = ViewMode::Composite;

    QString _tag;

    QPointer<Mpr3DViewCell> _cell;
    QPointer<QWidget> _panel;

private:
    static void VtkLogCallback(vtkObject *caller, unsigned long eid, void *client, void *);

};


#endif //CX_CT_X2_M_MPR3D_VIEW_H
