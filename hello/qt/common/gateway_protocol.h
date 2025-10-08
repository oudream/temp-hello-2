#ifndef GATEWAY_PROTOCOL_H
#define GATEWAY_PROTOCOL_H

#include <ccxx/cxthread.h>
#include <ccxx/cxchannel.h>

#include <common/device_ddl.h>

#include "gateway_definition.h"


class CxIGatewayProtocolSubject
{
public:
    virtual void gateway_sendFailureEvents(std::string &deviceId, AREABUG_YX &dbTable) = 0;
    virtual void gateway_sendPhaseInfo(std::string &deviceId, PHASE_NOTIFY &data) = 0;
    virtual void gateway_sendCNSOpenCover(unsigned char *mac, int openTimes) = 0;
    virtual void gateway_sendCNSLowVoltageDetect(CNSLowVoltageDetect *_pLowVoltageDetect) = 0;
    virtual void gateway_sendEventPowerOn(const char *mac) = 0;
    virtual void gateway_sendEventPowerOff(const char *mac) = 0;
    virtual void gateway_sendPriorityInfo(bool bIsSuccess, const std::vector<PROT_MIDWARE_PRI> &pris) = 0;
    virtual void gateway_sendPriorityResult(bool bIsSuccess, const std::vector<PROT_MIDWARE_PRI> &pris) = 0;

};

typedef void (*fn_deal_meter_t)(const std::string &sDeviceCode, const METERTable& meter);

class GatewayProtocol : public CxIChannelSubject, ICxTimerNotify
{
public:
    static GatewayProtocol* start(const char *appName, const char *deviceName, int devNameLen, int protocol,  CxIGatewayProtocolSubject *oGatewayProtocolSubject);

public:
    GatewayProtocol(CxIGatewayProtocolSubject *pProtocolSubject);
    ~GatewayProtocol();

protected:
    bool channel_canChangeConnect(const CxChannelBase *oChannel, bool bOldConnect, bool bNewConnect);

    void channel_connectChanged(const CxChannelBase *oChannel);

    void channel_beforeDelete(const CxChannelBase *oChannel);

    void channel_receivedData(const uchar *pData, int iLength, void *oSource);

    void timer_timeOut(const CxTimer* oTimer);

private:
    int on_connectBn_clicked();

    void on_closeBn_clicked();

    int init_client();

    int sendAppNameUp();
    int sendAppAnswer(HW_UINT mMid);

    void gatewayProcessData(const uchar *pData, int iLength);

    int mbPropGetResponse(APPDATA *pData);
    int mbProcessCommand(APPDATA *pData);
    int mbProcessDevStatus(APPDATA *pData);
    int mbPropSetProp(APPDATA *pData);
    int mbProcessAppTopoPattern(APPDATA *pData);
    int mbProcessApp104Propreport(APPDATA *pData);
    int mbProcessLineLoss(APPDATA *pData);
    int mbProcessEventPhase(APPDATA *pData);
    int mbProcessCNSOpenCover(APPDATA *pData);
    int mbProcessCNSLowVoltageDetect(APPDATA *pData);
    int mbProcessEventPowerOn(APPDATA *pData);
    int mbProcessEventPowerOff(APPDATA *pData);
    int mbProcessAppManage(APPDATA *pData);
    int iot_debug_config(APPDATA *pData);

public:
    int sendClockSync(char packetNum, const unsigned char *coreDate, unsigned short dataLen);
    int sendAreaIdentify(const unsigned char *codeDate, unsigned short dataLen);
    int sendBusiness(char code, char packetNum, char function, const unsigned char *coreData, unsigned short dataLen);
    int sendServiceStatus(int state, const char *sockAddr, const char *networkName,
                          unsigned long long int recvTotalLength, unsigned long long int recvLastTime);
    bool isConnection();

// Collect
public:
    int sendDeviceCollect(std::vector<std::string> sDevice);

    void registDeviceCollectAck(fn_void_int_t fn);

    void registDeviceCollect(fn_deal_meter_t fn);

    int doSendDeviceCollect();

// Priority
public:
    int sendRequestPriority();

    int sendUpdatePriority(const std::vector<PROT_MIDWARE_PRI> &pris);

// Bandwidth
public:
    int sendRequestBandwidth(short t);

private:
    CxChannelBase * _channel;
    CxTimer _sendTimer;
    std::vector<uchar> _sendData;
    int _sendInterval;

    CxIGatewayProtocolSubject *_pProtocolSubject;

};

#endif // GATEWAY_PROTOCOL_H
