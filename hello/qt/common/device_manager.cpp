#include "device_manager.h"

#include <ccxx/cxstring.h>
#include <ccxx/cxtime.h>
#include <ccxx/cxinterinfo_group_define.h>
#include <ccxx/cxinterinfo.h>

#include "db_manager.h"


using namespace std;


CXOUTINFO_DEFINE(IIG_NORMAL, IIG_NORMAL, "POINT-MANAGER")


static vector<DeviceTable> f_deviceTables;
static map<string, int> f_deviceTypes;

static const char * DEVICE_TYPES[DEVICE_TYPE_MAX_DEVICE_TABLE+1] = {
        "T_METER_TABLE",
        "TEMP_HUMI_TABLE",
        "TEMPCONTROLLER_TABLE",
        "SENSORSMOKE_TABLE",
        "SENSORLEAK_TABLE",
        "SENSORLEAK485_TABLE",
        "SENSORDOOR_TABLE",
        "WIRELESSTEMPTURE_TABLE",
        "METER_TABLE",
        "SWITCH_TABLE",
        "GUZHI_TABLE",
        "GATEWAY_TABLE",
        "DEV_CHARGE_KELU_TABLE",    //科陆充电设备
        "DEV_WATER_DEEP_TABLE",  //水深
        "DEV_IR_TABLE",    //红外
        "DEV_BOPO_TABLE",    //玻璃破碎传感器
        "DEV_METER_645_TABLE",    //台区645设备，直接通过645协议读取数据
        "LIGHT_TABLE",    // 光照传感器 17
        "RELAYPLC_TABLE",    // 载波开关，deviceNo编号0xb0~bf;
        "PVINVERTER_TABLE",    // 光伏逆变器,deviceNo编号0xc0~cf;
        "P485METER_TABLE",    // 485型导轨表,deviceNo编号0xd0~df;
        "MAX_DEVICE_TABLE_TABLE",
};

bool DeviceManager::getLastGUZHITable(sqlite3 *sqlite, const string &sTableName, GUZHITable *r)
{
    string sSql = CxString::format(C_GUZHI_TABLE_SELECT, sTableName.c_str(), sTableName.c_str());
    SQL::Con db(sqlite);
//    try {
    auto x = db.bindnquery<
            int,
            double,
            double,
            double,
            double,
            double,
            double,
            double,
            double,
            double,
            sqlite3_int64>(sSql);
    for (auto a:x) {
        r->ID = get<0>(a);
        r->GUZHI_CPU_OC = get<1>(a);
        r->GUZHI_PHASE_A_VOLT = get<2>(a);
        r->GUZHI_PHASE_B_VOLT = get<3>(a);
        r->GUZHI_PHASE_C_VOLT = get<4>(a);
        r->GUZHI_PHASE_A_ELEC = get<5>(a);
        r->GUZHI_PHASE_B_ELEC = get<6>(a);
        r->GUZHI_PHASE_C_ELEC = get<7>(a);
        r->GUZHI_POSTIVE_ENERGE = get<8>(a);
        r->GUZHI_REVERSE_ENERGE = get<9>(a);
        r->RECORD_TS = get<10>(a);
        return true;
    }
//    }
//    catch (int err)
//    {
//        std::cout << "Slave104 - ERROR: error = %d\n\n" << err;
//        return false;
//    }
//    catch(...)
//    {
//        std::cout << "Slave104 - ERROR:\n\n";
//        return false;
//    }
    return false;
}

bool DeviceManager::getLastMETERTable(sqlite3 *sqlite, const std::string &sTableName, METERTable *r)
{
    string sSql = CxString::format(C_METER_TABLE_SELECT, sTableName.c_str(), sTableName.c_str());
    SQL::Con db(sqlite);
    auto x = db.bindnquery<
            int,
            double,
            double,
            double,
            double,
            double,
            double,
            double,
            double,
            double,
            double,
            double,
            double,
            double,
            double,
            double,
            double,
            double,
            double,
            double,
            double,
            double,
            double,
            double,
            double,
            sqlite3_int64>(sSql);
    for (auto a:x)
    {
        r->ID = get<0>(a);
        r->POWER_METER_CPU_OC = get<1>(a);
        r->POWER_METER_ENERGE = get<2>(a);
        r->POWER_METER_BACK_ENERGE = get<3>(a);
        r->POWER_METER_FORTH_HAVE_POWER_TOTAL = get<4>(a);
        r->POWER_METER_BACK_HAVE_POWER_TOTAL = get<5>(a);
        r->POWER_METER_PHASE_A_FORTH_HAVE_POWER = get<6>(a);
        r->POWER_METER_PHASE_A_BACK_HAVE_POWER = get<7>(a);
        r->POWER_METER_PHASE_B_FORTH_HAVE_POWER = get<8>(a);
        r->POWER_METER_PHASE_B_BACK_HAVE_POWER = get<9>(a);
        r->POWER_METER_PHASE_C_FORTH_HAVE_POWER = get<10>(a);
        r->POWER_METER_PHASE_C_BACK_HAVE_POWER = get<11>(a);
        r->POWER_METER_PHASE_A_VOLT = get<12>(a);
        r->POWER_METER_PHASE_B_VOLT = get<13>(a);
        r->POWER_METER_PHASE_C_VOLT = get<14>(a);
        r->POWER_METER_PHASE_A_ELEC = get<15>(a);
        r->POWER_METER_PHASE_B_ELEC = get<16>(a);
        r->POWER_METER_PHASE_C_ELEC = get<17>(a);
        r->POWER_METER_PHASE_A_ENERGE = get<18>(a);
        r->POWER_METER_PHASE_B_ENERGE = get<19>(a);
        r->POWER_METER_PHASE_C_ENERGE = get<20>(a);
        r->POWER_METER_POWER_FATOR_A = get<21>(a);
        r->POWER_METER_POWER_FATOR_B = get<22>(a);
        r->POWER_METER_POWER_FATOR_C = get<23>(a);
        r->POWER_METER_POWER_FATOR = get<24>(a);
        r->POWER_METER_TS = get<25>(a);
        return true;
    }
    return false;
}

