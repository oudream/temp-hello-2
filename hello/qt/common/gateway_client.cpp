/**
1)	App和网关互发命令命令
appData.hdr.packetType = TYPE_COMMAND （0x2）; //发送命令到app
appData.hdr.packetType = TYPE_COMMAND_RESULT (0x9); //app命令应答
appData.hdr.packetLen = len //数据buf长度，不含头部长度
appData.buf 命令内容

2)	网关获取app数据
appData.hdr.packetType = TYPE_PROPGET (0x3);
appData.hdr.deviceId：设备id
appData.hdr. serverIdOrCommand:属性值
appData.hdr.packetLen = len //数据buf长度，不含头部长度

3)	appData.buf 数据内容
app通过网关发送数据到主站
appData.hdr.packetType = TYPE_PROPREPORT (0x5);
appData.hdr.deviceId：设备id
appData.hdr. serverIdOrCommand:属性值
appData.hdr.packetLen = len //数据buf长度，不含头部长度
appData.buf 数据内容

4)	网关发送拓扑文件到104
appData.hdr.packetType = TYPE_APP_TOPO_PATTERN (0x22); //查询拓扑识别进度
appData.hdr.packetLen = len //数据buf长度，不含头部长度
appData.buf 拓扑文件路径

5)	网关配置主站地址和端口号到104
appData.hdr.packetType = TYPE_PROPSET（0x4）; //配置主站服务器ip/port 属性
appData.hdr.packetLen = len //数据buf长度，不含头部长度
appData.buf 内容为：
typedef struct
{
	char serverIp[128];
	unsigned short port;
}Iec104Server;

6)	网关通过104发送数据到104主站；104app取表中最新的数据送往主站
appData.hdr.packetType = TYPE_APP_104_PROPREPORT (0x40);
appData.hdr.deviceId：设备id
appData.hdr. serverIdOrCommand:为空
appData.hdr.packetLen = len //数据buf长度，不含头部长度
appData.buf 数据内容为：
#define DB_TABLE_NAME_LEN 128
typedef struct db_table
{
	char db[DB_TABLE_NAME_LEN];
	char deviceTable[DB_TABLE_NAME_LEN];
}DB_TABLE;

7)	104发送拓扑识别命令到网关
appData.hdr.packetType = TYPE_APP_START_TOPO_RECOGNITION （0x20）; //启动拓扑识别
appData.hdr.packetType = TYPE_APP_QUERY_TOPO_PROCESSING (0x21); //查询拓扑识别进度
appData.hdr.packetLen = len //数据buf长度，不含头部长度

 */

#include <fcntl.h>
#include <vector>
#include <sys/types.h>
#ifdef __linux__
#include <ifaddrs.h>
#include <arpa/inet.h>
#endif
#include "gateway_client.h"
#include "device_ddl.h"
#include "app_definition.h"

#include <ccxx/cxfile.h>
#include <ccxx/cxsocket.h>
#include <ccxx/cxtime.h>

// 关闭日志 --- 会打印 error
//int debug_switch = 12;
// 打开日志 --- 打印全部日志
int debug_switch = 0;

typedef enum
{
    GATEWAY_CONNECT_STATE_DIS,
    GATEWAY_CONNECT_STATE_ING,
    GATEWAY_CONNECT_STATE_ED
} GATEWAY_CONNECT_STATE_ENUM;

static msepoch_t f_clientConnectTime = 0;
static msepoch_t f_clientDisConnectTime = 0;
static volatile bool f_clientRunning = false;
static volatile int f_clientConnectState = GATEWAY_CONNECT_STATE_DIS;
static volatile socket_t f_socketFD = INVALID_SOCKET;
static pthread_t f_clientTid = 0;
//
static CxMutex *f_clientEventLock = NULL;
static CxMutex *fn_clientEventLock()
{
    static CxMutex m;
    return &m;
}
static CxMutex f_clientBusinessStateLock;
//
static volatile int f_clientEventCount;
static std::list<IEC104Server> f_clientIEC104Servers;
static std::map<std::string, int> f_clientDB_TABLEs;
static std::map<unsigned int, std::map<std::string, AREABUG_YX>> f_clientFailureEvents;
static std::map<std::string, PHASE_NOTIFY> f_clientPhaseNotify;
static std::map<unsigned int, BusinessStateInfo> f_clitentBusinessState;


#define EN_LOG_LEVEL_ERROR 1
#define EN_LOG_LEVEL_DEBUG 3
#define EN_LOG_LEVEL_WARN  2

static const char PRINT_PRE_ERROR[] = "ERROR - ";
static const char PRINT_PRE_WARN[] = "WARN -- ";
static const char PRINT_PRE_INFO[] = "INFO -> ";
//static const char APP_NAME_TKIEC104_SERVER[] = "tkiec104_server";

static char S_APP_NAME[APP_NAME_LEN];
static char S_UART_NAME[UART_NAME_LEN];
static int I_PROTOCOL = APP_PACKET_VIRTUAL_104;

// *** *** ***
// log
// *** *** ***

static std::string logPath = "gateway_client.log";
static FILE *logFd = NULL;
static int logSize = 0;
#define ME_GOAHEAD_LOGMAXSIZE (10*1024*1024-1)

static int logOpen();

