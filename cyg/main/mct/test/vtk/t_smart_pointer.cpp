#include "t_smart_pointer.h"

#include <vtkObject.h>
#include <vtkSmartPointer.h>

// 一个简单类，继承 vtkObject，这样可以看到引用计数
class MyObject : public vtkObject
{
public:
    static MyObject *New()
    {
        return new MyObject();
    }

    void PrintSelf()
    {
        std::cout << "MyObject::PrintSelf() called" << std::endl;
    }

protected:
    MyObject()
    { std::cout << "MyObject::Ctor()" << std::endl; }

    ~MyObject() override
    {
        std::cout << "MyObject::Dtor()" << std::endl;
    }

};


void TSmartPointer::Destruction()
{
    vtkSmartPointer<MyObject> sp = vtkSmartPointer<MyObject>::New();
    std::cout << "RefCount after New(): " << sp->GetReferenceCount() << std::endl;

    // 裸指针赋值
    MyObject *raw = sp;
    raw->PrintSelf();

    std::cout << "RefCount after raw assignment: " << raw->GetReferenceCount() << std::endl;

    {
        // 再创建一个 SmartPointer
        vtkSmartPointer<MyObject> sp2 = raw;
        std::cout << "RefCount after sp2=sp: " << sp2->GetReferenceCount() << std::endl;
    }

    std::cout << "RefCount after sp2 goes out of scope: " << raw->GetReferenceCount() << std::endl;

    // 结束时 sp 会被析构，触发对象释放
    std::cout << "End of main()" << std::endl;
}