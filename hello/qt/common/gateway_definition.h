#ifndef HELLO_IEC104_GATEWAY_DEFINITION_H
#define HELLO_IEC104_GATEWAY_DEFINITION_H

#define APP_NAME_LEN (32)
// deviceSupport.h
#define MAX_ID_LEN 64
#define MAX_DEV_ID_LEN 128
//
#define MAX_MESSAGE_BUF_LEN 	1024
// uart.h
#define UART_NAME_LEN	(32)
#define TYPE_APP_NAME 	(10)
// Reply message
#define TYPE_APP_ACK (0xfe)
//
#define DB_TABLE_NAME_LEN 128
// socket address max len
#define SOCK_ADDR_MAX_LEN 64
// network name max len
#define NETWORK_NAME_MAX_LEN  255

// xin tong
#define PLC_MAIL_PACKET_INDEX       (0x01)
#define PLC_MAIL_PAYLOAD_INDEX      (0x02)
#define APP_CNS_COLLECT             (0x11)
#define APP_SYNC_TIME               (0x13)
#define APP_AREA_IDENTIFY           (0x14)
#define TYPE_SEND_SOUTHING_DATA     (14)
//#define SERVER_ID_OR_COMMAND        (62)


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
    APP_PACKET_MODBUS,
    APP_PACKET_MODBUS_PASSTHRU,
    APP_PACKAGE_PHASE_EVENT = 0x10,
    APP_PACKET_VIRTUAL_104,
    APP_PACKET_PHASE_BALANCE,
    APP_PACKET_CHARGE,
    APP_PACKET_645,
    APP_PACKET_DIO,
    APP_PACKET_VIRTUAL_MQTT,
    APP_PACKET_PORT_MAX
}APP_PACKET_PORT_TYPE;

typedef enum packageType
{
    TYPE_REPORT = 0,
    TYPE_RESPONSE = 1,
    TYPE_COMMAND = 2,
    TYPE_PROPGET = 3,
    TYPE_PROPSET = 4,
    TYPE_PROPREPORT = 5,
    TYPE_REGISTER = 6,
    TYPE_DEVICESTAUS = 7,

    TYPE_HB = 8,

    TYPE_APP_DEBUG = 11,

    TYPE_APP_TOPO_PATTERN = 0x22,

    TYPE_APP_GWDB_MODIFY = 0x50,

    //故障事件
    TYPE_APP_AREA_BUG_REPORT = 0x3C,

    // 相位信息通知
    TYPE_APP_EVENT_PHASE = 0x34,

    // 信通业务
    // 复电事件
    TYPE_APP_EVENT_POWERON = 0x30,

    // 停电事件
    TYPE_APP_EVENT_POWEROFF = 0x31,

    // 私有协议 app与appManage通信
    TYPE_APP_PROTOCOL_APPMANAGE = 0x61,

    // 主动上报业务状态
    TYPE_APP_TEST_COMMUNICATION_LINK = 0x68,

    // 开盖事件
    TYPE_APP_EVENT_CNS_OPEN_COVER = 0x70,

    // 低压事件
    TYPE_APP_EVENT_CNS_LOW_VOLTAGE_DETECT = 0x71

}PACKAGE_TYPE;


typedef struct appHeader
{
    HW_UINT  mMid;
    HW_UCHAR requestId[MAX_ID_LEN];
    HW_UCHAR deviceId[MAX_DEV_ID_LEN];
    HW_UCHAR serverIdOrCommand[MAX_ID_LEN];
    HW_UINT	 packetType;
    HW_INT	 packetLen;
}APPHEADER;

typedef struct appData
{
    APPHEADER hdr;
    HW_UCHAR buff[MAX_MESSAGE_BUF_LEN];
}APPDATA;

typedef struct registerInfo
{
    HW_CHAR appName[APP_NAME_LEN];
    HW_CHAR	deviceName[UART_NAME_LEN];
    HW_INT	protocol;
}REGISTER_INFO;

typedef struct iec104Server
{
    char serverIp[128];
    unsigned short port104;
    unsigned short portTopo;
    unsigned short flag;
}IEC104Server;

typedef struct _areaBug_yx
{
    int areaBugType;
    int areaBugStaA;
    int areaBugStaB;
    int areaBugStaC;
    char branchDevice[16];
    long recordTs;
}_AREABUG_YX;

typedef struct db_table
{
    char db[DB_TABLE_NAME_LEN];
    char deviceTable[DB_TABLE_NAME_LEN];
}DB_TABLE;

typedef struct _phaseStru
{
    char deviceId[16];
    int   PHASE_INFO;
}_PHASE_NOTIFY;

typedef struct businessstateInfo
{
    int state;                          // 0：异常；  >0：正常（一般表示连接质量）； <0 : 备用（一般是用到错误码中）
    char sockAddress[SOCK_ADDR_MAX_LEN];               // 连接上后的本地绑定地址
    char networkName[NETWORK_NAME_MAX_LEN];              // 网络接口名
    unsigned long long int recvTotalLength; // 连接上后收到的字节数
    unsigned long long int recvLastTime;    // 连接上后收到数据的最后时间
}BusinessStateInfo;

// xin tong
typedef struct meterPacket
{
    HW_CHAR packerInfo;
    HW_CHAR packetLen;
    HW_CHAR param[0];
}METER_PACKET;

typedef struct meterPaload
{
    HW_CHAR payloadInfo;
    HW_CHAR paloadLenHi;
    HW_CHAR paloadLenLow;
    HW_CHAR param[0];
}METER_PAYLOAD;

typedef struct CNSLowVoltageDetect
{
    unsigned char mac[7];
    char meterClass;
    char phaseResultA;
    char phaseResultB;
    char phaseResultC;
}CNS_LOW_VOLTAGE_DETECT;

typedef struct tagCNSLowVoltageDetect
{
    char meterClass;
    char phaseResultA;
    char phaseResultB;
    char phaseResultC;
}CNS_LOW_VOLTAGE_DATE;

//APPDATA ---> buff结构体定义
//------->调整app优先级
typedef struct protMidwarePri
{
    HW_INT  	protNum;				//--->检验--->被调整app的业务号
    HW_INT		priority;				//<---执行<---将被调整到的优先级
    HW_CHAR 	appName[APP_NAME_LEN];	//--->检验--->被调整app的名字
}PROT_MIDWARE_PRI;

#endif //HELLO_IEC104_GATEWAY_DEFINITION_H
