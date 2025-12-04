#ifndef UI_U_PAGE_BASE_H
#define UI_U_PAGE_BASE_H

#include <QWidget>

/**
 * @brief UPageBase - 中央工作区“页面”抽象基类
 * 作用：统一“页”的生命周期钩子与身份标识；页面仅负责UI呈现与发起命令请求，不直接修改Model。
 * 职责边界：
 *  - 提供 getName()/getWidget() 与 onEnter()/onLeave() 生命周期。
 *  - 不持有核心对象所有权；不跨线程写；仅在UI线程使用。
 * 原理：主窗口/页面管理器通过该接口切换 stacked pages，并转发 Ribbon 请求。
 */
class UPageBase : public QObject
{
Q_OBJECT
public:
    explicit UPageBase(QWidget *parent) : QObject(parent)
    {}

    ~UPageBase() override
    {}

    /// 页面注册名（用于路由/快捷键，如 "project","assets","reconstruct","three_d","inspect"）
    virtual const char *getName() const = 0;

    /// 返回页面的根 QWidget（供主窗加入中央区域）
    virtual QWidget *getWidget() = 0;

    /// 进入该页的回调（UI线程）
    virtual void onEnter() = 0;

    /// 离开该页的回调（UI线程）
    virtual void onLeave() = 0;

signals:

    /// 页面内发生可路由的UI动作，交由上层转为命令（字符串指令 + 键值参数）
    /// 例如：QVariantMap a; emit requestCommand("app.exit", a);
    void requestCommand(const QString &cmd, const QVariantMap &args);

    /// 请求在右侧显示/隐藏某个 Panel（panelId 参考 Panel 基类）
    void requestTogglePanel(const QString &panelId, bool visible);

    /// 请求四视图布局切换（仅 3D 页会发）
    /// 例如："grid_2x2" / "layout_1x3" / "equalize"
    void requestLayoutChange(const QString &layoutId);

};

#endif // UI_U_PAGE_BASE_H
