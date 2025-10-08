#ifndef CX_CT_X2_M_SLICE_EXPORTER_H
#define CX_CT_X2_M_SLICE_EXPORTER_H


#include <QString>
#include <vtkSmartPointer.h>
#include <vtkRenderWindow.h>
#include <vtkImageData.h>
#include <vtkResliceImageViewer.h>

// 切面保存：
//  A) saveRenderedWindow(...)   —— 从 RenderWindow 抓屏（所见即所得）
//  B) exportCurrentSliceData(...) —— 高质量：按当前切面重采样出 2D 图像并保存（16-bit TIFF/PNG）
class SliceExporter
{
public:
    // 方式 A：保存渲染结果（可设缩放倍率，提高像素分辨率）
    static bool saveRenderedWindow(vtkRenderWindow *win, const QString &path,
                                   int scale = 1 /*>=1*/);

    // 方式 B：保存高质量数据切片
    // 参数：
    //   view       —— 对应的 MPR 视窗（用于读取当前切面姿态/厚层设置）
    //   path       —— 保存路径（*.tiff; *.png 也可，但注意 16-bit PNG 的通用性）
    //   spacingXY  —— 想要的输出像素间距（mm/pixel）（<=0 则自动用输入 spacing 的均值）
    //   sizeXY     —— 想要的输出像素尺寸（像素）（<=0 则根据 spacingXY 和 FOV 自动推导）
    //   linear     —— 是否线性插值（false=最近邻，保持“原汁原味”）
    //   rescale16  —— 若输入是有符号 short/float，是否线性拉伸到 uint16 再写 16-bit 图（工业常见）
    static bool exportCurrentSliceData(vtkResliceImageViewer *view, const QString &path,
                                       double spacingXY = -1.0, int sizeX = -1, int sizeY = -1,
                                       bool linear = true, bool rescale16 = true);
};


#endif //CX_CT_X2_M_SLICE_EXPORTER_H
