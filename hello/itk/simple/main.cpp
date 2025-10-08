#include <fmt/core.h>                                  // vcpkg: fmt
#include <itkImage.h>
#include <itkImageFileReader.h>
#include <itkImageFileWriter.h>
#include <itkCurvatureAnisotropicDiffusionImageFilter.h>
#include <itkOtsuThresholdImageFilter.h>

int main(int argc, char* argv[])
{
    if (argc < 3) {
        fmt::print("Usage:\n  {} <inputImage> <outputMask>\n"
                   "Examples:\n  {} head.mha mask.mha\n  {} slice.png mask.png\n",
                   argv[0], argv[0], argv[0]);
        return 1;
    }

    // 支持 3D（常见 CT）; 若处理 2D 图片把 Dimension 改成 2 也可
    constexpr unsigned int Dimension = 3;
    using InPixel  = float;
    using OutPixel = unsigned char;
    using InImage  = itk::Image<InPixel,  Dimension>;
    using OutImage = itk::Image<OutPixel, Dimension>;

    // 1) 读图
    const char* inPath  = argv[1];
    const char* outPath = argv[2];

    auto reader = itk::ImageFileReader<InImage>::New();
    reader->SetFileName(inPath);

    // 2) 平滑（各向异性扩散，去噪保边）
    auto smooth = itk::CurvatureAnisotropicDiffusionImageFilter<InImage, InImage>::New();
    smooth->SetInput(reader->GetOutput());
    smooth->SetTimeStep(0.0625);        // 稳定步长
    smooth->SetNumberOfIterations(5);   // 迭代次数
    smooth->SetConductanceParameter(3); // 导通参数，越大越保边

    // 3) Otsu 自动阈值分割 -> 0/255 掩膜
    auto otsu = itk::OtsuThresholdImageFilter<InImage, OutImage>::New();
    otsu->SetInput(smooth->GetOutput());
    otsu->SetInsideValue(0);
    otsu->SetOutsideValue(255);

    // 4) 写图
    auto writer = itk::ImageFileWriter<OutImage>::New();
    writer->SetFileName(outPath);
    writer->SetInput(otsu->GetOutput());

    try {
        writer->Update();
    } catch (const itk::ExceptionObject& ex) {
        fmt::print("ITK error: {}\n", ex.what());
        return 2;
    }

    // 打印尺寸
    const auto size = reader->GetOutput()->GetLargestPossibleRegion().GetSize();
    if constexpr (Dimension == 3) {
        fmt::print("Read: {}  size: {}x{}x{}  ->  Wrote mask: {}\n",
                   inPath, size[0], size[1], size[2], outPath);
    } else {
        fmt::print("Read: {}  size: {}x{}  ->  Wrote mask: {}\n",
                   inPath, size[0], size[1], outPath);
    }
    return 0;
}