static void gateway_client_debug_close()
{
    if (logFd != NULL)
    {
        fclose(logFd);
        logFd = NULL;
    }
}

static void logCheck()
{
    if (logFd == NULL)
    {
        logOpen();
    }
    else if (logSize > ME_GOAHEAD_LOGMAXSIZE)
    {
        gateway_client_debug_close();
        logOpen();
    }
}

static void defaultLogHandler(int level, char *buf)
{
    if (debug_switch == 12 && level != EN_LOG_LEVEL_ERROR)
        return ;

    int bufLen;

    logCheck();
    bufLen = (int) strlen(buf);
    if (logFd != NULL)
    {
        printf("%s", buf);
        printf("%s", "\n");
        fwrite(buf, 1, bufLen, logFd);
        fwrite("\n", 1, 1, logFd);
        fflush(logFd);
        logSize += bufLen + 1;
    }
}

static int logOpen()
{
    logFd = fopen(logPath.c_str(), "w");
    if (logFd == NULL)
    {
        return -1;
    }
    fseek(logFd, 0, SEEK_SET);
    logSize = 0;
    return 0;
}

static void gateway_client_debug_print(int level, const char *format, ...)
{
    va_list args;
    va_start(args, format);
    const char *pre = (level == EN_LOG_LEVEL_ERROR) ? PRINT_PRE_ERROR : (level == EN_LOG_LEVEL_WARN) ? PRINT_PRE_WARN
                                                                                                     : PRINT_PRE_INFO;
    char buffer[4096];
    std::string dt = CxTime::currentMsepochString() + " ";
    strcpy(buffer, dt.c_str());
    strcpy(buffer + strlen(buffer), pre);
    vsnprintf(buffer + strlen(buffer), sizeof(buffer), format, args);
    defaultLogHandler(level, buffer);
    va_end(args);
}

// *** *** ***
// tcp client
// *** *** ***

//static int setnonblocking(HW_INT fd)
//{
//    int old_option = fcntl(fd, F_GETFL);
//    int new_option = old_option | O_NONBLOCK;
//    fcntl(fd, F_SETFL, new_option);
//    return old_option;
//}

static int sendDate(char *buf, int len, socket_t clientSocket)
{
    gateway_client_debug_print(EN_LOG_LEVEL_DEBUG, "Gateway.sendDate...");
    int times = 0;
    while (times++ < 3)
    {
        int sendBytes = CxSocket::sendto(clientSocket, buf, len);
//        gateway_client_debug_print(EN_LOG_LEVEL_DEBUG, "Gateway Send socket buffer: %s",
//                                   CxString::toHexstring(buf, len).c_str());
        if (sendBytes < 0)
        {
            if (errno != EAGAIN && errno != EWOULDBLOCK && errno != EINTR)
            {
                gateway_client_debug_print(EN_LOG_LEVEL_ERROR, "Gateway Send socket error! errno=%d", errno);
                return SOCKET_ERROR;
            }
            else
            {
                CxThread::sleep(100);
                continue;
            }
        }
        else
        {
            return sendBytes;
        }
    }
    gateway_client_debug_print(EN_LOG_LEVEL_ERROR, "Gateway Send socket error (Over 3 Times)! errno=%d", errno);
    return SOCKET_ERROR;
}

static void popBusinessState(BusinessStateInfo &dst)
{
    {

        if (f_clitentBusinessState.size() > 0)
        {
            CxMutexScope lock(&f_clientBusinessStateLock);
            dst.state = f_clitentBusinessState.begin()->second.state;
            strncpy(dst.sockAddress, f_clitentBusinessState.begin()->second.sockAddress, SOCK_ADDR_MAX_LEN);
            strncpy(dst.networkName, f_clitentBusinessState.begin()->second.networkName, NETWORK_NAME_MAX_LEN);
            dst.recvTotalLength = f_clitentBusinessState.begin()->second.recvTotalLength;
            dst.recvLastTime = f_clitentBusinessState.begin()->second.recvLastTime;
            f_clitentBusinessState.erase(f_clitentBusinessState.begin());
        }
    }
}

/**
 *  @param ip: in 获取网卡名的 ip
 *  @param ipName: out  存放获取到的网卡名
 *  @param ipLen: in 存放网名的长度
 *  @return 0：正常 >0: 异常
**/
int getSubnetMask(const char ip[], char *ipName, int ipLen)
{

#ifdef __linux__

    struct sockaddr_in *sin = NULL;
    struct ifaddrs *ifa = NULL, *ifList;

    if (getifaddrs(&ifList) < 0)
    {
        return -1;
    }

    for (ifa = ifList; ifa != NULL; ifa = ifa->ifa_next)
    {
        if (ifa->ifa_addr == NULL)
            continue;

        if(ifa->ifa_addr->sa_family == AF_INET)
        {
            std::string ifname(ifa->ifa_name);

            sin = (struct sockaddr_in *)ifa->ifa_addr;
            char *ipAddress = inet_ntoa(sin->sin_addr);
            if (!strcmp(ipAddress,ip))
            {
                strncpy(ipName,ifname.c_str(),ipLen);
                freeifaddrs(ifList);
                return 0;
            }
        }
    }

    freeifaddrs(ifList);
    return -1;

#endif

}