bool DeviceManager::getLastSWITCHTable(sqlite3 *sqlite, const std::string &sTableName, SWITCHTable *r)
{
    string sSql = CxString::format(C_SWITCH_TABLE_SELECT, sTableName.c_str(), sTableName.c_str());
    SQL::Con db(sqlite);
    auto x = db.bindnquery<
            int,
            double,
            double,
            double,
            double,
            double,
            double,
            double,
            double,
            double,
            double,
            double,
            double,
            double,
            double,
            double,
            double,
            double,
            double,
            double,
            double,
            int,
            double,
            double,
            double,
            int,
            int,
            int,
            int,
            int,
            int,
            int,
            sqlite3_int64>(sSql);
    for (auto a:x)
    {
        r->ID = get<0>(a);
        r->SWITCH_CPU_OC = get<1>(a);
        r->SWITCH_TEMPERATURE = get<2>(a);
        r->SWITCH_VOLTAGE = get<3>(a);
        r->SWITCH_CURRENT = get<4>(a);
        r->SWITCH_ENERGE = get<5>(a);
        r->SWITCH_FORTH_HAVE_POWER_TOTAL = get<6>(a);
        r->SWITCH_BACK_HAVE_POWER_TOTAL = get<7>(a);
        r->SWITCH_PHASE_A_FORTH_HAVE_POWER = get<8>(a);
        r->SWITCH_PHASE_A_BACK_HAVE_POWER = get<9>(a);
        r->SWITCH_PHASE_B_FORTH_HAVE_POWER = get<10>(a);
        r->SWITCH_PHASE_B_BACK_HAVE_POWER = get<11>(a);
        r->SWITCH_PHASE_C_FORTH_HAVE_POWER = get<12>(a);
        r->SWITCH_PHASE_C_BACK_HAVE_POWER = get<13>(a);
        r->SWITCH_PHASE_A_VOLT = get<14>(a);
        r->SWITCH_PHASE_B_VOLT = get<15>(a);
        r->SWITCH_PHASE_C_VOLT = get<16>(a);
        r->SWITCH_PHASE_A_ELEC = get<17>(a);
        r->SWITCH_PHASE_B_ELEC = get<18>(a);
        r->SWITCH_PHASE_C_ELEC = get<19>(a);
        r->SWITCH_ACTIVE_POWER = get<20>(a);
        r->SWITCH_ON_OFF = get<21>(a);
        r->SWITCH_RESIDUAL_CURRENT = get<22>(a);
        r->SWITCH_ZERO_SEQUENCE_CURRENT = get<23>(a);
        r->SWITCH_POWER_FATOR = get<24>(a);
        r->SWITCH_INPUT_A_TEMP = get<25>(a);
        r->SWITCH_INPUT_B_TEMP = get<26>(a);
        r->SWITCH_INPUT_C_TEMP = get<27>(a);
        r->SWITCH_OUTPUT_A_TEMP = get<28>(a);
        r->SWITCH_OUTPUT_B_TEMP = get<29>(a);
        r->SWITCH_OUTPUT_C_TEMP = get<30>(a);
        r->SWITCH_TEMP = get<31>(a);
        r->SWITCH_TS = get<32>(a);
        return true;
    }
    return false;
}

bool DeviceManager::getLastYXTABLE(sqlite3 *sqlite, const string &sTableName, YXTABLE *r)
{
    string sSql = CxString::format(C_YX_TABLE_SELECT, sTableName.c_str(), sTableName.c_str());
    SQL::Con db(sqlite);
//    try {
    auto x = db.bindnquery<
            int,
            int,
            int,
            int,
            int,
            double,
            double,
            double,
            double,
            double,
            double,
            sqlite3_int64>(sSql);
    for (auto a:x) {
        r->ID = get<0>(a);
        r->PHASE_A_YX = get<1>(a);
        r->PHASE_B_YX = get<2>(a);
        r->PHASE_C_YX = get<3>(a);
        r->PHASE_INFO = get<4>(a);
        r->PHASE_A_VOLT = get<5>(a);
        r->PHASE_B_VOLT = get<6>(a);
        r->PHASE_C_VOLT = get<7>(a);
        r->PHASE_A_ELEC = get<8>(a);
        r->PHASE_B_ELEC = get<9>(a);
        r->PHASE_C_ELEC = get<10>(a);
        r->RECORD_TS = get<11>(a);
        return true;
    }
//    }
//    catch (int err)
//    {
//        std::cout << "Slave104 - ERROR: error = %d\n\n";
//        return false;
//    }
//    catch(...)
//    {
//        std::cout << "Slave104 - ERROR\n\n";
//        return false;
//    }

    return false;
}

bool DeviceManager::getLastAREADAILYTable(sqlite3 *sqlite, const std::string &sTableName, AREADAILYTable *r)
{
    string sSql = CxString::format(C_AREA_DAILY_TABLE_SELECT, sTableName.c_str(), sTableName.c_str());
    SQL::Con db(sqlite);
    auto x = db.bindnquery<int, double, double, double, double, double, double, sqlite3_int64>(sSql);
    for (auto a:x)
    {
        r->ID = get<0>(a);
        r->DAILY_LINE_LOSS_IN_AREA = get<1>(a);
        r->THREE_PHASE_UNBALANCE_OF_DAILY_CURRENT = get<2>(a);
        r->OVER_LIMIT_RATE_OF_DAILY_VOLTAGE = get<3>(a);
        r->LOWER_LIMIT_RATE_OF_DAILY_VOLTAGE = get<4>(a);
        r->QUALIFIED_RATE_OF_DAILY_VOLTAGE = get<5>(a);
        r->DAILY_LOAD_RATE = get<6>(a);
        r->RECORD_TS = get<7>(a);
        return true;
    }
    return false;
}

bool DeviceManager::getLastAREAMONTHLYTable(sqlite3 *sqlite, const std::string &sTableName, AREAMONTHLYTable *r)
{
    string sSql = CxString::format(C_AREA_MONTHLY_TABLE_SELECT, sTableName.c_str(), sTableName.c_str());
    SQL::Con db(sqlite);
    auto x = db.bindnquery<int, double, double, double, double, double, double, sqlite3_int64>(sSql);
    for(auto a:x)
    {
        r->ID = get<0>(a);
        r->MONTHLY_LINE_LOSS = get<1>(a);
        r->THREE_PHASE_UNBALANCE_OF_MONTHLY_CURRENT = get<2>(a);
        r->OVER_LIMIT_RATE_OF_MONTHLY_VOLTAGE = get<3>(a);
        r->LOWER_LIMIT_RATE_OF_MONTHLY_VOLTAGE = get<4>(a);
        r->MONTHLY_VOLTAGE_QUALIFICATION_RATE = get<5>(a);
        r->MONTHLY_LOAD_RATE= get<6>(a);
        r->RECORD_TS = get<7>(a);
        return true;
    }
    return false;
}

bool DeviceManager::getLastPOWERLOSSTable(sqlite3 *sqlite, const std::string &sTableName, POWERLOSSTable *r)
{
    string sSql = CxString::format(C_POWERLOSS_TABLE_SELECT, C_POWERLOSS_TABLE_PREFIX, C_POWERLOSS_TABLE_PREFIX, sTableName.substr(strlen(C_POWERLOSS_TABLE_PREFIX) + 1).c_str());
    SQL::Con db(sqlite);
    auto x = db.bindnquery<int, int, int, int, int, string, sqlite3_int64>(sSql);
    for(auto a:x)
    {
        r->ID = get<0>(a);
        r->AREABUGTYPE = get<1>(a);
        r->AREABUGSTAA = get<2>(a);
        r->AREABUGSTAB = get<3>(a);
        r->AREABUGSTAC = get<4>(a);
        r->BRANCHDEVICE = get<5>(a);
        r->RECORD_TS = get<6>(a);
        return true;
    }
    return false;
}

bool DeviceManager::getLastAREAANNUALTable(sqlite3 *sqlite, const std::string &sTableName, AREAMETERYEARSTable *r)
{
    string sSql = CxString::format(C_AREA_ANNUAL_TABLE_SELECT, sTableName.c_str(), sTableName.c_str());
    SQL::Con db(sqlite);
    auto x = db.bindnquery<int, double, double, double, double, double, sqlite3_int64>(sSql);
    for(auto a:x)
    {
        r->ID = get<0>(a);
        r->THREE_PHASE_UNBALANCE_OF_ANNUAL_CURRENT = get<1>(a);
        r->ANNUAL_VOLTAGE_OVER_LIMIT_RATE = get<2>(a);
        r->ANNUAL_VOLTAGE_LOWER_LIMIT_RATE = get<3>(a);
        r->ANNUAL_VOLTAGE_QUALIFICATION_RATE = get<4>(a);
        r->ANNUAL_LOAD_RATE = get<5>(a);
        r->RECORD_TS = get<6>(a);
        return true;
    }
    return false;
}

