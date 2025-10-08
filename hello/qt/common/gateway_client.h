#ifndef HELLO_IEC104_GATEWAY_CLIENT_H
#define HELLO_IEC104_GATEWAY_CLIENT_H

#include <string>

#include "device_ddl.h"

#include <ccxx/cxglobal.h>

#define TCP_CONTAINER_SERVER_IP	"172.17.0.1"
//#define TCP_CONTAINER_SERVER_IP	"192.168.91.196"
//#define TCP_CONTAINER_SERVER_IP	"128.128.128.128"
#define TCP_SERVER_IP       	"10.168.2.128"
//#define TCP_SERVER_IP       	"192.168.91.253"
#define TCP_SERVER_PORT     	15623

    //packetType
#define TYPE_REPORT 	0
#define TYPE_RESPONSE 	1
#define TYPE_COMMAND 	2
#define TYPE_PROPGET 	3
#define TYPE_PROPSET 	4
#define TYPE_PROPREPORT 	5
#define TYPE_REGISTER 	6
#define TYPE_DEVICESTAUS 	7
//线损事件
#define  TYPE_APP_LINE_LOSS     (0x35)
//相位不平衡列表信息
#define  TYPE_APP_PHASE_BLANCE     (0x36)
//台区载荷状态
#define  TYPE_APP_AREA_LOAD     (0x37)
//台区相位不平衡状态
#define  TYPE_APP_PHASE_BLANCE_STATE    (0x38)
//台区日报表
#define  TYPE_APP_DAILY_REPORTFORM    (0x39)
//台区月报表
#define  TYPE_APP_MONTHLY_REPORTFORM    (0x3A)
//台区年报表
#define  TYPE_APP_ANNUAL_REPORTFORM    (0x3B)
//故障事件
#define  TYPE_APP_AREA_BUG_REPORT    (0x3C)

//
#define TYPE_HB 	8
#define TYPE_COMMAND_RESULT 	9
#define TYPE_APP_NAME 	(10)
#define TYPE_APP_DEBUG	(11)
//应答消息
#define TYPE_APP_ACK (0xfe)
//私有协议 app与appManage通信
#define	TYPE_APP_PROTOCOL_APPMANAGE (0x61)
// 主动上报业务状态
#define TYPE_APP_TEST_COMMUNICATION_LINK (0x68)

//
#define TYPE_SEND_SOUTHING_DATA	(12)
#define TYPE_RECEIVE_SOUTHING_DATA	(13)
//
#define TYPE_APP_START_TOPO_RECOGNITION (0x20)
#define TYPE_APP_QUERY_TOPO_PROCESSING (0x21)
#define TYPE_APP_TOPO_PATTERN (0x22)
//
#define TYPE_APP_104_PROPREPORT (0x40)
//
#define TYPE_APP_GWDB_MODIFY (0x50)
// 相位信息通知
#define  TYPE_APP_EVENT_PHASE    (0x34)

#define MAX_MESSAGE_BUF_LEN 	1024


#define MAGIC_NUM	0x20071230
#define MAX_SOCKET_APP_QUEUE_NODE_NUM 4096

#define MAX_APP_HB_DET	3
#define APP_NAME_LEN	(32)
#define MAX_VALUE_STR_LEN 64
//
// hw_type.h
#define HW_TRUE      1 /**< Indicates true */
#define HW_FALSE     0 /**< Indicates false */

#define HW_SUCCESS   0 /**< Indicates success */
#define HW_FAILED    1 /**< Indicates failed */
#define HW_NULL     ((void *)0) /**< Indicates null ptr */

typedef	short 			HW_SHORT; /**< Indicates type of int. */
typedef unsigned short 			HW_USHORT; /**< Indicates type of int. */
typedef int 			HW_INT; /**< Indicates type of int. */
typedef unsigned int 	HW_UINT; /**< Indicates type of unsigned int. */
typedef char 			HW_CHAR; /**< Indicates type of char. */
typedef unsigned char 	HW_UCHAR;/**< Indicates type of unsigned char. */
typedef int 			HW_BOOL; /**< Indicates type of bool. */
typedef void 			HW_VOID; /**< Indicates type of void. */
typedef long 			HW_LONG; /**< Indicates type of bool. */
typedef double 			HW_DOUBLE; /**< Indicates type of bool. */
typedef long long 		HW_LLONG; /**< Indicates type of bool. */

// uart.h
#define UART_NAME_LEN	(32)

// deviceSupport.h
#define MAX_ID_LEN 64
#define MAX_DEV_ID_LEN 128
#define MAX_DEVICEID_LEN (MAX_DEV_ID_LEN)

// iec104 app
#define APP_IEC104_NAME "tkiec104_server"

// socket address max len
#define SOCK_ADDR_MAX_LEN 64
// network name max len
#define NETWORK_NAME_MAX_LEN  255

