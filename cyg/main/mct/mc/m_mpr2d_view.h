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

class Mpr2DView : public QObject
{

Q_OBJECT

public:
    // 三种朝向（本窗观察到的法向）：Z/Y/X
    enum Axis
    {
        AX = 0, AY = 1, AZ = 2
    };

private:
    static void VtkLogCallback(vtkObject *caller, unsigned long eid, void *client, void *);

    static void vtkEventForwarder(vtkObject *, unsigned long, void *, void *);

public:
    explicit Mpr2DView(QVTKOpenGLNativeWidget *host, Axis a, QObject *parent = nullptr, const QString &tag = QString());

    // 图像数据（体素、体积）
    void setImageData(vtkImageData *img);

    // 共享 cursor（不接管）
    void setSharedCursor(vtkResliceCursor *cursor);

    // Axial/Coronal/Sagittal
    void setAxis(Axis a);

    // 窗宽窗位
    void setWL(double win, double lev);

    // 滚轮步进因子
    void setSliceScrollFactor(double f);

    // 获取内部对象（供导出器或上层使用）
    vtkRenderWindow *renderWindow() const
    { return _win; }

    vtkResliceImageViewer *viewer() const
    { return _view; }

    vtkImageData *image() const
    { return _image; }

    Axis axis() const
    { return _axis; }

    void refresh();

signals:
    // 任意切片或交互变化（滚轮、拖十字线）时发出，供三窗联动
    void sliceOrInteract();

private:
    // --- 装配 ---
    void buildViewers();              // 创建三个 viewer 与窗口/交互器

    void connectObservers();          // 建立 VTK 事件联动（切片变化即联动刷新）

    void applyAxis();

private:
    QVTKOpenGLNativeWidget *_host = nullptr;
    vtkSmartPointer<vtkGenericOpenGLRenderWindow> _win;
    vtkSmartPointer<vtkResliceImageViewer> _view;
    vtkSmartPointer<vtkLookupTable> _lut;
    vtkSmartPointer<vtkCallbackCommand> _evt;

    vtkSmartPointer<vtkImageData> _image;
    Axis _axis = AX;
    double _scrollFactor = 1.0;
    QString _tag;
    bool _bound;

};


#endif //CX_CT_X2_M_MPR2D_VIEW_H