bool DeviceManager::getLastCHARGETable(sqlite3 *sqlite, const std::string &sTableName, CHARGETable *r)
{
    string  sSql = CxString::format(C_CHARGE_TABLE_SELECT, sTableName.c_str(), sTableName.c_str());
    SQL::Con db(sqlite);
    auto x = db.bindnquery<int, string, double, double, double, double, double, double, double, double, double, sqlite3_int64>(sSql);
    for(auto a:x)
    {
        r->ID = get<0>(a);
        r->CHARGEID = get<1>(a);
        r->CHARGEVOLTAGE = get<2>(a);
        r->CHARGECURRENT = get<3>(a);
        r->CHARGEPOWER = get<4>(a);
        r->CHARGEENERGETOTAL = get<5>(a);
        r->CHARGEENERGETIP = get<6>(a);
        r->CHARGEENERGEPEAK = get<7>(a);
        r->CHARGEENERGEFLAT = get<8>(a);
        r->CHARGEENERGEBOTTOM = get<9>(a);
        r->CHARGETEMPTURE = get<10>(a);
        r->RECORD_TS = get<11>(a);
        return true;
    }
    return false;
}

bool DeviceManager::getLastBRANCHMONTH(sqlite3 *sqlite, const std::string &sTableName, BRANCHTable *r)
{
    msepoch_t dtNow = CxTime::currentSystemTime();
    int y, m, d, h, mi, se, ms;
    CxTime::decodeLocalTm(dtNow, y, m, d, h, mi, se, ms);
    nsepoch_t dtBegin = CxTime::encodeDateTime(y, m, d, 0, 0, 0, 0) - 24 * 3600 * 1000 * d;
    msepoch_t dtEnd = dtBegin + 24 * 3600 * 1000;

    string sSql = CxString::format(C_BRANCH_TABLE_MONTH_SELECT, C_BRANCH_TABLE_PREFIX, C_BRANCH_TABLE_PREFIX, sTableName.substr(strlen(C_BRANCH_TABLE_PREFIX) + 1).c_str(), dtBegin, dtEnd);
    SQL::Con db(sqlite);
    auto x = db.bindnquery<int, string, double, double, double, double, sqlite3_int64, sqlite3_int64>(sSql);

    for(auto a:x)
    {
//        r->ID = get<0>(a);
//        r->BRANCHDEVICE = get<1>(a);
        r->LINELOSS_MONTH = get<2>(a);
//        r->LINELOSSA = get<3>(a);
//        r->LINELOSSB = get<4>(a);
//        r->LINELOSSC = get<5>(a);
//        r->FROZTP = get<6>(a);
//        r->RECORD_TS = get<7>(a);
        return true;
    }

    r->LINELOSS_MONTH = 0;
    return true;
}

bool DeviceManager::getLastTHTABLE(sqlite3 *sqlite, const std::string &sTableName, THTable *r) {
    string sSql = CxString::format(C_TH_TABLE_SELECT, sTableName.c_str(), sTableName.c_str());
    SQL::Con db(sqlite);
    auto x = db.bindnquery<int, double, double, sqlite3_int64>(sSql);
    for (auto a:x)
    {
        r->ID = get<0>(a);
        r->TEMPERATURE = get<1>(a);
        r->HUMIDITY = get<2>(a);
        r->RECORD_TS = get<3>(a);
        return true;
    }
    return false;
}

bool DeviceManager::getLastSENSORSMOKETABLE(sqlite3 *sqlite, const std::string &sTableName, SENSORSMOKETable *r)
{
    string sSql = CxString::format(C_SENSORSMOKE_TABLE_SELECT, sTableName.c_str(), sTableName.c_str());
    SQL::Con db(sqlite);
    auto x = db.bindnquery<int, int, sqlite3_int64>(sSql);
    for (auto a:x)
    {
        r->ID = get<0>(a);
        r->SMOKESTATE = get<1>(a);
        r->RECORD_TS = get<2>(a);
        return true;
    }
    return false;
}

bool DeviceManager::getLastBRANCHTable(sqlite3 *sqlite, const std::string &sTableName, BRANCHTable *r)
{
    string sSql = CxString::format(C_BRANCH_TABLE_SELECT, C_BRANCH_TABLE_PREFIX, C_BRANCH_TABLE_PREFIX, sTableName.substr(strlen(C_BRANCH_TABLE_PREFIX) + 1).c_str());
    SQL::Con db(sqlite);
    auto x = db.bindnquery<int, string, double, double, double, double, sqlite3_int64, sqlite3_int64>(sSql);

    for(auto a:x)
    {
        r->ID = get<0>(a);
        r->BRANCHDEVICE = get<1>(a);
        r->LINELOSS = get<2>(a);
        r->LINELOSSA = get<3>(a);
        r->LINELOSSB = get<4>(a);
        r->LINELOSSC = get<5>(a);
        r->FROZTP = get<6>(a);
        r->RECORD_TS = get<7>(a);
        return true;
    }
    return false;
}

bool DeviceManager::getLastPHASEBLANCETable(sqlite3 *sqlite, const std::string &sTableName, PHASEBLANCETable *r)
{
    string sSql = CxString::format(C_PHASE_BLANCE_TABLE_SELECT, C_PHASE_BLANCE_TABLE_PREFIX, C_PHASE_BLANCE_TABLE_PREFIX, sTableName.substr(strlen(C_PHASE_BLANCE_TABLE_PREFIX) + 1).c_str());
    SQL::Con db(sqlite);
    auto x = db.bindnquery<int, string, double, sqlite3_int64>(sSql);
    for (auto a:x)
    {
        r->ID = get<0>(a);
        r->DEVICEID = get<1>(a);
        r->CURBALANCERATE = get<2>(a);
        r->RECORD_TS = get<3>(a);
        return true;
    }
    return false;
}

bool DeviceManager::getLastCHARGEYXTable(sqlite3 *sqlite, const std::string &sTableName, CHARGEYXTable *r)
{
    string sSql = CxString::format(C_CHARGE_YX_TABLE_SELECT, sTableName.c_str(), sTableName.c_str());
    SQL::Con db(sqlite);
    auto x = db.bindnquery<int, string, int, int, sqlite3_int64>(sSql);
    for (auto a:x)
    {
        r->ID = get<0>(a);
        r->CHARGEID = get<1>(a);
        r->CHARGESTATUS = get<2>(a);
        r->CHARGEHOOKSTATUS = get<3>(a);
        r->RECORD_TS = get<4>(a);
        return true;
    }
    return false;
}

std::string DeviceManager::getDeviceTableName(int iDeviceType, const string &sDeviceId)
{
    if (iDeviceType>=0 && iDeviceType<=DEVICE_TYPE_MAX_DEVICE_TABLE) {
        return string(DEVICE_TYPES[iDeviceType]) + "_" + sDeviceId;
    }
    return string();
}

