#include "m_slice_exporter.h"

#include <QFileInfo>
#include <vtkWindowToImageFilter.h>
#include <vtkPNGWriter.h>
#include <vtkJPEGWriter.h>
#include <vtkImageReslice.h>
#include <vtkImageCast.h>
#include <vtkImageChangeInformation.h>
#include <vtkTIFFWriter.h>
#include <vtkMatrix4x4.h>
#include <vtkImageExtractComponents.h>
#include <vtkPointData.h>
#include <vtkImageShiftScale.h>
#include <vtkResliceCursor.h>
#include <vtkResliceCursorRepresentation.h>
#include <vtkResliceCursorWidget.h>


// 兼容 VTK8/9：优先从表示层拿 reslice->GetResliceAxes()；
// 若当前是轴对齐模式（RESLICE_AXIS_ALIGNED）且 widget 被禁用，则在本地拼一个等价的 4x4。
static vtkSmartPointer<vtkMatrix4x4> GetCurrentSliceAxes(vtkResliceImageViewer *view)
{
    if (!view) return nullptr;

    // 1) Oblique 模式：表示层里有 vtkImageReslice，可直接拿
    if (view->GetResliceMode() == vtkResliceImageViewer::RESLICE_OBLIQUE)
    {
        auto *w = view->GetResliceCursorWidget();            // viewer 暴露的 widget
        if (!w) return nullptr;
        auto *rep = vtkResliceCursorRepresentation::SafeDownCast(w->GetRepresentation());
        if (!rep) return nullptr;
        auto *reslice = vtkImageReslice::SafeDownCast(rep->GetReslice());
        if (!reslice) return nullptr;                         // VTK8/9 通用写法
        vtkMatrix4x4 *axes = reslice->GetResliceAxes();
        return axes ? axes : nullptr;
    }

    // 2) 轴对齐模式：viewer 走的是 ImageActor 路径，没有启用 widget/reslice，
    //    我们根据 SliceOrientation + Slice + image 的 spacing/origin 构造一个等价的轴
    auto *img = view->GetInput();
    if (!img) return nullptr;

    int slice = view->GetSlice();
    double sp[3];
    img->GetSpacing(sp);
    double or0[3];
    img->GetOrigin(or0);

    // 注意：vtkImageData 默认方向为单位矩阵（若你有方向余弦，自行套进去即可）
    auto axes = vtkSmartPointer<vtkMatrix4x4>::New();
    axes->Identity();

    switch (view->GetSliceOrientation())                    // 参考 vtkImageViewer2 的三向定义
    {
        case vtkImageViewer2::SLICE_ORIENTATION_XY:
            // 观察 Z：X=(1,0,0), Y=(0,1,0), 原点在当前 Z 切层
            axes->SetElement(0, 0, 1);
            axes->SetElement(1, 0, 0);
            axes->SetElement(2, 0, 0); // X
            axes->SetElement(0, 1, 0);
            axes->SetElement(1, 1, 1);
            axes->SetElement(2, 1, 0); // Y
            axes->SetElement(0, 2, 0);
            axes->SetElement(1, 2, 0);
            axes->SetElement(2, 2, 1); // Z
            axes->SetElement(0, 3, or0[0]);
            axes->SetElement(1, 3, or0[1]);
            axes->SetElement(2, 3, or0[2] + slice * sp[2]);
            break;

        case vtkImageViewer2::SLICE_ORIENTATION_XZ:
            // 观察 Y：X=(1,0,0), Y=(0,0,1), 原点在当前 Y 切层
            axes->SetElement(0, 0, 1);
            axes->SetElement(1, 0, 0);
            axes->SetElement(2, 0, 0); // X
            axes->SetElement(0, 1, 0);
            axes->SetElement(1, 1, 0);
            axes->SetElement(2, 1, 1); // Y
            axes->SetElement(0, 2, 0);
            axes->SetElement(1, 2, 1);
            axes->SetElement(2, 2, 0); // Z
            axes->SetElement(0, 3, or0[0]);
            axes->SetElement(1, 3, or0[1] + slice * sp[1]);
            axes->SetElement(2, 3, or0[2]);
            break;

        case vtkImageViewer2::SLICE_ORIENTATION_YZ:
            // 观察 X：X=(0,1,0), Y=(0,0,1), 原点在当前 X 切层
            axes->SetElement(0, 0, 0);
            axes->SetElement(1, 0, 1);
            axes->SetElement(2, 0, 0); // X
            axes->SetElement(0, 1, 0);
            axes->SetElement(1, 1, 0);
            axes->SetElement(2, 1, 1); // Y
            axes->SetElement(0, 2, 1);
            axes->SetElement(1, 2, 0);
            axes->SetElement(2, 2, 0); // Z
            axes->SetElement(0, 3, or0[0] + slice * sp[0]);
            axes->SetElement(1, 3, or0[1]);
            axes->SetElement(2, 3, or0[2]);
            break;
    }
    return axes;
}


