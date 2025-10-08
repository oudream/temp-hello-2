#ifndef HELLO_IEC104_POINT_MANAGER_H
#define HELLO_IEC104_POINT_MANAGER_H

#include <ccxx/cxglobal.h>

#include "device_manager.h"

#define TOTAL_TABLE_NAME "999999999999"
#define QUALITY_INVALID 0xC0
#define QUALITY_VALID 0x00

// yc point type
enum
{
    YC_TIME = 1
};

//相位信息
enum
{
    //全相位
    YX_PHASE_ALL = 0,
    //相线A
    YX_PHASE_A = 1,
    //相线B
    YX_PHASE_B = 2,
    //相线C
    YX_PHASE_C = 3,
    //空相位
    YX_PHASE_NULL = 4
};

// 告警事件
//复归欠压-欠电压动作信号
#define YX_PHASE_UNDER_VOL_RESTORE (65536+1)
//复归过压-过电压动作信号
#define YX_PHASE_OVER_VOL_RESTORE (65536+2)
//复归过流--短路短延时动作信号
#define YX_PHASE_OVER_CURRENT_RESTORE (65536+3)
//复归失压-电压缺相动作信号
#define YX_PHASE_DROP_VOL_RESTORE (65536+6)
//复归重载-过载长延时动作信号
#define YX_OVERLOAD_RESTORE (65536+7)
//复归短路瞬时动作
#define YX_SWITCH_CUTSHORT_PROTECT_RESTORE (65536+8)
//复归零序动作
#define YX_SWITCH_ZERO_PROTECT_RESTORE (65536+9)
//复归时钟故障
#define YX_SWITCH_CLOCK_FAILED_RESTORE (65536+10)
//复归定值校验错误
#define YX_SWITCH_FIXED_VALUE_CHECK_RESTORE (65536+11)
//复归电流不平衡
#define YX_SWITCH_CURRENT_UNBLANCE_RESTORE (65536+12)
//复归读写定值错误
#define YX_SWITCH_FIXED_VALUE_RW_ERR_RESTORE (65536+13)
//复归过温跳闸动作
#define YX_SWITCH_HOOKDOWN_TEMPOVER_RESTORE (65536+14)
//复归超温跳闸动作
#define YX_SWITCH_HOOKDOWN_HIGHTEMP_RESTORE (65536+15)
//复归电压不平衡动作
#define YX_SWITCH_VOLTAGE_UNBLANCE_RESTORE (65536+16)
//复归电流缺相动作
#define YX_SWITCH_CURRENT_PHASE_DROP_RESTORE (65536+17)
//复归日欠压告警
#define YX_DAILY_UNDER_VOL_RESTORE (65536+18)
//复归月线路低电压
#define YX_MONTHLY_UNDER_VOLTAGE_RESTORE (65536+19)
//复归日过载告警
#define YX_DAILY_OVRELOAD_RESTORE (65536+20)
//复归月过载告警
#define YX_MONTHLY_OVRELOAD_RESTORE (65536+21)
//复归月重载
#define YX_MONTHLY_HEAVY_LOAD_RESTORE (65536+22)
//复归严重低电压告警
#define YX_LOWER_VOL_RESTORE (65536+23)