static int sendServiceStatus(socket_t clientSocket)
{
    gateway_client_debug_print(EN_LOG_LEVEL_DEBUG, "Gateway.sendServiceStatus...");

//    char networkName[255] = {0};
    BusinessStateInfo info;
    APPDATA appdata = {0};

    popBusinessState(info);
//    if(int err = getSubnetMask(info.sockAddress, networkName, sizeof(networkName)) < 0)
//    {
//        gateway_client_debug_print(EN_LOG_LEVEL_ERROR, "Gateway.sendServiceStatus is getSubnetMask ip: %s, error: %d",
//                                   info.sockAddress, err);
//        return -1;
//    }
//    gateway_client_debug_print(EN_LOG_LEVEL_DEBUG, "Gateway.sendServiceStatus is getSubnetMask ip: %s, networkName: %s",
//                               info.sockAddress, networkName);

    BusinessStateInfo *pInfo = (BusinessStateInfo *) appdata.buff;
    appdata.hdr.packetType = TYPE_APP_TEST_COMMUNICATION_LINK;
    appdata.hdr.packetLen = sizeof(BusinessStateInfo);

    pInfo->state = info.state;
    strncpy(pInfo->sockAddress, info.sockAddress, SOCK_ADDR_MAX_LEN);
    strncpy(pInfo->networkName, info.networkName, NETWORK_NAME_MAX_LEN);
    pInfo->recvTotalLength = info.recvTotalLength;
    pInfo->recvLastTime = info.recvLastTime;

#ifdef TEXT_LOG
    gateway_client_debug_print(EN_LOG_LEVEL_DEBUG, "Gateway.sendServiceStatus is state: %d,  networkName: %s, sockAddress: %s",
                               pInfo->state, pInfo->networkName, pInfo->sockAddress);
#endif

    return sendDate((char *) &appdata, sizeof(APPDATA), clientSocket);
}

static int sendAppNameUp(socket_t clientSocket)
{
    APPDATA appdata = {0};
    REGISTER_INFO *pInfo = (REGISTER_INFO *) appdata.buff;
    appdata.hdr.packetType = TYPE_APP_NAME;
    appdata.hdr.packetLen = sizeof(REGISTER_INFO);
    memcpy(pInfo->appName, S_APP_NAME, strlen(S_APP_NAME));
    memcpy(pInfo->deviceName, S_UART_NAME, strlen(S_UART_NAME));
    pInfo->protocol = I_PROTOCOL;
    int r = sendDate((char *) &appdata, sizeof(APPDATA), clientSocket);
    gateway_client_debug_print(EN_LOG_LEVEL_DEBUG, "Gateway.sendAppNameUp [ %s %s %d ]", pInfo->appName, pInfo->deviceName, pInfo->protocol);
    return r;
}

static int sendAppAnswer (socket_t clientSocket, HW_UINT mMid)
{
    APPDATA appdata = {0};
    appdata.hdr.mMid = mMid;
    appdata.hdr.packetType = TYPE_APP_ACK;
    appdata.hdr.packetLen = sizeof(REGISTER_INFO);
    int r = sendDate((char *) &appdata, sizeof(APPDATA), clientSocket);
    gateway_client_debug_print(EN_LOG_LEVEL_DEBUG, "Gateway.sendAppAnswer [ %d %d ]", appdata.hdr.mMid, appdata.hdr.packetType);
    return r;
}

static int mbPropGetResponse(APPDATA *appData, socket_t SocketFD)
{
    return -2;
}

static int mbProcessCommand(APPDATA *appData, socket_t SocketFD)
{
    return -2;
}

static int mbProcessDevStatus(APPDATA *appData, socket_t SocketFD)
{
    return -2;
}

static int mbProcessAppTopoPattern(APPDATA *appData, socket_t SocketFD)
{
    gateway_client_debug_print(EN_LOG_LEVEL_DEBUG, "Gateway.mbProcessAppTopoPattern...");
    appData->buff[MAX_MESSAGE_BUF_LEN - 1] = '\0';
    char *path = (char *) appData->buff;
    gateway_client_debug_print(EN_LOG_LEVEL_DEBUG, "Gateway Process App Topo Pattern : %s", path);
    return 0;
}

static int mbPropSetProp(APPDATA *appData, socket_t SocketFD)
{
    gateway_client_debug_print(EN_LOG_LEVEL_DEBUG, "Gateway.mbPropSetProp~~~");
    IEC104Server *iec104Server = (IEC104Server *) appData->buff;
    iec104Server->serverIp[127] = '\0';
    {
        CxMutexScope lock(f_clientEventLock);
        if (f_clientIEC104Servers.empty())
        {
            f_clientIEC104Servers.push_back(*iec104Server);
            f_clientEventCount = f_clientIEC104Servers.size() + f_clientDB_TABLEs.size() + f_clientFailureEvents.size();
        }
    }
    gateway_client_debug_print(EN_LOG_LEVEL_DEBUG, "Gateway Process Set Prop : ip=%s port104=%d, portTopo=%d", iec104Server->serverIp,
                               iec104Server->port104, iec104Server->portTopo);
    return sendAppAnswer(SocketFD, appData->hdr.mMid);
}

