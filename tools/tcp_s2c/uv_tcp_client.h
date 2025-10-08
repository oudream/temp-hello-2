#ifndef UV_TCP_CLIENT_H
#define UV_TCP_CLIENT_H

#include <ccxx/cxthread.h>
#include <ccxx/cxchannel.h>


class UVTcpClient : public CxIChannelSubject
{
public:
    static UVTcpClient* start(const std::string &sRemoteIp, int iRemotePort);

    static int sendData(const unsigned char *pData, int iLength);

public:
    UVTcpClient();
    ~UVTcpClient();

protected:
    bool channel_canChangeConnect(const CxChannelBase *oChannel, bool bOldConnect, bool bNewConnect);

    void channel_connectChanged(const CxChannelBase *oChannel);

    void channel_beforeDelete(const CxChannelBase *oChannel);

    void channel_receivedData(const uchar *pData, int iLength, void *oSource);

    void channel_roadConnectChanged(const CxChannelBase *oChannel, CxChannelRoad *oChannelRoad, bool bRemove);

private:
    void open(const std::string &sRemoteIp, int iRemotePort);

    void close();

private:
    CxChannelBase * _channel;
    cx_pthread_mutex_t _lock;

};

#endif //UV_TCP_CLIENT_H
