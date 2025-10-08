#include "gateway_protocol.h"

#include <ccxx/cxinterinfo.h>
#include <ccxx/cxchannel_udp.h>
#include <ccxx/cxchannel_tcpclient.h>
#include <ccxx/cxchannel_tcpserver.h>
#include <ccxx/cxchannel_serial.h>
#include <ccxx/cxchannel_global.h>
#include <ccxx/cxapplication.h>
#include <ccxx/cxarguments.h>
#include <ccxx/cxinterinfo_group_define.h>

#include "gateway_definition.h"


using namespace std;


CXOUTINFO_DEFINE(IIG_NORMAL, IIG_NORMAL, "GATEWAY-PROTOCOL")


GatewayProtocol * f_testor = NULL;

static CxMutex *f_clientEventLock = NULL;
static CxMutex *fn_clientEventLock()
{
    static CxMutex m;
    return &m;
}

static volatile int f_clientEventCount;
static std::list<IEC104Server> f_clientIEC104Servers;
static std::map<std::string, int> f_clientDB_TABLEs;
static std::map<unsigned int, std::map<std::string, AREABUG_YX>> f_clientFailureEvents;
static std::map<std::string, PHASE_NOTIFY> f_clientPhaseNotify;

static char _S_APP_NAME[APP_NAME_LEN] = {0};
static char _S_DEVICE_NAME[UART_NAME_LEN] = {0};
static int _I_PROTOCOL = APP_PACKET_VIRTUAL_MQTT;

static string s_RemoteIp = "172.17.0.1";
//static string s_RemoteIp = "192.168.91.150";
//static string s_RemoteIp = "127.0.0.1";
static string s_RemotePort = "15623";

// 关闭日志 --- 会打印 error
//int debug_switch = 12;
// 打开日志 --- 打印全部日志
int debug_switch = 0;
// Serial number of a communications message
static char plcMailSeq = 0;


int GatewayProtocol::sendAppNameUp()
{
    APPDATA appdata = {0};
    REGISTER_INFO *pInfo = (REGISTER_INFO *) appdata.buff;
    appdata.hdr.packetType = TYPE_APP_NAME;
    appdata.hdr.packetLen = sizeof(REGISTER_INFO);
    memcpy(pInfo->appName, _S_APP_NAME, strlen(_S_APP_NAME));
    memcpy(pInfo->deviceName, _S_DEVICE_NAME, strlen(_S_DEVICE_NAME));
    pInfo->protocol = _I_PROTOCOL;

//    fnDebug() << ".sendAppNameUp: " << CxString::toHexstring((char *)&appdata, sizeof(APPDATA));

    uchar *dataBegin = (uchar *)&appdata;
    uchar *dataEnd = dataBegin + sizeof(appdata);
    vector<uchar> res(dataBegin, dataEnd);
    int r = _channel->sendBytes(res);
    fnInfo() << "Gateway.sendAppNameUp... result=" << r;
    return r;
}

int GatewayProtocol::sendAppAnswer(HW_UINT mMid)
{
    APPDATA appdata = {0};
    appdata.hdr.mMid = mMid;
    appdata.hdr.packetType = TYPE_APP_ACK;
    appdata.hdr.packetLen = sizeof(REGISTER_INFO);

//    fnDebug() << ".sendAppAnswer: " << CxString::toHexstring((char *)&appdata, sizeof(APPDATA));

    uchar *dataBegin = (uchar *)&appdata;
    uchar *dataEnd = dataBegin + sizeof(appdata);
    vector<uchar> res(dataBegin, dataEnd);
    int r = _channel->sendBytes(res);
    fnInfo() << "Gateway.sendAppAnswer... result=" << r;
    return r;
}

void GatewayProtocol::gatewayProcessData(const uchar *data, int len)
{

    if (len < sizeof(APPDATA))
    {
        return;
    }

    int r = 0;
    APPDATA *pData = (APPDATA *) data;
    pData->buff[MAX_MESSAGE_BUF_LEN-1] = '\0';
    fnInfo() << "Gateway hdr.packetType=" << pData->hdr.packetType << "  pData->hdr.mMid=" << pData->hdr.mMid;
//    fnDebug() << ".readAppData: " << CxString::toHexstring((char *)pData, sizeof(APPDATA));

    switch (pData->hdr.packetType)
    {
        case TYPE_PROPGET:
            r = mbPropGetResponse(pData);
            break;
        case TYPE_COMMAND:
            r = mbProcessCommand(pData);
            break;
        case TYPE_PROPSET:
            r = mbPropSetProp(pData);
            break;
        case TYPE_DEVICESTAUS:
            r = mbProcessDevStatus(pData);
            break;
        case TYPE_APP_TOPO_PATTERN:
            r = mbProcessAppTopoPattern(pData);
            break;
        case TYPE_APP_GWDB_MODIFY:
            r = mbProcessApp104Propreport(pData);
            break;
        case TYPE_APP_AREA_BUG_REPORT:
            r = mbProcessLineLoss(pData);
            break;
        case TYPE_APP_EVENT_PHASE:
            r = mbProcessEventPhase(pData);
            break;
        case TYPE_APP_EVENT_CNS_OPEN_COVER:
            r = mbProcessCNSOpenCover(pData);
            break;
        case TYPE_APP_EVENT_CNS_LOW_VOLTAGE_DETECT:
            mbProcessCNSLowVoltageDetect(pData);
            break;
        case TYPE_APP_EVENT_POWERON:
            mbProcessEventPowerOn(pData);
            break;
        case TYPE_APP_EVENT_POWEROFF:
            mbProcessEventPowerOff(pData);
            break;
        case TYPE_APP_PROTOCOL_APPMANAGE:
            mbProcessAppManage(pData);
            break;

        case TYPE_HB:
            fnInfo() << "Gateway Heart beat!";
            break;
        case TYPE_APP_DEBUG:
            r = iot_debug_config(pData);
            fnInfo() << "Gateway TYPE_APP_DEBUG!";
            break;
        default:
            fnInfo() << "Gateway pData->hdr.packetType=" << pData->hdr.packetType << "!";
            break;
    }
    if (r == -2)
    {
        fnInfo() << "Gateway pData->hdr.packetType=" << pData->hdr.packetType << "-" << pData->hdr.packetLen << " do not impl!";
    }
    else if (r == SOCKET_ERROR)
    {
        fnError() << "Gateway pData->hdr.packetType=" << pData->hdr.packetType << "-" << pData->hdr.packetLen << " ERROR!";
    }
    else
    {
        fnInfo() << "Gateway pData->hdr.packetType=" << pData->hdr.packetType << "-" << pData->hdr.packetLen << "!";
    }
}

int GatewayProtocol::mbPropGetResponse(APPDATA *pData)
{
    return -2;
}

int GatewayProtocol::mbProcessCommand(APPDATA *pData)
{
    return -2;
}

int GatewayProtocol::mbProcessDevStatus(APPDATA *pData)
{
    return -2;
}

int GatewayProtocol::mbPropSetProp(APPDATA *pData)
{
    fnInfo() << "Gateway.mbPropSetProp~~~";
    IEC104Server *iec104Server = (IEC104Server *) pData->buff;
    iec104Server->serverIp[127] = '\0';
    {
        CxMutexScope lock(f_clientEventLock);
        if (f_clientIEC104Servers.empty())
        {
            f_clientIEC104Servers.push_back(*iec104Server);
            f_clientEventCount = f_clientIEC104Servers.size() + f_clientDB_TABLEs.size() + f_clientFailureEvents.size() + f_clientPhaseNotify.size();
        }
    }
    cxDebug() << "Gateway Process Set Prop : ip=" << iec104Server->serverIp << " port104=" << iec104Server->port104
              << ", portTopo=" << iec104Server->portTopo;
    return sendAppAnswer(pData->hdr.mMid);
}

int GatewayProtocol::mbProcessAppTopoPattern(APPDATA *appData)
{
    fnInfo() << "Gateway.mbProcessAppTopoPattern...";
    char *path = (char *) appData->buff;
    cxDebug() << "Gateway Process App Topo Pattern : " << path;
    return 0;
}

int GatewayProtocol::mbProcessApp104Propreport(APPDATA *appData)
{
    fnInfo() << "Gateway.mbProcessApp104Propreport...";
    DB_TABLE *dbTable = (DB_TABLE *) appData->buff;
    dbTable->db[DB_TABLE_NAME_LEN-1] = '\0';
    dbTable->deviceTable[DB_TABLE_NAME_LEN-1] = '\0';
    {
        CxMutexScope lock(f_clientEventLock);
        if (f_clientDB_TABLEs.size() > 100)
        {
            f_clientDB_TABLEs.clear();
        }
        std::string key = std::string(dbTable->db)+","+std::string(dbTable->deviceTable);
        f_clientDB_TABLEs[key] = 1;
        f_clientEventCount = f_clientIEC104Servers.size() + f_clientDB_TABLEs.size() + f_clientFailureEvents.size() + f_clientPhaseNotify.size();
    }
    std::string dt = CxTime::currentMsepochString() + " ";
    cxDebug() << dt.c_str() << "Gateway Process App 104 Propreport : db=" << dbTable->db << " deviceTable=" << dbTable->deviceTable;
    return sendAppAnswer(appData->hdr.mMid);
}

