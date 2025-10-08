#include "uv_tcp_client.h"

#include <ccxx/cxchannel_tcpclient.h>
#include <ccxx/cxapplication.h>
#include <ccxx/cxinterinfo.h>
#include <ccxx/cxinterinfo_group_define.h>

#include "uv_tcp_server.h"


using namespace std;


CXOUTINFO_DEFINE(IIG_NORMAL, IIG_NORMAL, "UVTcpClient")


UVTcpClient * f_protocol = NULL;


UVTcpClient::UVTcpClient()
{
    _channel = NULL;
    _lock = LX_PTHREAD_MUTEX_INITIALIZER;
}

UVTcpClient::~UVTcpClient()
{
    GM_DELETEANDNULL_OBJECT(_channel);
}

/**
 * [--listen-port] | [--remote-ip] [--remote-port] | [--port-name] [--baud-rate]
 * @return
 */
void UVTcpClient::open(const std::string &sRemoteIp, int iRemotePort)
{
    if (_channel)
    {
        if (! _channel->connected())
        {
            _channel->open();
        }
        return;
    }

    CxChannelTcpclient * oTcpclient = new CxChannelTcpclient();
    _channel = oTcpclient;
    oTcpclient->setRemoteIp(sRemoteIp);
    oTcpclient->setRemotePort(iRemotePort);
    _channel->addObserver(this);
    _channel->setAutoOpenInterval(3000);
    _channel->open();

    //
    int iCount = 0;
    while (! _channel->connected())
    {
        if (iCount > 1000)
            break;
        CxApplication::waiting(1);
        ++iCount;
    }

    fnDebug() << "Open To Connect " << sRemoteIp << ":" << iRemotePort << (_channel->connected() ? " success." : " fail.");
}

void UVTcpClient::close()
{
    if (_channel && _channel->connected())
    {
        _channel->close();
        GM_DELETEANDNULL_OBJECT(_channel);
    }
}

UVTcpClient* UVTcpClient::start(const std::string &sRemoteIp, int iRemotePort)
{
    if (f_protocol != NULL)
    {
        return f_protocol;
    }
    f_protocol = new UVTcpClient();
    f_protocol->open(sRemoteIp, iRemotePort);
    return f_protocol;
}

bool UVTcpClient::channel_canChangeConnect(const CxChannelBase *oChannel, bool bOldConnect, bool bNewConnect)
{
    return true;
}

void UVTcpClient::channel_connectChanged(const CxChannelBase *oChannel)
{
}

void UVTcpClient::channel_beforeDelete(const CxChannelBase *oChannel)
{
}

void UVTcpClient::channel_receivedData(const uchar *pData, int iLength, void *oSource)
{
    UVTcpServer::pushData(pData, iLength);
}

void UVTcpClient::channel_roadConnectChanged(const CxChannelBase *oChannel, CxChannelRoad *oChannelRoad, bool bRemove)
{
    fnDebug() << "channel_roadConnectChanged: " << CxChannelRoadManager::reportSelf() << (bRemove ? "\nremove - " : "\nadd - ") << oChannelRoad->remoteIpAddress().ip() << " : " << oChannelRoad->remoteIpAddress().port();
}

int UVTcpClient::sendData(const unsigned char *pData, int iLength)
{
    (void)cx_pthread_mutex_lock(&f_protocol->_lock);
    int r = f_protocol->_channel->sendData(pData, iLength);
    (void)cx_pthread_mutex_unlock(&f_protocol->_lock);
    return r;
}