bool DeviceManager::getLastJIEXI_MQTT_METER_TABLE_test(sqlite3 *sqlite, const string &sTableName,
                                                       JIEXI_MQTT_METER_TABLE_test *r)
{
    string sSql = CxString::format(C_JIEXI_MQTT_METER_TABLE_test_SELECT, sTableName.c_str(), sTableName.c_str());
    SQL::Con db(sqlite);
    auto x = db.bindnquery<int, double, double, double, double, double, double, double, double, double, double, double, double, double, double, double, sqlite3_int64>(sSql);
    for (auto a:x)
    {
        r->ID = get<0>(a);
        r->POWER_METER_FORTH_HAVE_POWER_TOTAL = get<1>(a);
        r->POWER_METER_PHASE_A_FORTH_HAVE_POWER = get<2>(a);
        r->POWER_METER_PHASE_B_FORTH_HAVE_POWER = get<3>(a);
        r->POWER_METER_PHASE_C_FORTH_HAVE_POWER = get<4>(a);
        r->POWER_METER_PHASE_A_VOLT = get<5>(a);
        r->POWER_METER_PHASE_B_VOLT = get<6>(a);
        r->POWER_METER_PHASE_C_VOLT = get<7>(a);
        r->POWER_METER_PHASE_A_ELEC = get<8>(a);
        r->POWER_METER_PHASE_B_ELEC = get<9>(a);
        r->POWER_METER_PHASE_C_ELEC = get<10>(a);
        r->tgI0 = get<11>(a);
        r->tgSupWh = get<12>(a);
        r->tgSupWhA = get<13>(a);
        r->tgSupWhB = get<14>(a);
        r->tgSupWhC = get<15>(a);
        r->POWER_METER_TS = get<16>(a);
        return true;
    }
    return false;
}

int DeviceManager::insertJIEXI_MQTT_METER_TABLE_test(sqlite3 *sqlite, const string &sTableName,
                                                      const JIEXI_MQTT_METER_TABLE_test &r)
{
    tuple<double, double, double, double, double, double, double, double, double, double, double, double, double, double, double, sqlite3_int64> row
            (
                r.POWER_METER_FORTH_HAVE_POWER_TOTAL,
                r.POWER_METER_PHASE_A_FORTH_HAVE_POWER,
                r.POWER_METER_PHASE_B_FORTH_HAVE_POWER,
                r.POWER_METER_PHASE_C_FORTH_HAVE_POWER,
                r.POWER_METER_PHASE_A_VOLT,
                r.POWER_METER_PHASE_B_VOLT,
                r.POWER_METER_PHASE_C_VOLT,
                r.POWER_METER_PHASE_A_ELEC,
                r.POWER_METER_PHASE_B_ELEC,
                r.POWER_METER_PHASE_C_ELEC,
                r.tgI0,
                r.tgSupWh,
                r.tgSupWhA,
                r.tgSupWhB,
                r.tgSupWhC,
                r.POWER_METER_TS
                    );
    SQL::Con db(sqlite);
    int result = db.bind1exec(C_JIEXI_MQTT_METER_TABLE_test_INSERT, row);
    return result;
}

bool DeviceManager::getLastIRTABLE(sqlite3 *sqlite, const std::string &sTableName, IRTable *r)
{
    string sSql = CxString::format(C_IR_TABLE_SELECT, sTableName.c_str(), sTableName.c_str());
    SQL::Con db(sqlite);
    auto x = db.bindnquery<int, int, int, sqlite3_int64>(sSql);
    for (auto a:x)
    {
        r->ID = get<0>(a);
        r->IRSTATE = get<1>(a);
        r->IRDELAYTIME = get<2>(a);
        r->RECORD_TS = get<3>(a);
        return true;
    }
    return false;
}

bool DeviceManager::getLastWATERDEEPTABLE(sqlite3 *sqlite, const std::string &sTableName, WATERDEEPTable *r)
{
    string sSql = CxString::format(C_WATER_DEEP_TABLE_SELECT, sTableName.c_str(), sTableName.c_str());
    SQL::Con db(sqlite);
    auto x = db.bindnquery<int, double, sqlite3_int64>(sSql);
    for (auto a:x)
    {
        r->ID = get<0>(a);
        r->DEEP = get<1>(a);
        r->RECORD_TS = get<2>(a);
        return true;
    }
    return false;
}

bool DeviceManager::getLastSENSORDOORTABLE(sqlite3 *sqlite, const std::string &sTableName, SENSORDOORTable *r)
{
    string sSql = CxString::format(C_SENSORDOOR_TABLE_SELECT, sTableName.c_str(), sTableName.c_str());
    SQL::Con db(sqlite);
    auto x = db.bindnquery<int, int, sqlite3_int64>(sSql);
    for (auto a:x)
    {
        r->ID = get<0>(a);
        r->DOORSTATE = get<1>(a);
        r->RECORD_TS = get<2>(a);
        return true;
    }
    return false;
}

bool DeviceManager::getLastSENSORLEAKTABLE(sqlite3 *sqlite, const std::string &sTableName, SENSORDOORTable *r)
{
    string sSql = CxString::format(C_SENSORLEAK_TABLE_SELECT, sTableName.c_str(), sTableName.c_str());
    SQL::Con db(sqlite);
    auto x = db.bindnquery<int, int, sqlite3_int64>(sSql);
    for (auto a:x)
    {
        r->ID = get<0>(a);
        r->DOORSTATE = get<1>(a);
        r->RECORD_TS = get<2>(a);
        return true;
    }
    return false;
}

bool DeviceManager::getLastRELAYPLCTABLE(sqlite3 *sqlite, const std::string &sTableName, SENSORDOORTable *r)
{
    string sSql = CxString::format(C_RELAYPLC_TABLE_SELECT, sTableName.c_str(), sTableName.c_str());
    SQL::Con db(sqlite);
    auto x = db.bindnquery<int, int, sqlite3_int64>(sSql);
    for (auto a:x)
    {
        r->ID = get<0>(a);
        r->DOORSTATE = get<1>(a);
        r->RECORD_TS = get<2>(a);
        return true;
    }
    return false;
}

bool DeviceManager::getLastBOPOTABLE(sqlite3 *sqlite, const std::string &sTableName, SENSORDOORTable *r)
{
    string sSql = CxString::format(C_BOPO_TABLE_SELECT, sTableName.c_str(), sTableName.c_str());
    SQL::Con db(sqlite);
    auto x = db.bindnquery<int, int, sqlite3_int64>(sSql);
    for (auto a:x)
    {
        r->ID = get<0>(a);
        r->DOORSTATE = get<1>(a);
        r->RECORD_TS = get<2>(a);
        return true;
    }
    return false;
}

void DeviceManager::start()
{
    //
    vector<string> sDeviceIds;
    sqlite3 *dbSqlite = DBManager::openSqlite(DBManager::getDbFilePath(DB_NAME_GW_SYS));
    SQL::Con db(dbSqlite);
    vector<DeviceTable> vs;
    map<string, int> deviceTypes;
    string sSql(C_DEVICE_TABLE_SELECT);
    {
        auto a = db.bindnquery<int, string, int, string, string, string, string, int, string>(sSql);
        for (auto x:a) {
            DeviceTable m;
            m.ID = get<0>(x);
            m.DATABASE_NAME = get<1>(x);
            m.DEVICE_TYPE = get<2>(x);
            m.DEVICE_ID = get<3>(x);
            m.DEVICE_MANUFACTOR = get<4>(x);
            m.DEVICE_SW_VER = get<5>(x);
            m.DEVICE_HW_VER = get<6>(x);
            m.DEVICE_MEM = get<7>(x);
            m.DEVICE_CPU = get<8>(x);
            m.TABLE_NAME = getDeviceTableName(m.DEVICE_TYPE, m.DEVICE_ID);
            vs.push_back(m);
            deviceTypes[m.DEVICE_ID] = m.DEVICE_TYPE;
            sDeviceIds.push_back(m.DEVICE_ID);
        }
    }
    f_deviceTables = vs;
    f_deviceTypes = deviceTypes;
    //
    DBManager::closeSqlite(dbSqlite);
    string textDeviceIds = CxString::join(sDeviceIds, " , ");
    fnInfo().out(".load DEVICE_TABLE %d [ %s ] ", sDeviceIds.size(), textDeviceIds.c_str());
}

