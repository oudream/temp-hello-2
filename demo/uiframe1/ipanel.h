#ifndef CX_CT_X2_IPANEL_H
#define CX_CT_X2_IPANEL_H


#include <QWidget>
#include <QVariantMap>

class IPanel : public QWidget {
Q_OBJECT
public:
    explicit IPanel(QWidget *parent = nullptr) : QWidget(parent) {}
    virtual ~IPanel() {}

public slots:
    virtual void setActionStates(const QVariantMap &states) { Q_UNUSED(states); }

signals:
    void panelEvent(const QString &eventType, const QVariant &payload);
};


#endif //CX_CT_X2_IPANEL_H