//
typedef enum {
 APP_PACKET_PORT_MIN,
 APP_PACKET_PORT0,
 APP_PACKET_PORT1,
 APP_PACKET_VIRTUAL_PORT1,
 APP_PACKET_VIRTUAL_PORT_RESERVE1,
 APP_PACKET_VIRTUAL_PORT_RESERVE2,
 APP_PACKET_VIRTUAL_PORT_RESERVE3,
 APP_PACKET_VIRTUAL_PORT_RESERVE4,
 APP_PACKET_VIRTUAL_METER,
 APP_PACKET_VIRTUAL_POWER,
 APP_PACKET_VIRTUAL_PORT8,
 APP_PACKET_VIRTUAL_TOPO,
 APP_PACKET_VIRTUAL_SWITCH,
 APP_PACKET_MODBUS,  // modbus串口
 APP_PACKET_MODBUS_PASSTHRU,  // modbus串口透传
 APP_PACKAGE_PHASE_EVENT = 0x10, // 获取相位信息
 APP_PACKET_VIRTUAL_104,
 APP_PACKET_PHASE_BALANCE, 
 APP_PACKET_CHARGE,  // 充电桩
 APP_PACKET_645, // 总表
 APP_PACKET_DIO, // DIO设备
 APP_PACKET_VIRTUAL_MQTT,
 APP_PACKET_PORT_MAX
}APP_PACKET_PORT_TYPE;

///*
//容器app和网关主任务之间交互消息体；考虑到app消息并发量，app的南向和北向统一通过一个socket和网关交互，网关和app根据消息重要程度，采用优先级队列进行处理
//容器和网关通过tcp socket通信，网关作为服务端，通信IP+端口为：
//	1 容器环境：172.17.0.1/15623
//	2 网关app：127.0.0.1/15623
//*/
typedef struct appHeader
{
	HW_UINT  mMid; //网关和iot平台消息ID;主动上报属性TYPE_PROPREPORT，不提供，清零
	HW_UCHAR requestId[MAX_ID_LEN]; //华为平台64位requestId;主动上报属性TYPE_PROPREPORT，不提供，清零
	HW_UCHAR deviceId[MAX_DEV_ID_LEN]; //华为平台64位Id，非设备属性，不提供，清零
	HW_UCHAR serverIdOrCommand[MAX_ID_LEN]; //属性的serverId，或命令名；华为平台50位服务Id
	HW_UINT	 packetType; //从容器app到服务端的报文类型为：上报或应答；从服务端到容器app的报文类型为命令、属性；取值为TYPE_REPORT ~ TYPE_RECEIVE_SOUTHING_DATA
	HW_INT	 packetLen;	//报文载荷长度，不包括报文头:方便起见，在packetType==TYPE_DEVICESTAUS时，packetLen借用为设备状态DEVICE_xxx
}APPHEADER;
//
///*如果app想和平台直接通信，那么buf内容为json报文，网关会根据消息头的信息选择合适的topic上报平台*/
typedef struct appData
{
	APPHEADER hdr;
	HW_UCHAR buff[MAX_MESSAGE_BUF_LEN];
}APPDATA;

typedef struct registerInfo
{
    HW_CHAR appName[APP_NAME_LEN];		//注册的app名称，用于网关对app的管理
    HW_CHAR	deviceName[UART_NAME_LEN];	//设备名取值为传递给app的设备名；可以是串口，也可以是其它接口
    HW_INT	protocol;	//订阅的业务协议类型，为-1表示该串口收到的所有数据都往该app转发
}REGISTER_INFO;

typedef struct businessstateInfo
{
    int state;                          // 0：异常；  >0：正常（一般表示连接质量）； <0 : 备用（一般是用到错误码中）
    char sockAddress[SOCK_ADDR_MAX_LEN];               // 连接上后的本地绑定地址
    char networkName[NETWORK_NAME_MAX_LEN];              // 网络接口名
    unsigned long long int recvTotalLength; // 连接上后收到的字节数
    unsigned long long int recvLastTime;    // 连接上后收到数据的最后时间
}BusinessStateInfo;

typedef struct iec104Server
{
    char serverIp[128];
    unsigned short port104;
    unsigned short portTopo;
    unsigned short flag;
}IEC104Server;

#define DB_TABLE_NAME_LEN 128
typedef struct db_table
{
    char db[DB_TABLE_NAME_LEN];
    char deviceTable[DB_TABLE_NAME_LEN];
}DB_TABLE;

//typedef struct phaseStru
//{
//    HW_CHAR deviceId[16];
//    int   PHASE_INFO;   //单相表：表示表的相位，取值为YX_PHASE_A~YX_PHASE_NULL，3相表表示相线的相位;取值为YX_PHASE_ALL~YX_PHASE_NULL;
//} PHASE_NOTIFY;

typedef void (*fn_deal_meter_t)(const std::string &sDeviceCode, const METERTable& meter);

class GatewayClient
{
public:
    static void init(const char *appName, int protocol, const char *uartName = NULL);

    static void start(const msepoch_t &dtNow);

    static void stop();

    static void pushBusinessState(int state, const char *sockAddress, const char *networkName, unsigned long long int recvTotalLength, unsigned long long int recvLastTime);

    static int getNewEvent();

    static IEC104Server popIEC104Server();

    static std::string popDB_TABLE();

    static void sendTopologyFileStart();

    static std::string popEventData(AREABUG_YX *ptDate);

    static std::string popPhaseNotif(PHASE_NOTIFY *ptDate);

    static int sendDeviceCollect(std::vector<std::string> sDevice);

    static int mbProcessAppManage(APPDATA *appData, socket_t SocketFD);

    static void registDeviceCollectAck(fn_void_msg_tlv_t fn);

    static void registDeviceCollect(fn_deal_meter_t fn);

};


#endif //HELLO_IEC104_GATEWAY_CLIENT_H