static int mbProcessApp104Propreport(APPDATA *appData, socket_t SocketFD)
{
    gateway_client_debug_print(EN_LOG_LEVEL_DEBUG, "Gateway.mbProcessApp104Propreport...");
    DB_TABLE *dbTable = (DB_TABLE *) appData->buff;
    dbTable->db[DB_TABLE_NAME_LEN - 1] = '\0';
    dbTable->deviceTable[DB_TABLE_NAME_LEN - 1] = '\0';
    {
        CxMutexScope lock(f_clientEventLock);
        if (f_clientDB_TABLEs.size() > 100)
        {
            f_clientDB_TABLEs.clear();
        }
        std::string key = std::string(dbTable->db)+","+std::string(dbTable->deviceTable);
        f_clientDB_TABLEs[key] = 1;
        f_clientEventCount = f_clientIEC104Servers.size() + f_clientDB_TABLEs.size() + f_clientFailureEvents.size();
    }
    std::string dt = CxTime::currentMsepochString() + " ";
    gateway_client_debug_print(EN_LOG_LEVEL_DEBUG, "%s Gateway Process App 104 Propreport : db=%s deviceTable=%s",
                               dt.c_str(), dbTable->db, dbTable->deviceTable);
    return sendAppAnswer(SocketFD, appData->hdr.mMid);
}

static int mbProcessLineLoss(APPDATA *appData, socket_t SocketFD)
{
    gateway_client_debug_print(EN_LOG_LEVEL_DEBUG, "Gateway.mbProcessLineLoss...");

    AREABUG_YX *dbTable = (AREABUG_YX *) appData->buff;
    dbTable->branchDevice[15] = '\0';
    if (dbTable->branchDevice)
    {
        CxMutexScope lock(f_clientEventLock);
        gateway_client_debug_print(EN_LOG_LEVEL_DEBUG, "f_clientFailureEvents.size=%d\n", f_clientFailureEvents.size());
        if (f_clientFailureEvents.size() > 100)
        {
            f_clientFailureEvents.clear();
        }
        std::map<std::string, AREABUG_YX> tempMap;
        std::string key = std::string((char *)appData->hdr.deviceId);
        tempMap[key] = *dbTable;
        f_clientFailureEvents[appData->hdr.mMid] = tempMap;
        f_clientEventCount = f_clientIEC104Servers.size() + f_clientDB_TABLEs.size() + f_clientFailureEvents.size() + f_clientPhaseNotify.size();
    }

    gateway_client_debug_print(EN_LOG_LEVEL_DEBUG, "appHeader:\n\t appData->hdr.mMid=%d\n\t appData->hdr.requestId=%s\n\t"
                               " appData->hdr.deviceId=%s\n\t appData->hdr.serverIdOrCommand=%s\n\t appData->hdr.packetType=0x%x\n\t appData->hdr.packetLen=%d",
                               appData->hdr.mMid, appData->hdr.requestId, appData->hdr.deviceId,
                               appData->hdr.serverIdOrCommand,appData->hdr.packetType, appData->hdr.packetLen);

    gateway_client_debug_print(EN_LOG_LEVEL_DEBUG, "appBuff:\n\t dbTable->areaBugType=%d\n\t dbTable->areaBugStaA=%d\n\t"
                                                   " dbTable->areaBugStaB=%d\n\t dbTable->areaBugStaC=%d\n\t dbTable->branchDevice=%s\n\t dbTable->recordTs=%ld",
                               dbTable->areaBugType, dbTable->areaBugStaA, dbTable->areaBugStaB, dbTable->areaBugStaC, dbTable->branchDevice, dbTable->recordTs);

//    gateway_client_debug_print(EN_LOG_LEVEL_DEBUG, "%s Gateway Process App 104 Powerloss : hdr.packetType=%d  appData->hdr.deviceId=%s\n\n\n",
//                               dt.c_str(), appData->hdr.packetType, appData->hdr.deviceId);

    gateway_client_debug_print(EN_LOG_LEVEL_DEBUG, "f_clientFailureEvents.size=%d\n", f_clientFailureEvents.size());

    return sendAppAnswer(SocketFD, appData->hdr.mMid);
}

static int mbProcessEventPhase(APPDATA *appData, socket_t SocketFD)
{
    gateway_client_debug_print(EN_LOG_LEVEL_DEBUG, "Gateway.mbProcessEventPhase");
    PHASE_NOTIFY *phaseNotify = (PHASE_NOTIFY *)appData->buff;
    phaseNotify->deviceId[15] = '\0';
    {
        CxMutexScope lock(f_clientEventLock);
        if(f_clientPhaseNotify.size() > 100)
        {
            f_clientPhaseNotify.clear();
        }
        std::string key = std::string((char *)appData->hdr.deviceId);
        f_clientPhaseNotify[key] = *phaseNotify;
        f_clientEventCount = f_clientIEC104Servers.size() + f_clientDB_TABLEs.size() + f_clientFailureEvents.size() + f_clientPhaseNotify.size();
    }

//    std::string dt = CxTime::currentMsepochString() + " ";
    gateway_client_debug_print(EN_LOG_LEVEL_DEBUG, "appHeader:\n\t appData->hdr.mMid=%d\n\t appData->hdr.requestId=%s\n\t"
                                                   " appData->hdr.deviceId=%s\n\t appData->hdr.serverIdOrCommand=%s\n\t appData->hdr.packetType=0x%x\n\t appData->hdr.packetLen=%d",
                               appData->hdr.mMid, appData->hdr.requestId, appData->hdr.deviceId,
                               appData->hdr.serverIdOrCommand,appData->hdr.packetType, appData->hdr.packetLen);

    gateway_client_debug_print(EN_LOG_LEVEL_DEBUG, "appBuff:\n\t phaseNotify->deviceId=%s\n\t phaseNotify->PHASE_INFO=%d\n\t",
                               phaseNotify->deviceId, phaseNotify->PHASE_INFO);

    return sendAppAnswer(SocketFD, appData->hdr.mMid);
}

