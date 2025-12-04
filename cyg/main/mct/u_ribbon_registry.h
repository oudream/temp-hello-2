#ifndef CX_CT_X2_U_RIBBON_REGISTRY_H
#define CX_CT_X2_U_RIBBON_REGISTRY_H


#include <QList>
#include "u_ribbon_module.h"


class URibbonRegistry
{
public:
    // 项目、原图、重建、切面、渲染、测量、分割、配准、窗体、转换、试检
    // project、acquisition、reconstruction、reslice、rendering、measurement、segmentation、registration、windowing、transform、inspection
    // project、acquisition、reconstruction、slice、render、measure、segmentation、register、windowing、convert、inspect
    static void registerModule(IURibbonModule *module);

    static const QList<IURibbonModule *> &getModules();

    static void init();

private:
    static QList<IURibbonModule *> &modules();

};


#endif //CX_CT_X2_U_RIBBON_REGISTRY_H
