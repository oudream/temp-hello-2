#ifndef CX_CT_X2_ITK_HELPER_H
#define CX_CT_X2_ITK_HELPER_H


#include <vtkSmartPointer.h>
#include <vtkCommand.h>
#include <vtkImageData.h>


class ITKHelper
{
public:
    static vtkSmartPointer<vtkImageData> LoadBMPStack(const std::string& dir, int first, int last);

};


#endif //CX_CT_X2_ITK_HELPER_H