bool DeviceManager::getLastPVINVERTERTable(sqlite3 *sqlite, const string &sTableName, PVINVERTERTable *r)
{
    string sSql = CxString::format(C_PVINVERTER_TABLE_SELECT, sTableName.c_str(), sTableName.c_str());
    SQL::Con db(sqlite);
    auto x = db.bindnquery<int, int, int, int, int, int, int, int, int, int,
            int, int, int, int, int, int, int, int, int, int,
            int, int, int, int, int, int, int, int, int, int,
            int, int, int, int, int, int, int, int, int, int,
            int, int, int, int, int, int, int, int, int, int,
            int, int, int, int, int, int, int, int, int, int,
            int, int, int, int, int, int, int, int, int, int,
            int, int, int, int, int, int, int, int, int, int,
            int, int, int, int, int, int, int, int, int, int,
            int, int, int, int, int, int, int, int, int, int,
            int, int, int, int, int, int, int, int, int, int,
            int, int, int, int, int, int, int, int, int, int,
            sqlite3_int64>(sSql);
    for (auto a:x)
    {
        r->ID = get<0>(a);
        r->PVINVERTER_SYSSTATE = get<1>(a);
        r->PVINVERTER_GENERATIONTIME_TOTAL = get<2>(a);
        r->PVINVERTER_SERVICETIME_TOTAL = get<3>(a);
        r->PVINVERTER_FREQUENCY_GRID = get<4>(a);
        r->PVINVERTER_ACTIVEPOWER_OUTPUT_TOTAL = get<5>(a);
        r->PVINVERTER_REACTIVEPOWER_OUTPUT_TOTAL = get<6>(a);
        r->PVINVERTER_APPARENTPOWER_OUTPUT_TOTAL = get<7>(a);
        r->PVINVERTER_ACTIVEPOWER_PCC_TOTAL = get<8>(a);
        r->PVINVERTER_REACTIVEPOWER_PCC_TOTAL = get<9>(a);
        r->PVINVERTER_APPARENTPOWER_PCC_TOTAL = get<10>(a);
        r->PVINVERTER_VOLTAGE_PHASE_R = get<11>(a);
        r->PVINVERTER_CURRENT_OUTPUT_R = get<12>(a);
        r->PVINVERTER_ACTIVEPOWER_OUTPUT_R = get<13>(a);
        r->PVINVERTER_REACTIVEPOWER_OUTPUT_R = get<14>(a);
        r->PVINVERTER_POWERFACTOR_OUTPUT_R = get<15>(a);
        r->PVINVERTER_CURRENT_PCC_R = get<16>(a);
        r->PVINVERTER_ACTIVEPOWER_PCC_R = get<17>(a);
        r->PVINVERTER_REACTIVEPOWER_PCC_R = get<18>(a);
        r->PVINVERTER_POWERFACTOR_PCC_R = get<19>(a);
        r->PVINVERTER_VOLTAGE_PHASE_S = get<20>(a);
        r->PVINVERTER_CURRENT_OUTPUT_S = get<21>(a);
        r->PVINVERTER_ACTIVEPOWER_OUTPUT_S = get<22>(a);
        r->PVINVERTER_REACTIVEPOWER_OUTPUT_S = get<23>(a);
        r->PVINVERTER_POWERFACTOR_OUTPUT_S = get<24>(a);
        r->PVINVERTER_CURRENT_PCC_S = get<25>(a);
        r->PVINVERTER_ACTIVEPOWER_PCC_S = get<26>(a);
        r->PVINVERTER_REACTIVEPOWER_PCC_S = get<27>(a);
        r->PVINVERTER_POWERFACTOR_PCC_S = get<28>(a);
        r->PVINVERTER_VOLTAGE_PHASE_T = get<29>(a);
        r->PVINVERTER_CURRENT_OUTPUT_T = get<30>(a);
        r->PVINVERTER_ACTIVEPOWER_OUTPUT_T = get<31>(a);
        r->PVINVERTER_REACTIVEPOWER_OUTPUT_T = get<32>(a);
        r->PVINVERTER_POWERFACTOR_OUTPUT_T = get<33>(a);
        r->PVINVERTER_CURRENT_PCC_T = get<34>(a);
        r->PVINVERTER_ACTIVEPOWER_PCC_T = get<35>(a);
        r->PVINVERTER_REACTIVEPOWER_PCC_T = get<36>(a);
        r->PVINVERTER_POWERFACTOR_PCC_T = get<37>(a);
        r->PVINVERTER_ACTIVEPOWER_PV_EXT = get<38>(a);
        r->PVINVERTER_ACTIVEPOWER_LOAD_SYS = get<39>(a);
        r->PVINVERTER_ACTIVEPOWER_LOAD_TOTAL = get<40>(a);
        r->PVINVERTER_REACTIVEPOWER_LOAD_TOTAL = get<41>(a);
        r->PVINVERTER_APPARENTPOWER_LOAD_TOTAL = get<42>(a);
        r->PVINVERTER_FREQUENCY_OUTPUT = get<43>(a);
        r->PVINVERTER_VOLTAGE_OUTPUT_R = get<44>(a);
        r->PVINVERTER_CURRENT_LOAD_R = get<45>(a);
        r->PVINVERTER_ACTIVEPOWER_LOAD_R = get<46>(a);
        r->PVINVERTER_REACTIVEPOWER_LOAD_R = get<47>(a);
        r->PVINVERTER_APPARENTPOWER_LOAD_R = get<48>(a);
        r->PVINVERTER_LOADPEAKRATIO_R = get<49>(a);
        r->PVINVERTER_VOLTAGE_OUTPUT_S = get<50>(a);
        r->PVINVERTER_CURRENT_LOAD_S = get<51>(a);
        r->PVINVERTER_ACTIVEPOWER_LOAD_S = get<52>(a);
        r->PVINVERTER_REACTIVEPOWER_LOAD_S = get<53>(a);
        r->PVINVERTER_APPARENTPOWER_LOAD_S = get<54>(a);
        r->PVINVERTER_LOADPEAKRATIO_S = get<55>(a);
        r->PVINVERTER_VOLTAGE_OUTPUT_T = get<56>(a);
        r->PVINVERTER_CURRENT_LOAD_T = get<57>(a);
        r->PVINVERTER_ACTIVEPOWER_LOAD_T = get<58>(a);
        r->PVINVERTER_REACTIVEPOWER_LOAD_T = get<59>(a);
        r->PVINVERTER_APPARENTPOWER_LOAD_T = get<60>(a);
        r->PVINVERTER_LOADPEAKRATIO_T = get<61>(a);
        r->PVINVERTER_VOLTAGE_PV1 = get<62>(a);
        r->PVINVERTER_CURRENT_PV1 = get<63>(a);
        r->PVINVERTER_POWER_PV1 = get<64>(a);
        r->PVINVERTER_VOLTAGE_PV2 = get<65>(a);
        r->PVINVERTER_CURRENT_PV2 = get<66>(a);
        r->PVINVERTER_POWER_PV2 = get<67>(a);
        r->PVINVERTER_VOLTAGE_PV3 = get<68>(a);
        r->PVINVERTER_CURRENT_PV3 = get<69>(a);
        r->PVINVERTER_POWER_PV3 = get<70>(a);
        r->PVINVERTER_VOLTAGE_PV4 = get<71>(a);
        r->PVINVERTER_CURRENT_PV4 = get<72>(a);
        r->PVINVERTER_POWER_PV4 = get<73>(a);
        r->PVINVERTER_VOLTAGE_PV5 = get<74>(a);
        r->PVINVERTER_CURRENT_PV5 = get<75>(a);
        r->PVINVERTER_POWER_PV5 = get<76>(a);
        r->PVINVERTER_VOLTAGE_PV6 = get<77>(a);
        r->PVINVERTER_CURRENT_PV6 = get<78>(a);
        r->PVINVERTER_POWER_PV6 = get<79>(a);
        r->PVINVERTER_VOLTAGE_PV7 = get<80>(a);
        r->PVINVERTER_CURRENT_PV7 = get<81>(a);
        r->PVINVERTER_POWER_PV7 = get<82>(a);
        r->PVINVERTER_VOLTAGE_PV8 = get<83>(a);
        r->PVINVERTER_CURRENT_PV8 = get<84>(a);
        r->PVINVERTER_POWER_PV8 = get<85>(a);
        r->PVINVERTER_VOLTAGE_PV9 = get<86>(a);
        r->PVINVERTER_CURRENT_PV9 = get<87>(a);
        r->PVINVERTER_POWER_PV9 = get<88>(a);
        r->PVINVERTER_VOLTAGE_PV10 = get<89>(a);
        r->PVINVERTER_CURRENT_PV10 = get<90>(a);
        r->PVINVERTER_POWER_PV10 = get<91>(a);
        r->PVINVERTER_VOLTAGE_PV11 = get<92>(a);
        r->PVINVERTER_CURRENT_PV11 = get<93>(a);
        r->PVINVERTER_POWER_PV11 = get<94>(a);
        r->PVINVERTER_VOLTAGE_PV12 = get<95>(a);
        r->PVINVERTER_CURRENT_PV12 = get<96>(a);
        r->PVINVERTER_POWER_PV12 = get<97>(a);
        r->PVINVERTER_VOLTAGE_PV13 = get<98>(a);
        r->PVINVERTER_CURRENT_PV13 = get<99>(a);
        r->PVINVERTER_POWER_PV13 = get<100>(a);
        r->PVINVERTER_VOLTAGE_PV14 = get<101>(a);
        r->PVINVERTER_CURRENT_PV14 = get<102>(a);
        r->PVINVERTER_POWER_PV14 = get<103>(a);
        r->PVINVERTER_VOLTAGE_PV15 = get<104>(a);
        r->PVINVERTER_CURRENT_PV15 = get<105>(a);
        r->PVINVERTER_POWER_PV15 = get<106>(a);
        r->PVINVERTER_VOLTAGE_PV16 = get<107>(a);
        r->PVINVERTER_CURRENT_PV16 = get<108>(a);
        r->PVINVERTER_POWER_PV16 = get<109>(a);
        r->PVINVERTER_GENERATIONTIME_TODAY = get<110>(a);
        r->PVINVERTER_PV_GENERATION_TODAY = get<111>(a);
        r->PVINVERTER_PV_GENERATION_TOTAL = get<112>(a);
        r->PVINVERTER_LOAD_CONSUMPTION_TODAY = get<113>(a);
        r->PVINVERTER_LOAD_CONSUMPTION_TOTAL = get<114>(a);
        r->PVINVERTER_ENERGY_PURCHASE_TODAY = get<115>(a);
        r->PVINVERTER_ENERGY_PURCHASE_TOTAL = get<116>(a);
        r->PVINVERTER_ENERGY_SELLING_TODAY = get<117>(a);
        r->PVINVERTER_ENERGY_SELLING_TOTAL = get<118>(a);
        r->PVINVERTER_REMOTE_ON_OFF_CONTROL = get<119>(a);
        r->RECORD_TS = get<120>(a);
        return true;
    }
    return false;
}