// ============ A) 渲染截图 ============
bool SliceExporter::saveRenderedWindow(vtkRenderWindow *win, const QString &path, int scale)
{
    if (!win || path.isEmpty()) return false;
    QFileInfo fi(path);
    auto w2i = vtkSmartPointer<vtkWindowToImageFilter>::New();
    w2i->SetInput(win);
#if VTK_MAJOR_VERSION >= 9
    // VTK9: 用 SetScale 放大像素分辨率（不是重采样数据，只是更高像素截图）
    w2i->SetScale(scale < 1 ? 1 : scale, scale < 1 ? 1 : scale);
#else
    w2i->SetMagnification(scale<1?1:scale);
#endif
    w2i->Update();

    const QString ext = fi.suffix().toLower();
    if (ext == "png")
    {
        auto writer = vtkSmartPointer<vtkPNGWriter>::New();
        writer->SetInputConnection(w2i->GetOutputPort());
        writer->SetFileName(path.toLocal8Bit().constData());
        writer->Write();
        return true;
    }
    else if (ext == "jpg" || ext == "jpeg")
    {
        auto writer = vtkSmartPointer<vtkJPEGWriter>::New();
        writer->SetInputConnection(w2i->GetOutputPort());
        writer->SetFileName(path.toLocal8Bit().constData());
        writer->Write();
        return true;
    }
    return false;
}

// ============ B) 高质量数据导出（重采样当前切面） ============
// 关键思路：从 ResliceImageViewer 取当前切面的 4x4 变换（ResliceAxes），
// 用 vtkImageReslice 直接生成 2D vtkImageData，再以 16-bit TIFF/PNG 等格式写出。
bool SliceExporter::exportCurrentSliceData(vtkResliceImageViewer *view, const QString &path,
                                           double spacingXY, int sizeX, int sizeY,
                                           bool linear, bool rescale16)
{
    if (!view || path.isEmpty()) return false;
    auto img = vtkImageData::SafeDownCast(view->GetInput());
    if (!img) return false;

    // 1) 拿当前切面的 ResliceAxes（4x4：列为 X/Y/Z 方向余弦 + 原点）
    auto axes = GetCurrentSliceAxes(view);
    if (!axes) return false;

    // 2) 估计输出 spacing / size
    double inSp[3];
    img->GetSpacing(inSp);
    double inOri[3];
    img->GetOrigin(inOri);
    int inDim[3];
    img->GetDimensions(inDim);

    // 默认输出 spacing 取各向同性（xy 取三轴均值）
    if (spacingXY <= 0)
        spacingXY = (inSp[0] + inSp[1] + inSp[2]) / 3.0;

    // FOV 估算：按输入尺寸与 spacing（粗略；可按你项目实际 FOV 更精确）
    double fovX = inDim[0] * inSp[0];
    double fovY = inDim[1] * inSp[1];
    // 如果 size 没给，根据 spacingXY 推一个“近似等比例”的输出大小
    if (sizeX <= 0) sizeX = std::max(64, int(std::round(fovX / spacingXY)));
    if (sizeY <= 0) sizeY = std::max(64, int(std::round(fovY / spacingXY)));

    // 3) 重采样
    auto reslice = vtkSmartPointer<vtkImageReslice>::New();
    reslice->SetInputData(img);
    reslice->SetResliceAxes(axes);
    reslice->SetInterpolationMode(linear ? VTK_RESLICE_LINEAR : VTK_RESLICE_NEAREST);
    // 输出 2D：Z 尺寸为 1
    reslice->SetOutputExtent(0, sizeX - 1, 0, sizeY - 1, 0, 0);
    reslice->SetOutputSpacing(spacingXY, spacingXY, inSp[2]); // Z 随意（只有一层）
    reslice->SetOutputOrigin(0, 0, 0);
    reslice->Update();
    vtkImageData *out2D = reslice->GetOutput();

    // 4) 写文件 —— 尽量保持动态范围
    QFileInfo fi(path);
    const QString ext = fi.suffix().toLower();

    // 如果是 short/float，按需拉伸到 uint16（工业/医学常见）
    vtkSmartPointer<vtkImageData> toWrite = out2D;
    int scalarType = out2D->GetScalarType();

    if (rescale16 && (
            scalarType == VTK_SHORT || scalarType == VTK_SIGNED_CHAR
            || scalarType == VTK_FLOAT || scalarType == VTK_DOUBLE))
    {
        double r[2];
        out2D->GetScalarRange(r);
        // 把 [r0,r1] 线性映射到 [0,65535]
        auto sh = vtkSmartPointer<vtkImageShiftScale>::New();
        sh->SetInputData(out2D);
        sh->SetShift(-r[0]);
        sh->SetScale(65535.0 / std::max(1e-9, (r[1] - r[0])));
        sh->SetOutputScalarTypeToUnsignedShort();
        sh->Update();
        toWrite = sh->GetOutput();
        scalarType = VTK_UNSIGNED_SHORT;
    }

    if (ext == "tif" || ext == "tiff")
    {
        auto w = vtkSmartPointer<vtkTIFFWriter>::New();
        w->SetInputData(toWrite);
        w->SetFileName(path.toLocal8Bit().constData());
        w->Write();
        return true;
    }
    else if (ext == "png")
    {
        auto w = vtkSmartPointer<vtkPNGWriter>::New();
        w->SetInputData(toWrite);
        w->SetFileName(path.toLocal8Bit().constData());
        w->Write();
        return true;
    }
    else if (ext == "jpg" || ext == "jpeg")
    {
        // 注意：JPEG 只有 8-bit，会丢动态范围
        auto w = vtkSmartPointer<vtkJPEGWriter>::New();
        w->SetInputData(toWrite);
        w->SetFileName(path.toLocal8Bit().constData());
        w->Write();
        return true;
    }
    return false;
}
