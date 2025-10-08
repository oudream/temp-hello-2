#include "vtk_helper.h"

#include <vtkBMPReader.h>
#include <vtkImageAppend.h>


// 加载 BMP 文件序列，返回 vtkImageData（3D volume）
vtkSmartPointer<vtkImageData> VTKHelper::LoadBMPStack(const std::vector<std::string>& files)
{
    vtkSmartPointer<vtkImageAppend> append =
            vtkSmartPointer<vtkImageAppend>::New();
    append->SetAppendAxis(2); // 沿 Z 轴堆叠

    for (auto& f : files)
    {
        vtkSmartPointer<vtkBMPReader> reader = vtkSmartPointer<vtkBMPReader>::New();
        reader->SetFileName(f.c_str());
        reader->Update();

        append->AddInputData(reader->GetOutput());
    }

    append->Update();
    return append->GetOutput();
}

vtkSmartPointer<vtkImageData> VTKHelper::LoadBMPStack(const std::string &dir)
{
    // 如果你的 VTK 版本支持 SetFilePattern/SetFilePrefix 之类（部分格式可行）：
    auto reader = vtkSmartPointer<vtkBMPReader>::New();
    reader->SetFilePrefix(dir.c_str());               // e.g. "D:/slices/"
    reader->SetFilePattern("%s%d.bmp");               // 0..200
// reader->SetDataExtent(0,1199, 0,1199, 0,200);  // 必要时指定范围
    reader->Update();
    vtkSmartPointer<vtkImageData> vol = reader->GetOutput();
    return vol;
}
