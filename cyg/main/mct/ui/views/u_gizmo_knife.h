#ifndef U_GIZMO_KNIFE_H
#define U_GIZMO_KNIFE_H

#include <vector>

// 刀型 gizmo 类，用于场景中切割操作的交互组件
class UGizmoKnife
{
public:
    // 构造函数：初始化切割线起点
    UGizmoKnife();

    // 析构函数
    ~UGizmoKnife();

private:
    bool m_isActive;        // 是否处于激活状态（是否正在切割）

};

#endif // U_GIZMO_KNIFE_H