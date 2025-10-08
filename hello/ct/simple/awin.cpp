#include "awin.h"
#include "ui_awin.h"

// Qt
#include <QPixmap>
#include <QImage>

// OpenCV
#include <opencv2/opencv.hpp>

// ITK
#include "itkImage.h"
#include "itkEllipseSpatialObject.h"
#include "itkImageRegionIterator.h"
#include "itkExtractImageFilter.h"
#include "itkRescaleIntensityImageFilter.h"
#include "itkVTKImageExport.h"

// RTK
#include "rtkThreeDCircularProjectionGeometry.h"
#include "rtkConstantImageSource.h"
#include "rtkRayEllipsoidIntersectionImageFilter.h"
#include "rtkFDKConeBeamReconstructionFilter.h"

// VTK
#include <vtkGenericOpenGLRenderWindow.h>
#include <vtkRenderer.h>
#include <vtkSmartVolumeMapper.h>
#include <vtkVolume.h>
#include <vtkVolumeProperty.h>
#include <vtkPiecewiseFunction.h>
#include <vtkColorTransferFunction.h>
#include <vtkImageImport.h>

AWin::AWin(QWidget *parent)
        : QMainWindow(parent), ui(std::make_unique<Ui::AWin>())
{
    ui->setupUi(this);
    statusBar()->showMessage("Running RTK reconstruction pipeline…");
    runRtkPipelineAndVisualize();
    statusBar()->showMessage("Done.", 5000);
}

AWin::~AWin() = default;

