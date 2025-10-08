#ifndef UI_U_PANEL_BASE_H
#define UI_U_PANEL_BASE_H

#include <QDockWidget>

class MObject;

/**
 * @brief UPanelBase - 右侧Dock面板抽象基类
 * 作用：统一面板显示/隐藏与上下文更新入口；面板只读/写UI，不直接操作Model。
 * 约定：getName() 返回用于注册的 panelId，如 "prop","render","log","camera","knife","measure","assets"
 */
class UPanelBase : public QDockWidget
{
Q_OBJECT
public:
    explicit UPanelBase(const QString &title, QWidget *parent = 0)
            : QDockWidget(title, parent)
    { setObjectName(title); }

    virtual ~UPanelBase()
    {}

    /// 面板注册名（与 Ribbon 下拉勾选保持一致）
    virtual const char *getName() const = 0;

public slots:

    /// 根据当前激活对象/上下文刷新面板显示（UI线程）
    virtual void updateContext(MObject *activeObject) = 0;

signals:

    /// 发起命令（例如设置窗宽窗位、切换渲染模式、相机操作、三刀调整等）
    void requestCommand(const QString &cmd, const QVariantMap &args);

};

#endif // UI_U_PANEL_BASE_H