bool DeviceManager::getLastP485METERTable(sqlite3 *sqlite, const string &sTableName, P485METERTable *r)
{
    string sSql = CxString::format(C_P485METER_TABLE_SELECT, sTableName.c_str(), sTableName.c_str());
    SQL::Con db(sqlite);
    auto x = db.bindnquery<int,
            double, double, double, double, double, double, double, double, double, double,
            double, double, double, double, double, double, double, double, double, double,
            double, double, double, double, double, double, double, double, double, double,
            double, double, double, double, double, double, double, double, double, double,
            double, double, double, double, double, double, double, double, double, double,
            double, double, double, double, double, double, double, double, double, double,
            double, double, double, double, double, double, double, double, double, double,
            double, double, double, double, double, double, double, double, double, double,
            double, double, double, double, double, double,
            sqlite3_int64>(sSql);
    for (auto a:x)
    {
        r->ID = get<0>(a);
        r->P485METER_UA = get<1>(a);
        r->P485METER_UB = get<2>(a);
        r->P485METER_UC = get<3>(a);
        r->P485METER_IA = get<4>(a);
        r->P485METER_IB = get<5>(a);
        r->P485METER_IC = get<6>(a);
        r->P485METER_ACTIVEPOWER_TOTAL = get<7>(a);
        r->P485METER_APPARENTPOWER_TOTAL = get<8>(a);
        r->P485METER_REACTIVEPOWER_TOTAL = get<9>(a);
        r->P485METER_POWERFACTOR_TOTAL = get<10>(a);
        r->P485METER_ACTIVEPOWERDEMAND_TOTAL = get<11>(a);
        r->P485METER_UAB = get<12>(a);
        r->P485METER_UBC = get<13>(a);
        r->P485METER_UCA = get<14>(a);
        r->P485METER_CURRENTIMBALANCE = get<15>(a);
        r->P485METER_ZEROSEQUENCECURRENT = get<16>(a);
        r->P485METER_ACTIVEPOWER_A = get<17>(a);
        r->P485METER_ACTIVEPOWER_B = get<18>(a);
        r->P485METER_ACTIVEPOWER_C = get<19>(a);
        r->P485METER_REACTIVEPOWER_A = get<20>(a);
        r->P485METER_REACTIVEPOWER_B = get<21>(a);
        r->P485METER_REACTIVEPOWER_C = get<22>(a);
        r->P485METER_APPARENTPOWER_A = get<23>(a);
        r->P485METER_APPARENTPOWER_B = get<24>(a);
        r->P485METER_APPARENTPOWER_C = get<25>(a);
        r->P485METER_POWERFACTOR_A = get<26>(a);
        r->P485METER_POWERFACTOR_B = get<27>(a);
        r->P485METER_POWERFACTOR_C = get<28>(a);
        r->P485METER_FREQUENCY = get<29>(a);
        r->P485METER_UATHD = get<30>(a);
        r->P485METER_UBTHD = get<31>(a);
        r->P485METER_UCTHD = get<32>(a);
        r->P485METER_IATHD = get<33>(a);
        r->P485METER_IBTHD = get<34>(a);
        r->P485METER_ICTHD = get<35>(a);
        r->P485METER_UATH_3 = get<36>(a);
        r->P485METER_UATH_5 = get<37>(a);
        r->P485METER_UATH_7 = get<38>(a);
        r->P485METER_UATH_9 = get<39>(a);
        r->P485METER_UATH_11 = get<40>(a);
        r->P485METER_UATH_13 = get<41>(a);
        r->P485METER_UATH_15 = get<42>(a);
        r->P485METER_UATH_17 = get<43>(a);
        r->P485METER_UBTH_3 = get<44>(a);
        r->P485METER_UBTH_5 = get<45>(a);
        r->P485METER_UBTH_7 = get<46>(a);
        r->P485METER_UBTH_9 = get<47>(a);
        r->P485METER_UBTH_11 = get<48>(a);
        r->P485METER_UBTH_13 = get<49>(a);
        r->P485METER_UBTH_15 = get<50>(a);
        r->P485METER_UBTH_17 = get<51>(a);
        r->P485METER_UCTH_3 = get<52>(a);
        r->P485METER_UCTH_5 = get<53>(a);
        r->P485METER_UCTH_7 = get<54>(a);
        r->P485METER_UCTH_9 = get<55>(a);
        r->P485METER_UCTH_11 = get<56>(a);
        r->P485METER_UCTH_13 = get<57>(a);
        r->P485METER_UCTH_15 = get<58>(a);
        r->P485METER_UCTH_17 = get<59>(a);
        r->P485METER_IATH_3 = get<60>(a);
        r->P485METER_IATH_5 = get<61>(a);
        r->P485METER_IATH_7 = get<62>(a);
        r->P485METER_IATH_9 = get<63>(a);
        r->P485METER_IATH_11 = get<64>(a);
        r->P485METER_IATH_13 = get<65>(a);
        r->P485METER_IATH_15 = get<66>(a);
        r->P485METER_IATH_17 = get<67>(a);
        r->P485METER_IBTH_3 = get<68>(a);
        r->P485METER_IBTH_5 = get<69>(a);
        r->P485METER_IBTH_7 = get<70>(a);
        r->P485METER_IBTH_9 = get<71>(a);
        r->P485METER_IBTH_11 = get<72>(a);
        r->P485METER_IBTH_13 = get<73>(a);
        r->P485METER_IBTH_15 = get<74>(a);
        r->P485METER_IBTH_17 = get<75>(a);
        r->P485METER_ICTH_3 = get<76>(a);
        r->P485METER_ICTH_5 = get<77>(a);
        r->P485METER_ICTH_7 = get<78>(a);
        r->P485METER_ICTH_9 = get<79>(a);
        r->P485METER_ICTH_11 = get<80>(a);
        r->P485METER_ICTH_13 = get<81>(a);
        r->P485METER_ICTH_15 = get<82>(a);
        r->P485METER_ICTH_17 = get<83>(a);
        r->P485METER_VOLTAGE_IMBALANCE = get<84>(a);
        r->P485METER_ACTIVEENERGY_TOTAL = get<85>(a);
        r->P485METER_REACTIVEENERGY_TOTAL = get<86>(a);
        r->RECORD_TS = get<87>(a);
        return true;
    }
    return false;
}