void AWin::runRtkPipelineAndVisualize()
{
    using PixelType = float;
    constexpr unsigned int Dim = 3;
    using VolumeImageType = itk::Image<PixelType, Dim>;
    using ProjImageType   = itk::Image<PixelType, Dim>;

    // ----------------------- 1) 创建一个 128^3 的球体 phantom（仅用于演示/比较） -----------------------
    auto phantom = VolumeImageType::New();
    VolumeImageType::SizeType vsize;   vsize[0]=128; vsize[1]=128; vsize[2]=128;
    VolumeImageType::IndexType vstart; vstart.Fill(0);
    VolumeImageType::RegionType vregion(vstart, vsize);
    phantom->SetRegions(vregion);
    VolumeImageType::SpacingType vsp; vsp[0]=1.0; vsp[1]=1.0; vsp[2]=1.0;
    VolumeImageType::PointType   vorg; vorg[0]=-64.0; vorg[1]=-64.0; vorg[2]=-64.0;
    phantom->SetSpacing(vsp);
    phantom->SetOrigin(vorg);
    phantom->Allocate();
    phantom->FillBuffer(0);

    using SOType = itk::EllipseSpatialObject<Dim>;
    auto sphere = SOType::New();
    sphere->SetRadiusInObjectSpace(32.0);
    VolumeImageType::PointType center; center[0]=0; center[1]=0; center[2]=0;
    sphere->GetModifiableObjectToParentTransform()->SetOffset(center.GetVectorFromOrigin());

    itk::ImageRegionIterator<VolumeImageType> it(phantom, vregion);
    VolumeImageType::PointType p;
    for (; !it.IsAtEnd(); ++it) {
        phantom->TransformIndexToPhysicalPoint(it.GetIndex(), p); // 两个参数，第二个为输出
        if (sphere->IsInsideInWorldSpace(p)) it.Set(1.0f);
    }

    // ----------------------- 2) RTK 几何 + 椭球解析投影（REI） -----------------------
    using GeometryType = rtk::ThreeDCircularProjectionGeometry;
    GeometryType::Pointer geometry = GeometryType::New();
    const unsigned int numberOfProjections = 180;
    const double sid = 600.0;  // 源到等中心距离
    const double sdd = 1200.0; // 源到探测器距离
    for (unsigned int i=0; i<numberOfProjections; ++i)
        geometry->AddProjection(sid, sdd, i * 360.0 / numberOfProjections);

    using ProjSourceType = rtk::ConstantImageSource<ProjImageType>;
    ProjSourceType::Pointer projSource = ProjSourceType::New();
    ProjImageType::SizeType psize;  psize[0]=256; psize[1]=256; psize[2]=numberOfProjections;
    ProjImageType::SpacingType psp; psp[0]=1.0; psp[1]=1.0; psp[2]=1.0;
    ProjImageType::PointType   pog; pog[0]=-128.0; pog[1]=-128.0; pog[2]=0.0;
    projSource->SetSize(psize);
    projSource->SetSpacing(psp);
    projSource->SetOrigin(pog);
    projSource->SetConstant(0.0f);

    using REIFilter = rtk::RayEllipsoidIntersectionImageFilter<ProjImageType, ProjImageType>;
    REIFilter::Pointer rei = REIFilter::New();
    rei->SetInput(projSource->GetOutput());
    rei->SetGeometry(geometry);
    // 正确 API：SetAxis / SetCenter
    REIFilter::VectorType axis; axis[0]=32.0; axis[1]=32.0; axis[2]=32.0;
    rei->SetAxis(axis);
    REIFilter::PointType rcenter; rcenter[0]=0.0; rcenter[1]=0.0; rcenter[2]=0.0;
    rei->SetCenter(rcenter);
    rei->Update(); // 得到投影堆栈 (u,v,proj)

    // ----------------------- 3) FDK 重建：Input0=初始体，Input1=投影 -----------------------
    using VolSourceType = rtk::ConstantImageSource<VolumeImageType>;
    VolSourceType::Pointer volSource = VolSourceType::New();
    volSource->SetSize(vsize);
    volSource->SetSpacing(vsp);
    volSource->SetOrigin(vorg);
    volSource->SetConstant(0.0f);

    using FDKType = rtk::FDKConeBeamReconstructionFilter<VolumeImageType>;
    FDKType::Pointer fdk = FDKType::New();
    fdk->SetInput(0, volSource->GetOutput());   // 初始重建体
    fdk->SetInput(1, rei->GetOutput());         // 投影堆栈
    fdk->SetGeometry(geometry);
    fdk->Update();

    VolumeImageType::Pointer reconstructed = fdk->GetOutput();

    // ----------------------- 4a) ITK -> OpenCV -> Qt 显示中间切片 -----------------------
    using SliceImageType = itk::Image<PixelType, 2>;
    using ExtractFilter  = itk::ExtractImageFilter<VolumeImageType, SliceImageType>;
    ExtractFilter::Pointer extract = ExtractFilter::New();
    extract->SetInput(reconstructed);
    VolumeImageType::RegionType exRegion = reconstructed->GetLargestPossibleRegion();
    exRegion.SetSize(2, 0);
    exRegion.SetIndex(2, vsize[2] / 2);
    extract->SetExtractionRegion(exRegion);
    extract->SetDirectionCollapseToIdentity();

    using U8Slice = itk::Image<unsigned char, 2>;
    using Rescale = itk::RescaleIntensityImageFilter<SliceImageType, U8Slice>;
    Rescale::Pointer rescale = Rescale::New();
    rescale->SetInput(extract->GetOutput());
    rescale->SetOutputMinimum(0);
    rescale->SetOutputMaximum(255);
    rescale->Update();

    U8Slice::Pointer u8 = rescale->GetOutput();
    auto sreg = u8->GetLargestPossibleRegion().GetSize();
    cv::Mat cvImg((int)sreg[1], (int)sreg[0], CV_8UC1, u8->GetBufferPointer());
    QImage qimg(cvImg.data, cvImg.cols, cvImg.rows, cvImg.step, QImage::Format_Grayscale8);
    ui->imageLabel->setPixmap(QPixmap::fromImage(qimg.copy()));
    ui->imageLabel->setScaledContents(true);

    // ----------------------- 4b) ITK -> VTK 体绘制（保持 exporter 生命周期） -----------------------
    // 1) 创建 importer 并连接到 exporter
    vtkImporter = vtkSmartPointer<vtkImageImport>::New();
    using ExporterType = itk::VTKImageExport<VolumeImageType>;
    ExporterType::Pointer exporter = ExporterType::New();
    exporter->SetInput(reconstructed);

    vtkImporter->SetUpdateInformationCallback(exporter->GetUpdateInformationCallback());
    vtkImporter->SetPipelineModifiedCallback(exporter->GetPipelineModifiedCallback());
    vtkImporter->SetWholeExtentCallback(exporter->GetWholeExtentCallback());
    vtkImporter->SetSpacingCallback(exporter->GetSpacingCallback());
    vtkImporter->SetOriginCallback(exporter->GetOriginCallback());
    vtkImporter->SetScalarTypeCallback(exporter->GetScalarTypeCallback());
    vtkImporter->SetNumberOfComponentsCallback(exporter->GetNumberOfComponentsCallback());
    vtkImporter->SetPropagateUpdateExtentCallback(exporter->GetPropagateUpdateExtentCallback());
    vtkImporter->SetUpdateDataCallback(exporter->GetUpdateDataCallback());
    vtkImporter->SetDataExtentCallback(exporter->GetDataExtentCallback());
    vtkImporter->SetBufferPointerCallback(exporter->GetBufferPointerCallback());
    vtkImporter->SetCallbackUserData(exporter->GetCallbackUserData());

    // 2) 关键：保存 exporter 到成员，防止回调 userData 悬空导致崩溃
    itkExporterKeepAlive = exporter;

    vtkImporter->Update();

    // 3) VTK 渲染
    renderer = vtkSmartPointer<vtkRenderer>::New();
    renderWindow = vtkSmartPointer<vtkGenericOpenGLRenderWindow>::New();
    renderWindow->AddRenderer(renderer);
    ui->vtkWidget->setRenderWindow(renderWindow);

    auto volMapper = vtkSmartPointer<vtkSmartVolumeMapper>::New();
    volMapper->SetInputData(vtkImporter->GetOutput());

    auto volProp = vtkSmartPointer<vtkVolumeProperty>::New();
    volProp->SetInterpolationTypeToLinear();
    volProp->ShadeOn();
    volProp->SetAmbient(0.2);
    volProp->SetDiffuse(0.7);
    volProp->SetSpecular(0.1);

    auto opacity = vtkSmartPointer<vtkPiecewiseFunction>::New();
    opacity->AddPoint(0.0, 0.0);
    opacity->AddPoint(0.3, 0.05);
    opacity->AddPoint(1.0, 0.6);

    auto color = vtkSmartPointer<vtkColorTransferFunction>::New();
    color->AddRGBPoint(0.0, 0.0, 0.0, 0.0);
    color->AddRGBPoint(0.5, 1.0, 1.0, 0.0);
    color->AddRGBPoint(1.0, 1.0, 0.0, 0.0);

    volProp->SetScalarOpacity(opacity);
    volProp->SetColor(color);

    volume = vtkSmartPointer<vtkVolume>::New();
    volume->SetMapper(volMapper);
    volume->SetProperty(volProp);

    renderer->AddVolume(volume);
    renderer->SetBackground(0.1, 0.2, 0.4);
    renderer->ResetCamera();
}
