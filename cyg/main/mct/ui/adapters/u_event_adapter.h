// u_event_adapter.h
#ifndef UI_ADAPTERS_U_EVENT_ADAPTER_H
#define UI_ADAPTERS_U_EVENT_ADAPTER_H

#include <QObject>

/// 事件适配器：订阅 Model 事件 -> 转为 UI 信号回填（示例壳）
/// 实际应连接到 MEventHub（见系统设计文档 单向数据流）                          :contentReference[oaicite:20]{index=20}
class UEventAdapter : public QObject
{
Q_OBJECT
public:
    explicit UEventAdapter(QObject *parent = 0) : QObject(parent)
    {}

signals:

    void onLogLine(const QString &line);

    void onActiveObjectChanged(void *ptr);
};

#endif