enum
{
    // 相线正常
    YX_PHASE_OK = 0,
    // 欠压 - 欠电压动作信号
    YX_PHASE_UNDER_VOL = 1,
    // 过压 - 过电压动作信号
    YX_PHASE_OVER_VOL = 2,
    // 过流 - 短路短延时动作信号
    YX_PHASE_OVER_CURRENT = 3,
    // 停电 - 电压缺相动作信号
    YX_POWER_DOWN = 4,
    // 复电
    YX_POWER_UP = 5,
    // 失压 - 电压缺相动作信号
    YX_PHASE_DROP_VOL = 6,
    // 重载 - 过载长延时动作信号
    YX_OVERLOAD = 7,
    // 短路瞬时动作
    YX_SWITCH_CUTSHORT_PROTECT = 8,
    // 零序动作
    YX_SWITCH_ZERO_PROTECT = 9,
    // 时钟故障
    YX_SWITCH_CLOCK_FAILED = 10,
    // 定值校验错误
    YX_SWITCH_FIXED_VALUE_CHECK = 11,
    // 电流不平衡
    YX_SWITCH_CURRENT_UNBLANCE = 12,
    // 读写定值错误
    YX_SWITCH_FIXED_VALUE_RW_ERR = 13,
    // 过温跳闸动作
    YX_SWITCH_HOOKDOWN_TEMPOVER = 14,
    // 超温跳闸动作
    YX_SWITCH_HOOKDOWN_HIGHTEMP = 15,
    // 电压不平衡动作
    YX_SWITCH_VOLTAGE_UNBLANCE = 16,
    // 电流缺相动作
    YX_SWITCH_CURRENT_PHASE_DROP = 17,
    // 日欠压告警
    YX_DAILY_UNDER_VOL = 18,
    // 月线路低电压
    YX_MONTHLY_UNDER_VOLTAGE = 19,
    // 日过载告警
    YX_DAILY_OVRELOAD = 20,
    // 月过载告警
    YX_MONTHLY_OVRELOAD = 21,
    // 月重载告警
    YX_MONTHLY_HEAVY_LOAD = 22,
    // 严重低电压告警
    YX_LOWER_VOL = 23
};


// 告警类型
enum
{
    // 拓扑变化告警
    POINT_TOPOLOGY_CHANGE = 0,
    // CIM模型变化告警
    POINT_CIM_CHANGE = 1,
    // ICD变化告警
    POINT_ICD_CHANGE = 2,
    // 拓扑生成遥控位置
    POINT_TOPOLOGY_CONTROL = 3,
    // 停电告警
    POINT_POWER_OUTAGES = 4,
    // 开关位置
    POINT_SWITCH_POSITION = 5,
    // 全失压
    POINT_ALL_PRESSURE_LOSS = 6,
    // A相失压
    POINT_A_PRESSURE_LOSS = 7,
    // B相失压
    POINT_B_PRESSURE_LOSS = 8,
    // C相失压
    POINT_C_PRESSURE_LOSS = 9,
    // A相欠压
    POINT_A_PHASE_VOLTAGE = 10,
    // B相欠压
    POINT_B_PHASE_VOLTAGE = 11,
    // C相欠压
    POINT_C_PHASE_VOLTAGE = 12,
    // A相过压
    POINT_A_PHASE_OVERVOLTAGE = 13,
    // B相过压
    POINT_B_PHASE_OVERVOLTAGE = 14,
    // C相过压
    POINT_C_PHASE_OVERVOLTAGE = 15,
    // A相过流
    POINT_A_PHASE_FLOW = 16,
    // B相过流
    POINT_B_PHASE_FLOW = 17,
    // C相过流
    POINT_C_PHASE_FLOW = 18,
    // A相重载
    POINT_A_PHASE_OVERLOADED = 19,
    // B相重载
    POINT_B_PHASE_OVERLOADED = 20,
    // C相重载
    POINT_C_PHASE_OVERLOADED = 21,
    // 停电故障
    POINT_SWITCH_OUTAGES = 22,
    // 本体故障
    POINT_ONTOLOGY_FAULT = 23,
    // 电流不平衡
    POINT_CURRENT_IMBALANCE = 25,
    // 电压不平衡
    POINT_VOLTAGE_IMBALANCE = 24,
    // 剩余电流告警
    POINT_RESIDUAL_CURRENTS = 26,
    // 开关_过温
    POINT_SWITCH_TEMPERATURE = 27,
    // 事故总信号
    POINT_ACCIDENT_SIGNAL = 30,
    // 日欠压
    POINT_PHASE_VOLTAGE = 40,
    // 低电压
    PINT_LOW_VOLTAGE = 41,
    // 严重低电压
    PINT_LOWER_VOL = 42,
    // 日重载
    POINT_OVERLOADING = 45,
    // 月重载
    POINT_MONTHLY_HEAVY_LOAD = 46,
    // 三相电流不平衡
    POINT_CURRENT_UNBLANCE = 50,
    // 电流缺相动作
    POINT_CURRENT_PHASE_DROP = 55,
    // 相位信息
    POINT_PHASE_INFO = 60,
    // 零序电流告警
    POINT_ZERO_SEQUENCE = 65,
    // 月线路低电压
    POINT_MONTHLY_UNDER_VOLTAGE = 70,
    // 日过载
    POINT_DAILY_OVRELOAD = 75,
    // 月过载
    POINT_MONTHLY_OVRELOAD = 76
};