static int iot_debug_config(APPDATA *appData, socket_t SocketFD)
{
    debug_switch = appData->hdr.packetLen;
    return sendAppAnswer(SocketFD, appData->hdr.mMid);
}

static int gatewayProcessData(HW_CHAR *msg, socket_t SocketFD)
{
    int r = 0;
    APPDATA *pData = (APPDATA *) msg;

    gateway_client_debug_print(EN_LOG_LEVEL_DEBUG, "Gateway hdr.packetType=0x%x  pData->hdr.mMid=%d", pData->hdr.packetType, pData->hdr.mMid);
//    gateway_client_debug_print(EN_LOG_LEVEL_DEBUG, "Gateway read socket buffer: %s",
//                               CxString::toHexstring((char *) msg, sizeof(APPDATA)).c_str());
//    gateway_client_debug_print(EN_LOG_LEVEL_DEBUG, "Gateway read socket buffer: %s",
//                               CxString::toHexstring((char *) pData, sizeof(APPDATA)).c_str());
    switch (pData->hdr.packetType)
    {
        case TYPE_PROPGET:
            r = mbPropGetResponse(pData, SocketFD);
            break;
        case TYPE_COMMAND:
            r = mbProcessCommand(pData, SocketFD);
            break;
        case TYPE_PROPSET:
            r = mbPropSetProp(pData, SocketFD);
            break;
        case TYPE_DEVICESTAUS:
            r = mbProcessDevStatus(pData, SocketFD);
            break;
        case TYPE_APP_TOPO_PATTERN:
            r = mbProcessAppTopoPattern(pData, SocketFD);
            break;
        case TYPE_APP_GWDB_MODIFY:
            r = mbProcessApp104Propreport(pData, SocketFD);
            break;
        case TYPE_APP_AREA_BUG_REPORT:
            r = mbProcessLineLoss(pData, SocketFD);
            break;
        case TYPE_APP_EVENT_PHASE:
            r = mbProcessEventPhase(pData, SocketFD);
            break;
        case TYPE_APP_PROTOCOL_APPMANAGE:
            r = GatewayClient::mbProcessAppManage(pData, SocketFD);
            break;
        case TYPE_HB:
            gateway_client_debug_print(EN_LOG_LEVEL_DEBUG, "Gateway Heart beat!");
            break;
        case TYPE_APP_DEBUG:
            r = iot_debug_config(pData, SocketFD);
            gateway_client_debug_print(EN_LOG_LEVEL_DEBUG, "Gateway TYPE_APP_DEBUG!");
            break;
        default:
            gateway_client_debug_print(EN_LOG_LEVEL_DEBUG, "Gateway pData->hdr.packetType=%d!",pData->hdr.packetType);
            break;
    }
    if (r == -2)
    {
        gateway_client_debug_print(EN_LOG_LEVEL_DEBUG, "Gateway pData->hdr.packetType=%d-%d do not impl!", pData->hdr.packetType, pData->hdr.packetLen);
    }
    else if (r == SOCKET_ERROR)
    {
        gateway_client_debug_print(EN_LOG_LEVEL_ERROR, "Gateway pData->hdr.packetType=%d-%d ERROR!", pData->hdr.packetType, pData->hdr.packetLen);
    }
    else
    {
        gateway_client_debug_print(EN_LOG_LEVEL_DEBUG, "Gateway pData->hdr.packetType=%d-%d!", pData->hdr.packetType, pData->hdr.packetLen);
    }
    return r;
}