int GatewayProtocol::mbProcessLineLoss(APPDATA *pData)
{
    fnInfo() << "Gateway.mbProcessLineLoss...";

    AREABUG_YX *dbTable = (AREABUG_YX *) pData->buff;
    pData->hdr.deviceId[127] = '\0';
    dbTable->branchDevice[15] = '\0';
    std::string deviceId = std::string((char *)pData->hdr.deviceId);

    if (_pProtocolSubject != NULL)
    {
        _pProtocolSubject->gateway_sendFailureEvents(deviceId, *dbTable);
    }
//    {
//        CxMutexScope lock(f_clientEventLock);
////        gateway_client_debug_print(EN_LOG_LEVEL_DEBUG, "f_clientFailureEvents.size=%d\n", f_clientFailureEvents.size());
//        if (f_clientFailureEvents.size() > 100)
//        {
//            f_clientFailureEvents.clear();
//        }
//        std::map<std::string, AREABUG_YX> tempMap;
//        std::string key = std::string((char *)pData->hdr.deviceId);
//        tempMap[key] = *dbTable;
//        f_clientFailureEvents[pData->hdr.mMid] = tempMap;
//        f_clientEventCount = f_clientIEC104Servers.size() + f_clientDB_TABLEs.size() + f_clientFailureEvents.size() + f_clientPhaseNotify.size();
//    }

//    gateway_client_debug_print(EN_LOG_LEVEL_DEBUG, "appHeader:\n\t appData->hdr.mMid=%d\n\t appData->hdr.requestId=%s\n\t"
//                                                   " appData->hdr.deviceId=%s\n\t appData->hdr.serverIdOrCommand=%s\n\t appData->hdr.packetType=0x%x\n\t appData->hdr.packetLen=%d",
//                               appData->hdr.mMid, appData->hdr.requestId, appData->hdr.deviceId,
//                               appData->hdr.serverIdOrCommand,appData->hdr.packetType, appData->hdr.packetLen);
//
//    gateway_client_debug_print(EN_LOG_LEVEL_DEBUG, "appBuff:\n\t dbTable->areaBugType=%d\n\t dbTable->areaBugStaA=%d\n\t"
//                                                   " dbTable->areaBugStaB=%d\n\t dbTable->areaBugStaC=%d\n\t dbTable->branchDevice=%s\n\t dbTable->recordTs=%ld",
//                               dbTable->areaBugType, dbTable->areaBugStaA, dbTable->areaBugStaB, dbTable->areaBugStaC, dbTable->branchDevice, dbTable->recordTs);

//    gateway_client_debug_print(EN_LOG_LEVEL_DEBUG, "%s Gateway Process App 104 Powerloss : hdr.packetType=%d  appData->hdr.deviceId=%s\n\n\n",
//                               dt.c_str(), appData->hdr.packetType, appData->hdr.deviceId);

//    gateway_client_debug_print(EN_LOG_LEVEL_DEBUG, "f_clientFailureEvents.size=%d\n", f_clientFailureEvents.size());

    return sendAppAnswer(pData->hdr.mMid);
}

int GatewayProtocol::mbProcessEventPhase(APPDATA *pData)
{
    fnInfo() << "Gateway.mbProcessEventPhase";
    PHASE_NOTIFY *phaseNotify = (PHASE_NOTIFY *)pData->buff;
    phaseNotify->deviceId[15] = '\0';
    pData->hdr.deviceId[MAX_DEV_ID_LEN - 1] = '\0';
    string sDeviceId = std::string((char *)pData->hdr.deviceId);

    if (_pProtocolSubject != NULL)
    {
        _pProtocolSubject->gateway_sendPhaseInfo(sDeviceId, *phaseNotify);
    }
//    {
//        CxMutexScope lock(f_clientEventLock);
//        if(f_clientPhaseNotify.size() > 100)
//        {
//            f_clientPhaseNotify.clear();
//        }
//        std::string key = std::string((char *)pData->hdr.deviceId);
//        f_clientPhaseNotify[key] = *phaseNotify;
//        f_clientEventCount = f_clientIEC104Servers.size() + f_clientDB_TABLEs.size() + f_clientFailureEvents.size() + f_clientPhaseNotify.size();
//    }

//    std::string dt = CxTime::currentMsepochString() + " ";
//    gateway_client_debug_print(EN_LOG_LEVEL_DEBUG, "appHeader:\n\t appData->hdr.mMid=%d\n\t appData->hdr.requestId=%s\n\t"
//                                                   " appData->hdr.deviceId=%s\n\t appData->hdr.serverIdOrCommand=%s\n\t appData->hdr.packetType=0x%x\n\t appData->hdr.packetLen=%d",
//                               appData->hdr.mMid, appData->hdr.requestId, appData->hdr.deviceId,
//                               appData->hdr.serverIdOrCommand,appData->hdr.packetType, appData->hdr.packetLen);
//
//    gateway_client_debug_print(EN_LOG_LEVEL_DEBUG, "appBuff:\n\t phaseNotify->deviceId=%s\n\t phaseNotify->PHASE_INFO=%d\n\t",
//                               phaseNotify->deviceId, phaseNotify->PHASE_INFO);

    return sendAppAnswer(pData->hdr.mMid);
}

int GatewayProtocol::iot_debug_config(APPDATA *pData)
{
    debug_switch = pData->hdr.packetLen;
    return sendAppAnswer(pData->hdr.mMid);
}


GatewayProtocol::GatewayProtocol(CxIGatewayProtocolSubject *pProtocolSubject)
{
    _channel = NULL;
    _pProtocolSubject = pProtocolSubject;
}

GatewayProtocol::~GatewayProtocol()
{
    GM_DELETEANDNULL_OBJECT(_channel);
}


/**
 * [--listen-port] | [--remote-ip] [--remote-port] | [--port-name] [--baud-rate]
 * @return
 */
int GatewayProtocol::on_connectBn_clicked()
{
    if (_channel)
    {
        if (! _channel->connected())
        {
            _channel->open();
        }
        return 0;
    }

    string sListenPort = CxArguments::getArgValue("listen-port");
    string sRemoteIp = CxArguments::getArgValue("remote-ip");
    string sRemotePort = CxArguments::getArgValue("remote-port");
    string sPortName = CxArguments::getArgValue("port-name");
    string sBaudRate = CxArguments::getArgValue("baud-rate");
    string sSendData = CxArguments::getArgValue("send-data");
    string sSendInterval = CxArguments::getArgValue("send-interval");
    if (!sRemoteIp.empty() && ! CxString::isValidIp(sRemoteIp))
    {
        return -1;
    }
    int iSendInterval = 0;
    iSendInterval = CxString::toInt32(sSendInterval);
    if (sSendData.size() > 0)
    {
        _sendData = CxString::fromHexstring(sSendData);
        iSendInterval = CxString::toInt32(sSendInterval);
        if (iSendInterval < 10 || iSendInterval > 999999)
        {
            iSendInterval = 1000;
        }
    }
    if (iSendInterval > 0)
    {
        _sendTimer.init(this, iSendInterval);
        _sendTimer.start();
    }
    //udp
    //tcp client
    //tcp server
    //udp (multi road)
    //tcp (multi road)
    CHANNEL_TYPE_ENUM_T eType = CHANNEL_TYPE_MAX;
    if (sListenPort.size() > 0)
    {
        eType = sRemotePort.size() > 0 ? CHANNEL_TYPE_UDP : CHANNEL_TYPE_TCP_SERVER;
    }
    else
    {
        eType = sRemotePort.size() > 0 ? CHANNEL_TYPE_TCP_CLIENT : CHANNEL_TYPE_SERIAL;
    }

    switch (eType)
    {
        case CHANNEL_TYPE_TCP_CLIENT:
        {
            int iRemotePort = CxString::toInt32(sRemotePort);
            if (iRemotePort < 1 || iRemotePort > 0xFFFF)
            {
                return -1;
            }
            if (sRemoteIp.empty())
            {
                sRemoteIp = "127.0.0.1";
            }
            CxChannelTcpclient * oTcpclient = new CxChannelTcpclient();
            _channel = oTcpclient;
            oTcpclient->setRemoteIp(sRemoteIp);
            oTcpclient->setRemotePort(iRemotePort);
        }
            break;
        case CHANNEL_TYPE_UDP:
        {
            int iListPort = CxString::toInt32(sListenPort);
            int iRemotePort = CxString::toInt32(sRemotePort);
            if (iRemotePort < 1 || iRemotePort > 0xFFFF || iListPort < 1 || iListPort > 0xFFFF)
            {
                return -1;
            }
            if (sRemoteIp.empty())
            {
                sRemoteIp = "127.0.0.1";
            }
            CxChannelUdp * oUdp = new CxChannelUdp();
            _channel = oUdp;
            oUdp->setLocalIp("0.0.0.0");
            oUdp->setLocalPort(iListPort);
            oUdp->setRemoteIp(sRemoteIp);
            oUdp->setRemotePort(iRemotePort);
        }
            break;
        case CHANNEL_TYPE_TCP_SERVER:
        {
            int iListPort = CxString::toInt32(sListenPort);
            if (iListPort < 1 || iListPort > 0xFFFF)
            {
                return -1;
            }
            CxChannelTcpserver * oTcpServer = new CxChannelTcpserver();
            _channel = oTcpServer;
            oTcpServer->setLocalIp("0.0.0.0");
            oTcpServer->setLocalPort(iListPort);
        }
            break;
        case CHANNEL_TYPE_SERIAL:
        {
            CxChannelSerial::BaudRateEnum eBaudRate = CxChannelSerial::toBaudRateEnum(CxString::toInt32(sBaudRate));
            if (sPortName.empty())
            {
                return -1;
            }
            CxChannelSerial * oSerial = new CxChannelSerial();
            _channel = oSerial;
            oSerial->setPortName(sPortName);
            oSerial->setBaudRate(eBaudRate);
        }
            break;
        default:
            break;
    }
    _channel->addObserver(this);
    _channel->setAutoOpenInterval(3000);
    if (eType == CHANNEL_TYPE_TCP_SERVER || eType == CHANNEL_TYPE_UDP)
    {
        _channel->setIsMultiRoad(true);
    }
    _channel->open();

    //
    int iCount = 0;
    while (! _channel->connected())
    {
        if (iCount > 3000)
            break;
        CxApplication::waiting(1);
        ++iCount;
    }

    if (!_channel->connected())
    {
        fnDebug() << " connect fail!!!";
        GM_DELETEANDNULL_OBJECT(_channel);
        return 1;
    }
    else
    {
        fnDebug() << " connect success!!!";
        return 0;
    }
}