bool DeviceManager::getLastCNSDevInfoTable(sqlite3 *sqlite, const string &sTableName, CNS_DEVINFOTable *r)
{
    try
    {
        string sSql = CxString::format(C_CNS_DEVINFO_TABLE_SELECT, sTableName.c_str(), sTableName.c_str());
        SQL::Con db(sqlite);
        auto x = db.bindnquery<int, string, string, int, int, int, int, int, string, string, int, string, int, int, int, int, int, int, int, int, int, string, int, int, int, int, sqlite3_int64>(
                sSql);
        for (auto a:x)
        {
            r->ID = get<0>(a);
            r->MODULE_ADDR = get<1>(a);
            r->MAC = get<2>(a);
            r->PRODUCT_TYPE = get<3>(a);
            r->DEVICE_TYPE = get<4>(a);
            r->ROLE = get<5>(a);
            r->TEI = get<6>(a);
            r->PROXY_TEI = get<7>(a);
            r->SZCHIPCODE = get<8>(a);
            r->SZMANUCODE = get<9>(a);
            r->UCBOOTVER = get<10>(a);
            r->VRCVERSION = get<11>(a);
            r->us_year = get<12>(a);
            r->us_month = get<13>(a);
            r->us_day = get<14>(a);
            r->PHASE_STATUS = get<15>(a);
            r->PHASE_RESULT = get<16>(a);
            r->PHASE_RESULT_B = get<17>(a);
            r->PHASE_RESULT_C = get<18>(a);
            r->phase_flag = get<19>(a);
            r->STATUS = get<20>(a);
            r->LOCATION = get<21>(a);
            r->PROXY_CHANGE_CNT = get<22>(a);
            r->LEAVE_CNT = get<23>(a);
            r->LEAVE_TOTAL_TIME = get<24>(a);
            r->JOIN_NET_TIME = get<25>(a);
            r->RECORD_TS = get<26>(a);
            return true;
        }
    }
    catch (...)
    {
    }
    return false;
}

bool DeviceManager::getLastCNSDevStatusTable(sqlite3 *sqlite, const string &sTableName, CNS_DEV_STATUSTable *r)
{
    try
    {
        string sSql = CxString::format(C_CNS_DEV_STATUS_TABLE_SELECT, sTableName.c_str(), sTableName.c_str());
        SQL::Con db(sqlite);
        auto x = db.bindnquery<int, string, int, int, sqlite3_int64>(sSql);
        for (auto a:x)
        {
            r->ID = get<0>(a);
            r->MAC = get<1>(a);
            r->ROLE = get<2>(a);
            r->STATUS = get<3>(a);
            r->RECORD_TS = get<4>(a);
            return true;
        }
    }
    catch (...)
    {
    }
    return false;
}

bool DeviceManager::getLastCNSTOPOTable(sqlite3 *sqlite, const string &sTableName, CNS_TOPOTable *r)
{
    try
    {
        string sSql = CxString::format(C_CNS_TOPO_TABLE_SELECT, sTableName.c_str(), sTableName.c_str());
        SQL::Con db(sqlite);
        auto x = db.bindnquery<int, string, int, int, int, int, sqlite3_int64>(sSql);
        for (auto a:x)
        {
            r->ID = get<0>(a);
            r->MAC = get<1>(a);
            r->ROLE = get<2>(a);
            r->LEVEL = get<3>(a);
            r->TEI = get<4>(a);
            r->PROXY_TEI = get<5>(a);
            r->RECORD_TS = get<6>(a);
            return true;
        }
    }
    catch (...)
    {
    }
    return false;
}

bool DeviceManager::getLastCNSSnrParTable(sqlite3 *sqlite, const string &sTableName, CNS_CNS_SNR_PARTable *r)
{
    string sSql = CxString::format(C_CNS_SNR_PAR_TABLE_TABLE_SELECT, sTableName.c_str(), sTableName.c_str());
    SQL::Con db(sqlite);
    auto x = db.bindnquery<int, string, int, int, sqlite3_int64>(sSql);
    for (auto a:x)
    {
        r->ID = get<0>(a);
        r->MAC = get<1>(a);
        r->SNR = get<2>(a);
        r->PAR = get<3>(a);
        r->RECORD_TS = get<4>(a);
        return true;
    }
    return false;
}

bool DeviceManager::getLastVOCTable(sqlite3 *sqlite, const string &sTableName, TVOC_table *r)
{
    string sSql = CxString::format(C_TVOC_TABLE_SELECT, sTableName.c_str(), sTableName.c_str());
    SQL::Con db(sqlite);
    auto x = db.bindnquery<int, int, sqlite3_int64>(sSql);
    for (auto a:x)
    {
        r->ID = get<0>(a);
        r->TVOC = get<1>(a);
        r->RECORD_TS = get<2>(a);
        return true;
    }
    return false;
}