/**
 * IEC60870_5_TypeID :
 *
 * M_SP_NA_1 1   SinglePointInformation_initialize
 * M_SP_TA_1 2   SinglePointWithCP24Time2a_initialize
 * M_SP_TB_1 30  SinglePointWithCP56Time2a_initialize
 * M_DP_NA_1 3   DoublePointInformation_initialize
 * M_DP_TA_1 4   DoublePointWithCP24Time2a_initialize
 * M_DP_TB_1 31  DoublePointWithCP56Time2a_initialize
 * M_ME_NA_1 9   MeasuredValueNormalized_initialize
 * M_ME_TA_1 10  MeasuredValueNormalizedWithCP24Time2a_initialize
 * M_ME_TD_1 34  MeasuredValueNormalizedWithCP56Time2a_initialize
 * M_ME_NB_1 11  MeasuredValueScaled_initialize
 * M_ME_TB_1 12  MeasuredValueScaledWithCP24Time2a_initialize
 * M_ME_TE_1 35  MeasuredValueScaledWithCP56Time2a_initialize
 * M_ME_NC_1 13  MeasuredValueShort_initialize
 * M_ME_TC_1 14  MeasuredValueShortWithCP24Time2a_initialize
 * M_ME_TF_1 36  MeasuredValueShortWithCP56Time2a_initialize
 *
 */

//APDU：
//APCI + ASDU:
//
//启动字符 + APDU的长度 + 控制域 +
//类型标识 + 可变结构限定词 + 传送原因 + 应用服务数据单元公共地址 +
//信息对象数据
//
//1 + 1 + 4 +
//1 + 1 + 2 + 2 +
//x * N
//
//遥信单点、双点的数据结构
//( 3 + N )              连续 不带时标
//        ( [3 + 1] * N )        不连续 不带时标
//        ( 3 + [1 + 7] * N )    连续 带时标
//        ( [3 + 1 + 7] * N )    不连续 带时标
//
//遥测规一化、标度化的数据结构
//( 3 + [3] * N )        连续 不带时标
//        ( [3 + 3] * N )        不连续 不带时标
//        ( 3 + [3 + 7] * N )    连续 带时标
//        ( [3 + 3 + 7] * N )    不连续 带时标
//
//遥测短浮点的数据结构
//( 3 + [5] * N )        连续 不带时标
//        ( [3 + 5] * N )        不连续 不带时标
//        ( 3 + [5 + 7] * N )    连续 带时标
//        ( [3 + 5 + 7] * N )    不连续 带时标

#define IEC60870_PAYLOAD_MAX_SIZE (240)

