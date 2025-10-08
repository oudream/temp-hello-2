#ifndef AWIND_H
#define AWIND_H

#include <QMainWindow>
#include <memory>

// VTK fwd
#include <vtkSmartPointer.h>
class vtkRenderer;
class vtkGenericOpenGLRenderWindow;
class vtkVolume;
class vtkImageImport;

namespace Ui { class AWin; }

// 仅为保存 exporter 生命周期，引入轻量级头
#include <itkProcessObject.h>

class AWin : public QMainWindow {
Q_OBJECT
public:
    explicit AWin(QWidget *parent = nullptr);
    ~AWin();

private:
    void runRtkPipelineAndVisualize();

    std::unique_ptr<Ui::AWin> ui;

    // VTK
    vtkSmartPointer<vtkRenderer> renderer;
    vtkSmartPointer<vtkGenericOpenGLRenderWindow> renderWindow;
    vtkSmartPointer<vtkVolume> volume;
    vtkSmartPointer<vtkImageImport> vtkImporter;   // ITK->VTK

    // 保持 ITK exporter 的生命周期，避免 VTK 回调悬空
    itk::ProcessObject::Pointer itkExporterKeepAlive;
};

#endif // AWIND_H
