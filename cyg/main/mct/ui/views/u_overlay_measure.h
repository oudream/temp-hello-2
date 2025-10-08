#ifndef U_OVERLAY_MEASURE_H
#define U_OVERLAY_MEASURE_H

#include <string>

// 测量叠加层类，用于在场景中显示测量信息（如距离、角度等）
class UOverlayMeasure
{
public:
    // 构造函数：初始化测量类型和起点
    UOverlayMeasure();

    // 析构函数
    ~UOverlayMeasure();

private:
    bool m_isValid;             // 测量是否有效（起点终点是否设置）

};

#endif // U_OVERLAY_MEASURE_H