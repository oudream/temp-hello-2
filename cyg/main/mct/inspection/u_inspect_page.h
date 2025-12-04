#ifndef UI_U_INSPECT_PAGE_H
#define UI_U_INSPECT_PAGE_H

#include "pages/u_page_base.h"
#include <QWidget>
#include <QPointer>

#include <QPointer>
#include <QComboBox>
#include <QSlider>
#include <QLabel>
#include <QAction>
#include <QVTKOpenGLNativeWidget.h>    // 来自 VTK::GUISupportQt
#include <QVBoxLayout>
#include <QTableWidget>
#include <QHeaderView>
#include <QPushButton>
#include <QVariantMap>

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
    explicit UInspectPage(QWidget *parent);

    ~UInspectPage() override;

    const char *getName() const override;

    QWidget *getWidget() override;

    void onEnter() override;

    void onLeave() override;

private slots:
    void onTest1();
    void onTest2();
    void onTest3();

private slots:

private:
    void buildUi(QWidget *parent);

private:
    QWidget *_root;

    QPointer<QPushButton> _btnTest1;
    QPointer<QPushButton> _btnTest2;
    QPointer<QPushButton> _btnTest3;
};

#endif // UI_U_INSPECT_PAGE_H
