#ifndef CX_CT_X2_M_WINDOW_LEVEL_ENHANCER_H
#define CX_CT_X2_M_WINDOW_LEVEL_ENHANCER_H


#include <QObject>

class Mpr2DView;

class WindowLevelEnhancer : public QObject
{
Q_OBJECT
public:
    explicit WindowLevelEnhancer(QObject *parent = nullptr) : QObject(parent)
    {}

    void setWindowLevel(double win, double lev);   // 设置并广播
    void applyTo(Mpr2DView &v) const;              // 应用到某个 2D 视窗

    // 常用预设
    void presetBone();      // 骨窗（示例）
    void presetSoft();      // 软组织窗（示例）
    void presetMetal();     // 金属增强（示例）

    double window() const
    { return _win; }

    double level() const
    { return _lev; }

signals:

    void windowLevelChanged(double win, double lev);

private:
    double _win = 1500.0;
    double _lev = 300.0;

};


#endif //CX_CT_X2_M_WINDOW_LEVEL_ENHANCER_H
