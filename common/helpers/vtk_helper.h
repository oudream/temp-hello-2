#ifndef CX_CT_X2_VTK_HELPER_H
#define CX_CT_X2_VTK_HELPER_H


#include <vtkSmartPointer.h>
#include <vtkCommand.h>
#include <vtkImageData.h>


class CxVTKErrorObserver : public vtkCommand
{
public:
    static CxVTKErrorObserver *New()
    { return new CxVTKErrorObserver; }

    void Execute(vtkObject *caller, unsigned long eventId, void *callData) override
    {
        if (eventId == vtkCommand::ErrorEvent)
        {
            const char *msg = static_cast<const char *>(callData);
            this->ErrorMessage = msg ? msg : "";
            this->Error = true;
        }
    }

    bool GetError() const
    { return this->Error; }

    std::string GetErrorMessage() const
    { return this->ErrorMessage; }

private:
    bool Error = false;
    std::string ErrorMessage;

};

class VTKHelper
{
public:
    static vtkSmartPointer<vtkImageData> LoadBMPStack(const std::vector<std::string> &files);

    static vtkSmartPointer<vtkImageData> LoadBMPStack(const std::string &dir);

};


#endif //CX_CT_X2_VTK_HELPER_H
