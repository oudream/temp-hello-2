#ifndef CX_CT_X2_M_MPR2D_VIEW_H
#define CX_CT_X2_M_MPR2D_VIEW_H


#include <QVTKOpenGLNativeWidget.h>
#include <vtkSmartPointer.h>
#include <vtkImageData.h>
#include <vtkGenericOpenGLRenderWindow.h>
#include <vtkResliceImageViewer.h>
#include <vtkResliceCursor.h>
#include <vtkCallbackCommand.h>
#include <vtkLookupTable.h>


#include "m_mpr2d_view_cell.h"
#include "base/app_ids.h"


class Mpr2DView : public QObject
{

Q_OBJECT

private:
    static void VtkLogCallback(vtkObject *caller, unsigned long eid, void *client, void *);

    static void vtkEventForwarder(vtkObject *, unsigned long, void *, void *);

public:
    explicit Mpr2DView(QVTKOpenGLNativeWidget *host, AppIds::EAxis a, QObject *parent = nullptr, const QString &tag = QString());

    // 图像数据（体素、体积）
    void setImageData(vtkImageData *img);

    // 共享 cursor（不接管）
    void setSharedCursor(vtkResliceCursor *cursor);

    // Axial/Coronal/Sagittal
    void setAxis(AppIds::EAxis a);

    // 窗宽窗位
    void setWL(double win, double lev);

    // 滚轮步进因子
    void setSliceScrollFactor(double f);

    inline QWidget* panel() const
    { return _panel; }

    // 获取内部对象（供导出器或上层使用）
    inline vtkRenderWindow *renderWindow() const
    { return _win; }

    inline vtkResliceImageViewer *viewer() const
    { return _view; }

    inline vtkImageData *image() const
    { return _image; }

    inline AppIds::EAxis axis() const
    { return _axis; }

    void refresh();

signals:

    // 任意切片或交互变化（滚轮、拖十字线）时发出，供三窗联动
    void sliceOrInteract();

private:
    // --- 装配 ---
    // 创建三个 viewer 与窗口/交互器
    void buildViewers();

    // 建立 VTK 事件联动（切片变化即联动刷新）
    void connectObservers();

    void applyAxis();

private:
    QVTKOpenGLNativeWidget *_host = nullptr;
    vtkSmartPointer<vtkGenericOpenGLRenderWindow> _win;
    vtkSmartPointer<vtkResliceImageViewer> _view;
    vtkSmartPointer<vtkLookupTable> _lut;
    vtkSmartPointer<vtkCallbackCommand> _evt;

    vtkSmartPointer<vtkImageData> _image;
    AppIds::EAxis _axis = AppIds::EAxis::X;
    double _scrollFactor = 1.0;
    QString _tag;
    bool _bound;

    QPointer<Mpr2DViewCell> _cell;
    QPointer<QWidget> _panel;

};


#endif //CX_CT_X2_M_MPR2D_VIEW_H
