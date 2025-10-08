#ifndef CX_CT_X2_IVIEW_H
#define CX_CT_X2_IVIEW_H


#include <QWidget>
#include <QVariantMap>

class IView : public QWidget {
Q_OBJECT
public:
    explicit IView(QWidget *parent = nullptr) : QWidget(parent) {}
    virtual ~IView() {}

public slots:
    // 设置视图相关按钮/动作的状态（enabled/checked/value等）
    virtual void setActionStates(const QVariantMap &states) { Q_UNUSED(states); }

signals:
    // 视图触发事件（如：鼠标选点、切面变更等）
    void viewEvent(const QString &eventType, const QVariant &payload);
};


#endif //CX_CT_X2_IVIEW_H