void GatewayProtocol::on_closeBn_clicked()
{
    if (_channel && _channel->connected())
    {
        _channel->close();
        GM_DELETEANDNULL_OBJECT(_channel);
    }
}

int GatewayProtocol::init_client()
{
    if (_channel)
    {
        if (! _channel->connected())
        {
            _channel->open();
        }
        return 0;
    }

    int iRemotePort = CxString::toInt32(s_RemotePort);
    if (iRemotePort < 1 || iRemotePort > 0xFFFF)
    {
        return -1;
    }
    if (s_RemoteIp.empty())
    {
        s_RemoteIp = "127.0.0.1";
    }
    CxChannelTcpclient * oTcpclient = new CxChannelTcpclient();
    _channel = oTcpclient;
    oTcpclient->setRemoteIp(s_RemoteIp);
    oTcpclient->setRemotePort(iRemotePort);

    _channel->addObserver(this);
    _channel->setAutoOpenInterval(3000);
    _channel->open();

    CxApplication::waiting(3);

    if (!_channel->connected())
    {
        fnDebug() << " connect fail." << " connect ip " << s_RemoteIp << " port " << iRemotePort;
//        GM_DELETEANDNULL_OBJECT(_channel);
        return 1;
    }
    else
    {
        fnDebug() << " connect success." << " connect ip " << s_RemoteIp << " port " << iRemotePort;
        return 0;
    }
}

GatewayProtocol* GatewayProtocol::start(const char *appName, const char *deviceName, int devNameLen, int protocol, CxIGatewayProtocolSubject *oGatewayProtocolSubject)
{
    if (appName != NULL)
    {
        memcpy(_S_APP_NAME, appName, strlen(appName));
        memcpy(_S_DEVICE_NAME, deviceName, devNameLen);
        _I_PROTOCOL = protocol;
    }

    if (f_testor != NULL)
    {
        return f_testor;
    }
    f_testor = new GatewayProtocol(oGatewayProtocolSubject);
    f_testor->init_client();
    return f_testor;
}

bool GatewayProtocol::channel_canChangeConnect(const CxChannelBase *oChannel, bool bOldConnect, bool bNewConnect)
{
    return true;
}

void GatewayProtocol::channel_connectChanged(const CxChannelBase *oChannel)
{
    fnDebug() << ".connect ----------";
    sendAppNameUp();
}

void GatewayProtocol::channel_beforeDelete(const CxChannelBase *oChannel)
{

}

void GatewayProtocol::channel_receivedData(const uchar *pData, int iLength, void *oSource)
{
    gatewayProcessData(pData, iLength);
//    fnDebug() << ".recv: " << CxString::toHexstring(pData, iLength);
//    GatewayMessAssembly::gatewayProcessData((HW_CHAR *)pData);
}

void GatewayProtocol::timer_timeOut(const CxTimer *oTimer)
{
    if (_channel && _channel->isOpen())
    {
        _channel->sendBytes(_sendData);
        _sendData.clear();
    }
}

bool GatewayProtocol::isConnection()
{
    return _channel->isOpen();
}

int GatewayProtocol::mbProcessCNSOpenCover(APPDATA *pData)
{
    fnInfo() << "Gateway.mbProcessCNSOpenCover";
    unsigned char mac[7] = {0};
    int openTimes = 0;

    memcpy(mac, pData->hdr.deviceId, 6);
    openTimes = pData->buff[1] << 24;
    openTimes += pData->buff[2] << 16;
    openTimes += pData->buff[3] << 8;
    openTimes += pData->buff[4];

    if(_pProtocolSubject != NULL)
    {
        _pProtocolSubject->gateway_sendCNSOpenCover(mac, openTimes);
    }

    return sendAppAnswer(pData->hdr.mMid);
}

int GatewayProtocol::mbProcessCNSLowVoltageDetect(APPDATA *pData)
{
    fnInfo() << "Gateway.mbProcessCNSLowVoltageDetect";

    CNSLowVoltageDetect lowVoltageDetect;

    memcpy(lowVoltageDetect.mac, pData->hdr.deviceId, 6);
    CNS_LOW_VOLTAGE_DATE *lowVoltageData = (CNS_LOW_VOLTAGE_DATE *)pData->buff;

    lowVoltageDetect.meterClass = lowVoltageData->meterClass;
    lowVoltageDetect.phaseResultA = lowVoltageData->phaseResultA;
    lowVoltageDetect.phaseResultB = lowVoltageData->phaseResultB;
    lowVoltageDetect.phaseResultC = lowVoltageData->phaseResultC;

    fnDebug() << "mac: " << lowVoltageDetect.mac
              << "meterClass: " << lowVoltageDetect.meterClass
              << "phaseResultA: " << lowVoltageDetect.phaseResultA
              << "phaseResultB: " << lowVoltageDetect.phaseResultB
              << "phaseResultC: " << lowVoltageDetect.phaseResultC;

    if(_pProtocolSubject != NULL)
    {
        _pProtocolSubject->gateway_sendCNSLowVoltageDetect(&lowVoltageDetect);
    }

    return sendAppAnswer(pData->hdr.mMid);
}

int GatewayProtocol::mbProcessEventPowerOn(APPDATA *pData)
{
    fnInfo() << "Gateway.mbProcessEventPowerOn";

    pData->hdr.deviceId[MAX_DEV_ID_LEN-1] = '\0';
    if(_pProtocolSubject != NULL)
    {
        _pProtocolSubject->gateway_sendEventPowerOn((char *)pData->hdr.deviceId);
    }

    return sendAppAnswer(pData->hdr.mMid);
}

int GatewayProtocol::mbProcessEventPowerOff(APPDATA *pData)
{
    fnInfo() << "Gateway.mbProcessEventPowerOff";

    pData->hdr.deviceId[MAX_DEV_ID_LEN-1] = '\0';
    if(_pProtocolSubject != NULL)
    {
        _pProtocolSubject->gateway_sendEventPowerOff((char *)pData->hdr.deviceId);
    }

    return sendAppAnswer(pData->hdr.mMid);
}



static std::vector<std::vector<HW_UCHAR>> f_sDeviceCollectBuffers;
static std::vector<int> f_sDeviceCollectCounts;
static int f_iDeviceCollectSend = 0;
static int f_iDeviceCollectTotal = 0;


/*******************************************************************************serverID开始****************************************************************************/

/************************************APPDATA_HDR_SERVERID配置区**************************************************/
//固定头
#define SERVERID_INDEX_H00_M00_J00_DEFAULT					(0xFF)		//默认固定头_00
#define SERVERID_INDEX_H01_M00_J00_DEFAULT					(0xFF)		//默认固定头_01
#define SERVERID_INDEX_H02_M00_J00_DEFAULT					(0xFF)		//默认固定头_02
#define SERVERID_INDEX_H03_M00_J00_DEFAULT					(0xFF)		//默认固定头_03



//版本号
#define SERVERID_INDEX_MAJOR_VERSION_NUM		(0x00)		//主版本号
#define SERVERID_INDEX_JUNIOR_VERSION_NUM		(0x00)		//次版本号

