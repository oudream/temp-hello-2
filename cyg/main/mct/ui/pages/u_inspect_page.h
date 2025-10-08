#ifndef UI_U_INSPECT_PAGE_H
#define UI_U_INSPECT_PAGE_H

#include "u_page_base.h"
#include <QWidget>
#include <QPointer>

#include <QPointer>
#include <QComboBox>
#include <QSlider>
#include <QLabel>
#include <QAction>
#include <QVTKOpenGLNativeWidget.h>    // 来自 VTK::GUISupportQt

#include <vtkSmartPointer.h>
#include <vtkImageData.h>
#include <vtkRenderer.h>
#include <vtkGenericOpenGLRenderWindow.h>
#include <vtkGPUVolumeRayCastMapper.h>
#include <vtkVolumeProperty.h>
#include <vtkColorTransferFunction.h>
#include <vtkPiecewiseFunction.h>
#include <vtkVolume.h>
#include <vtkImageSliceMapper.h>
#include <vtkImageSlice.h>
#include <vtkXMLImageDataReader.h>
#include <vtkNamedColors.h>

// 简单三向枚举：Axial(Z向)、Coronal(Y向)、Sagittal(X向)
enum class SliceOrientation
{
    Axial = 2, Coronal = 1, Sagittal = 0
};

/**
 * @brief InspectPage - 试检页
 * 作用：规则/阈值/AI 检测流程、批量任务/统计/报表
 * 职责：触发检测/导出/报表命令；显示批量任务队列（轻量表格）
 */
class UInspectPage : public UPageBase
{
Q_OBJECT
public:
    explicit UInspectPage(QObject *parent = 0);

    virtual ~UInspectPage();

    virtual const char *getName() const;

    virtual QWidget *getWidget();

    virtual void onEnter();

    virtual void onLeave();

private slots:

private:
    void buildUi();

private:
    QWidget *_root;

};

#endif // UI_U_INSPECT_PAGE_H
