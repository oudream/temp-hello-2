#ifndef CX_CT_X2_M_GRAY_HISTOGRAM_H
#define CX_CT_X2_M_GRAY_HISTOGRAM_H


#include <QVector>
#include <vtkSmartPointer.h>
#include <vtkImageData.h>
#include <vtkImageAccumulate.h>

class GrayHistogram
{
public:
    static QVector<double> compute(vtkImageData *img, int bins = 512, double *rangeOrNull = nullptr);
};


#endif //CX_CT_X2_M_GRAY_HISTOGRAM_H