//功能-->app发给myapp
#define SERVERID_INDEX_PROTOCOL_MIDWARE_PLCMAIL		(0x00)		//使用PLC协议
#define SERVERID_INDEX_PROTOCOL_APPMANAGE_TRANS		(0x01)		//在appManage时直接转发
#define SERVERID_INDEX_PROTOCOL_MYAPP_PARA			(0x04)		//对myapp的参数进行操作（get,set...）
//#define SERVERID_INDEX_PROTOCOL_MYAPP_CMD			(0x05)		//在myapp中执行cmd

//功能-->myapp发给app
#define SERVERID_INDEX_PROTOCOL_MYAPP_NOTIFY		(0xf0)		//myapp对其他业务app的通知




/**********************************************V00通用配置区*******************************************************/
#define	SERVERID_WAIT_NO						(0x00)		//不需要应答
#define	SERVERID_WAIT_NEED						(0x01)		//需要应答
#define	SERVERID_SEND_PROT_DEFAULT				(0x00)		//使用注册时的协议
#define	SERVERID_DESTPROT_DEFAULT				(0x00)		//自发自收
#define	SERVERID_SEND_PACKET_TYPE_DEFAULT		(0x00)		//使用发送时的包类型




/****************************************MIDWARE_HDR_SERVERID_V00配置区*********************************************/


/****************************************APPMANAGE_HDR_SERVERID_V00配置区*********************************************/
//packetType为TYPE_APP_PROTOCOL_APPMANAGE时
#define TYPE_APP_APPMANAGE_PROT_PLCMAIL				(0x01)		//执行邮箱协议
#define APPMANAGE_PROT_PLCMAIL_DEV_GDO					(0x01)		//实时抄读此设备(DGO--get dev data ontime)
#define APPMANAGE_PROT_PLCMAIL_DEV_SDO              (0x02)      //实时抄读此设备结果(DGO--get dev data ontime)
#define APPMANAGE_PLCMAIL_FLAG_DEVGDO_OFF					(0x00)		//不进行实时抄读
#define APPMANAGE_PLCMAIL_FLAG_DEVGDO_ON					(0x01)		//接收实时抄读指令,清理现场
#define APPMANAGE_PLCMAIL_FLAG_DEVGDO_START					(0x02)		//开始实时抄读
#define APPMANAGE_PLCMAIL_FLAG_DEVGDO_SUCC					(0x03)		//实时抄读成功

#define TYPE_APP_APPMANAGE_NOTIFY_RECV_PACKET		(0xf0)		//uchar-通知收到报文
#define  APPMANAGE_NOTIFY_RECV_PACKET_LOCKED			(0x01)	//uchar-通知收到报文子集--->收到报文，条件不符合，拒绝执行
#define  APPMANAGE_NOTIFY_RECV_PACKET_ACK				(0x02)	//uchar-通知收到报文子集--->收到报文，回复确认
#define  APPMANAGE_NOTIFY_RECV_PACKET_DEAL_SUC			(0x11)	//uchar-通知收到报文子集--->收到报文，执行成功
#define  APPMANAGE_NOTIFY_RECV_PACKET_DEAL_FAI			(0x12)	//uchar-通知收到报文子集--->收到报文，执行失败



typedef struct plcMail_DevGDO_BufHdr	//dev --> get data on time
{
    unsigned char sumPacket;			//总包数
    unsigned char seqPacket;			//当前包序号
    unsigned short sumAllDev;			//总设备数
    unsigned short sumHaveDev;		//此包包含设备数
    unsigned char	devType;			//包含设备类型
    unsigned char	*devAddrBuff;		//设备地址存储buf
}PLCMAIL_DEVGDO_BUFHDR;


/****************************************MYAPPPARA_HDR_SERVERID_V00配置区*********************************************/
//模块命名
#define  TYPE_MYAPPPARA_MIDWARE						(0x0a)	//uchar-中间件模块
#define  MIDWARE_MODE_PRI								(0x01)	//uchar-中间件模块的优先级

//功能码
#define  TYPE_MYAPPPARA_GET									(0x10)	//uchar-获取参数
#define  TYPE_MYAPPPARA_GET_SUCC									(0x11)	//uchar-获取参数成功
#define  TYPE_MYAPPPARA_GET_FAIL									(0x12)	//uchar-获取参数失败
#define  TYPE_MYAPPPARA_SET									(0x20)	//uchar-设置参数
#define  TYPE_MYAPPPARA_SET_SUCC									(0x21)	//uchar-设置参数成功
#define  TYPE_MYAPPPARA_SET_FAIL									(0x22)	//uchar-设置参数失败

/****************************************NOTIFY_HDR_SERVERID_V00配置区*********************************************/
//中间件超时通知
#define  TYPE_APP_MIDWARE_NOTIFY_TIMEOUT			(0x0a)	//uchar-通知超时
#define  MIDWARE_NOTIFY_TIME_SEND						(0x01)	//uchar-通知超时子集--->报文在等待发送时超时
#define  MIDWARE_NOTIFY_TIME_RECV						(0x02)	//uchar-通知超时子集--->报文在等待应答时超时
#define  MIDWARE_NOTIFY_TIME_APPMANAGE_IDEL				(0x10)	//uchar-通知超时子集--->appManage长时间接收不到报文

//中间件对收到报文的响应
#define  TYPE_APP_MIDWARE_NOTIFY_RECV_PACKET		(0x0b)	//uchar-通知收到报文
#define  MIDWARE_NOTIFY_RECV_PACKET_LOCKED				(0x01)	//uchar-通知收到报文子集--->收到报文，无权限访问，拒绝执行
#define  MIDWARE_NOTIFY_RECV_PACKET_ACK					(0x02)	//uchar-通知收到报文子集--->收到报文，回复确认
#define  MIDWARE_NOTIFY_RECV_PACKET_REG_CLI_SUC			(0x11)	//uchar-通知收到报文子集--->收到程序注册Client报文,注册成功
#define  MIDWARE_NOTIFY_RECV_PACKET_REG_CLI_FAI			(0x12)	//uchar-通知收到报文子集--->收到程序注册Client报文，注册失败
#define  MIDWARE_NOTIFY_RECV_PACKET_REG_MID_SUC			(0x15)	//uchar-通知收到报文子集--->收到程序注册Midware报文,注册成功
#define  MIDWARE_NOTIFY_RECV_PACKET_REG_MID_FAI			(0x16)	//uchar-通知收到报文子集--->收到程序注册Midware报文，注册失败
#define  MIDWARE_NOTIFY_RECV_PACKET_BUS_SUC				(0x19)	//uchar-通知收到报文子集--->收到业务报文,向中间件注册成功
#define  MIDWARE_NOTIFY_RECV_PACKET_BUS_FAI				(0x1a)	//uchar-通知收到报文子集--->收到注册报文，向中间件注册失败

/***************************************************************************************下标定义区*************************************************************************/
typedef enum {
    SERVERID_INDEX_HEAD_00,				//固定头 00
    SERVERID_INDEX_HEAD_01,				//固定头 00
    SERVERID_INDEX_HEAD_02,				//固定头 00
    SERVERID_INDEX_HEAD_03,				//固定头 00
    SERVERID_INDEX_MAJOR_VERSION,		//主版本号
    SERVERID_INDEX_JUNIOR_VERSION,		//次版本号
    SERVERID_INDEX_PROTOCOL,			//功能 00->使用PLC，应答报文转发;  01->直接转发；0xf0->myapp通知client
    SERVERID_INDEX_MAX
}APPDATA_HDR_SERVERID;


typedef enum {
    MW_SERVERID_INDEX_SEQ=0x10,					//包序号；
    MW_SERVERID_INDEX_WAIT,						//报文是否需要回复，0x00->不需要应答（默认）；0x01->需要应答；
    MW_SERVERID_INDEX_SEND_PROT,				//通过plc时使用的prot,00->不修改，使用注册的prot(默认);其他->对应的业务号；
    MW_SERVERID_INDEX_SEND_PRIO,				//通过plc时使用的优先级
    MW_SERVERID_INDEX_DESTPROT_START_1=0x30,	//应答报文的目的地注册业务号起始1，0x00->自发自收（默认）；start1与end1有一个为0则自发自收；
    MW_SERVERID_INDEX_DESTPROT_END_1,			//应答报文的目的地注册业务号结束1，0x00->自发自收（默认）；start1与end1有一个为0则自发自收；
    MW_SERVERID_INDEX_MAX
}MIDWARE_HDR_SERVERID_V00;	//SERVERID_INDEX_PROTOCOL == 0x00;版本V00;