bool DeviceManager::getLastSENSORLEAKTable(sqlite3 *sqlite, const string &sTableName, SENSORLEAK_table *r)
{
    string sSql = CxString::format(C_SENSORLEAK_TABLE_SELECT, sTableName.c_str(), sTableName.c_str());
    SQL::Con db(sqlite);
    auto x = db.bindnquery<int, int, sqlite3_int64>(sSql);
    for (auto a:x)
    {
        r->ID = get<0>(a);
        r->LEAKSTATE = get<1>(a);
        r->RECORD_TS = get<2>(a);
        return true;
    }
    return false;
}

bool DeviceManager::getLastNOISETable(sqlite3 *sqlite, const string &sTableName, NOISE_table *r)
{
    string sSql = CxString::format(C_NOISE_TABLE_SELECT, sTableName.c_str(), sTableName.c_str());
    SQL::Con db(sqlite);
    auto x = db.bindnquery<int, double, sqlite3_int64>(sSql);
    for (auto a:x)
    {
        r->ID = get<0>(a);
        r->NOISE = get<1>(a);
        r->RECORD_TS = get<2>(a);
        return true;
    }
    return false;
}

bool DeviceManager::getLastDAILYTable(sqlite3 *sqlite, const string &sTableName, DAILY_Table *r)
{
    string sSql = CxString::format(C_DAILY_TABLE_SELECT, sTableName.c_str(), sTableName.c_str());
    SQL::Con db(sqlite);
    auto x = db.bindnquery<int, int, double, sqlite3_int64>(sSql);
    for (auto a:x)
    {
        r->ID = get<0>(a);
        r->PHASE_INFO = get<1>(a);
        r->POWER_METER_ENERGE = get<2>(a);
        r->RECORD_TS = get<3>(a);
        return true;
    }
    return false;
}

bool DeviceManager::getLastONTIMETable(sqlite3 *sqlite, const string &sTableName, ONTIME_Table *r)
{
    string sSql = CxString::format(C_ONTIME_TABLE_SELECT, sTableName.c_str(), sTableName.c_str());
    SQL::Con db(sqlite);
    auto x = db.bindnquery<int, int, double, double, string, sqlite3_int64>(sSql);
    for (auto a:x)
    {
        r->ID = get<0>(a);
        r->PHASE_INFO = get<1>(a);
        r->ONTIME_POSITIVE_ENERGEY = get<2>(a);
        r->ONTIME_REACTIVE_ENERGEY = get<3>(a);
        r->ONTIME_FREEZEN = get<4>(a);
        r->RECORD_TS = get<5>(a);
        return true;
    }
    return false;
}

bool DeviceManager::getLastO2Table(sqlite3 *sqlite, const string &sTableName, O2_table *r)
{
    string sSql = CxString::format(C_O2_TABLE_SELECT, sTableName.c_str(), sTableName.c_str());
    SQL::Con db(sqlite);
    auto x = db.bindnquery<int, double, sqlite3_int64>(sSql);
    for (auto a:x)
    {
        r->ID = get<0>(a);
        r->O2 = get<1>(a);
        r->RECORD_TS = get<2>(a);
        return true;
    }
    return false;
}

bool DeviceManager::getLastO3Table(sqlite3 *sqlite, const string &sTableName, O3_table *r)
{
    string sSql = CxString::format(C_O3_TABLE_SELECT, sTableName.c_str(), sTableName.c_str());
    SQL::Con db(sqlite);
    auto x = db.bindnquery<int, double, sqlite3_int64>(sSql);
    for (auto a:x)
    {
        r->ID = get<0>(a);
        r->O3 = get<1>(a);
        r->RECORD_TS = get<2>(a);
        return true;
    }
    return false;
}

bool DeviceManager::getLastQXTable(sqlite3 *sqlite, const string &sTableName, QX_table *r)
{
    string sSql = CxString::format(C_QX_TABLE_SELECT, sTableName.c_str(), sTableName.c_str());
    SQL::Con db(sqlite);
    auto x = db.bindnquery<int, double, double, double, double, double, double, sqlite3_int64>(sSql);
    for (auto a:x)
    {
        r->ID = get<0>(a);
        r->TEMPERATURE = get<1>(a);
        r->HUMIDITY = get<2>(a);
        r->NAP = get<3>(a);
        r->WIND_SPEED = get<4>(a);
        r->WIND_DIRECTION = get<5>(a);
        r->RAINFULL = get<6>(a);
        r->RECORD_TS = get<7>(a);
        return true;
    }
    return false;
}

bool DeviceManager::getLastSF6Table(sqlite3 *sqlite, const string &sTableName, SF6_table *r)
{
    string sSql = CxString::format(C_SF6_TABLE_SELECT, sTableName.c_str(), sTableName.c_str());
    SQL::Con db(sqlite);
    auto x = db.bindnquery<int, double, sqlite3_int64>(sSql);
    for (auto a:x)
    {
        r->ID = get<0>(a);
        r->SF6 = get<1>(a);
        r->RECORD_TS = get<2>(a);
        return true;
    }
    return false;
}

bool DeviceManager::getLastOILTable(sqlite3 *sqlite, const string &sTableName, OIL_table *r)
{
    string sSql = CxString::format(C_OIL_TABLE_SELECT, sTableName.c_str(), sTableName.c_str());
    SQL::Con db(sqlite);
    auto x = db.bindnquery<int, int, int, int, int, sqlite3_int64>(sSql);
    for (auto a:x)
    {
        r->ID = get<0>(a);
        r->R = get<1>(a);
        r->G = get<2>(a);
        r->B = get<3>(a);
        r->state = get<4>(a);
        r->RECORD_TS = get<5>(a);
        return true;
    }
    return false;
}

bool DeviceManager::getLastSENSORGUZHITable(sqlite3 *sqlite, const string &sTableName, SENSORGUZHI_table *r)
{
    string sSql = CxString::format(C_SENSORGUZHI_TABLE_SELECT, sTableName.c_str(), sTableName.c_str());
    SQL::Con db(sqlite);
    //    try {
    auto x = db.bindnquery<
            int,
            double,
            double,
            double,
            double,
            double,
            double,
            sqlite3_int64>(sSql);
    for (auto a:x) {
        r->ID = get<0>(a);
        r->GUZHI_PHASE_A_VOLT = get<1>(a);
        r->GUZHI_PHASE_B_VOLT = get<2>(a);
        r->GUZHI_PHASE_C_VOLT = get<3>(a);
        r->GUZHI_PHASE_A_ELEC = get<4>(a);
        r->GUZHI_PHASE_B_ELEC = get<5>(a);
        r->GUZHI_PHASE_C_ELEC = get<6>(a);
        r->RECORD_TS = get<7>(a);
        return true;
    }
    return false;
}

bool DeviceManager::getLastRFIDTable(sqlite3 *sqlite, const string &sTableName, RFID_table *r)
{
    string sSql = CxString::format(C_RFID_TABLE_SELECT, sTableName.c_str(), sTableName.c_str());
    SQL::Con db(sqlite);
    auto x = db.bindnquery<int, double, double, double, double, double, double, double, double, double, sqlite3_int64>(sSql);
    for (auto a:x)
    {
        r->ID = get<0>(a);
        r->RFID_0 = get<1>(a);
        r->RFID_1 = get<2>(a);
        r->RFID_2 = get<3>(a);
        r->RFID_3 = get<4>(a);
        r->RFID_4 = get<5>(a);
        r->RFID_5 = get<6>(a);
        r->RFID_6 = get<7>(a);
        r->RFID_7 = get<8>(a);
        r->RFID_8 = get<9>(a);
        r->RECORD_TS = get<10>(a);
        return true;
    }
    return false;
}