static void *gatewayMain(void *)
{
    socket_t SocketFD = INVALID_SOCKET;
    CxIpAddress stSockAddr(TCP_CONTAINER_SERVER_IP, TCP_SERVER_PORT);
    HW_CHAR recvbuf[1536];
    HW_INT iDataNum;

    while (f_clientRunning)
    {               // In order to receive client data continuously
        if (SocketFD == INVALID_SOCKET)
        {
            SocketFD = CxSocket::create(PF_INET, SOCK_STREAM, IPPROTO_TCP);
            if (INVALID_SOCKET == SocketFD)
            {
                gateway_client_debug_print(EN_LOG_LEVEL_ERROR, "Gateway Create socket error! errno=%d", errno);
                CxThread::sleep(1000);
                continue;
            }
            while (f_clientRunning)
            {
                if (CxSocket::connectto(SocketFD, stSockAddr.getSockAddr()) == 0)
                {
                    break;
                }
                else
                {
                    if ((errno == ECONNREFUSED || errno == ETIMEDOUT) && f_clientRunning)
                    {
                        gateway_client_debug_print(EN_LOG_LEVEL_ERROR, "Gateway Connect socket errno=%d", errno);
                        CxThread::sleep(2000);
                        continue;
                    }
                    else
                    {
                        gateway_client_debug_print(EN_LOG_LEVEL_ERROR, "Gateway Connect socket error! errno=%d", errno);
                        goto exit;
                    }
                }
            }
            CxSocket::blocking(SocketFD, false);
//            setnonblocking(SocketFD);
            gateway_client_debug_print(EN_LOG_LEVEL_DEBUG, "Gateway Successfully connected to the Server %s...",
                                       TCP_CONTAINER_SERVER_IP);
            f_clientConnectState = GATEWAY_CONNECT_STATE_ED;
            f_clientConnectTime = CxTime::currentMsepoch();
            f_socketFD = SocketFD;
            if (sendAppNameUp(SocketFD) == SOCKET_ERROR)
            {
                goto exit;
            }
        }
        else
        {
            static fd_set rset;
            static struct timeval timeout;
            static socket_t maxfd = 0;

            FD_ZERO(&rset);
            FD_SET(SocketFD, &rset);

            maxfd = maxfd >= SocketFD ? maxfd : SocketFD;
            timeout = {0, 100};

            switch (::select(maxfd+1, &rset, nullptr, nullptr, &timeout))
            {
            case SOCKET_ERROR:
                goto exit;
            case 0:
                break;
            default:
                if (FD_ISSET(SocketFD, &rset))
                {
                    memset(recvbuf, 0, sizeof(recvbuf));
                    iDataNum = CxSocket::recvfrom(SocketFD, recvbuf, sizeof(APPDATA));


                    if (iDataNum > 0)
                    {
                        gateway_client_debug_print(EN_LOG_LEVEL_DEBUG, "gatewayProcessDate - Gateway read socket buffer  length: %d,  APPDATA  length: %d",
                                                   iDataNum, sizeof(APPDATA));
//                        gateway_client_debug_print(EN_LOG_LEVEL_DEBUG, "gatewayProcessDate - Gateway read socket buffer: %s",
//                                                   CxString::toHexstring((char *) recvbuf, iDataNum).c_str());
                    }
                    if (iDataNum == sizeof(APPDATA))
                    {
                        if (gatewayProcessData(recvbuf, SocketFD) == SOCKET_ERROR)
                        {
                            goto exit;
                        }
                    }
                    else if (iDataNum == 0)
                    {
                        if(errno != 11)
                        {
                            gateway_client_debug_print(EN_LOG_LEVEL_ERROR, "Gateway Recv socket == 0! errno=%d", errno);
                            goto exit;
                        }
                        else
                        {
                            CxThread::sleep(100);
                            continue;
                        }
                    }
                    else if (iDataNum < 0)
                    {
                        if (errno != EAGAIN && errno != EWOULDBLOCK && errno != EINTR)
                        {
                            gateway_client_debug_print(EN_LOG_LEVEL_ERROR, "Gateway Recv socket error! errno=%d", errno);
                            goto exit;
                        }
                        else
                        {
                            CxThread::sleep(100);
                            continue;
                        }
                    }
                    else
                    {
                        gateway_client_debug_print(EN_LOG_LEVEL_DEBUG, "Gateway Recv socket len == %d", iDataNum);
                    }
                }
                break;
            }

            {
                CxMutexScope lock(&f_clientBusinessStateLock);
                if (f_clitentBusinessState.size() <= 0)
                {
                    continue;
                }
            }
            sendServiceStatus(SocketFD);
        }
    }

    exit:
    if (f_clientRunning)
    {
        f_clientRunning = false;
        if (f_socketFD != INVALID_SOCKET)
        {
            CxSocket::close(f_socketFD);
            f_socketFD = INVALID_SOCKET;
        }
        f_clientConnectState = GATEWAY_CONNECT_STATE_DIS;
        f_clientDisConnectTime = CxTime::currentMsepoch();
        f_clientTid = 0;
    }
    return NULL;
}

void GatewayClient::init(const char *appName, int protocol, const char *uartName)
{
    f_clientEventLock = fn_clientEventLock();
    if (appName != NULL && strlen(appName) < APP_NAME_LEN)
    {
        memcpy(S_APP_NAME, appName, strlen(appName));
        S_APP_NAME[APP_NAME_LEN - 1] = '\0';
    }
    else
    {
        gateway_client_debug_print(EN_LOG_LEVEL_ERROR, ".start - appName error! appName tool long.");
    }
    if (uartName != NULL && strlen(uartName) < UART_NAME_LEN && strlen(uartName) > 0)
    {
        memcpy(S_UART_NAME, uartName, strlen(uartName));
        S_UART_NAME[UART_NAME_LEN - 1] = '\0';
    }
    else
    {
        const char DefaultUartName[] = "/dev/ttyS6";
        int i = strlen(DefaultUartName);
        memcpy(S_UART_NAME, DefaultUartName, i);
        S_UART_NAME[i] = '\0';
    }
    I_PROTOCOL = protocol;
}

void GatewayClient::start(const msepoch_t &dtNow)
{
    if (!f_clientRunning && (dtNow - f_clientDisConnectTime) > 1000)
    {
        f_clientRunning = true;
//    pthread_create(&clientpid, NULL, reinterpret_cast<void *(*)(void *)>(modbusMain),NULL);
        //
        pthread_attr_t attr;
        pthread_attr_init(&attr);
        pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
        pthread_attr_setinheritsched(&attr, PTHREAD_INHERIT_SCHED);
        //
        pthread_create(&f_clientTid, &attr, gatewayMain, NULL);
        if (f_clientTid == 0)
        {
            gateway_client_debug_print(EN_LOG_LEVEL_ERROR, "Gateway pthread_create error! errno=%d", errno);
            f_clientDisConnectTime = CxTime::currentMsepoch();
            f_clientRunning = false;
        }
        //
        pthread_attr_destroy(&attr);
    }
}

