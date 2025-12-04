#ifndef CX_CT_X2_M_MPR_VIEW_CELL_H
#define CX_CT_X2_M_MPR_VIEW_CELL_H

#include <QPointer>
#include <QScrollBar>
#include <QLabel>
#include <QVBoxLayout>
#include <QVTKOpenGLNativeWidget.h>

#include <vtkSmartPointer.h>
#include <vtkRenderer.h>
#include <vtkRenderWindow.h>
#include <vtkCamera.h>
#include <vtkInteractorStyleImage.h>
#include <vtkObjectFactory.h>

#include "base/app_ids.h"


// 单窗（Axial / Sagittal / Coronal / 3D）复用模块：
// - 负责：面板控件行与滚动条、平移/缩放/适应、Ctrl+滚轮缩放、MPR交互样式（可选）
// - 对外 API：
//    1) 构造时给定标题
//    2) createHost(view, renderer) 完成控件与交互安装（panel 是承载视图的面板，包含右侧竖滚动条与 "ctrlPlaceholder" 占位）
//    3) setMprInteractorStyleFor() 如需单独安装滚轮缩放（通常 attachTo 内会自动调用）
//
// 备注：本类不拥有 renderer 与 view，仅做“安装与同步”。
class MprViewCell : public QObject
{
    Q_OBJECT

public:
    using EAxis = AppIds::EAxis;

    explicit MprViewCell(EAxis axis, QObject* parent = nullptr);

    // 创建布局骨架: 绑定一个视图单元（面板 + QVTK 视图 + renderer）并创建控件行（标题、水平滚动条、按钮）
    QWidget* createHost(QVTKOpenGLNativeWidget* view, vtkRenderer* renderer, const QString& hostObjectName = QString());

    // （可选）为 2D MPR 视图设置交互样式（右键平移、R复位、Ctrl+滚轮缩放）
    static void setMprInteractorStyleFor(const QVTKOpenGLNativeWidget* view, vtkRenderer* renderer);

private:
    // 如需单独安装 Ctrl+滚轮 缩放过滤器（attachTo 会自动装，不必重复调用）
    void installCtrlWheelZoom();

    // —— 状态 —— //
    struct PanelState
    {
        int lastH = 0;
        int lastV = 0;
        double zoom = 1.0; // 当前缩放倍数（1=适应）
        double baseParallelScale = 0.0; // 适应时记录相机并行尺度
    };

    EAxis _axis;
    QString _title;
    QWidget* _root = nullptr;
    QPointer<QWidget> _hostPanel;
    QVTKOpenGLNativeWidget* _view;
    vtkSmartPointer<vtkRenderer> _renderer;
    PanelState state_;

    // UI 句柄（从 hostPanel_ 中查找/构建）
    QScrollBar* _hScroll = nullptr; // 控件行里的水平滚动条
    QScrollBar* _vScroll = nullptr; // 视图右侧竖滚动条（和视图等高）

private:
    // —— 交互样式（内置一个 MPR 交互） —— //
    class InteractorStyleMPR : public vtkInteractorStyleImage
    {
    public:
        static InteractorStyleMPR* New();

        vtkTypeMacro(InteractorStyleMPR, vtkInteractorStyleImage);

        void SetRenderer(vtkRenderer* ren)
        {
            ren_ = ren;
        }

        void OnMouseWheelForward() override;

        void OnMouseWheelBackward() override;

        void OnRightButtonDown() override
        {
            this->StartPan();
        }

        void OnRightButtonUp() override
        {
            this->EndPan();
        }

        void OnChar() override;

    protected:
        ~InteractorStyleMPR() override = default;

    private:
        vtkRenderer* ren_ = nullptr;
    };

    // —— Ctrl+滚轮 事件过滤器 —— //
    class CtrlWheelFilter : public QObject
    {
    public:
        CtrlWheelFilter(MprViewCell* owner);

    protected:
        bool eventFilter(QObject* obj, QEvent* e) override;

    private:
        MprViewCell* owner_;
    };

private:
    // —— 内部工具 —— //
    QWidget* buildControlRow(); // 在 "ctrlPlaceholder" 处插入控件行

    void updateScrollBars() const; // 根据 zoom 动态设置滚动范围

    void onPanH(int val);

    void onPanV(int val);

    void onZoomIn();

    void onZoomOut();

    void onFit();

    void panRenderer(double ndx, double ndy) const;

    void zoomRenderer(double factor) const;

    void fitRenderer() const;

    bool hasRenderableContent() const; // 渲染器里是否有“连好输入且可见”的 Prop

};


#endif //CX_CT_X2_M_MPR_VIEW_CELL_H