#define IEC60870_TYPE_NO_SEQUENCE_MAX_COUNT_M_SP_NA_1 (IEC60870_PAYLOAD_MAX_SIZE / (3 + 1))
#define IEC60870_TYPE_NO_SEQUENCE_MAX_COUNT_M_SP_TA_1 (IEC60870_PAYLOAD_MAX_SIZE / (3 + 1 + 3))
#define IEC60870_TYPE_NO_SEQUENCE_MAX_COUNT_M_SP_TB_1 (IEC60870_PAYLOAD_MAX_SIZE / (3 + 1 + 7))
#define IEC60870_TYPE_NO_SEQUENCE_MAX_COUNT_M_DP_NA_1 (IEC60870_PAYLOAD_MAX_SIZE / (3 + 1))
#define IEC60870_TYPE_NO_SEQUENCE_MAX_COUNT_M_DP_TA_1 (IEC60870_PAYLOAD_MAX_SIZE / (3 + 1 + 3))
#define IEC60870_TYPE_NO_SEQUENCE_MAX_COUNT_M_DP_TB_1 (IEC60870_PAYLOAD_MAX_SIZE / (3 + 1 + 7))
#define IEC60870_TYPE_NO_SEQUENCE_MAX_COUNT_M_ME_NA_1 (IEC60870_PAYLOAD_MAX_SIZE / (3 + 3))
#define IEC60870_TYPE_NO_SEQUENCE_MAX_COUNT_M_ME_TA_1 (IEC60870_PAYLOAD_MAX_SIZE / (3 + 3 + 3))
#define IEC60870_TYPE_NO_SEQUENCE_MAX_COUNT_M_ME_TD_1 (IEC60870_PAYLOAD_MAX_SIZE / (3 + 3 + 7))
#define IEC60870_TYPE_NO_SEQUENCE_MAX_COUNT_M_ME_NB_1 (IEC60870_PAYLOAD_MAX_SIZE / (3 + 3))
#define IEC60870_TYPE_NO_SEQUENCE_MAX_COUNT_M_ME_TB_1 (IEC60870_PAYLOAD_MAX_SIZE / (3 + 3 + 3))
#define IEC60870_TYPE_NO_SEQUENCE_MAX_COUNT_M_ME_TE_1 (IEC60870_PAYLOAD_MAX_SIZE / (3 + 3 + 7))
#define IEC60870_TYPE_NO_SEQUENCE_MAX_COUNT_M_ME_NC_1 (IEC60870_PAYLOAD_MAX_SIZE / (3 + 5))
#define IEC60870_TYPE_NO_SEQUENCE_MAX_COUNT_M_ME_TC_1 (IEC60870_PAYLOAD_MAX_SIZE / (3 + 5 + 3))
#define IEC60870_TYPE_NO_SEQUENCE_MAX_COUNT_M_ME_TF_1 (IEC60870_PAYLOAD_MAX_SIZE / (3 + 5 + 7))

struct CP24Time2a_t
{
    uint8_t encodedValue[3];
};
struct CP56Time2a_t
{
    uint8_t encodedValue[7];
};

#define YX_POINT_TYPE_NORMAL (-1)

typedef struct YxPoint
{
    // *** config ****
    int ID;
    std::string DATABASE_NAME;
    std::string TABLE_NAME;
    std::string DEVICE_ATTR;
    int POINT_TYPE;
    int CA;
    int IOA;
    // IEC60870_5_TypeID
    int TYPE_ID;
    // event
    int EVENT_TYPE;
    // -- mqtt
    // roma.serviceId
    // jiexi.model
    std::string MODEL;
    // jiexi.name
    std::string MODEL_ATTR;
    // jiexi.type
    std::string MODEL_ATTR_TYPE;
    std::string MODEL_EVENT_TYPE;
    std::string MODEL_EVENT_PARAM;
    // jiexi.guid
    std::string MODEL_GUID;
    // -- RES
    std::string RES1;
    std::string RES2;
    std::string RES3;
    // *** preset ****
    std::string DeviceId;
    std::map<std::string, std::string> ModelEventParams;
    // *** real time ***
    int VALUE;
    int OldVALUE;
    int QUALITY;
    msepoch_t TIME_MS;
    bool SINGLE_VALUE;
    // DoublePointValue
    int DOUBLE_VALUE;
    CP24Time2a_t TIME24;
    CP56Time2a_t TIME56;
    bool Tag;
} YxPoint;