typedef enum {
    AM_SERVERID_INDEX_SEQ=0x10,						//包序号；
    AM_SERVERID_INDEX_WAIT,							//接收方app报文是否需要回复，0x00->不需要应答（默认）；0x01->需要应答；
    AM_SERVERID_INDEX_PACKET_TYPE,					//接收方app报文包类型，00->不修改，使用原来的包类型(默认)；其他->转发时将appData->hdr.packetType修改为该值
    AM_SERVERID_INDEX_FUNC,							//根据包类型使用不通的主功能码
    AM_SERVERID_INDEX_SUBFUNC,						//根据包类型和主功能码使用不通的子功能码
    AM_SERVERID_INDEX_RESLPROT_START_1=0x20,		//应答报文目的地app业务号起始1，appManage不解析
    AM_SERVERID_INDEX_RESLPROT_END_1,				//应答报文目的地app业务号结束1，appManage不解析
    AM_SERVERID_INDEX_DESTPROT_START_1=0x30,		//发送报文的目的地app业务号起始1，0x00->自发自收（默认）；start1与end1有一个为0则自发自收；
    AM_SERVERID_INDEX_DESTPROT_END_1,				//发送报文的目的地app业务号结束1，0x00->自发自收（默认）；start1与end1有一个为0则自发自收；
    AM_SERVERID_INDEX_MAX
}APPMANAGE_HDR_SERVERID_V00;	//SERVERID_INDEX_PROTOCOL == 0x01;版本V00;

typedef enum {
    MP_SERVERID_INDEX_SEQ=0x10,					//包序号；
    MP_SERVERID_INDEX_WAIT,						//报文是否需要回复，0x00->需要应答（默认）；0x01->不需要应答；
    MP_SERVERID_INDEX_FUNC,						//功能码（具体操作set,get...）
    MP_SERVERID_INDEX_MODE,						//模块编号(midware...)（0x0a
    MP_SERVERID_INDEX_SUBMODE,					//子模块编号(midware的优先级)(0x01
    MP_SERVERID_INDEX_MAX
}MYAPPPARA_HDR_SERVERID_V00;	//MYAPPPARA_HDR_SERVERID_V00 == 0x04;版本V00;


typedef enum {
    NY_SERVERID_INDEX_SEQ=0x10,					//包序号；
    NY_SERVERID_INDEX_WAIT,						//报文是否需要回复，0x00->需要应答（默认）；0x01->不需要应答；
    NY_SERVERID_INDEX_FUNC,						//主功能码
    NY_SERVERID_INDEX_SUBFUNC,					//子功能码
    NY_SERVERID_INDEX_MAX
}NOTIFY_HDR_SERVERID_V00;	//SERVERID_INDEX_PROTOCOL == 0xf0;版本V00;

/*******************************************************************************serverID结束****************************************************************************/


int GatewayProtocol::sendClockSync(char packetNum, const unsigned char *coreDate, unsigned short dataLen)
{
    APPDATA appdata = {0};

    HW_UCHAR *serverIdOrCommand = appdata.hdr.serverIdOrCommand;
    // serverIdOrCommand
    serverIdOrCommand[SERVERID_INDEX_HEAD_00] = SERVERID_INDEX_H00_M00_J00_DEFAULT;
    serverIdOrCommand[SERVERID_INDEX_HEAD_01] = SERVERID_INDEX_H01_M00_J00_DEFAULT;
    serverIdOrCommand[SERVERID_INDEX_HEAD_02] = SERVERID_INDEX_H02_M00_J00_DEFAULT;
    serverIdOrCommand[SERVERID_INDEX_HEAD_03] = SERVERID_INDEX_H03_M00_J00_DEFAULT;
    serverIdOrCommand[SERVERID_INDEX_MAJOR_VERSION] = SERVERID_INDEX_MAJOR_VERSION_NUM;
    serverIdOrCommand[SERVERID_INDEX_JUNIOR_VERSION] = SERVERID_INDEX_JUNIOR_VERSION_NUM;
    serverIdOrCommand[SERVERID_INDEX_PROTOCOL] = SERVERID_INDEX_PROTOCOL_MIDWARE_PLCMAIL;
    serverIdOrCommand[MW_SERVERID_INDEX_SEQ] = 0x00;
    serverIdOrCommand[MW_SERVERID_INDEX_WAIT] = 0x01;
    serverIdOrCommand[MW_SERVERID_INDEX_SEND_PROT] = 0x08;
    serverIdOrCommand[MW_SERVERID_INDEX_SEND_PRIO] = 0x02;
    serverIdOrCommand[MW_SERVERID_INDEX_DESTPROT_START_1] = 0x08;
    serverIdOrCommand[MW_SERVERID_INDEX_DESTPROT_END_1] = 0x08;

    METER_PACKET *pInfo = (METER_PACKET *)appdata.buff;
    appdata.hdr.serverIdOrCommand[0] = 0x00;
    appdata.hdr.serverIdOrCommand[1] = 0x08;
    appdata.hdr.serverIdOrCommand[2] = 0x08;
    appdata.hdr.packetType = TYPE_SEND_SOUTHING_DATA;
    //    appdata.hdr.packetLen = sizeof(METER_PACKET);
    pInfo->packerInfo = PLC_MAIL_PACKET_INDEX;
    pInfo->packetLen = 3;
    pInfo->param[0] = plcMailSeq;
    plcMailSeq = plcMailSeq + 1 == 255 ? 0 : plcMailSeq + 1;
    pInfo->param[1] = APP_SYNC_TIME;
    pInfo->param[2] = packetNum;

    METER_PAYLOAD *meterPayload = (METER_PAYLOAD *)&appdata.buff[2+pInfo->packetLen];
    meterPayload->payloadInfo = PLC_MAIL_PAYLOAD_INDEX;
    if(coreDate != NULL && dataLen != 0 ){
        meterPayload->paloadLenHi = dataLen >> 8;
        meterPayload->paloadLenLow = (char)dataLen;
        //        printf("meterPayload->paloadLenHi:%x\n",meterPayload->paloadLenHi);
        //        printf("meterPayload->paloadLenLow:%x\n",meterPayload->paloadLenLow);
        memcpy(meterPayload->param, coreDate , dataLen);
        appdata.hdr.packetLen = 8 + dataLen;
    }else{
        meterPayload->paloadLenHi = 0x00;
        meterPayload->paloadLenLow = 0x00;
        appdata.hdr.packetLen = 8;
    }
    //    fnDebug() << ".sendClockSync: " << CxString::toHexstring((char *)&appdata, sizeof(APPDATA));
    uchar *dataBegin = (uchar *)&appdata;
    uchar *dataEnd = dataBegin + sizeof(appdata);
    vector<uchar> res(dataBegin, dataEnd);
    int r = _channel->sendBytes(res);
    fnInfo() << "Send Clock Sync, result= " << r;
    string sHexServerIdOrCommand = CxString::toHexstring(serverIdOrCommand, MAX_ID_LEN);
    string sHexBuff = CxString::toHexstring(appdata.buff, appdata.hdr.packetLen);
    fnDebug() <<"Send Clock Sync, Hex ServerIdOrCommand :" << sHexServerIdOrCommand;
    fnDebug() <<"Send Clock Sync, Hex Buff :" << sHexBuff;
    return r;
}

int GatewayProtocol::sendAreaIdentify(const unsigned char *codeDate, unsigned short dataLen)
{
    APPDATA appdata = {0};

    METER_PACKET *pInfo = (METER_PACKET *)appdata.buff;
    appdata.hdr.serverIdOrCommand[0] = 0x00;
    appdata.hdr.serverIdOrCommand[1] = 0x08;
    appdata.hdr.serverIdOrCommand[2] = 0x08;
    appdata.hdr.packetType = TYPE_SEND_SOUTHING_DATA;
    //    appdata.hdr.packetLen = sizeof(METER_PACKET);
    pInfo->packerInfo = PLC_MAIL_PACKET_INDEX;
    pInfo->packetLen = dataLen;
    pInfo->param[0] = plcMailSeq;
    plcMailSeq = plcMailSeq + 1 == 255 ? 0 : plcMailSeq + 1;
    pInfo->param[1] = APP_AREA_IDENTIFY;
    if (codeDate != NULL)
    {
        memcpy(pInfo->param + 2, codeDate , dataLen);
    }

    METER_PAYLOAD *meterPayload = (METER_PAYLOAD *)&appdata.buff[2+pInfo->packetLen];
    meterPayload->payloadInfo = PLC_MAIL_PAYLOAD_INDEX;

    meterPayload->paloadLenHi = 0x00;
    meterPayload->paloadLenLow = 0x00;
    appdata.hdr.packetLen = 8;

    //    fnDebug() << ".sendClockSync: " << CxString::toHexstring((char *)&appdata, sizeof(APPDATA));
    uchar *dataBegin = (uchar *)&appdata;
    uchar *dataEnd = dataBegin + sizeof(appdata);
    vector<uchar> res(dataBegin, dataEnd);
    int r = _channel->sendBytes(res);
    fnInfo() << "Gateway.sendClockSync... result=" << r;
    return r;
}

