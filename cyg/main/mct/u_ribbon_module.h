#ifndef CX_CT_X2_U_RIBBON_MODULE_H
#define CX_CT_X2_U_RIBBON_MODULE_H


#include "u_ribbon_block.h"
#include "u_ribbon_handler.h"


class IURibbonModule
{
public:
    IURibbonModule();
    virtual ~IURibbonModule() = default;

    /**
     * @brief 创建模块的 UI 定义
     */
    virtual URibbonBlock createModule(QObject* parent, QMap<QString, QAction*>& actions, IURibbonHandler* handler) const = 0;

    /**
     * @brief (新增) 创建与此模块对应的逻辑 Handler
     * @param parent Handler 的父对象
     * @return 返回一个 QObject 指针，由集成者负责管理
     */
    virtual IURibbonHandler* createHandler(QObject* parent) const = 0;
};


#endif //CX_CT_X2_U_RIBBON_MODULE_H