typedef struct YcPoint
{
    // *** config ****
    int ID;
    std::string DATABASE_NAME;
    std::string TABLE_NAME;
    std::string DEVICE_ATTR;
    int POINT_TYPE;
    int CA;
    int IOA;
    // IEC60870_5_TypeID
    int TYPE_ID;
    //
    double NORMALIZED_MAX;
    double SCALED_FACTOR;
    int PRECISION;
    // -- mqtt
    // roma.serviceId
    // jiexi.model
    std::string MODEL;
    // roma.name
    // jiexi.name
    std::string MODEL_ATTR;
    // roma.datatype
    // jiexi.type
    std::string MODEL_ATTR_TYPE;
    // roma.客户端ID
    // jiexi.guid
    std::string MODEL_GUID;
    // -- jiexi
    // jiexi.unit
    std::string MODEL_UNIT;
    // jiexi.deadzone
    std::string MODEL_DEAD_ZONE;
    // jiexi.ratio
    std::string MODEL_RATIO;
    // jiexi.isReport
    std::string MODEL_IS_REPORT;
    // jiexi.userdefine
    std::string MODEL_USER_DEFINE;
    // -- RES
    std::string RES1;
    std::string RES2;
    std::string RES3;
    // *** preset ****
    // TABLE_NAME.substr(pos'_'+1);
    std::string DeviceId;
    // *** real time ***
    double VALUE;
    double OldVALUE;
    int QUALITY;
    msepoch_t TIME_MS;
    float SHORT_VALUE;
    float NORMALIZED_VALUE;
    int SCALED_VALUE;
    CP24Time2a_t TIME24;
    CP56Time2a_t TIME56;
    bool Tag;
    // ** string value **
    std::string VString;
} YcPoint;

typedef struct IotDevice
{
    int ID;
    std::string PARENT_DEVICE_ID;
    // jiexi.nodeID
    std::string NODE_ID;
    // roma.客户端ID
    // 设备ID Model Guid
    // jiexi.guid ( running time)
    std::string DEVICE_ID;
    std::string TOKEN;
    std::string NAME;
    // 资产编号
    std::string CODE;
    // jiexi.desc
    std::string DESCRIPTION;
    // jiexi.manuID
    std::string MANUFACTURER_ID;
    // 模型
    // roma.model
    // jiexi.model
    std::string MODEL;
    // jiexi.productID
    std::string PRODUCT_ID;
    int FW_VERSION;
    int SW_VERSION;
    int IOT_VERSION;
    int WRITE_TIME;
    int STATUS;
    std::string EXTENSION_INFO;
    std::string APP_NAME;
    // jiexi.addr
    std::string DEVICE_ADDRESS;
    std::string ONLINE_FLAG;
    std::string SLAVE;
    // for jiexi
    // jiexi.manuName
    std::string MANUFACTURER_NAME;
    // jiexi.port
    std::string DEVICE_PORT;
    // jiexi.dev ( running time)
    std::string DEV;
    // jiexi.ProType
    std::string PROTOCOL_TYPE;
    // jiexi.isReport
    std::string IS_REPORT;
} IotDevice;

typedef struct YxDate
{
    int IOA;
    int VALUE;
} YxDate;

#define DB_IEC104_YX_TABLE "app104Server"

#define CS_YX_DATE_TABLE_SELECT \
    "SELECT `IOA`, `VALUE` FROM `IEC104SLAVE_YX_TABLE` WHERE (`IOA` == %d);"

#define CS_YX_DATE_TABLE_INSERT \
    "REPLACE INTO `IEC104SLAVE_YX_TABLE`(`IOA`, `VALUE`) VALUES (%d, %d);"

class PointManager
{
public:
    static void start(sqlite3 *sqlite);

    static void start(const std::vector<YcPoint> &ycs, const std::vector<YxPoint> &yxs);

    static void preCheck();

    static void stop();

    static const std::vector<YxPoint> &getYxs();

    static const std::vector<YcPoint> &getYcs();

    static const YxPoint *getYxById(int id);

    static const YcPoint *getYcById(int id);

    static void refreshOldValue(const YxPoint *yxPoint);

    static const YxPoint *getYxByDeviceAttr(const std::string &sDeviceId, const std::string &sDeviceAttr);

    static const YcPoint *getYcByDeviceAttr(const std::string &sDeviceId, const std::string &sDeviceAttr);

