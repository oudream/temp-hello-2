#ifndef CX_CT_X2_U_RIBBON_HANDLER_H
#define CX_CT_X2_U_RIBBON_HANDLER_H


#include <QObject>

/**
 * @brief IURibbonHandler
 * 统一的 Ribbon 逻辑处理器基类。具体业务在子类里定义 slots。
 * 生命周期：由 URibbon 负责（作为其子对象）。
 */
class IURibbonHandler : public QObject
{
Q_OBJECT
public:
    using QObject::QObject;
    ~IURibbonHandler() override = default;
};

#endif //CX_CT_X2_U_RIBBON_HANDLER_H
