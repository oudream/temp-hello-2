#include "u_ribbon_registry.h"

// 按你的目录把头文件补齐（已按常见命名给出示例路径/类名，若与你项目不一致，改成实际即可）
#include "project/project_module.h"
#include "acquisition/acquisition_module.h"
#include "reconstruction/reconstruction_module.h"
#include "mc/reslice_module.h"
#include "mc/rendering_module.h"
#include "mc/measurement_module.h"
#include "mc/segmentation_module.h"
#include "mc/registration_module.h"
#include "mc/transform_module.h"
#include "windowing/windowing_module.h"
#include "inspection/inspection_module.h"


// 统一静态存储，避免静态初始化顺序问题
QList<IURibbonModule *> &URibbonRegistry::modules()
{
    static QList<IURibbonModule *> s_modules;
    return s_modules;
}

void URibbonRegistry::registerModule(IURibbonModule *module)
{
    if (!module) return;
    auto &list = modules();
    // 去重
    if (!list.contains(module))
        list.append(module);
}

const QList<IURibbonModule *> &URibbonRegistry::getModules()
{
    return modules();
}

// project、acquisition、reconstruction、reslice、rendering、measurement、segmentation、registration、windowing、transform、inspection
void URibbonRegistry::init()
{
    // 统一注册宏：使用静态对象，避免堆分配与释放问题
#define MY_REG(mod_type)                      \
        do {                                  \
            auto *mod = new mod_type();       \
            registerModule(mod);              \
        } while (0)

    MY_REG(ProjectModule);
    MY_REG(AcquisitionModule);
    MY_REG(ReconstructionModule);
    MY_REG(ResliceModule);
    MY_REG(RenderingModule);
    MY_REG(MeasurementModule);
    MY_REG(SegmentationModule);
    MY_REG(RegistrationModule);
    MY_REG(WindowingModule);
    MY_REG(TransformModule);
    MY_REG(InspectionModule);
}