    static const YcPoint *getYcByDeviceModelAttr(const std::string &sDeviceId, const std::string &ModelAttr);

    // call up IEC60870_5_TypeID
    static std::vector<YcPoint> getYcByCA(int CA, int typeId);

    // call up IEC60870_5_TypeID
    static std::vector<YxPoint> getYxByCA(int CA, int typeId);

    // soe
    static std::vector<const YxPoint*> getYxsByDeviceTBName(const std::string &dbName, const std::string &tbName);

    static std::vector<const YxPoint*> getYxsByDevice2(const std::string &deviceId, const std::string &orDeviceId);

    static std::vector<const YxPoint*> getYxsByDeviceTagTrue(const std::string &deviceId);

    // set phaseinfo
    static void setPhaseinfo(std::string &deviceId, PHASE_NOTIFY &data);

    // soe
    static std::vector<const YcPoint*> getYcsByDevice(const std::string &dbName, const std::string &tbName);

    static int loadAllYXTavleDate();

    static int insertYxTavleDate(int IOA, int VALUE);

    static int refreshAllPointValue();

    static void refreshTablePointValue(sqlite3 *db, const std::string &dbName, const std::string &tbName);

    // 事件处理（电表、开关、故指）
    static void refreshYxPonitValue(const std::string &deviceId, AREABUG_YX &eventData);

    static int refreshGUZHITablePointValue(const std::string &sDatabaseName, const std::string &sTableName, GUZHITable &guzhiTable);

    static int refreshMETERTablePointValue(const std::string &sDatabaseName, const std::string &sTableName, const METERTable &meterTable);

    static int refreshSWITCHTablePointValue(const std::string &sDatabaseName, const std::string &sTableName, SWITCHTable &switchTable);

    static int refreshYXTablePointValue(const std::string &sDatabaseName, const std::string &sTableName, YXTABLE &yxTable);

    static int refreshAREADAILYTablePointValue(const std::string &sDatabaseName, const std::string &sTableName, AREADAILYTable &areadailyTable);

    static int refreshAREAMONTHLYTablePointValue(const std::string &sDatabaseName, const std::string &sTableName, AREAMONTHLYTable &areamonthlyTable);

    static int refreshPOWERLOSSTablePointValue(const std::string &sDatabaseName, const std::string &sTableName, POWERLOSSTable &powerlossTable);

    static int refreshAREAANNUALTablePointValue(const std::string &sDatabaseName, const std::string &sTableName, AREAMETERYEARSTable &areameteryearsTable);

    static int refreshCHARGETablePointValue(const std::string &sDatabaseName, const std::string &sTableName, CHARGETable &chargeTable);

    static int refreshBRANCHTablePointValue(const std::string &sDatabaseName, const std::string &sTableName, BRANCHTable &branchTable);

    static int refreshPHASEBLANCETablePointValue(const std::string &sDatabaseName, const std::string &sTableName, PHASEBLANCETable &phaseblanceTable);

    static int refreshCHARGEYXTablePointValue(const std::string &sDatabaseName, const std::string &sTableName, CHARGEYXTable &chargeyxTable);

    static int refreshTHTablePointValue(const std::string &sDatabaseName, const std::string &sTableName, THTable &thTable);

    static int refreshSENSORSMOKETablePointValue(const std::string &sDatabaseName, const std::string &sTableName, SENSORSMOKETable &sensorsmokeTable);

    static int refreshStandbyPointValue(const std::string &sDatabaseName);

    static void resetYxPoint(YxPoint *yx);

    static void resetYcPoint(YcPoint *yc);

    static void resetIotDevice(IotDevice *device);

    static void setYxValue(YxPoint *yx, int value, msepoch_t t, bool tag);

    static void setYxValue(YxPoint *yx, int value, msepoch_t t);

    static void setYcValue(YcPoint *yc, double value, msepoch_t t, bool tag);

    static void setYcValue(YcPoint *yc, double value, msepoch_t t);

    static void setYcString(YcPoint *yc, const std::string &value, msepoch_t t);

    static bool toSingleValue(int value);