int GatewayProtocol::sendBusiness(char code, char packetNum, char function, const unsigned char *coreData, unsigned short dataLen)
{
    APPDATA appdata = {0};

    appdata.hdr.serverIdOrCommand[0] = 0x00;
    appdata.hdr.serverIdOrCommand[1] = 0x08;
    appdata.hdr.serverIdOrCommand[2] = 0x08;
    appdata.hdr.packetType = TYPE_SEND_SOUTHING_DATA;
    METER_PACKET *meterPacket = (METER_PACKET*)appdata.buff;
    meterPacket->packerInfo = PLC_MAIL_PACKET_INDEX;
    meterPacket->packetLen = 4;

    meterPacket->param[0] = plcMailSeq;
    plcMailSeq = plcMailSeq + 1 == 255 ? 0 : plcMailSeq + 1;
    meterPacket->param[1] = APP_CNS_COLLECT;
    meterPacket->param[2] = code;
    meterPacket->param[3] = packetNum;

    METER_PAYLOAD *meterPayload = (METER_PAYLOAD*)&appdata.buff[2+meterPacket->packetLen];
    meterPayload->payloadInfo = PLC_MAIL_PAYLOAD_INDEX;
    meterPayload->param[0] = function;
    if(coreData != NULL && dataLen != 0 ){
        meterPayload->paloadLenHi = dataLen >> 8;
        meterPayload->paloadLenLow = (char)dataLen;
        memcpy(meterPayload->param+1, coreData , dataLen);
        appdata.hdr.packetLen = 9 + dataLen + 1;
    }else{
        meterPayload->paloadLenHi = 0x00;
        meterPayload->paloadLenLow = 0x00;
        appdata.hdr.packetLen = 9;
    }

    //    fnDebug() << ".sendBusiness: " << CxString::toHexstring((char *)&appdata, sizeof(APPDATA));

    uchar *dataBegin = (uchar *)&appdata;
    uchar *dataEnd = dataBegin + sizeof(appdata);
    vector<uchar> res(dataBegin, dataEnd);
    int r = _channel->sendBytes(res);
    fnInfo() << "Gateway.sendBusiness... result=" << r;
    return r;
}

int GatewayProtocol::sendServiceStatus(int state, const char *sockAddr, const char *networkName,
                                       unsigned long long recvTotalLength, unsigned long long recvLastTime)
{
    fnInfo() << "Gateway.sendServiceStatus...";

    APPDATA appdata = {0};

    BusinessStateInfo *pInfo = (BusinessStateInfo *) appdata.buff;
    appdata.hdr.packetType = TYPE_APP_TEST_COMMUNICATION_LINK;
    appdata.hdr.packetLen = sizeof(BusinessStateInfo);

    pInfo->state = state;
    strncpy(pInfo->sockAddress, sockAddr, SOCK_ADDR_MAX_LEN);
    strncpy(pInfo->networkName, networkName, NETWORK_NAME_MAX_LEN);
    pInfo->recvTotalLength = recvTotalLength;
    pInfo->recvLastTime = recvLastTime;

    //    fnDebug() << ".sendServiceStatus: " << CxString::toHexstring((char *)&appdata, sizeof(APPDATA));

    uchar *dataBegin = (uchar *)&appdata;
    uchar *dataEnd = dataBegin + sizeof(appdata);
    vector<uchar> res(dataBegin, dataEnd);
    return _channel->sendBytes(res);
}

int GatewayProtocol::doSendDeviceCollect()
{
    if (! isConnection())
    {
        fnDebug().out("Send Device Collect Fail INVALID_SOCKET [ %d / %d ].", f_iDeviceCollectSend, f_sDeviceCollectBuffers.size());
        return -1;
    }
    if (f_iDeviceCollectSend >= f_sDeviceCollectBuffers.size())
    {
        fnDebug().out("Send Device Collect Fail [ %d / %d ].", f_iDeviceCollectSend, f_sDeviceCollectBuffers.size());
        return -1;
    }
    APPDATA appdata = {0};
    HW_UCHAR *serverIdOrCommand = appdata.hdr.serverIdOrCommand;
    // serverIdOrCommand
    serverIdOrCommand[SERVERID_INDEX_HEAD_00] = SERVERID_INDEX_H00_M00_J00_DEFAULT;
    serverIdOrCommand[SERVERID_INDEX_HEAD_01] = SERVERID_INDEX_H01_M00_J00_DEFAULT;
    serverIdOrCommand[SERVERID_INDEX_HEAD_02] = SERVERID_INDEX_H02_M00_J00_DEFAULT;
    serverIdOrCommand[SERVERID_INDEX_HEAD_03] = SERVERID_INDEX_H03_M00_J00_DEFAULT;
    serverIdOrCommand[SERVERID_INDEX_MAJOR_VERSION] = SERVERID_INDEX_MAJOR_VERSION_NUM;
    serverIdOrCommand[SERVERID_INDEX_JUNIOR_VERSION] = SERVERID_INDEX_JUNIOR_VERSION_NUM;
    serverIdOrCommand[SERVERID_INDEX_PROTOCOL] = SERVERID_INDEX_PROTOCOL_APPMANAGE_TRANS;
    serverIdOrCommand[AM_SERVERID_INDEX_PACKET_TYPE] = SERVERID_SEND_PACKET_TYPE_DEFAULT;
    serverIdOrCommand[AM_SERVERID_INDEX_FUNC] = TYPE_APP_APPMANAGE_PROT_PLCMAIL;
    serverIdOrCommand[AM_SERVERID_INDEX_SUBFUNC] = APPMANAGE_PROT_PLCMAIL_DEV_GDO;
    serverIdOrCommand[AM_SERVERID_INDEX_RESLPROT_START_1] = APP_PACKET_VIRTUAL_104;
    serverIdOrCommand[AM_SERVERID_INDEX_RESLPROT_END_1] = APP_PACKET_VIRTUAL_104;
    serverIdOrCommand[AM_SERVERID_INDEX_DESTPROT_START_1] = APP_PACKET_VIRTUAL_METER;
    serverIdOrCommand[AM_SERVERID_INDEX_DESTPROT_END_1] = APP_PACKET_VIRTUAL_METER;
    // buffer
    const std::vector<HW_UCHAR> &curText = f_sDeviceCollectBuffers.at(f_iDeviceCollectSend);
    unsigned short curCount = f_sDeviceCollectCounts[f_iDeviceCollectSend];
    HW_UCHAR *buffer = appdata.buff;
    buffer[0x00] = f_sDeviceCollectBuffers.size();
    buffer[0x01] = f_iDeviceCollectSend;
    buffer[0x02] = (f_iDeviceCollectTotal & 0xFF00) >> 8;
    buffer[0x03] = f_iDeviceCollectTotal;
    buffer[0x04] = (curCount & 0xFF00) >> 8;
    buffer[0x05] = curCount;
    buffer[0x06] = 0x08;
    memcpy(buffer+0x10, &curText.front(), curText.size());
    // hdr
    appdata.hdr.packetType = TYPE_APP_PROTOCOL_APPMANAGE;
    appdata.hdr.packetLen = curText.size();
    int rst = _channel->sendData((char *) &appdata, sizeof(APPDATA));
    fnDebug().out("Send Device Collect [ %d / %d ]. %s.", f_iDeviceCollectSend, f_sDeviceCollectBuffers.size(), (char *)(appdata.buff+0x10));
    return rst;
}

int GatewayProtocol::sendDeviceCollect(std::vector<std::string> sDevices)
{
    if (f_testor == NULL || ! f_testor->isConnection() || sDevices.size() <= 0)
    {
        return -1;
    }
    f_sDeviceCollectBuffers.clear();
    f_sDeviceCollectCounts.clear();
    HW_UCHAR buff[MAX_MESSAGE_BUF_LEN];
    int index = 0;
    int count = 0;
    int iSize = sDevices.size();
    for (int i = 0; i < iSize; ++i)
    {
        const std::string &sDevice = sDevices.at(i);
        int after = index + sDevice.size();
        if (after > (MAX_MESSAGE_BUF_LEN-24))
        {
            f_sDeviceCollectBuffers.push_back(std::vector<HW_UCHAR>(buff, buff + index));
            f_sDeviceCollectCounts.push_back(count);
            index = 0;
            count = 0;
        }
        memcpy(buff + index, sDevice.c_str(), sDevice.size());
        index = after;
        if (i == iSize - 1)
        {
            buff[index] = 0;
        }
        else
        {
            buff[index] = ',';
        }
        index++;
        count++;
    }
    f_sDeviceCollectBuffers.push_back(std::vector<HW_UCHAR>(buff, buff + index));
    f_sDeviceCollectCounts.push_back(count);
    f_iDeviceCollectSend = 0;
    f_iDeviceCollectTotal = sDevices.size();
    return doSendDeviceCollect();
}

static fn_void_int_t fn_deal_collect_ack = NULL;

void GatewayProtocol::registDeviceCollectAck(fn_void_int_t fn)
{
    fn_deal_collect_ack = fn;
}

static fn_deal_meter_t fn_deal_meter = NULL;

// POWER_METER_Table
void GatewayProtocol::registDeviceCollect(fn_deal_meter_t fn)
{
    fn_deal_meter = fn;
}