void GatewayClient::stop()
{
    if (f_clientRunning)
    {
        f_clientRunning = false;
        if (f_socketFD != INVALID_SOCKET)
        {
            CxSocket::close(f_socketFD);
            f_socketFD = INVALID_SOCKET;
        }
        if (f_clientTid != 0)
        {
            pthread_join(f_clientTid, NULL);
            f_clientTid = 0;
        }
        gateway_client_debug_print(EN_LOG_LEVEL_DEBUG, "Gateway stop !");
        gateway_client_debug_close();
    }
}

void GatewayClient::pushBusinessState(int state, const char *sockAddress, const char *networkName, unsigned long long int recvTotalLength, unsigned long long int recvLastTime)
{
    BusinessStateInfo info;
    info.state = state;
    strncpy(info.sockAddress, sockAddress, SOCK_ADDR_MAX_LEN);
    strncpy(info.networkName, networkName, NETWORK_NAME_MAX_LEN);
    info.recvTotalLength = recvTotalLength;
    info.recvLastTime = recvLastTime;
    {
        CxMutexScope lock(f_clientBusinessStateLock);
        f_clitentBusinessState[f_clitentBusinessState.size()] = info;
    }
}

int GatewayClient::getNewEvent()
{
    if (f_clientEventCount > 0)
    {
        CxMutexScope lock(f_clientEventLock);
        if (f_clientIEC104Servers.size() > 0)
        {
            return TYPE_PROPSET;
        }

        if (f_clientPhaseNotify.size() > 0)
        {
            return TYPE_APP_EVENT_PHASE;
        }

        if (f_clientFailureEvents.size() > 0)
        {
            return TYPE_APP_AREA_BUG_REPORT;
        }

        if (f_clientDB_TABLEs.size() > 0)
        {
            return TYPE_APP_GWDB_MODIFY;
        }
    }

    return 0;
}

IEC104Server GatewayClient::popIEC104Server()
{
    IEC104Server r{0};
    CxMutexScope lock(f_clientEventLock);
    if (f_clientIEC104Servers.size() > 0)
    {
        r = f_clientIEC104Servers.front();
        f_clientIEC104Servers.pop_front();
        f_clientEventCount = f_clientIEC104Servers.size() + f_clientDB_TABLEs.size() + f_clientFailureEvents.size() + f_clientPhaseNotify.size();
    }
    return r;
}

std::string GatewayClient::popDB_TABLE()
{
    std::string r;
    CxMutexScope lock(f_clientEventLock);
    if (f_clientDB_TABLEs.size() > 0)
    {
        r = f_clientDB_TABLEs.begin()->first;
        f_clientDB_TABLEs.erase(f_clientDB_TABLEs.begin());
        f_clientEventCount = f_clientIEC104Servers.size() + f_clientDB_TABLEs.size() + f_clientFailureEvents.size() + f_clientPhaseNotify.size();
    }
    return r;
}

std::string GatewayClient::popEventData(AREABUG_YX *ptDate)
{
    std::string r;
    std::map<std::string, AREABUG_YX> tempMap;
    CxMutexScope lock(f_clientEventLock);
    if (f_clientFailureEvents.size() > 0)
    {
        tempMap = f_clientFailureEvents.begin()->second;
        r = tempMap.begin()->first;
        *ptDate = tempMap.begin()->second;
        f_clientFailureEvents.erase(f_clientFailureEvents.begin());
        f_clientEventCount = f_clientIEC104Servers.size() + f_clientDB_TABLEs.size() + f_clientFailureEvents.size() + f_clientPhaseNotify.size();
    }
    return r;
}

std::string GatewayClient::popPhaseNotif(PHASE_NOTIFY *ptDate)
{
    std::string r;
    CxMutexScope lock(f_clientEventLock);
    if (f_clientPhaseNotify.size() > 0)
    {
        r = f_clientPhaseNotify.begin()->first;
        *ptDate = f_clientPhaseNotify.begin()->second;
        f_clientPhaseNotify.erase(f_clientPhaseNotify.begin());
        f_clientEventCount = f_clientIEC104Servers.size() + f_clientDB_TABLEs.size() + f_clientFailureEvents.size() + f_clientPhaseNotify.size();
    }
    return r;
}

void GatewayClient::sendTopologyFileStart()
{
    if (f_socketFD == INVALID_SOCKET)
    {
        return;
    }

    APPDATA appdata = {0};
    REGISTER_INFO *pInfo = (REGISTER_INFO *) appdata.buff;
    appdata.hdr.packetType = TYPE_APP_START_TOPO_RECOGNITION;
    appdata.hdr.packetLen = 0;
    if (sendDate((char *) &appdata, sizeof(APPDATA), f_socketFD) == SOCKET_ERROR)
    {
        f_clientRunning = false;
        if (f_socketFD != INVALID_SOCKET)
        {
            CxSocket::close(f_socketFD);
            f_socketFD = INVALID_SOCKET;
        }
        if (f_clientTid != 0)
        {
            pthread_join(f_clientTid, NULL);
            f_clientTid = 0;
        }
    }
}

