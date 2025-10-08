#ifndef CX_CT_X2_U_THREED_PROXY_PAGE_H
#define CX_CT_X2_U_THREED_PROXY_PAGE_H

#include "u_page_base.h"
#include <QWidget>
#include <QPointer>

// 用一个匿名简单的 Page 代理（也可做 UThreeDPage 派生，这里保守用“托管页”）
class UThreeDProxyPage : public UPageBase
{
Q_OBJECT
public:
    explicit UThreeDProxyPage(QWidget *host, QObject *parent = 0) : UPageBase(parent), _host(host)
    {}

    const char *getName() const override
    { return "three_d"; }

    QWidget *getWidget() override
    { return _host; }

    void onEnter() override
    {}

    void onLeave() override
    {}

private:
    QWidget *_host;
};

#endif //CX_CT_X2_U_THREED_PROXY_PAGE_H