int GatewayProtocol::mbProcessAppManage(APPDATA *appData)
{
    // 集抄
    if (appData->hdr.serverIdOrCommand[SERVERID_INDEX_PROTOCOL] == SERVERID_INDEX_PROTOCOL_APPMANAGE_TRANS)
    {
        if (appData->hdr.serverIdOrCommand[AM_SERVERID_INDEX_FUNC] == TYPE_APP_APPMANAGE_NOTIFY_RECV_PACKET)
        {
            if (appData->hdr.serverIdOrCommand[AM_SERVERID_INDEX_SUBFUNC] == APPMANAGE_NOTIFY_RECV_PACKET_DEAL_SUC)
            {
                f_iDeviceCollectSend++;
                doSendDeviceCollect();
                // ack
                if (fn_deal_collect_ack != NULL)
                {
                    fn_deal_collect_ack(TRUE);
                }
            }
            else
            {
                // nack
                if (fn_deal_collect_ack != NULL)
                {
                    fn_deal_collect_ack(FALSE);
                }
            }
        }
        else if (appData->hdr.serverIdOrCommand[AM_SERVERID_INDEX_FUNC] == TYPE_APP_APPMANAGE_PROT_PLCMAIL)
        {
            if (appData->hdr.serverIdOrCommand[AM_SERVERID_INDEX_SUBFUNC] == APPMANAGE_PROT_PLCMAIL_DEV_SDO)
            {
                // 抄到电表数据
                POWER_METER_Table *dbTable = (POWER_METER_Table *) appData->buff;
                //
                METERTable meterTable;
                meterTable.POWER_METER_CPU_OC = dbTable->POWER_METER_CPU_OC;
                meterTable.POWER_METER_ENERGE = dbTable->POWER_METER_ENERGE;
                meterTable.POWER_METER_FORTH_HAVE_POWER_TOTAL = dbTable->POWER_METER_FORTH_HAVE_POWER_TOTAL;
                meterTable.POWER_METER_BACK_HAVE_POWER_TOTAL = dbTable->POWER_METER_BACK_HAVE_POWER_TOTAL;
                meterTable.POWER_METER_PHASE_A_FORTH_HAVE_POWER = dbTable->POWER_METER_PHASE_A_FORTH_HAVE_POWER;
                meterTable.POWER_METER_PHASE_A_BACK_HAVE_POWER = dbTable->POWER_METER_PHASE_A_BACK_HAVE_POWER;
                meterTable.POWER_METER_PHASE_B_FORTH_HAVE_POWER = dbTable->POWER_METER_PHASE_B_FORTH_HAVE_POWER;
                meterTable.POWER_METER_PHASE_B_BACK_HAVE_POWER = dbTable->POWER_METER_PHASE_B_BACK_HAVE_POWER;
                meterTable.POWER_METER_PHASE_C_FORTH_HAVE_POWER = dbTable->POWER_METER_PHASE_C_FORTH_HAVE_POWER;
                meterTable.POWER_METER_PHASE_C_BACK_HAVE_POWER = dbTable->POWER_METER_PHASE_C_BACK_HAVE_POWER;
                meterTable.POWER_METER_PHASE_A_VOLT = dbTable->POWER_METER_PHASE_A_VOLT;
                meterTable.POWER_METER_PHASE_B_VOLT = dbTable->POWER_METER_PHASE_B_VOLT;
                meterTable.POWER_METER_PHASE_C_VOLT = dbTable->POWER_METER_PHASE_C_VOLT;
                meterTable.POWER_METER_PHASE_A_ELEC = dbTable->POWER_METER_PHASE_A_ELEC;
                meterTable.POWER_METER_PHASE_B_ELEC = dbTable->POWER_METER_PHASE_B_ELEC;
                meterTable.POWER_METER_PHASE_C_ELEC = dbTable->POWER_METER_PHASE_C_ELEC;
                meterTable.POWER_METER_PHASE_A_ENERGE = dbTable->POWER_METER_PHASE_A_ENERGE;
                meterTable.POWER_METER_PHASE_B_ENERGE = dbTable->POWER_METER_PHASE_B_ENERGE;
                meterTable.POWER_METER_PHASE_C_ENERGE = dbTable->POWER_METER_PHASE_C_ENERGE;
                meterTable.POWER_METER_POWER_FATOR_A = dbTable->POWER_METER_POWER_FATOR_A;
                meterTable.POWER_METER_POWER_FATOR_B = dbTable->POWER_METER_POWER_FATOR_B;
                meterTable.POWER_METER_POWER_FATOR_C = dbTable->POWER_METER_POWER_FATOR_C;
                meterTable.POWER_METER_POWER_FATOR = dbTable->POWER_METER_POWER_FATOR;
                meterTable.POWER_METER_TS = dbTable->POWER_METER_TS;
                //
                if (fn_deal_meter != NULL)
                {
                    fn_deal_meter(std::string((char *)(appData->hdr.deviceId)), meterTable);
                }
            }
        }
        fnDebug().out("Collect appHeader: appData->hdr.mMid=%d appData->hdr.requestId=%s"
                      " appData->hdr.deviceId=%s appData->hdr.serverIdOrCommand=[%x/%x] appData->hdr.packetType=0x%x appData->hdr.packetLen=%d Buff=%s.",
                      appData->hdr.mMid, appData->hdr.requestId, appData->hdr.deviceId,
                      appData->hdr.serverIdOrCommand[AM_SERVERID_INDEX_FUNC], appData->hdr.serverIdOrCommand[AM_SERVERID_INDEX_SUBFUNC] ,appData->hdr.packetType, appData->hdr.packetLen, appData->buff);
    }
    // 优先级
    /*
        -0x00总包数
        -0x01当前包序号
        -0x02总结构体数-ushort
        -0x03总结构体数-ushort
        -0x04此包包含结构体数-ushort
        -0x05此包包含结构体数-ushort
        -0x06设备类型：0x01-优先级结构体
        -保留--------------------
        -0x10设备1+'，'
        -0x10设备1+'，'+x设备2+'，'+...+'\0'
    */
    else if(appData->hdr.serverIdOrCommand[SERVERID_INDEX_PROTOCOL] == SERVERID_INDEX_PROTOCOL_MYAPP_PARA)
    {
        // parse PROT_MIDWARE_PRI
        string sAppNames;
        std::vector<PROT_MIDWARE_PRI> pris;
        int iCount = appData->buff[0x04] << 8;
        iCount += appData->buff[0x05];
        if (iCount > 0 && iCount < ((MAX_MESSAGE_BUF_LEN - 0x10) / sizeof(PROT_MIDWARE_PRI)))
        {
            PROT_MIDWARE_PRI *priBegin = (PROT_MIDWARE_PRI *) (appData->buff + 0x10);
            for (int i = 0; i < iCount; ++i)
            {
                PROT_MIDWARE_PRI *pri = priBegin + i;
                pris.push_back(*pri);
                sAppNames += CxString::format(",[%d,%s,%d]", pri->protNum, pri->appName, pri->priority);
            }
        }
        fnDebug().out("Parse Priority, Struct [ %d ], MP_SERVERID_INDEX_FUNC [ %d ] %s.", iCount, appData->hdr.serverIdOrCommand[MP_SERVERID_INDEX_FUNC], sAppNames.c_str());
        // 回复优先级的app列表 正确
        if (appData->hdr.serverIdOrCommand[MP_SERVERID_INDEX_FUNC] == TYPE_MYAPPPARA_GET_SUCC)
        {
            _pProtocolSubject->gateway_sendPriorityInfo(true, pris);
        }
        // 回复优先级的app列表 错误
        else if (appData->hdr.serverIdOrCommand[MP_SERVERID_INDEX_FUNC] == TYPE_MYAPPPARA_GET_FAIL)
        {
            _pProtocolSubject->gateway_sendPriorityInfo(false, pris);
        }
        // 回复优先级操作结果 正确
        else if (appData->hdr.serverIdOrCommand[MP_SERVERID_INDEX_FUNC] == TYPE_MYAPPPARA_SET_SUCC)
        {
            _pProtocolSubject->gateway_sendPriorityResult(true, pris);
        }
        // 回复优先级操作结果 错误
        else if (appData->hdr.serverIdOrCommand[MP_SERVERID_INDEX_FUNC] == TYPE_MYAPPPARA_SET_FAIL)
        {
            _pProtocolSubject->gateway_sendPriorityResult(false, pris);
        }
    }
    return 0;
}