static std::vector<std::vector<HW_UCHAR>> f_sDeviceCollectBuffers;
static std::vector<int> f_sDeviceCollectCounts;
static int f_iDeviceCollectSend = 0;
static int f_iDeviceCollectTotal = 0;

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

//APPDATA ---> buff结构体定义
//------->调整app优先级
typedef struct protMidwarePri
{
    HW_INT  	protNum;				//--->检验--->被调整app的业务号
    HW_INT		priority;				//<---执行<---将被调整到的优先级
    HW_CHAR 	appName[APP_NAME_LEN];	//--->检验--->被调整app的名字
}PROT_MIDWARE_PRI;

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



static int fn_sendDeviceCollect()
{
    if (f_socketFD == INVALID_SOCKET)
    {
        gateway_client_debug_print(EN_LOG_LEVEL_DEBUG, "Send Device Collect Fail INVALID_SOCKET [ %d / %d ].", f_iDeviceCollectSend, f_sDeviceCollectBuffers.size());
        return -1;
    }
    if (f_iDeviceCollectSend >= f_sDeviceCollectBuffers.size())
    {
        gateway_client_debug_print(EN_LOG_LEVEL_DEBUG, "Send Device Collect Fail [ %d / %d ].", f_iDeviceCollectSend, f_sDeviceCollectBuffers.size());
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
    int rst = sendDate((char *) &appdata, sizeof(APPDATA), f_socketFD);
    if (rst == SOCKET_ERROR)
    {
        f_clientRunning = false;
        if (f_socketFD != INVALID_SOCKET)
        {
            CxSocket::close(f_socketFD);
            f_socketFD = INVALID_SOCKET;
        }
        if (f_clientTid != 0)
        {
            pthread_join(f_clientTid, NULL);
            f_clientTid = 0;
        }
    }
    gateway_client_debug_print(EN_LOG_LEVEL_DEBUG, "Send Device Collect [ %d / %d ]. %s.", f_iDeviceCollectSend, f_sDeviceCollectBuffers.size(), (char *)(appdata.buff+0x10));
    return rst;
}

int GatewayClient::sendDeviceCollect(std::vector<std::string> sDevices)
{
    if (f_socketFD == INVALID_SOCKET || sDevices.size() <= 0)
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
        if (sDevice.empty() || sDevice.size() > 32)
        {
            gateway_client_debug_print(EN_LOG_LEVEL_DEBUG, "Send Device Collect sDevice.empty() || sDevice.size()(%d) > 32.", sDevice.size());
            continue;
        }
        int after = index + sDevice.size();
        if (after > (MAX_MESSAGE_BUF_LEN-24))
        {
            f_sDeviceCollectBuffers.push_back(std::vector<HW_UCHAR>(buff, buff + index));
            f_sDeviceCollectCounts.push_back(count);
            index = 0;
            after = sDevice.size();
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
    return fn_sendDeviceCollect();
}

static fn_void_msg_tlv_t fn_deal_collect_ack = NULL;

void GatewayClient::registDeviceCollectAck(fn_void_msg_tlv_t fn)
{
    fn_deal_collect_ack = fn;
}

static fn_deal_meter_t fn_deal_meter = NULL;

// POWER_METER_Table
void GatewayClient::registDeviceCollect(fn_deal_meter_t fn)
{
    fn_deal_meter = fn;
}

int GatewayClient::mbProcessAppManage(APPDATA *appData, socket_t SocketFD)
{
    if (appData->hdr.serverIdOrCommand[AM_SERVERID_INDEX_FUNC] == TYPE_APP_APPMANAGE_NOTIFY_RECV_PACKET)
    {
        if (appData->hdr.serverIdOrCommand[AM_SERVERID_INDEX_SUBFUNC] == APPMANAGE_NOTIFY_RECV_PACKET_DEAL_SUC)
        {
            f_iDeviceCollectSend++;
            fn_sendDeviceCollect();
            // ack
            if (fn_deal_collect_ack != NULL)
            {
                fn_deal_collect_ack(TRUE, f_sDeviceCollectBuffers.size(), NULL, f_iDeviceCollectSend-1, NULL, NULL);
            }
        }
        else
        {
            // nack
            if (fn_deal_collect_ack != NULL)
            {
                fn_deal_collect_ack(FALSE, f_sDeviceCollectBuffers.size(), NULL, f_iDeviceCollectSend, NULL, NULL);
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
    gateway_client_debug_print(EN_LOG_LEVEL_DEBUG, "Collect appHeader: appData->hdr.mMid=%d appData->hdr.requestId=%s"
                                                   " appData->hdr.deviceId=%s appData->hdr.serverIdOrCommand=[%x/%x] appData->hdr.packetType=0x%x appData->hdr.packetLen=%d Buff=%s.",
                               appData->hdr.mMid, appData->hdr.requestId, appData->hdr.deviceId,
                               appData->hdr.serverIdOrCommand[AM_SERVERID_INDEX_FUNC], appData->hdr.serverIdOrCommand[AM_SERVERID_INDEX_SUBFUNC] ,appData->hdr.packetType, appData->hdr.packetLen, appData->buff);
    return 0;
}
