#include "u_ribbon_module.h"


#include "u_ribbon_registry.h"


IURibbonModule::IURibbonModule()
{
    URibbonRegistry::registerModule(this);
}