int GatewayProtocol::sendRequestPriority()
{
    if (! isConnection())
    {
        fnDebug().out("Request Priority Fail INVALID_SOCKET.");
        return -1;
    }
    APPDATA appdata = {0};
    HW_UCHAR *serverIdOrCommand = appdata.hdr.serverIdOrCommand;
    // serverIdOrCommand
    serverIdOrCommand[SERVERID_INDEX_HEAD_00] = SERVERID_INDEX_H00_M00_J00_DEFAULT;
    serverIdOrCommand[SERVERID_INDEX_HEAD_01] = SERVERID_INDEX_H01_M00_J00_DEFAULT;
    serverIdOrCommand[SERVERID_INDEX_HEAD_02] = SERVERID_INDEX_H02_M00_J00_DEFAULT;
    serverIdOrCommand[SERVERID_INDEX_HEAD_03] = SERVERID_INDEX_H03_M00_J00_DEFAULT;
    serverIdOrCommand[SERVERID_INDEX_MAJOR_VERSION] = SERVERID_INDEX_MAJOR_VERSION_NUM;
    serverIdOrCommand[SERVERID_INDEX_JUNIOR_VERSION] = SERVERID_INDEX_JUNIOR_VERSION_NUM;
    serverIdOrCommand[SERVERID_INDEX_PROTOCOL] = SERVERID_INDEX_PROTOCOL_MYAPP_PARA;
    serverIdOrCommand[MP_SERVERID_INDEX_SEQ] = 0x00;
    serverIdOrCommand[MP_SERVERID_INDEX_WAIT] = 0x00;
    serverIdOrCommand[MP_SERVERID_INDEX_FUNC] = TYPE_MYAPPPARA_GET;
    serverIdOrCommand[MP_SERVERID_INDEX_MODE] = 0x0A;
    serverIdOrCommand[MP_SERVERID_INDEX_SUBMODE] = 0x01;
    // buffer
    HW_UCHAR *buffer = appdata.buff;
    buffer[0x00] = 0x00;
    buffer[0x01] = 0x00;
    // hdr
    appdata.hdr.packetType = TYPE_APP_PROTOCOL_APPMANAGE;
    appdata.hdr.packetLen = 1;
    int rst = _channel->sendData((char *) &appdata, sizeof(APPDATA));
    fnDebug().out("Send Request Priority.");
    return rst;
}

int GatewayProtocol::sendUpdatePriority(const vector<PROT_MIDWARE_PRI> &pris)
{
    if (! isConnection())
    {
        fnDebug().out("Request Priority Fail INVALID_SOCKET.");
        return -1;
    }
    APPDATA appdata = {0};
    HW_UCHAR *serverIdOrCommand = appdata.hdr.serverIdOrCommand;
    // serverIdOrCommand
    serverIdOrCommand[SERVERID_INDEX_HEAD_00] = SERVERID_INDEX_H00_M00_J00_DEFAULT;
    serverIdOrCommand[SERVERID_INDEX_HEAD_01] = SERVERID_INDEX_H01_M00_J00_DEFAULT;
    serverIdOrCommand[SERVERID_INDEX_HEAD_02] = SERVERID_INDEX_H02_M00_J00_DEFAULT;
    serverIdOrCommand[SERVERID_INDEX_HEAD_03] = SERVERID_INDEX_H03_M00_J00_DEFAULT;
    serverIdOrCommand[SERVERID_INDEX_MAJOR_VERSION] = SERVERID_INDEX_MAJOR_VERSION_NUM;
    serverIdOrCommand[SERVERID_INDEX_JUNIOR_VERSION] = SERVERID_INDEX_JUNIOR_VERSION_NUM;
    serverIdOrCommand[SERVERID_INDEX_PROTOCOL] = SERVERID_INDEX_PROTOCOL_MYAPP_PARA;
    serverIdOrCommand[MP_SERVERID_INDEX_SEQ] = 0x00;
    serverIdOrCommand[MP_SERVERID_INDEX_WAIT] = 0x00;
    serverIdOrCommand[MP_SERVERID_INDEX_FUNC] = TYPE_MYAPPPARA_SET;
    serverIdOrCommand[MP_SERVERID_INDEX_MODE] = 0x0A;
    serverIdOrCommand[MP_SERVERID_INDEX_SUBMODE] = 0x01;
    // buffer
    /*
     -0x00总包数
     -0x01当前包序号
     -0x02总结构体数-ushort
     -0x03总结构体数-ushort
     -0x04此包包含结构体数-ushort
     -0x05此包包含结构体数-ushort
     -0x06设备类型：0x01-优先级结构体
     -保留--------------------
     -0x10设备1+'，'
     -0x10设备1+'，'+x设备2+'，'+...+'\0'
     */
    HW_UCHAR *buffer = appdata.buff;
    buffer[0x00] = 1;
    buffer[0x01] = 1;
    buffer[0x02] = pris.size() >> 8;
    buffer[0x03] = pris.size();
    buffer[0x04] = pris.size() >> 8;
    buffer[0x05] = pris.size();
    buffer[0x06] = 0x01;
    //
    string sAppNames;
    PROT_MIDWARE_PRI *priBegin = (PROT_MIDWARE_PRI *) (appdata.buff + 0x10);
    for (int i = 0; i < pris.size(); ++i)
    {
        const PROT_MIDWARE_PRI &pri = pris.at(i);
        PROT_MIDWARE_PRI *toPri = priBegin+i;
        memcpy(toPri, &pri, sizeof(PROT_MIDWARE_PRI));
        sAppNames += CxString::format(",[%d,%s,%d]", pri.protNum, pri.appName, pri.priority);
    }
    // hdr
    appdata.hdr.packetType = TYPE_APP_PROTOCOL_APPMANAGE;
    appdata.hdr.packetLen = sizeof(PROT_MIDWARE_PRI) + 0x10;
    int rst = _channel->sendData((char *) &appdata, sizeof(APPDATA));
    fnDebug().out("Send Update Priority, Struct [ %d ] %s.", pris.size(), sAppNames.c_str());
    return rst;
}

int GatewayProtocol::sendRequestBandwidth(short t)
{
    APPDATA data = {0};
    data.hdr.packetType = TYPE_SEND_SOUTHING_DATA;
    //
    HW_UCHAR *serverIdOrCommand = data.hdr.serverIdOrCommand;
    // serverIdOrCommand
    serverIdOrCommand[SERVERID_INDEX_HEAD_00] = SERVERID_INDEX_H00_M00_J00_DEFAULT;
    serverIdOrCommand[SERVERID_INDEX_HEAD_01] = SERVERID_INDEX_H01_M00_J00_DEFAULT;
    serverIdOrCommand[SERVERID_INDEX_HEAD_02] = SERVERID_INDEX_H02_M00_J00_DEFAULT;
    serverIdOrCommand[SERVERID_INDEX_HEAD_03] = SERVERID_INDEX_H03_M00_J00_DEFAULT;
    serverIdOrCommand[SERVERID_INDEX_MAJOR_VERSION] = SERVERID_INDEX_MAJOR_VERSION_NUM;
    serverIdOrCommand[SERVERID_INDEX_JUNIOR_VERSION] = SERVERID_INDEX_JUNIOR_VERSION_NUM;
    serverIdOrCommand[SERVERID_INDEX_PROTOCOL] = SERVERID_INDEX_PROTOCOL_MIDWARE_PLCMAIL;
    serverIdOrCommand[MW_SERVERID_INDEX_SEQ] = 0x00;
    serverIdOrCommand[MW_SERVERID_INDEX_WAIT] = 0x01;
    serverIdOrCommand[MW_SERVERID_INDEX_SEND_PROT] = 0x08;
    serverIdOrCommand[MW_SERVERID_INDEX_SEND_PRIO] = 0x02;
    serverIdOrCommand[MW_SERVERID_INDEX_DESTPROT_START_1] = 0x08;
    serverIdOrCommand[MW_SERVERID_INDEX_DESTPROT_END_1] = 0x08;
    //
    METER_PACKET *meterPacket = reinterpret_cast<METER_PACKET *>(data.buff);
    meterPacket->packerInfo = PLC_MAIL_PACKET_INDEX;//包头 01
    meterPacket->packetLen = 4; //seq+func+TK_METER_ADDR_LEN   包长??
    meterPacket->param[0] = plcMailSeq;  //序号
    plcMailSeq++;
    if(plcMailSeq == 255)
    {
        plcMailSeq = 0;
    }
    meterPacket->param[1] = APP_CNS_COLLECT; // 业务??
    //memcpy(&meterPacket->param[2],0x01,1);  //数据：cns：业务一
    meterPacket->param[2] = 0x09; // 功能方式：cns 0x11
    meterPacket->param[3] = 1; // 需获取的数据包序号
    //
    METER_PAYLOAD *meterPayload = reinterpret_cast<METER_PAYLOAD *>(&data.buff[2 + meterPacket->packetLen]);
    meterPayload->payloadInfo = PLC_MAIL_PAYLOAD_INDEX;//包信??
    meterPayload->paloadLenHi = 0x00;   //包信息长度，第一??
    meterPayload->paloadLenLow = 0x02;  //包信息长度，第二??
    memcpy(meterPayload->param, &t , 2);
    data.hdr.packetLen = 6 + 5;//报文长度，
    int rst = _channel->sendData((char *) &data, sizeof(APPDATA));
    string sHexServerIdOrCommand = CxString::toHexstring(serverIdOrCommand, MAX_ID_LEN);
    string sHexBuff = CxString::toHexstring(data.buff, data.hdr.packetLen);
    fnDebug() <<"Send Request Bandwidth, Hex ServerIdOrCommand :" << sHexServerIdOrCommand;
    fnDebug() <<"Send Request Bandwidth, Hex Buff :" << sHexBuff;
    return rst;
}
