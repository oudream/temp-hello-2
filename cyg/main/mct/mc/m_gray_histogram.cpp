#include "m_gray_histogram.h"

#include <vtkPointData.h>
#include <vtkDataArray.h>

QVector<double> GrayHistogram::compute(vtkImageData *img, int bins, double *range)
{
    QVector<double> out;
    if (!img || bins <= 0) return out;
    double r[2];
    if (range)
    {
        r[0] = range[0];
        r[1] = range[1];
    }
    else
    { img->GetScalarRange(r); }

    auto acc = vtkSmartPointer<vtkImageAccumulate>::New();
    acc->SetInputData(img);
    acc->SetComponentExtent(0, bins - 1, 0, 0, 0, 0);
    acc->SetComponentOrigin(r[0], 0, 0);
    acc->SetComponentSpacing((r[1] - r[0]) / bins, 1, 1);
    acc->Update();

    auto *scalars = acc->GetOutput()->GetPointData()->GetScalars();
    out.resize(bins);
    for (int i = 0; i < bins; ++i) out[i] = scalars->GetTuple1(i);
    return out;
}
