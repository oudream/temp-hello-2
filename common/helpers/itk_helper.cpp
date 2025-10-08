#include "itk_helper.h"

#include <itkImage.h>
#include <itkImageSeriesReader.h>
#include <itkBMPImageIO.h>
#include <itkNumericSeriesFileNames.h>
#include <itkImageToVTKImageFilter.h>
#include <vtkSmartPointer.h>
#include <vtkImageData.h>

using PixelT = unsigned char;
constexpr unsigned Dim = 3;
using Image3D = itk::Image<PixelT, Dim>;

itk::SmartPointer<itk::ImageSeriesReader<Image3D>> reader;

vtkSmartPointer<vtkImageData> ITKHelper::LoadBMPStack(const std::string& dir, int first, int last)
{
    auto names = itk::NumericSeriesFileNames::New();
    names->SetStartIndex(first);
    names->SetEndIndex(last);
    names->SetIncrementIndex(1);
    // 例：dir = "D:/slices", 文件名 "0.bmp" ~ "200.bmp"
    names->SetSeriesFormat((dir + "/%d.bmp").c_str());

    auto io = itk::BMPImageIO::New(); // BMP 解码器
    reader = itk::ImageSeriesReader<Image3D>::New();
    reader->SetImageIO(io);
    reader->SetFileNames(names->GetFileNames());

    // 多线程（也可以显式设置线程数）
    // itk::MultiThreaderBase::SetGlobalMaximumNumberOfThreads(std::thread::hardware_concurrency());

    reader->Update(); // 一次性读取到连续3D缓冲

    // ITK -> VTK 桥接（零拷贝/浅拷贝语义）
    auto bridge = itk::ImageToVTKImageFilter<Image3D>::New();
    bridge->SetInput(reader->GetOutput());
    bridge->Update();

    vtkSmartPointer<vtkImageData> vol = bridge->GetOutput();
    return vol;
}