    static float toNormalizedValue(double value, double max);

    static int toScaledValue(double value, double scaled);

    static int refreshIRTablePointValue(const std::string &sDatabaseName, const std::string &sTableName, IRTable &irTable);

    static int refreshWATERDEEPTablePointValue(const std::string &sDatabaseName, const std::string &sTableName,WATERDEEPTable &waterdeepTable);

    static int refreshSENSORDOORTablePointValue(const std::string &sDatabaseName, const std::string &sTableName,SENSORDOORTable &sensordoorTable);

    static int refreshRELAYPLCTablePointValue(const std::string &sDatabaseName, const std::string &sTableName,SENSORDOORTable &sensordoorTable);

    static int refreshBOPOTablePointValue(const std::string &sDatabaseName, const std::string &sTableName,SENSORDOORTable &sensordoorTable);

    static int refreshPVINVERTERTablePointValue(const std::string &sDatabaseName, const std::string &sTableName, PVINVERTERTable &pvInverterTable);

    static int refreshP485METERTablePointValue(const std::string &sDatabaseName, const std::string &sTableName, P485METERTable &p485MeterTable);

    static int refreshCNS_DEVINFOTablePointValue(const std::string &sDatabaseName, const std::string &sTableName, CNS_DEVINFOTable &cnsDevinfoTable);

    static int refreshCNS_DEV_STATUSTablePointValue(const std::string &sDatabaseName, const std::string &sTableName, CNS_DEV_STATUSTable &cnsDevStatusTable);

    static int refreshCNS_TOPOTablePointValue(const std::string &sDatabaseName, const std::string &sTableName, CNS_TOPOTable &cnsTOPOTable);

    static int refreshCNS_CNS_SNR_PARTablePointValue(const std::string &sDatabaseName, const std::string &sTableName, CNS_CNS_SNR_PARTable &cnsCnsSnrParTable);

    static int refreshVOCTablePointValue(const std::string &sDatabaseName, const std::string &sTableName, TVOC_table &tvocTable);

    static int refreshSENSORLEAKTablePointValue(const std::string &sDatabaseName, const std::string &sTableName, SENSORLEAK_table &sensorleakTable);

    static int refreshNOISETablePointValue(const std::string &sDatabaseName, const std::string &sTableName, NOISE_table &noiseTable);

    static int refreshDAILYTablePointValue(const std::string &sDatabaseName, const std::string &sTableName, DAILY_Table &dailyTable);

    static int refreshONTIMETablePointValue(const std::string &sDatabaseName, const std::string &sTableName, ONTIME_Table &ontimeTable);

    static int refreshO2TablePointValue(const std::string &sDatabaseName, const std::string &sTableName, O2_table &o2Table);

    static int refreshO3TablePointValue(const std::string &sDatabaseName, const std::string &sTableName, O3_table &o3Table);
    
    static int refreshQXTablePointValue(const std::string &sDatabaseName, const std::string &sTableName, QX_table &qxTable);

    static int refreshSF6TablePointValue(const std::string &sDatabaseName, const std::string &sTableName, SF6_table &sf6Table);

    static int refreshOILTablePointValue(const std::string &sDatabaseName, const std::string &sTableName, OIL_table &oilTable);

    static int refreshSENSORGUZHITablePointValue(const std::string &sDatabaseName, const std::string &sTableName, SENSORGUZHI_table &sensorguzhiTable);

    static int refreshRFIDTablePointValue(const std::string &sDatabaseName, const std::string &sTableName, RFID_table &rfidTable);

    static msepoch_t getRefreshTime(const std::string &sDbName, const std::string &sTbName);

    static void setBBPvInverter(const std::string &bb);

    static const YcPoint * refreshABCByModelAttr(const std::string &sModelGuid);

    static std::string ycToString();

    static std::string getCharLastString(const std::string &str, const char ch);

private:
    static int refreshAllPointOldValue();

    static void setRefreshTime(const std::string &sDbName, const std::string &sTbName);

};

#endif //HELLO_IEC104_POINT_MANAGER_H
