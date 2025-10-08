#include "point_manager.h"

#include "db_manager.h"

#include <ccxx/cxstring.h>
#include <ccxx/cxtime.h>
#include <ccxx/cxfile.h>
#include <ccxx/cxcontainer.h>
#include <ccxx/cxinterinfo.h>
#include <ccxx/cxinterinfo_group_define.h>


enum { divide = 0, shut = 1 , invalid = -1 };

// 事件告警类型:0台区;1分支;
enum
{
    // 台区
    COURTS = 0,
    // 分支
    BRANCH = 1
};


using namespace std;


CXOUTINFO_DEFINE(IIG_NORMAL, IIG_NORMAL, "POINT_MANAGER")

#define CS_YX_POINT_TABLE_NAME "YX_POINT_TABLE"

#define CS_YX_POINT_TABLE_SELECT \
    "SELECT `ID`, `DATABASE_NAME`, `TABLE_NAME`, `DEVICE_ATTR`, `POINT_TYPE`, `CA`, `IOA`, `TYPE_ID` FROM `YX_POINT_TABLE`;"

#define CS_YX_POINT_TABLE_SELECT2 \
    "SELECT `ID`, `DATABASE_NAME`, `TABLE_NAME`, `DEVICE_ATTR`, `POINT_TYPE`, `CA`, `IOA`, `TYPE_ID`, `EVENT_TYPE`, `MODEL`, `MODEL_ATTR`, `MODEL_ATTR_TYPE`, `MODEL_EVENT_TYPE`, `MODEL_EVENT_PARAM`, `MODEL_GUID`, `MODEL_DEAD_ZONE`, `MODEL_RATIO`, `MODEL_IS_REPORT`, `MODEL_USER_DEFINE`, `RES1`, `RES2`, `RES3` FROM `YX_POINT_TABLE`;"

#define CS_YC_POINT_TABLE_NAME "YC_POINT_TABLE"

#define CS_YC_POINT_TABLE_SELECT \
    "SELECT `ID`, `DATABASE_NAME`, `TABLE_NAME`, `DEVICE_ATTR`, `CA`, `IOA`, `TYPE_ID`, `NORMALIZED_MAX`, `SCALED_FACTOR`, `PRECISION` FROM `YC_POINT_TABLE`;"

#define CS_YC_POINT_TABLE_SELECT2 \
    "SELECT `ID`, `DATABASE_NAME`, `TABLE_NAME`, `DEVICE_ATTR`, `POINT_TYPE`, `CA`, `IOA`, `TYPE_ID`, `NORMALIZED_MAX`, `SCALED_FACTOR`, `PRECISION`, `MODEL`, `MODEL_ATTR`, `MODEL_ATTR_TYPE`, `MODEL_GUID`, `MODEL_DEAD_ZONE`, `MODEL_RATIO`, `MODEL_IS_REPORT`, `MODEL_USER_DEFINE`, `RES1`, `RES2`, `RES3` FROM `YC_POINT_TABLE`;"





    
#define SEPOCH_2022  (1640966400)

static int
getMillisecond(uint8_t* encodedValue)
{
    return (encodedValue[0] + (encodedValue[1] * 0x100)) % 1000;
}

static void
setMillisecond(uint8_t* encodedValue, int value)
{
    int millies = encodedValue[0] + (encodedValue[1] * 0x100);

    /* erase sub-second part */
    millies = millies - (millies % 1000);

    millies = millies + value;

    encodedValue[0] = (uint8_t) (millies & 0xff);
    encodedValue[1] = (uint8_t) ((millies / 0x100) & 0xff);
}

static int
getSecond(uint8_t* encodedValue)
{
    return  (encodedValue[0] + (encodedValue[1] * 0x100)) / 1000;
}

static void
setSecond(uint8_t* encodedValue, int value)
{
    int millies = encodedValue[0] + (encodedValue[1] * 0x100);

    int msPart = millies % 1000;

    millies = (value * 1000) + msPart;

    encodedValue[0] = (uint8_t) (millies & 0xff);
    encodedValue[1] = (uint8_t) ((millies / 0x100) & 0xff);
}

static int
getMinute(uint8_t* encodedValue)
{
    return (encodedValue[2] & 0x3f);
}

static void
setMinute(uint8_t* encodedValue, int value)
{
    encodedValue[2] = (uint8_t) ((encodedValue[2] & 0xc0) | (value & 0x3f));
}

static void
CP24Time2a_setMillisecond(CP24Time2a_t *self, int value)
{
    setMillisecond(self->encodedValue, value);
}

static int
CP24Time2a_getSecond(CP24Time2a_t *self)
{
    return getSecond(self->encodedValue);
}

static void
CP24Time2a_setSecond(CP24Time2a_t *self, int value)
{
    setSecond(self->encodedValue, value);
}

static int
CP24Time2a_getMinute(CP24Time2a_t *self)
{
    return getMinute(self->encodedValue);
}


static void
CP24Time2a_setMinute(CP24Time2a_t *self, int value)
{
    setMinute(self->encodedValue, value);
}

// ------------------------------------------------------------------------------------------------


static int
CP56Time2a_getMillisecond(CP56Time2a_t *self)
{
    return getMillisecond(self->encodedValue);
}

static void
CP56Time2a_setMillisecond(CP56Time2a_t *self, int value)
{
    setMillisecond(self->encodedValue, value);
}

static int
CP56Time2a_getSecond(CP56Time2a_t *self)
{
    return getSecond(self->encodedValue);
}

static void
CP56Time2a_setSecond(CP56Time2a_t *self, int value)
{
    setSecond(self->encodedValue, value);
}

static int
CP56Time2a_getMinute(CP56Time2a_t *self)
{
    return getMinute(self->encodedValue);
}

static void
CP56Time2a_setMinute(CP56Time2a_t *self, int value)
{
    setMinute(self->encodedValue, value);
}

static int
CP56Time2a_getHour(CP56Time2a_t *self)
{
    return (self->encodedValue[3] & 0x1f);
}

static void
CP56Time2a_setHour(CP56Time2a_t *self, int value)
{
    self->encodedValue[3] = (uint8_t) ((self->encodedValue[3] & 0xe0) | (value & 0x1f));
}

static int
CP56Time2a_getDayOfWeek(CP56Time2a_t *self)
{
    return ((self->encodedValue[4] & 0xe0) >> 5);
}

static void
CP56Time2a_setDayOfWeek(CP56Time2a_t *self, int value)
{
    self->encodedValue[4] = (uint8_t) ((self->encodedValue[4] & 0x1f) | ((value & 0x07) << 5));
}

static int
CP56Time2a_getDayOfMonth(CP56Time2a_t *self)
{
    return (self->encodedValue[4] & 0x1f);
}

static void
CP56Time2a_setDayOfMonth(CP56Time2a_t *self, int value)
{
    self->encodedValue[4] = (uint8_t) ((self->encodedValue[4] & 0xe0) + (value & 0x1f));
}

static int
CP56Time2a_getMonth(CP56Time2a_t *self)
{
    return (self->encodedValue[5] & 0x0f);
}

static void
CP56Time2a_setMonth(CP56Time2a_t *self, int value)
{
    self->encodedValue[5] = (uint8_t) ((self->encodedValue[5] & 0xf0) + (value & 0x0f));
}

static int
CP56Time2a_getYear(CP56Time2a_t *self)
{
    return (self->encodedValue[6] & 0x7f);
}

static void
CP56Time2a_setYear(CP56Time2a_t *self, int value)
{
    value = value % 100;

    self->encodedValue[6] = (uint8_t) ((self->encodedValue[6] & 0x80) + (value & 0x7f));
}

// ----------------------------------------------


CP24Time2a_t toCP24Time2a(msepoch_t dt)
{
    int y, m, d, h, mi, se, ms;
    CxTime::decodeLocalTm(dt, y, m, d, h, mi, se, ms);
    CP24Time2a_t r{0};
    CP24Time2a_setMillisecond(&r, se * 1000 + ms);
    CP24Time2a_setMinute(&r, mi);
    return r;
}

CP56Time2a_t toCP56Time2a(msepoch_t t)
{
    msepoch_t dt = 0;
    if (t > 1000000000000 && t < 9999999999999) {
        dt = t;
    } else if (t > 1000000000 && t < 9999999999) {
        dt = t * 1000;
    }
    int y, m, d, h, mi, se, ms;
    CxTime::decodeLocalTm(dt, y, m, d, h, mi, se, ms);
    CP56Time2a_t r{0};
    CP56Time2a_setMillisecond(&r, ms);
    CP56Time2a_setSecond(&r, se);
    CP56Time2a_setMinute(&r, mi);
    CP56Time2a_setHour(&r, h);
    CP56Time2a_setDayOfMonth(&r, d);
    /* set day of week to 0 = not present */
    CP56Time2a_setDayOfWeek(&r, 0);
    CP56Time2a_setMonth(&r, m);
    CP56Time2a_setYear(&r, y);
    return r;
}

static vector<YxPoint> f_yxPoints;
static vector<YcPoint> f_ycPoints;
static map<string, map<string, msepoch_t> > f_deviceTables;
static map<string, int> f_deviceTypes;

static void pushDeviceTables(const string & dbName, const string & tbName, map<string, map<string, msepoch_t> > &deviceTables)
{
    if (dbName.empty() || tbName.empty())
    {
        return;
    }
    map<string, map<string, msepoch_t> >::iterator it = deviceTables.find(dbName);
    if (it != deviceTables.end())
    {
        map<string, msepoch_t> & tbNames = it->second;
        tbNames[tbName] = 0;
    }
    else
    {
        map<string, msepoch_t> tbNames;
        tbNames[tbName] = 0;
        deviceTables[dbName] = tbNames;
    }
}

/*
typedef enum  {
    IEC60870_DOUBLE_POINT_INTERMEDIATE = 0,
    IEC60870_DOUBLE_POINT_OFF = 1,
    IEC60870_DOUBLE_POINT_ON = 2,
    IEC60870_DOUBLE_POINT_INDETERMINATE = 3
} DoublePointValue;
 */
int toDoublePointValue(int value)
{
    if (value == 0)
    {
        //        return IEC60870_DOUBLE_POINT_OFF;
        return 1;
    } else if (value == 1)
    {
        //        return IEC60870_DOUBLE_POINT_ON;
        return 2;
    } else
    {
        //        return IEC60870_DOUBLE_POINT_INDETERMINATE;
        return 3;
    }
}

static void printAllPointConfig()
{
    fnInfo().out(".printAllPointConfig - YX_POINT_TABLE: [%d] YxPoint", f_yxPoints.size());
    fnInfo().out(".printAllPointConfig - YC_POINT_TABLE: [%d] YcPoint", f_ycPoints.size());
    for(map<string, map<string, msepoch_t> >::const_iterator it = f_deviceTables.begin(); it != f_deviceTables.end(); ++it)
    {
        const string & dbName = it->first;
        const map<string, msepoch_t> & tbNames = it->second;
        string sTableNames = CxString::join(CxContainer::keys(tbNames), ',');
        if (sTableNames.size() < 4000)
        {
            fnInfo().out(".printAllPointConfig - DATABASE_NAME[%s] TABLE_NAME[%s]", dbName.c_str(), sTableNames.c_str());
        }
        else
        {
            fnInfo().out(".printAllPointConfig - DATABASE_NAME[%s] TABLE_NAME TOO BIG", dbName.c_str());
        }
    }
}

void PointManager::start(sqlite3 *sqlite)
{
    map<string, map<string, msepoch_t> > deviceTables;
    map<string, int> deviceTypes;
    SQL::Con db(sqlite);
    {
        vector<YxPoint> vs;
        if (DBManager::sqliteTableColumnExist(sqlite, CS_YX_POINT_TABLE_NAME, "MODEL"))
        {
            auto a = db.bindnquery<int, string, string, string, int, int, int, int, int, string, string, string, string, string, string, string, string, string>(CS_YX_POINT_TABLE_SELECT2);
            for (auto x:a)
            {
                YxPoint m;
                PointManager::resetYxPoint(&m);
                m.ID = get<0>(x);
                m.DATABASE_NAME = get<1>(x);
                m.TABLE_NAME = get<2>(x);
                string::size_type posDeviceId = m.TABLE_NAME.find_last_of('_');
                if (posDeviceId != string::npos)
                {
                    m.DeviceId = m.TABLE_NAME.substr(posDeviceId+1);
                }
                m.DEVICE_ATTR = get<3>(x);
                m.POINT_TYPE = get<4>(x);
                m.CA = get<5>(x);
                m.IOA = get<6>(x);
                // IEC60870_5_TypeID
                m.TYPE_ID = get<7>(x);
                // gateway
                m.EVENT_TYPE = get<8>(x);
                // mqtt
                m.MODEL = get<9>(x);
                m.MODEL_ATTR = get<10>(x);
                m.MODEL_ATTR_TYPE = get<11>(x);
                m.MODEL_EVENT_TYPE = get<12>(x);
                m.MODEL_EVENT_PARAM = get<13>(x);
                m.ModelEventParams = CxString::splitToMap(m.MODEL_EVENT_PARAM, '=', ':');
                m.MODEL_GUID = get<14>(x);
                m.RES1 = get<15>(x);
                m.RES2 = get<16>(x);
                m.RES1 = get<17>(x);
                vs.push_back(m);
                pushDeviceTables(m.DATABASE_NAME, m.TABLE_NAME, deviceTables);
            }
        }
        else
        {
            auto a = db.bindnquery<int, string, string, string, int, int, int, int>(CS_YX_POINT_TABLE_SELECT);
            for (auto x:a)
            {
                YxPoint m;
                resetYxPoint(&m);
                m.ID = get<0>(x);
                m.DATABASE_NAME = get<1>(x);
                m.TABLE_NAME = get<2>(x);
                string::size_type posDeviceId = m.TABLE_NAME.find_last_of('_');
                if (posDeviceId != string::npos)
                {
                    m.DeviceId = m.TABLE_NAME.substr(posDeviceId+1);
                }
                m.DEVICE_ATTR = get<3>(x);
                m.POINT_TYPE = get<4>(x);
                m.CA = get<5>(x);
                m.IOA = get<6>(x);
                // IEC60870_5_TypeID
                m.TYPE_ID = get<7>(x);
                vs.push_back(m);
                pushDeviceTables(m.DATABASE_NAME, m.TABLE_NAME, deviceTables);
            }
        }
        f_yxPoints = vs;
    }
    {
        vector<YcPoint> vs;
        if (DBManager::sqliteTableColumnExist(sqlite, CS_YC_POINT_TABLE_NAME, "MODEL"))
        {
            auto a = db.bindnquery<int, string, string, string, int, int, int, int, double, double, int, string, string, string, string, string, string, string, string, string, string, string, string>(CS_YC_POINT_TABLE_SELECT2);
            for (auto x:a)
            {
                YcPoint m;
                PointManager::resetYcPoint(&m);
                m.ID = get<0>(x);
                m.DATABASE_NAME = get<1>(x);
                m.TABLE_NAME = get<2>(x);
                string::size_type posDeviceId = m.TABLE_NAME.find_last_of('_');
                if (posDeviceId != string::npos)
                {
                    m.DeviceId = m.TABLE_NAME.substr(posDeviceId+1);
                }
                m.DEVICE_ATTR = get<3>(x);
                m.POINT_TYPE = get<4>(x);
                m.CA = get<5>(x);
                m.IOA = get<6>(x);
                m.TYPE_ID = get<7>(x);
                m.NORMALIZED_MAX = get<8>(x);
                m.SCALED_FACTOR = get<9>(x);
                m.PRECISION = get<10>(x);
                m.MODEL = get<11>(x);
                m.MODEL_ATTR = get<12>(x);
                m.MODEL_ATTR_TYPE = get<13>(x);
                m.MODEL_GUID = get<14>(x);
                m.MODEL_DEAD_ZONE = get<15>(x);
                m.MODEL_RATIO = get<16>(x);
                m.MODEL_IS_REPORT = get<17>(x);
                m.MODEL_USER_DEFINE = get<18>(x);
                m.RES1 = get<19>(x);
                m.RES2 = get<20>(x);
                m.MODEL_UNIT = get<21>(x);
                m.Tag = true;
                vs.push_back(m);
                pushDeviceTables(m.DATABASE_NAME, m.TABLE_NAME, deviceTables);
            }
        }
        else
        {
            auto a = db.bindnquery<int, string, string, string, int, int, int, double, double, int>(CS_YC_POINT_TABLE_SELECT);
            for (auto x:a)
            {
                YcPoint m;
                resetYcPoint(&m);
                m.ID = get<0>(x);
                m.DATABASE_NAME = get<1>(x);
                m.TABLE_NAME = get<2>(x);
                string::size_type posDeviceId = m.TABLE_NAME.find_last_of('_');
                if (posDeviceId != string::npos)
                {
                    m.DeviceId = m.TABLE_NAME.substr(posDeviceId+1);
                }
                m.DEVICE_ATTR = get<3>(x);
                m.CA = get<4>(x);
                m.IOA = get<5>(x);
                // IEC60870_5_TypeID
                m.TYPE_ID = get<6>(x);
                m.NORMALIZED_MAX = get<7>(x);
                m.SCALED_FACTOR = get<8>(x);
                m.PRECISION = get<9>(x);
                m.Tag = true;
                vs.push_back(m);
                pushDeviceTables(m.DATABASE_NAME, m.TABLE_NAME, deviceTables);
            }
        }
        f_ycPoints = vs;
    }
    f_deviceTables = deviceTables;
    printAllPointConfig();
//    loadAllYXTavleDate();
}

void PointManager::start(const std::vector<YcPoint> &ycs, const std::vector<YxPoint> &yxs)
{
    map<string, map<string, msepoch_t> > deviceTables;
    for (int i = 0; i < ycs.size(); ++i)
    {
        const YcPoint &yc = ycs[i];
        pushDeviceTables(yc.DATABASE_NAME, yc.TABLE_NAME, deviceTables);
    }
    for (int i = 0; i < yxs.size(); ++i)
    {
        const YxPoint &yx = yxs[i];
        pushDeviceTables(yx.DATABASE_NAME, yx.TABLE_NAME, deviceTables);
    }
    //
    f_yxPoints = yxs;
    f_ycPoints = ycs;
    f_deviceTables = deviceTables;
    printAllPointConfig();
//    loadAllYXTavleDate();
}

void PointManager::stop()
{

}

int PointManager::loadAllYXTavleDate()
{
    sqlite3 *sqlite = DBManager::open_sqlite(DBManager::getDbFilePath(DB_IEC104_YX_TABLE));

    for (int i = 0; i < f_yxPoints.size(); ++i)
    {
        YxPoint &yx = f_yxPoints.at(i);
        std::string sSql = CxString::format(CS_YX_DATE_TABLE_SELECT, yx.IOA);

        SQL::Con db(sqlite);
        auto x = db.bindnquery<int, int>(sSql);
        if (!x.empty())
        {
            YxDate yd;
            for (auto a:x)
            {
                yd.IOA = get<0>(a);
                yd.VALUE = get<1>(a);
            }
            if (yd.VALUE == divide || yd.VALUE == shut)
            {
                setYxValue(&yx, yd.VALUE, 0);
            }
            else if (yd.VALUE == invalid)
            {
                yx.QUALITY = QUALITY_INVALID;
            }
        }
        else
        {
            string sql;
            if (yx.TABLE_NAME == "") {
                sql = CxString::format(CS_YX_DATE_TABLE_INSERT, yx.IOA, -1);
                yx.QUALITY = QUALITY_INVALID;
            }
            else
                sql = CxString::format(CS_YX_DATE_TABLE_INSERT, yx.IOA, yx.VALUE);
            DBManager::sqlite3Exec(sqlite, sql);
        }
    }

    DBManager::close_sqlite(sqlite);
    return 0;
}

int PointManager::insertYxTavleDate(int IOA, int VALUE)
{
    sqlite3 *sqlite = DBManager::open_sqlite(DBManager::getDbFilePath(DB_IEC104_YX_TABLE));

    string sql = CxString::format(CS_YX_DATE_TABLE_INSERT, IOA, VALUE);
    DBManager::sqlite3Exec(sqlite, sql);

    DBManager::close_sqlite(sqlite);
    return 0;
}

int PointManager::refreshAllPointValue()
{
    map<string, sqlite3*> dbs;
    for(map<string, map<string, msepoch_t> >::const_iterator it = f_deviceTables.begin(); it != f_deviceTables.end(); ++it)
    {
        const string &dbName = it->first;
        const map<string, msepoch_t> &tbTimes = it->second;
        sqlite3 *db = NULL;
        map<string, sqlite3*>::const_iterator itDb = dbs.find(dbName);
        if (itDb != dbs.end())
        {
            db = itDb->second;
        }
        else
        {
            string sFilePath = DBManager::getDbFilePath(dbName);
            if (CxFileSystem::isExist(sFilePath))
            {
                db = DBManager::openSqlite(sFilePath);
                if (db == NULL)
                {
                    fnDebug().out("refreshAllPointValue DATABASE_NAME[%s] open error!", dbName.c_str());
                    continue;
                }
                dbs[dbName] = db;
            }
            else
            {
//                if(dbName == "STANDBY")
//                {
//                    refreshStandbyPointValue(dbName);
//                }
//                else {
                fnDebug().out("refreshAllPointValue DATABASE_NAME[%s] do not exist!", dbName.c_str());
                continue;
//                }
            }
        }
        for (map<string, msepoch_t>::const_iterator it2 = tbTimes.begin(); it2 != tbTimes.end(); ++it2)
        {
            const string &tbName = it2->first;
            // int err = DBManager::sqliteTableExist(db, (char *)tbName.c_str());
            //            if (0 == err)
            //            {
            //                lib60870_debug_print(LIB60870_LOG_LEVEL_ERROR,
            //                                     "refreshAllPointValue DATABASE_NAME[%s] - TABLE_NAME[%s] do not exist!", dbName.c_str(), tbName.c_str());
            //                continue;
            //            }
            if(CxString::beginWith(tbName, "YX_TABLE_tajiao-gz"))
            {
                continue;
            }

            try
            {
                refreshTablePointValue(db, dbName, tbName);
            }
            catch (std::exception& e)
            {
                fnDebug().out("refreshAllPointValue DATABASE_NAME[%s] - TABLE_NAME[%s] error[%s]!", dbName.c_str(), tbName.c_str(), e.what());
            }
            catch (...)
            {
                fnDebug().out("refreshAllPointValue DATABASE_NAME[%s] - TABLE_NAME[%s] do not exist!", dbName.c_str(), tbName.c_str());
            }
        }
    }
    for(map<string, sqlite3*>::const_iterator it = dbs.begin(); it != dbs.end(); ++it)
    {
        DBManager::closeSqlite(it->second);
    }
    refreshAllPointOldValue();
    return 0;
}

void PointManager::refreshYxPonitValue(const std::string &deviceId, AREABUG_YX &eventData)
{
    for (int i = 0; i < f_yxPoints.size(); ++i)
    {
        YxPoint &yx = f_yxPoints.at(i);
        const std::string &sDeviceId = yx.DeviceId;
        if (sDeviceId == deviceId || (sDeviceId == eventData.branchDevice))
        {
            switch (yx.POINT_TYPE)
            {
                case PINT_LOW_VOLTAGE:
                    if ((eventData.areaBugStaA == YX_PHASE_UNDER_VOL || eventData.areaBugStaB == YX_PHASE_UNDER_VOL || eventData.areaBugStaC == YX_PHASE_UNDER_VOL) && (yx.VALUE == 0 || yx.VALUE == -1))
                        setYxValue(&yx, 1, eventData.recordTs, true);
                    else if ((eventData.areaBugStaA == YX_PHASE_UNDER_VOL_RESTORE || eventData.areaBugStaB == YX_PHASE_UNDER_VOL_RESTORE || eventData.areaBugStaC == YX_PHASE_UNDER_VOL_RESTORE) && (yx.VALUE == 1 || yx.VALUE == -1))
                        setYxValue(&yx, 0, eventData.recordTs, true);
                    break;
                case POINT_A_PHASE_VOLTAGE:
                    if ((YX_PHASE_UNDER_VOL == eventData.areaBugStaA) && (yx.VALUE == 0 || yx.VALUE == -1))
                        setYxValue(&yx, 1, eventData.recordTs, true);
                    else if((YX_PHASE_UNDER_VOL_RESTORE == eventData.areaBugStaA) && (yx.VALUE == 1 || yx.VALUE == -1))
                        setYxValue(&yx, 0, eventData.recordTs, true);
                    else if (eventData.areaBugStaA == YX_PHASE_OK && yx.QUALITY == QUALITY_INVALID)
                        setYxValue(&yx, yx.VALUE, eventData.recordTs);
                    break;
                case POINT_B_PHASE_VOLTAGE:
                    if ((YX_PHASE_UNDER_VOL == eventData.areaBugStaB) && (yx.VALUE == 0 || yx.VALUE == -1))
                        setYxValue(&yx, 1, eventData.recordTs, true);
                    else if((YX_PHASE_UNDER_VOL_RESTORE == eventData.areaBugStaB) && (yx.VALUE == 1 || yx.VALUE == -1))
                        setYxValue(&yx, 0, eventData.recordTs, true);
                    else if (eventData.areaBugStaA == YX_PHASE_OK && yx.QUALITY == QUALITY_INVALID)
                        setYxValue(&yx, yx.VALUE, eventData.recordTs);
                    break;
                case POINT_C_PHASE_VOLTAGE:
                    if ((YX_PHASE_UNDER_VOL == eventData.areaBugStaC) && (yx.VALUE == 0 || yx.VALUE == -1))
                        setYxValue(&yx, 1, eventData.recordTs, true);
                    else if((YX_PHASE_UNDER_VOL_RESTORE == eventData.areaBugStaC) && (yx.VALUE == 1 || yx.VALUE == -1))
                        setYxValue(&yx, 0, eventData.recordTs, true);
                    else if (eventData.areaBugStaA == YX_PHASE_OK && yx.QUALITY == QUALITY_INVALID)
                        setYxValue(&yx, yx.VALUE, eventData.recordTs);
                    break;
                case POINT_ALL_PRESSURE_LOSS:
                    if ((eventData.areaBugStaA == YX_PHASE_DROP_VOL || eventData.areaBugStaB == YX_PHASE_DROP_VOL || eventData.areaBugStaC == YX_PHASE_DROP_VOL) && (yx.VALUE == 0 || yx.VALUE == -1))
                        setYxValue(&yx, 1, eventData.recordTs, true);
                    else if ((eventData.areaBugStaA == YX_PHASE_DROP_VOL_RESTORE || eventData.areaBugStaB == YX_PHASE_DROP_VOL_RESTORE || eventData.areaBugStaC == YX_PHASE_DROP_VOL_RESTORE) && (yx.VALUE == 1 || yx.VALUE == -1))
                        setYxValue(&yx, 0, eventData.recordTs, true);
                    break;
                case POINT_A_PHASE_OVERVOLTAGE:
                    if ((YX_PHASE_OVER_VOL == eventData.areaBugStaA) && (yx.VALUE == 0 || yx.VALUE == -1))
                        setYxValue(&yx, 1, eventData.recordTs, true);
                    else if((YX_PHASE_OVER_VOL_RESTORE == eventData.areaBugStaA) && (yx.VALUE == 1 || yx.VALUE == -1))
                        setYxValue(&yx, 0, eventData.recordTs, true);
                    else if (eventData.areaBugStaA == YX_PHASE_OK && yx.QUALITY == QUALITY_INVALID)
                        setYxValue(&yx, yx.VALUE, eventData.recordTs);
                    break;
                case POINT_B_PHASE_OVERVOLTAGE:
                    if ((YX_PHASE_OVER_VOL == eventData.areaBugStaB) && (yx.VALUE == 0 || yx.VALUE == -1))
                        setYxValue(&yx, 1, eventData.recordTs, true);
                    else if((YX_PHASE_OVER_VOL_RESTORE == eventData.areaBugStaB) && (yx.VALUE == 1 || yx.VALUE == -1))
                        setYxValue(&yx, 0, eventData.recordTs, true);
                    else if (eventData.areaBugStaA == YX_PHASE_OK && yx.QUALITY == QUALITY_INVALID)
                        setYxValue(&yx, yx.VALUE, eventData.recordTs);
                    break;
                case POINT_C_PHASE_OVERVOLTAGE:
                    if ((YX_PHASE_OVER_VOL == eventData.areaBugStaC) && (yx.VALUE == 0 || yx.VALUE == -1))
                        setYxValue(&yx, 1, eventData.recordTs, true);
                    else if((YX_PHASE_OVER_VOL_RESTORE == eventData.areaBugStaC) && (yx.VALUE == 1 || yx.VALUE == -1))
                        setYxValue(&yx, 0, eventData.recordTs, true);
                    else if (eventData.areaBugStaA == YX_PHASE_OK && yx.QUALITY == QUALITY_INVALID)
                        setYxValue(&yx, yx.VALUE, eventData.recordTs);
                    break;
                case POINT_A_PHASE_FLOW:
                    if ((YX_PHASE_OVER_CURRENT == eventData.areaBugStaA) && (yx.VALUE == 0 || yx.VALUE == -1))
                        setYxValue(&yx, 1, eventData.recordTs, true);
                    else if((YX_PHASE_OVER_CURRENT_RESTORE == eventData.areaBugStaA) && (yx.VALUE == 1 || yx.VALUE == -1))
                        setYxValue(&yx, 0, eventData.recordTs, true);
                    else if (eventData.areaBugStaA == YX_PHASE_OK && yx.QUALITY == QUALITY_INVALID)
                        setYxValue(&yx, yx.VALUE, eventData.recordTs);
                    break;
                case POINT_B_PHASE_FLOW:
                    if ((YX_PHASE_OVER_CURRENT == eventData.areaBugStaB) && (yx.VALUE == 0 || yx.VALUE == -1))
                        setYxValue(&yx, 1, eventData.recordTs, true);
                    else if((YX_PHASE_OVER_CURRENT_RESTORE == eventData.areaBugStaB) && (yx.VALUE == 1 || yx.VALUE == -1))
                        setYxValue(&yx, 0, eventData.recordTs, true);
                    else if (eventData.areaBugStaA == YX_PHASE_OK && yx.QUALITY == QUALITY_INVALID)
                        setYxValue(&yx, yx.VALUE, eventData.recordTs);
                    break;
                case POINT_C_PHASE_FLOW:
                    if ((YX_PHASE_OVER_CURRENT == eventData.areaBugStaC) && (yx.VALUE == 0 || yx.VALUE == -1))
                        setYxValue(&yx, 1, eventData.recordTs, true);
                    else if((YX_PHASE_OVER_CURRENT_RESTORE == eventData.areaBugStaC) && (yx.VALUE == 1 || yx.VALUE == -1))
                        setYxValue(&yx, 0, eventData.recordTs, true);
                    else if (eventData.areaBugStaA == YX_PHASE_OK && yx.QUALITY == QUALITY_INVALID)
                        setYxValue(&yx, yx.VALUE, eventData.recordTs);
                    break;
                case POINT_POWER_OUTAGES:
                    if ((YX_POWER_DOWN == eventData.areaBugStaA || YX_POWER_DOWN == eventData.areaBugStaB || YX_POWER_DOWN == eventData.areaBugStaC) && (yx.VALUE == 0 || yx.VALUE == -1))
                        setYxValue(&yx, 1, eventData.recordTs, true);
                    else if ((YX_POWER_UP == eventData.areaBugStaA || YX_POWER_UP == eventData.areaBugStaB || YX_POWER_UP == eventData.areaBugStaC) && (yx.VALUE == 1 || yx.VALUE == -1))
                        setYxValue(&yx, 0, eventData.recordTs, true);
                    break;
                case POINT_A_PRESSURE_LOSS:
                    if ((YX_PHASE_DROP_VOL == eventData.areaBugStaA) && (yx.VALUE == 0 || yx.VALUE == -1))
                        setYxValue(&yx, 1, eventData.recordTs, true);
                    else if((YX_PHASE_DROP_VOL_RESTORE == eventData.areaBugStaA) && (yx.VALUE == 1 || yx.VALUE == -1))
                        setYxValue(&yx, 0, eventData.recordTs, true);
                    else if (eventData.areaBugStaA == YX_PHASE_OK && yx.QUALITY == QUALITY_INVALID)
                        setYxValue(&yx, yx.VALUE, eventData.recordTs);
                    break;
                case POINT_B_PRESSURE_LOSS:
                    if ((YX_PHASE_DROP_VOL == eventData.areaBugStaB) && (yx.VALUE == 0 || yx.VALUE == -1))
                        setYxValue(&yx, 1, eventData.recordTs, true);
                    else if((YX_PHASE_DROP_VOL_RESTORE == eventData.areaBugStaB) && (yx.VALUE == 1 || yx.VALUE == -1))
                        setYxValue(&yx, 0, eventData.recordTs, true);
                    else if (eventData.areaBugStaA == YX_PHASE_OK && yx.QUALITY == QUALITY_INVALID)
                        setYxValue(&yx, yx.VALUE, eventData.recordTs);
                    break;
                case POINT_C_PRESSURE_LOSS:
                    if ((YX_PHASE_DROP_VOL == eventData.areaBugStaC) && (yx.VALUE == 0 || yx.VALUE == -1))
                        setYxValue(&yx, 1, eventData.recordTs, true);
                    else if((YX_PHASE_DROP_VOL_RESTORE == eventData.areaBugStaC) && (yx.VALUE == 1 || yx.VALUE == -1))
                        setYxValue(&yx, 0, eventData.recordTs, true);
                    else if (eventData.areaBugStaA == YX_PHASE_OK && yx.QUALITY == QUALITY_INVALID)
                        setYxValue(&yx, yx.VALUE, eventData.recordTs);
                    break;
                case POINT_OVERLOADING:
                    if ((YX_OVERLOAD == eventData.areaBugStaA || YX_OVERLOAD == eventData.areaBugStaB || YX_OVERLOAD == eventData.areaBugStaC) && (yx.VALUE == 0 || yx.VALUE == -1))
                        setYxValue(&yx, 1, eventData.recordTs, true);
                    else if ((YX_OVERLOAD_RESTORE == eventData.areaBugStaA || YX_OVERLOAD_RESTORE == eventData.areaBugStaB || YX_OVERLOAD_RESTORE == eventData.areaBugStaC) && (yx.VALUE == 1 || yx.VALUE == -1))
                        setYxValue(&yx, 0, eventData.recordTs, true);
                    break;
                case POINT_A_PHASE_OVERLOADED:
                    if ((YX_OVERLOAD == eventData.areaBugStaA) && (yx.VALUE == 0 || yx.VALUE == -1))
                        setYxValue(&yx, 1, eventData.recordTs, true);
                    else if ((YX_OVERLOAD_RESTORE == eventData.areaBugStaA) && (yx.VALUE == 1 || yx.VALUE == -1))
                        setYxValue(&yx, 0, eventData.recordTs, true);
                    else if (eventData.areaBugStaA == YX_PHASE_OK && yx.QUALITY == QUALITY_INVALID)
                        setYxValue(&yx, yx.VALUE, eventData.recordTs);
                    break;
                case POINT_B_PHASE_OVERLOADED:
                    if ((YX_OVERLOAD == eventData.areaBugStaB) && (yx.VALUE == 0 || yx.VALUE == -1))
                        setYxValue(&yx, 1, eventData.recordTs, true);
                    else if ((YX_OVERLOAD_RESTORE == eventData.areaBugStaB) && (yx.VALUE == 1 || yx.VALUE == -1))
                        setYxValue(&yx, 0, eventData.recordTs, true);
                    else if (eventData.areaBugStaA == YX_PHASE_OK && yx.QUALITY == QUALITY_INVALID)
                        setYxValue(&yx, yx.VALUE, eventData.recordTs);
                    break;
                case POINT_C_PHASE_OVERLOADED:
                    if ((YX_OVERLOAD == eventData.areaBugStaC) && (yx.VALUE == 0 || yx.VALUE == -1))
                        setYxValue(&yx, 1, eventData.recordTs, true);
                    else if ((YX_OVERLOAD_RESTORE == eventData.areaBugStaC) && (yx.VALUE == 1 || yx.VALUE == -1))
                        setYxValue(&yx, 0, eventData.recordTs, true);
                    else if (eventData.areaBugStaA == YX_PHASE_OK && yx.QUALITY == QUALITY_INVALID)
                        setYxValue(&yx, yx.VALUE, eventData.recordTs);
                    break;
                case POINT_CURRENT_IMBALANCE:
                    if ((YX_SWITCH_CURRENT_UNBLANCE == eventData.areaBugStaA || YX_SWITCH_CURRENT_UNBLANCE == eventData.areaBugStaB || YX_SWITCH_CURRENT_UNBLANCE == eventData.areaBugStaC) && (yx.VALUE == 0 || yx.VALUE == -1))
                        setYxValue(&yx, 1, eventData.recordTs, true);
                    else if ((YX_SWITCH_CURRENT_UNBLANCE_RESTORE == eventData.areaBugStaA || YX_SWITCH_CURRENT_UNBLANCE_RESTORE == eventData.areaBugStaB || YX_SWITCH_CURRENT_UNBLANCE_RESTORE == eventData.areaBugStaC) && (yx.VALUE == 1 || yx.VALUE == -1))
                        setYxValue(&yx, 0, eventData.recordTs, true);
                    break;
                case POINT_SWITCH_TEMPERATURE:
                    if ((YX_SWITCH_HOOKDOWN_TEMPOVER == eventData.areaBugStaA || YX_SWITCH_HOOKDOWN_TEMPOVER == eventData.areaBugStaB || YX_SWITCH_HOOKDOWN_TEMPOVER == eventData.areaBugStaC) && (yx.VALUE == 0 || yx.VALUE == -1))
                        setYxValue(&yx, 1, eventData.recordTs, true);
                    else if ((YX_SWITCH_HOOKDOWN_TEMPOVER_RESTORE == eventData.areaBugStaA || YX_SWITCH_HOOKDOWN_TEMPOVER_RESTORE == eventData.areaBugStaB || YX_SWITCH_HOOKDOWN_TEMPOVER_RESTORE == eventData.areaBugStaC) && (yx.VALUE == 1 || yx.VALUE == -1))
                        setYxValue(&yx, 0, eventData.recordTs, true);
                    break;
                case POINT_VOLTAGE_IMBALANCE:
                    if ((YX_SWITCH_VOLTAGE_UNBLANCE == eventData.areaBugStaA || YX_SWITCH_VOLTAGE_UNBLANCE == eventData.areaBugStaB || YX_SWITCH_VOLTAGE_UNBLANCE == eventData.areaBugStaC) && (yx.VALUE == 0 || yx.VALUE == -1))
                        setYxValue(&yx, 1, eventData.recordTs, true);
                    else if ((YX_SWITCH_VOLTAGE_UNBLANCE_RESTORE == eventData.areaBugStaA || YX_SWITCH_VOLTAGE_UNBLANCE_RESTORE == eventData.areaBugStaB || YX_SWITCH_VOLTAGE_UNBLANCE_RESTORE == eventData.areaBugStaC) && (yx.VALUE == 1 || yx.VALUE == -1))
                        setYxValue(&yx, 0, eventData.recordTs, true);
                    break;
                case POINT_CURRENT_PHASE_DROP:
                    if ((YX_SWITCH_CURRENT_PHASE_DROP == eventData.areaBugStaA || YX_SWITCH_CURRENT_PHASE_DROP == eventData.areaBugStaB || YX_SWITCH_CURRENT_PHASE_DROP == eventData.areaBugStaC) && (yx.VALUE == 0 || yx.VALUE == -1))
                        setYxValue(&yx, 1, eventData.recordTs, true);
                    else if ((YX_SWITCH_CURRENT_PHASE_DROP_RESTORE == eventData.areaBugStaA || YX_SWITCH_CURRENT_PHASE_DROP_RESTORE == eventData.areaBugStaB || YX_SWITCH_CURRENT_PHASE_DROP_RESTORE == eventData.areaBugStaC) && (yx.VALUE == 1 || yx.VALUE == -1))
                        setYxValue(&yx, 0, eventData.recordTs, true);
                    break;
                case POINT_ZERO_SEQUENCE:
                    if ((YX_SWITCH_ZERO_PROTECT == eventData.areaBugStaA || YX_SWITCH_ZERO_PROTECT == eventData.areaBugStaB || YX_SWITCH_ZERO_PROTECT == eventData.areaBugStaC) && (yx.VALUE == 0 || yx.VALUE == -1))
                        setYxValue(&yx, 1, eventData.recordTs, true);
                    else if ((YX_SWITCH_ZERO_PROTECT_RESTORE == eventData.areaBugStaA || YX_SWITCH_ZERO_PROTECT_RESTORE == eventData.areaBugStaB || YX_SWITCH_ZERO_PROTECT_RESTORE == eventData.areaBugStaC) && (yx.VALUE == 1 || yx.VALUE == -1))
                        setYxValue(&yx, 0, eventData.recordTs, true);
                    break;
                case POINT_PHASE_VOLTAGE:
                    if ((YX_DAILY_UNDER_VOL == eventData.areaBugStaA || YX_DAILY_UNDER_VOL == eventData.areaBugStaB || YX_DAILY_UNDER_VOL == eventData.areaBugStaC) && (yx.VALUE == 0 || yx.VALUE == -1))
                        setYxValue(&yx, 1, eventData.recordTs, true);
                    else if ((YX_DAILY_UNDER_VOL_RESTORE == eventData.areaBugStaA || YX_DAILY_UNDER_VOL_RESTORE == eventData.areaBugStaB || YX_DAILY_UNDER_VOL_RESTORE == eventData.areaBugStaC) && (yx.VALUE == 1 || yx.VALUE == -1))
                        setYxValue(&yx, 0, eventData.recordTs, true);
                    break;
                case POINT_MONTHLY_UNDER_VOLTAGE:
                    if ((YX_MONTHLY_UNDER_VOLTAGE == eventData.areaBugStaA || YX_MONTHLY_UNDER_VOLTAGE == eventData.areaBugStaB || YX_MONTHLY_UNDER_VOLTAGE == eventData.areaBugStaC) && (yx.VALUE == 0 || yx.VALUE == -1))
                        setYxValue(&yx, 1, eventData.recordTs, true);
                    else if ((YX_MONTHLY_UNDER_VOLTAGE_RESTORE == eventData.areaBugStaA || YX_MONTHLY_UNDER_VOLTAGE_RESTORE == eventData.areaBugStaB || YX_MONTHLY_UNDER_VOLTAGE_RESTORE == eventData.areaBugStaC) && (yx.VALUE == 1 || yx.VALUE == -1))
                        setYxValue(&yx, 0, eventData.recordTs, true);
                    break;
                case POINT_DAILY_OVRELOAD:
                    if ((YX_DAILY_OVRELOAD == eventData.areaBugStaA || YX_DAILY_OVRELOAD == eventData.areaBugStaB || YX_DAILY_OVRELOAD == eventData.areaBugStaC) && (yx.VALUE == 0 || yx.VALUE == -1))
                        setYxValue(&yx, 1, eventData.recordTs, true);
                    else if ((YX_DAILY_OVRELOAD_RESTORE == eventData.areaBugStaA || YX_DAILY_OVRELOAD_RESTORE == eventData.areaBugStaB || YX_DAILY_OVRELOAD_RESTORE == eventData.areaBugStaC) && (yx.VALUE == 1 || yx.VALUE == -1))
                        setYxValue(&yx, 0, eventData.recordTs, true);
                    break;
                case POINT_MONTHLY_OVRELOAD:
                    if ((YX_MONTHLY_OVRELOAD == eventData.areaBugStaA || YX_MONTHLY_OVRELOAD == eventData.areaBugStaB || YX_MONTHLY_OVRELOAD == eventData.areaBugStaC) && (yx.VALUE == 0 || yx.VALUE == -1))
                        setYxValue(&yx, 1, eventData.recordTs, true);
                    else if ((YX_MONTHLY_OVRELOAD_RESTORE == eventData.areaBugStaA || YX_MONTHLY_OVRELOAD_RESTORE == eventData.areaBugStaB || YX_MONTHLY_OVRELOAD_RESTORE == eventData.areaBugStaC) && (yx.VALUE == 1 || yx.VALUE == -1))
                        setYxValue(&yx, 0, eventData.recordTs, true);
                    break;
                case POINT_MONTHLY_HEAVY_LOAD:
                    if ((YX_MONTHLY_HEAVY_LOAD == eventData.areaBugStaA || YX_MONTHLY_HEAVY_LOAD == eventData.areaBugStaB || YX_MONTHLY_HEAVY_LOAD == eventData.areaBugStaC) && (yx.VALUE == 0 || yx.VALUE == -1))
                        setYxValue(&yx, 1, eventData.recordTs, true);
                    else if ((YX_MONTHLY_HEAVY_LOAD_RESTORE == eventData.areaBugStaA || YX_MONTHLY_HEAVY_LOAD_RESTORE == eventData.areaBugStaB || YX_MONTHLY_HEAVY_LOAD_RESTORE == eventData.areaBugStaC) && (yx.VALUE == 1 || yx.VALUE == -1))
                        setYxValue(&yx, 0, eventData.recordTs, true);
                    break;
                case PINT_LOWER_VOL:
                    if ((YX_LOWER_VOL == eventData.areaBugStaA || YX_LOWER_VOL == eventData.areaBugStaB || YX_LOWER_VOL == eventData.areaBugStaC) && (yx.VALUE == 0 || yx.VALUE == -1))
                        setYxValue(&yx, 1, eventData.recordTs, true);
                    else if ((YX_LOWER_VOL_RESTORE == eventData.areaBugStaA || YX_LOWER_VOL_RESTORE == eventData.areaBugStaB || YX_LOWER_VOL_RESTORE == eventData.areaBugStaC) && (yx.VALUE == 1 || yx.VALUE == -1))
                        setYxValue(&yx, 0, eventData.recordTs, true);
                    break;
                default:
                    break;
            }
        }
    }
}

void time_print(time_t t)
{
    struct tm *p;
    p = gmtime(&t);
    char s[100];
    strftime(s, sizeof(s), "%Y-%m-%d %H:%M:%S", p);
    printf("%d : %s\n\n", (int)t, s);
}

static map<string, msepoch_t> f_refreshTimes;

void PointManager::refreshTablePointValue(sqlite3 *db, const string &dbName, const string &tbName)
{
    // set refresh time
//    setRefreshTime(dbName, tbName);
    string sKey = dbName+tbName;
    map<string, msepoch_t>::iterator it = f_refreshTimes.find(sKey);
    msepoch_t dtNow = CxTime::currentMsepoch();
    if (it != f_refreshTimes.end()) {
        msepoch_t dt = it->second;
        if (dtNow - dt < 3000) {
            return;
        }
    }
    f_refreshTimes[sKey] = dtNow;
    //
    if (CxString::beginWith(tbName, C_GUZHI_TABLE_PREFIX))
    {
        GUZHITable guzhiTable;
        if (DeviceManager::getLastGUZHITable(db, tbName, &guzhiTable))
        {
            refreshGUZHITablePointValue(dbName, tbName, guzhiTable);
//            time_print(guzhiTable.RECORD_TS);
        }
    }
    else if (CxString::beginWith(tbName, C_METER_TABLE_PREFIX))
    {
        METERTable meterTable;
        if (DeviceManager::getLastMETERTable(db, tbName, &meterTable))
        {
            refreshMETERTablePointValue(dbName, tbName, meterTable);
//            time_print(meterTable.POWER_METER_TS);
        }
    }
    else if (CxString::beginWith(tbName, C_SWITCH_TABLE_PREFIX))
    {
        SWITCHTable switchTable;
        if (DeviceManager::getLastSWITCHTable(db, tbName, &switchTable))
        {
            refreshSWITCHTablePointValue(dbName, tbName, switchTable);
//            time_print(switchTable.SWITCH_TS);
        }
    }
    else if (CxString::beginWith(tbName, C_YX_TABLE_PREFIX))
    {
        bool isCharge = CxString::contain(tbName, C_CHARGE_TABLE_PREFIX);
        if (isCharge)
        {
            CHARGEYXTable chargeyxtable;
            if (DeviceManager::getLastCHARGEYXTable(db, tbName, &chargeyxtable))
            {
                refreshCHARGEYXTablePointValue(dbName, tbName, chargeyxtable);
                //            time_print(chargeyxtable.RECORD_TS);
            }
        }
        else
        {
            YXTABLE yxtable;
            if (DeviceManager::getLastYXTABLE(db, tbName, &yxtable))
            {
                refreshYXTablePointValue(dbName, tbName, yxtable);
                //            time_print(yxtable.RECORD_TS);
            }
        }
    }
    else if (CxString::beginWith(tbName, C_AREA_DAILY_TABLE_PREFIX))
    {
        AREADAILYTable areadailytable;
        if (DeviceManager::getLastAREADAILYTable(db, C_AREA_DAILY_TABLE_PREFIX, &areadailytable))
        {
            refreshAREADAILYTablePointValue(dbName, tbName, areadailytable);
//            time_print(areadailytable.RECORD_TS);
        }
    }
    else if (CxString::beginWith(tbName, C_AREA_MONTHLY_TABLE_PREFIX))
    {
        AREAMONTHLYTable areamonthlytable;
        if (DeviceManager::getLastAREAMONTHLYTable(db, C_AREA_MONTHLY_TABLE_PREFIX, &areamonthlytable))
        {
            refreshAREAMONTHLYTablePointValue(dbName, tbName, areamonthlytable);
//            time_print(areamonthlytable.RECORD_TS);
        }
    }
    else if (CxString::beginWith(tbName, C_AREA_ANNUAL_TABLE_PREFIX))
    {
        AREAMETERYEARSTable areameteryearstable;
        if (DeviceManager::getLastAREAANNUALTable(db, C_AREA_ANNUAL_TABLE_PREFIX, &areameteryearstable))
        {
            refreshAREAANNUALTablePointValue(dbName, tbName, areameteryearstable);
//            time_print(areameteryearstable.RECORD_TS);
        }
    }
    else if (CxString::beginWith(tbName, C_POWERLOSS_TABLE_PREFIX))
    {
        POWERLOSSTable powerlosstable;
        if (DeviceManager::getLastPOWERLOSSTable(db, tbName, &powerlosstable))
        {
            refreshPOWERLOSSTablePointValue(dbName, tbName, powerlosstable);
//            refreshYxPonitValue(dbName, tbName, powerlosstable);
//            time_print(powerlosstable.RECORD_TS);
        }
    }
    else if (CxString::beginWith(tbName, C_CHARGE_TABLE_PREFIX))
    {
        CHARGETable chargetable;
        if (DeviceManager::getLastCHARGETable(db, tbName, &chargetable))
        {
            refreshCHARGETablePointValue(dbName, tbName, chargetable);
//            time_print(chargetable.RECORD_TS);
        }
    }
    else if (CxString::beginWith(tbName, C_BRANCH_TABLE_PREFIX))
    {
        BRANCHTable branchtable;
        if (DeviceManager::getLastBRANCHTable(db, tbName, &branchtable) && DeviceManager::getLastBRANCHMONTH(db, tbName, &branchtable))
        {
            refreshBRANCHTablePointValue(dbName, tbName, branchtable);
//            time_print(branchtable.RECORD_TS);
        }
    }
    else if (CxString::beginWith(tbName, C_PHASE_BLANCE_TABLE_PREFIX))
    {
        PHASEBLANCETable phaseblancetable;
        if (DeviceManager::getLastPHASEBLANCETable(db, tbName, &phaseblancetable))
        {
            refreshPHASEBLANCETablePointValue(dbName, tbName, phaseblancetable);
//            time_print(phaseblancetable.RECORD_TS);
        }
    }
    else if (CxString::beginWith(tbName, C_CHARGE_YX_TABLE_PREFIX))
    {
        CHARGEYXTable chargeyxtable;
        if (DeviceManager::getLastCHARGEYXTable(db, tbName, &chargeyxtable))
        {
            refreshCHARGEYXTablePointValue(dbName, tbName, chargeyxtable);
//            time_print(chargeyxtable.RECORD_TS);
        }
    }
    else if (CxString::beginWith(tbName, C_TH_TABLE_PREFIX))
    {
        THTable thTable;
        if (DeviceManager::getLastTHTABLE(db, tbName, &thTable))
        {
            refreshTHTablePointValue(dbName, tbName, thTable);
//            time_print(thTable.RECORD_TS);
        }
    }
    else if(CxString::beginWith(tbName, C_SENSORSMOKE_TABLE_PREFIX))
    {
        SENSORSMOKETable sensorsmokeTable;
        if (DeviceManager::getLastSENSORSMOKETABLE(db, tbName, &sensorsmokeTable))
        {
            refreshSENSORSMOKETablePointValue(dbName, tbName, sensorsmokeTable);
//            time_print(sensorsmokeTable.RECORD_TS);
        }
    }
    else if (CxString::beginWith(tbName, C_IR_TABLE_PREFIX))
    {
        IRTable irTable;
        if (DeviceManager::getLastIRTABLE(db, tbName, &irTable))
        {
            refreshIRTablePointValue(dbName, tbName, irTable);
        }
    }
    else if(CxString::beginWith(tbName, C_WATER_DEEP_TABLE))
    {
        WATERDEEPTable waterdeepTable;
        if (DeviceManager::getLastWATERDEEPTABLE(db, tbName, &waterdeepTable))
        {
            refreshWATERDEEPTablePointValue(dbName, tbName, waterdeepTable);
        }
    }
    else if(CxString::beginWith(tbName, C_SENSORDOOR_TABLE))
    {
        SENSORDOORTable sensordoorTable;
        if (DeviceManager::getLastSENSORDOORTABLE(db, tbName, &sensordoorTable))
        {
            refreshSENSORDOORTablePointValue(dbName, tbName, sensordoorTable);
        }
    }
    else if (CxString::beginWith(tbName, C_RELAYPLC_TABLE))
    {
        SENSORDOORTable sensordoorTable;
        if (DeviceManager::getLastRELAYPLCTABLE(db, tbName, &sensordoorTable))
        {
            refreshRELAYPLCTablePointValue(dbName, tbName, sensordoorTable);
        }
    }
    else if (CxString::beginWith(tbName, C_BOPO_TABLE))
    {
        SENSORDOORTable sensordoorTable;
        if (DeviceManager::getLastBOPOTABLE(db, tbName, &sensordoorTable))
        {
            refreshBOPOTablePointValue(dbName, tbName, sensordoorTable);
        }
    }
    else if (CxString::beginWith(tbName, C_PVINVERTER_TABLE))
    {
        PVINVERTERTable pvInverterTable;
        if (DeviceManager::getLastPVINVERTERTable(db, tbName, &pvInverterTable))
        {
            refreshPVINVERTERTablePointValue(dbName, tbName, pvInverterTable);
        }
    }
    else if (CxString::beginWith(tbName, C_P485METER_TABLE))
    {
        P485METERTable p485MeterTable;
        if (DeviceManager::getLastP485METERTable(db, tbName, &p485MeterTable))
        {
            refreshP485METERTablePointValue(dbName, tbName, p485MeterTable);
        }
    }
    else if (CxString::beginWith(tbName, C_CNS_DEVINFO_TABLE_PREFIX))
    {
        CNS_DEVINFOTable cnsDevinfoTable;
        if (DeviceManager::getLastCNSDevInfoTable(db, tbName, &cnsDevinfoTable))
        {
            refreshCNS_DEVINFOTablePointValue(dbName, tbName, cnsDevinfoTable);
        }
    }
    else if (CxString::beginWith(tbName, C_CNS_DEV_STATUS_TABLE_PREFIX))
    {
        CNS_DEV_STATUSTable cnsDevStatusTable;
        if (DeviceManager::getLastCNSDevStatusTable(db, tbName, &cnsDevStatusTable))
        {
            refreshCNS_DEV_STATUSTablePointValue(dbName, tbName, cnsDevStatusTable);
        }
    }
    else if (CxString::beginWith(tbName, C_CNS_TOPO_TABLE_PREFIX))
    {
        CNS_TOPOTable cnsTopoTable;
        if (DeviceManager::getLastCNSTOPOTable(db, tbName, &cnsTopoTable))
        {
            refreshCNS_TOPOTablePointValue(dbName, tbName, cnsTopoTable);
        }
    }
    else if (CxString::beginWith(tbName, C_CNS_SNR_PAR_TABLE_PREFIX))
    {
        CNS_CNS_SNR_PARTable cnsCnsSnrParTable;
        if (DeviceManager::getLastCNSSnrParTable(db, tbName, &cnsCnsSnrParTable))
        {
            refreshCNS_CNS_SNR_PARTablePointValue(dbName, tbName, cnsCnsSnrParTable);
        }
    }
    else if (CxString::beginWith(tbName, C_TVOC_TABLE_PREFIX))
    {
        TVOC_table tvocTable;
        if (DeviceManager::getLastVOCTable(db, tbName, &tvocTable))
        {
            refreshVOCTablePointValue(dbName, tbName, tvocTable);
        }
    }
    else if (CxString::beginWith(tbName, C_SENSORLEAK_TABLE_PREFIX))
    {
        SENSORLEAK_table sensorleakTable;
        if (DeviceManager::getLastSENSORLEAKTable(db, tbName, &sensorleakTable))
        {
            refreshSENSORLEAKTablePointValue(dbName, tbName, sensorleakTable);
        }
    }
    else if (CxString::beginWith(tbName, C_NOISE_TABLE_PREFIX))
    {
        NOISE_table noiseTable;
        if (DeviceManager::getLastNOISETable(db, tbName, &noiseTable))
        {
            refreshNOISETablePointValue(dbName, tbName, noiseTable);
        }
    }
    else if (CxString::beginWith(tbName, C_DAILY_TABLE_PREFIX))
    {
        DAILY_Table dailyTable;
        if (DeviceManager::getLastDAILYTable(db, tbName, &dailyTable))
        {
            refreshDAILYTablePointValue(dbName, tbName, dailyTable);
        }
    }
    else if (CxString::beginWith(tbName, C_ONTIME_TABLE_PREFIX))
    {
        ONTIME_Table ontimeTable;
        if (DeviceManager::getLastONTIMETable(db, tbName, &ontimeTable))
        {
            refreshONTIMETablePointValue(dbName, tbName, ontimeTable);
        }
    }
    else if (CxString::beginWith(tbName, C_O2_TABLE_PREFIX))
    {
        O2_table o2Table;
        if (DeviceManager::getLastO2Table(db, tbName, &o2Table))
        {
            refreshO2TablePointValue(dbName, tbName, o2Table);
        }
    }
    else if (CxString::beginWith(tbName, C_O3_TABLE_PREFIX))
    {
        O3_table o3Table;
        if (DeviceManager::getLastO3Table(db, tbName, &o3Table))
        {
            refreshO3TablePointValue(dbName, tbName, o3Table);
        }
    }
    else if (CxString::beginWith(tbName, C_QX_TABLE_PREFIX))
    {
        QX_table qxTable;
        if (DeviceManager::getLastQXTable(db, tbName, &qxTable))
        {
            refreshQXTablePointValue(dbName, tbName, qxTable);
        }
    }
    else if (CxString::beginWith(tbName, C_SF6_TABLE_PREFIX))
    {
        SF6_table sf6Table;
        if (DeviceManager::getLastSF6Table(db, tbName, &sf6Table))
        {
            refreshSF6TablePointValue(dbName, tbName, sf6Table);
        }
    }
    else if (CxString::beginWith(tbName, C_OIL_TABLE_PREFIX))
    {
        OIL_table oilTable;
        if (DeviceManager::getLastOILTable(db, tbName, &oilTable))
        {
            refreshOILTablePointValue(dbName, tbName, oilTable);
        }
    }
    else if (CxString::beginWith(tbName, C_SENSORGUZHI_TABLE_PREFIX))
    {
        SENSORGUZHI_table sensorguzhiTable;
        if (DeviceManager::getLastSENSORGUZHITable(db, tbName, &sensorguzhiTable))
        {
            refreshSENSORGUZHITablePointValue(dbName, tbName, sensorguzhiTable);
        }
    }
    else if (CxString::beginWith(tbName, C_RFID_TABLE_PREFIX))
    {
        RFID_table oilTable;
        if (DeviceManager::getLastRFIDTable(db, tbName, &oilTable))
        {
            refreshRFIDTablePointValue(dbName, tbName, oilTable);
        }
    }
    else
    {
        fnDebug().out("refreshAllPointValue TABLE_NAME[%s] open error!", tbName.c_str());
    }
}

const YxPoint *PointManager::getYxById(int id)
{
    for (int i = 0; i < f_yxPoints.size(); ++i)
    {
        YxPoint &yx = f_yxPoints.at(i);
        if (yx.ID == id)
        {
            return &yx;
        }
    }
    return NULL;
}

const YcPoint *PointManager::getYcById(int id)
{
    for (int i = 0; i < f_ycPoints.size(); ++i)
    {
        YcPoint &yc = f_ycPoints.at(i);
        if (yc.ID == id)
        {
            return &yc;
        }
    }
    return NULL;
}

const YxPoint *PointManager::getYxByDeviceAttr(const string &sDeviceId, const string &sDeviceAttr)
{
    for (int i = 0; i < f_yxPoints.size(); ++i)
    {
        YxPoint &yx = f_yxPoints.at(i);
        if (yx.DeviceId == sDeviceId && yx.DEVICE_ATTR == sDeviceAttr)
        {
            return &yx;
        }
    }
    return NULL;
}

const YcPoint *PointManager::getYcByDeviceAttr(const string &sDeviceId, const string &sDeviceAttr)
{
    for (int i = 0; i < f_ycPoints.size(); ++i)
    {
        YcPoint &yc = f_ycPoints.at(i);
        if (yc.DeviceId == sDeviceId && yc.DEVICE_ATTR == sDeviceAttr)
        {
            return &yc;
        }
    }
    return NULL;
}

const YcPoint *PointManager::getYcByDeviceModelAttr(const string &sDeviceId, const string &sModelAttr)
{
    for (int i = 0; i < f_ycPoints.size(); ++i)
    {
        YcPoint &yc = f_ycPoints.at(i);
        if (yc.DeviceId == sDeviceId && yc.MODEL_ATTR == sModelAttr)
        {
            return &yc;
        }
    }
    return NULL;
}

std::vector<YxPoint> PointManager::getYxByCA(int CA, int typeId)
{
    vector<YxPoint> r;
    for (int i = 0; i < f_yxPoints.size(); ++i)
    {
        YxPoint &yx = f_yxPoints.at(i);
        if (yx.CA == CA && yx.TYPE_ID == typeId)
        {
            r.push_back(yx);
        }
    }
    return r;
}

std::vector<YcPoint> PointManager::getYcByCA(int CA, int typeId)
{
    vector<YcPoint> r;
    for (int i = 0; i < f_ycPoints.size(); ++i)
    {
        YcPoint &yc = f_ycPoints.at(i);
        if (yc.CA == CA && yc.TYPE_ID == typeId)
        {
            r.push_back(yc);
        }
    }
    return r;
}

int PointManager::refreshGUZHITablePointValue(const std::string &sDatabaseName, const string &sTableName, GUZHITable &guzhiTable)
{
    for (int i = 0; i < f_ycPoints.size(); ++i)
    {
        YcPoint &yc = f_ycPoints.at(i);
        if (yc.DATABASE_NAME == sDatabaseName && yc.TABLE_NAME == sTableName)
        {
            if (yc.DEVICE_ATTR == DEVICE_TABLE_ATTR_GUZHI_CPU_OC) { setYcValue(&yc, guzhiTable.GUZHI_CPU_OC, guzhiTable.RECORD_TS);}
            if (yc.DEVICE_ATTR == DEVICE_TABLE_ATTR_GUZHI_PHASE_A_VOLT) { setYcValue(&yc, guzhiTable.GUZHI_PHASE_A_VOLT, guzhiTable.RECORD_TS);}
            if (yc.DEVICE_ATTR == DEVICE_TABLE_ATTR_GUZHI_PHASE_B_VOLT) { setYcValue(&yc, guzhiTable.GUZHI_PHASE_B_VOLT, guzhiTable.RECORD_TS);}
            if (yc.DEVICE_ATTR == DEVICE_TABLE_ATTR_GUZHI_PHASE_C_VOLT) { setYcValue(&yc, guzhiTable.GUZHI_PHASE_C_VOLT, guzhiTable.RECORD_TS);}
            if (yc.DEVICE_ATTR == DEVICE_TABLE_ATTR_GUZHI_PHASE_A_ELEC) { setYcValue(&yc, guzhiTable.GUZHI_PHASE_A_ELEC, guzhiTable.RECORD_TS);}
            if (yc.DEVICE_ATTR == DEVICE_TABLE_ATTR_GUZHI_PHASE_B_ELEC) { setYcValue(&yc, guzhiTable.GUZHI_PHASE_B_ELEC, guzhiTable.RECORD_TS);}
            if (yc.DEVICE_ATTR == DEVICE_TABLE_ATTR_GUZHI_PHASE_C_ELEC) { setYcValue(&yc, guzhiTable.GUZHI_PHASE_C_ELEC, guzhiTable.RECORD_TS);}
            if (yc.DEVICE_ATTR == DEVICE_TABLE_ATTR_GUZHI_POSTIVE_ENERGE) { setYcValue(&yc, guzhiTable.GUZHI_POSTIVE_ENERGE, guzhiTable.RECORD_TS);}
            if (yc.DEVICE_ATTR == DEVICE_TABLE_ATTR_GUZHI_REVERSE_ENERGE) { setYcValue(&yc, guzhiTable.GUZHI_REVERSE_ENERGE, guzhiTable.RECORD_TS);}
            if (yc.DEVICE_ATTR == DEVICE_TABLE_ATTR_RECORD_TS) { setYcValue(&yc, (guzhiTable.RECORD_TS - SEPOCH_2022) / 60, guzhiTable.RECORD_TS); }
        }
    }
    return 0;
}

int PointManager::refreshMETERTablePointValue(const std::string &sDatabaseName, const string &sTableName, const METERTable &meterTable)
{
    // refresh yx
    //
    // refresh yc
    for (int i = 0; i < f_ycPoints.size(); ++i)
    {
        YcPoint &yc = f_ycPoints.at(i);
        if (yc.DATABASE_NAME == sDatabaseName && yc.TABLE_NAME == sTableName)
        {
            if (yc.DEVICE_ATTR == DEVICE_TABLE_ATTR_METER_CPU_OC) { setYcValue(&yc, meterTable.POWER_METER_CPU_OC, meterTable.POWER_METER_TS);}
            if (yc.DEVICE_ATTR == DEVICE_TABLE_ATTR_METER_ENERGE) { setYcValue(&yc, meterTable.POWER_METER_ENERGE, meterTable.POWER_METER_TS);}
            if (yc.DEVICE_ATTR == DEVICE_TABLE_ATTR_METER_BACK_ENERGE) { setYcValue(&yc, meterTable.POWER_METER_BACK_ENERGE, meterTable.POWER_METER_TS);}
            if (yc.DEVICE_ATTR == DEVICE_TABLE_ATTR_METER_FORTH_HAVE_POWER_TOTAL) { setYcValue(&yc, meterTable.POWER_METER_FORTH_HAVE_POWER_TOTAL, meterTable.POWER_METER_TS);}
            if (yc.DEVICE_ATTR == DEVICE_TABLE_ATTR_METER_BACK_HAVE_POWER_TOTAL) { setYcValue(&yc, meterTable.POWER_METER_BACK_HAVE_POWER_TOTAL, meterTable.POWER_METER_TS);}
            if (yc.DEVICE_ATTR == DEVICE_TABLE_ATTR_METER_PHASE_A_FORTH_HAVE_POWER) { setYcValue(&yc, meterTable.POWER_METER_PHASE_A_FORTH_HAVE_POWER, meterTable.POWER_METER_TS);}
            if (yc.DEVICE_ATTR == DEVICE_TABLE_ATTR_METER_PHASE_A_BACK_HAVE_POWER) { setYcValue(&yc, meterTable.POWER_METER_PHASE_A_BACK_HAVE_POWER, meterTable.POWER_METER_TS);}
            if (yc.DEVICE_ATTR == DEVICE_TABLE_ATTR_METER_PHASE_B_FORTH_HAVE_POWER) { setYcValue(&yc, meterTable.POWER_METER_PHASE_B_FORTH_HAVE_POWER, meterTable.POWER_METER_TS);}
            if (yc.DEVICE_ATTR == DEVICE_TABLE_ATTR_METER_PHASE_B_BACK_HAVE_POWER) { setYcValue(&yc, meterTable.POWER_METER_PHASE_B_BACK_HAVE_POWER, meterTable.POWER_METER_TS);}
            if (yc.DEVICE_ATTR == DEVICE_TABLE_ATTR_METER_PHASE_C_FORTH_HAVE_POWER) { setYcValue(&yc, meterTable.POWER_METER_PHASE_C_FORTH_HAVE_POWER, meterTable.POWER_METER_TS);}
            if (yc.DEVICE_ATTR == DEVICE_TABLE_ATTR_METER_PHASE_C_BACK_HAVE_POWER) { setYcValue(&yc, meterTable.POWER_METER_PHASE_C_BACK_HAVE_POWER, meterTable.POWER_METER_TS);}
            if (yc.DEVICE_ATTR == DEVICE_TABLE_ATTR_METER_PHASE_A_VOLT) { setYcValue(&yc, meterTable.POWER_METER_PHASE_A_VOLT, meterTable.POWER_METER_TS);}
            if (yc.DEVICE_ATTR == DEVICE_TABLE_ATTR_METER_PHASE_B_VOLT) { setYcValue(&yc, meterTable.POWER_METER_PHASE_B_VOLT, meterTable.POWER_METER_TS);}
            if (yc.DEVICE_ATTR == DEVICE_TABLE_ATTR_METER_PHASE_C_VOLT) { setYcValue(&yc, meterTable.POWER_METER_PHASE_C_VOLT, meterTable.POWER_METER_TS);}
            if (yc.DEVICE_ATTR == DEVICE_TABLE_ATTR_METER_PHASE_A_ELEC) { setYcValue(&yc, meterTable.POWER_METER_PHASE_A_ELEC, meterTable.POWER_METER_TS);}
            if (yc.DEVICE_ATTR == DEVICE_TABLE_ATTR_METER_PHASE_B_ELEC) { setYcValue(&yc, meterTable.POWER_METER_PHASE_B_ELEC, meterTable.POWER_METER_TS);}
            if (yc.DEVICE_ATTR == DEVICE_TABLE_ATTR_METER_PHASE_C_ELEC) { setYcValue(&yc, meterTable.POWER_METER_PHASE_C_ELEC, meterTable.POWER_METER_TS);}
            if (yc.DEVICE_ATTR == DEVICE_TABLE_ATTR_METER_PHASE_A_ENERGE) { setYcValue(&yc, meterTable.POWER_METER_PHASE_A_ENERGE, meterTable.POWER_METER_TS);}
            if (yc.DEVICE_ATTR == DEVICE_TABLE_ATTR_METER_PHASE_B_ENERGE) { setYcValue(&yc, meterTable.POWER_METER_PHASE_B_ENERGE, meterTable.POWER_METER_TS);}
            if (yc.DEVICE_ATTR == DEVICE_TABLE_ATTR_METER_PHASE_C_ENERGE) { setYcValue(&yc, meterTable.POWER_METER_PHASE_C_ENERGE, meterTable.POWER_METER_TS);}
            if (yc.DEVICE_ATTR == DEVICE_TABLE_ATTR_METER_POWER_FATOR_A) { setYcValue(&yc, meterTable.POWER_METER_POWER_FATOR_A, meterTable.POWER_METER_TS);}
            if (yc.DEVICE_ATTR == DEVICE_TABLE_ATTR_METER_POWER_FATOR_B) { setYcValue(&yc, meterTable.POWER_METER_POWER_FATOR_B, meterTable.POWER_METER_TS);}
            if (yc.DEVICE_ATTR == DEVICE_TABLE_ATTR_METER_POWER_FATOR_C) { setYcValue(&yc, meterTable.POWER_METER_POWER_FATOR_C, meterTable.POWER_METER_TS);}
            if (yc.DEVICE_ATTR == DEVICE_TABLE_ATTR_METER_POWER_FATOR) { setYcValue(&yc, meterTable.POWER_METER_POWER_FATOR, meterTable.POWER_METER_TS);}
            if (yc.DEVICE_ATTR == DEVICE_TABLE_ATTR_RECORD_TS) { setYcValue(&yc, meterTable.POWER_METER_TS * 1000, meterTable.POWER_METER_TS); }
        }
    }
    return 0;
}

int PointManager::refreshSWITCHTablePointValue(const std::string &sDatabaseName, const string &sTableName, SWITCHTable &switchTable)
{
    // refresh yx
    for (int i = 0; i < f_yxPoints.size(); ++i)
    {
        YxPoint &yx = f_yxPoints.at(i);
        if (yx.DATABASE_NAME == sDatabaseName && yx.TABLE_NAME == sTableName)
        {
            if (yx.DEVICE_ATTR == DEVICE_TABLE_ATTR_SWITCH_ON_OFF) { setYxValue(&yx, switchTable.SWITCH_ON_OFF, switchTable.SWITCH_TS);}
        }
    }
    // refresh yc
    for (int i = 0; i < f_ycPoints.size(); ++i)
    {
        YcPoint &yc = f_ycPoints.at(i);
        if (yc.DATABASE_NAME == sDatabaseName && yc.TABLE_NAME == sTableName)
        {
            if (yc.DEVICE_ATTR == DEVICE_TABLE_ATTR_SWITCH_CPU_OC) { setYcValue(&yc, switchTable.SWITCH_CPU_OC, switchTable.SWITCH_TS);}
            if (yc.DEVICE_ATTR == DEVICE_TABLE_ATTR_SWITCH_TEMPERATURE) { setYcValue(&yc, switchTable.SWITCH_TEMPERATURE, switchTable.SWITCH_TS);}
            if (yc.DEVICE_ATTR == DEVICE_TABLE_ATTR_SWITCH_VOLTAGE) { setYcValue(&yc, switchTable.SWITCH_VOLTAGE, switchTable.SWITCH_TS);}
            if (yc.DEVICE_ATTR == DEVICE_TABLE_ATTR_SWITCH_CURRENT) { setYcValue(&yc, switchTable.SWITCH_CURRENT, switchTable.SWITCH_TS);}
            if (yc.DEVICE_ATTR == DEVICE_TABLE_ATTR_SWITCH_ENERGE) { setYcValue(&yc, switchTable.SWITCH_ENERGE, switchTable.SWITCH_TS);}
            if (yc.DEVICE_ATTR == DEVICE_TABLE_ATTR_SWITCH_FORTH_HAVE_POWER_TOTAL) { setYcValue(&yc, switchTable.SWITCH_FORTH_HAVE_POWER_TOTAL, switchTable.SWITCH_TS);}
            if (yc.DEVICE_ATTR == DEVICE_TABLE_ATTR_SWITCH_BACK_HAVE_POWER_TOTAL) { setYcValue(&yc, switchTable.SWITCH_BACK_HAVE_POWER_TOTAL, switchTable.SWITCH_TS);}
            if (yc.DEVICE_ATTR == DEVICE_TABLE_ATTR_SWITCH_PHASE_A_FORTH_HAVE_POWER) { setYcValue(&yc, switchTable.SWITCH_PHASE_A_FORTH_HAVE_POWER, switchTable.SWITCH_TS);}
            if (yc.DEVICE_ATTR == DEVICE_TABLE_ATTR_SWITCH_PHASE_A_BACK_HAVE_POWER) { setYcValue(&yc, switchTable.SWITCH_PHASE_A_BACK_HAVE_POWER, switchTable.SWITCH_TS);}
            if (yc.DEVICE_ATTR == DEVICE_TABLE_ATTR_SWITCH_PHASE_B_FORTH_HAVE_POWER) { setYcValue(&yc, switchTable.SWITCH_PHASE_B_FORTH_HAVE_POWER, switchTable.SWITCH_TS);}
            if (yc.DEVICE_ATTR == DEVICE_TABLE_ATTR_SWITCH_PHASE_B_BACK_HAVE_POWER) { setYcValue(&yc, switchTable.SWITCH_PHASE_B_BACK_HAVE_POWER, switchTable.SWITCH_TS);}
            if (yc.DEVICE_ATTR == DEVICE_TABLE_ATTR_SWITCH_PHASE_C_FORTH_HAVE_POWER) { setYcValue(&yc, switchTable.SWITCH_PHASE_C_FORTH_HAVE_POWER, switchTable.SWITCH_TS);}
            if (yc.DEVICE_ATTR == DEVICE_TABLE_ATTR_SWITCH_PHASE_C_BACK_HAVE_POWER) { setYcValue(&yc, switchTable.SWITCH_PHASE_C_BACK_HAVE_POWER, switchTable.SWITCH_TS);}
            if (yc.DEVICE_ATTR == DEVICE_TABLE_ATTR_SWITCH_PHASE_A_VOLT) { setYcValue(&yc, switchTable.SWITCH_PHASE_A_VOLT, switchTable.SWITCH_TS);}
            if (yc.DEVICE_ATTR == DEVICE_TABLE_ATTR_SWITCH_PHASE_B_VOLT) { setYcValue(&yc, switchTable.SWITCH_PHASE_B_VOLT, switchTable.SWITCH_TS);}
            if (yc.DEVICE_ATTR == DEVICE_TABLE_ATTR_SWITCH_PHASE_C_VOLT) { setYcValue(&yc, switchTable.SWITCH_PHASE_C_VOLT, switchTable.SWITCH_TS);}
            if (yc.DEVICE_ATTR == DEVICE_TABLE_ATTR_SWITCH_PHASE_A_ELEC) { setYcValue(&yc, switchTable.SWITCH_PHASE_A_ELEC, switchTable.SWITCH_TS);}
            if (yc.DEVICE_ATTR == DEVICE_TABLE_ATTR_SWITCH_PHASE_B_ELEC) { setYcValue(&yc, switchTable.SWITCH_PHASE_B_ELEC, switchTable.SWITCH_TS);}
            if (yc.DEVICE_ATTR == DEVICE_TABLE_ATTR_SWITCH_PHASE_C_ELEC) { setYcValue(&yc, switchTable.SWITCH_PHASE_C_ELEC, switchTable.SWITCH_TS);}
            if (yc.DEVICE_ATTR == DEVICE_TABLE_ATTR_SWITCH_ACTIVE_POWER) { setYcValue(&yc, switchTable.SWITCH_ACTIVE_POWER, switchTable.SWITCH_TS);}
            if (yc.DEVICE_ATTR == DEVICE_TABLE_ATTR_SWITCH_ON_OFF) { setYcValue(&yc, switchTable.SWITCH_ON_OFF, switchTable.SWITCH_TS);}
            if (yc.DEVICE_ATTR == DEVICE_TABLE_ATTR_SWITCH_RESIDUAL_CURRENT) { setYcValue(&yc, switchTable.SWITCH_RESIDUAL_CURRENT, switchTable.SWITCH_TS);}
            if (yc.DEVICE_ATTR == DEVICE_TABLE_ATTR_SWITCH_ZERO_SEQUENCE_CURRENT) { setYcValue(&yc, switchTable.SWITCH_ZERO_SEQUENCE_CURRENT, switchTable.SWITCH_TS);}
            if (yc.DEVICE_ATTR == DEVICE_TABLE_ATTR_SWITCH_POWER_FATOR) { setYcValue(&yc, switchTable.SWITCH_POWER_FATOR, switchTable.SWITCH_TS);}
            if (yc.DEVICE_ATTR == DEVICE_TABLE_ATTR_SWITCH_INPUT_A_TEMP) { setYcValue(&yc, switchTable.SWITCH_INPUT_A_TEMP, switchTable.SWITCH_TS);}
            if (yc.DEVICE_ATTR == DEVICE_TABLE_ATTR_SWITCH_INPUT_B_TEMP) { setYcValue(&yc, switchTable.SWITCH_INPUT_B_TEMP, switchTable.SWITCH_TS);}
            if (yc.DEVICE_ATTR == DEVICE_TABLE_ATTR_SWITCH_INPUT_C_TEMP) { setYcValue(&yc, switchTable.SWITCH_INPUT_C_TEMP, switchTable.SWITCH_TS);}
            if (yc.DEVICE_ATTR == DEVICE_TABLE_ATTR_SWITCH_OUTPUT_A_TEMP) { setYcValue(&yc, switchTable.SWITCH_OUTPUT_A_TEMP, switchTable.SWITCH_TS);}
            if (yc.DEVICE_ATTR == DEVICE_TABLE_ATTR_SWITCH_OUTPUT_B_TEMP) { setYcValue(&yc, switchTable.SWITCH_OUTPUT_B_TEMP, switchTable.SWITCH_TS);}
            if (yc.DEVICE_ATTR == DEVICE_TABLE_ATTR_SWITCH_OUTPUT_C_TEMP) { setYcValue(&yc, switchTable.SWITCH_OUTPUT_C_TEMP, switchTable.SWITCH_TS);}
            if (yc.DEVICE_ATTR == DEVICE_TABLE_ATTR_SWITCH_TEMP) { setYcValue(&yc, switchTable.SWITCH_TEMP, switchTable.SWITCH_TS);}
            if (yc.DEVICE_ATTR == DEVICE_TABLE_ATTR_SWITCH_TS) { setYcValue(&yc, (switchTable.SWITCH_TS - SEPOCH_2022) / 60, switchTable.SWITCH_TS); }
        }
    }
    return 0;
}

int PointManager::refreshYXTablePointValue(const std::string &sDatabaseName, const string &sTableName, YXTABLE &yxTable)
{
    // refresh yx
    for (int i = 0; i < f_yxPoints.size(); ++i)
    {
        YxPoint &yx = f_yxPoints.at(i);
        if (yx.DATABASE_NAME == sDatabaseName && yx.TABLE_NAME == sTableName)
        {
            if (yx.DEVICE_ATTR == DEVICE_TABLE_ATTR_PHASE_A_YX) { setYxValue(&yx, yxTable.PHASE_A_YX, yxTable.RECORD_TS);}
            if (yx.DEVICE_ATTR == DEVICE_TABLE_ATTR_PHASE_B_YX) { setYxValue(&yx, yxTable.PHASE_B_YX, yxTable.RECORD_TS);}
            if (yx.DEVICE_ATTR == DEVICE_TABLE_ATTR_PHASE_C_YX) { setYxValue(&yx, yxTable.PHASE_C_YX, yxTable.RECORD_TS);}
            if (yx.DEVICE_ATTR == DEVICE_TABLE_ATTR_PHASE_INFO)
            {
                // 三相
//                if(yxTable.PHASE_INFO == YX_PHASE_ALL)
//            {
                    switch (yx.POINT_TYPE)
                    {
                        case POINT_POWER_OUTAGES:
                             if(yxTable.PHASE_A_YX == YX_POWER_DOWN || yxTable.PHASE_B_YX == YX_POWER_DOWN || yxTable.PHASE_C_YX == YX_POWER_DOWN)
                                 setYxValue(&yx, 1, yxTable.RECORD_TS);
                             else if(yxTable.PHASE_A_YX == YX_POWER_UP || yxTable.PHASE_B_YX == YX_POWER_UP || yxTable.PHASE_C_YX == YX_POWER_UP)
                                 setYxValue(&yx, 0, yxTable.RECORD_TS);
//                             else if(yxTable.PHASE_A_YX == YX_PHASE_OK || yxTable.PHASE_B_YX == YX_PHASE_OK || yxTable.PHASE_C_YX == YX_PHASE_OK)
//                                 setYxValue(&yx, 0, yxTable.RECORD_TS);
                            break;
                        case POINT_ALL_PRESSURE_LOSS:
                            if(yxTable.PHASE_A_YX == YX_PHASE_DROP_VOL && yxTable.PHASE_B_YX == YX_PHASE_DROP_VOL && yxTable.PHASE_C_YX == YX_PHASE_DROP_VOL)
                                setYxValue(&yx, 1, yxTable.RECORD_TS);
                            else if(yxTable.PHASE_A_YX == YX_PHASE_DROP_VOL_RESTORE && yxTable.PHASE_B_YX == YX_PHASE_DROP_VOL_RESTORE && yxTable.PHASE_C_YX == YX_PHASE_DROP_VOL_RESTORE)
                                setYxValue(&yx, 0, yxTable.RECORD_TS);
                            break;
                        case POINT_A_PRESSURE_LOSS:
                            if(yxTable.PHASE_A_YX == YX_PHASE_DROP_VOL)
                                setYxValue(&yx, 1, yxTable.RECORD_TS);
                            else if (yxTable.PHASE_A_YX == YX_PHASE_DROP_VOL_RESTORE)
                                setYxValue(&yx, 0, yxTable.RECORD_TS);
                            break;
                        case POINT_B_PRESSURE_LOSS:
                            if(yxTable.PHASE_B_YX == YX_PHASE_DROP_VOL)
                                setYxValue(&yx, 1, yxTable.RECORD_TS);
                            else if(yxTable.PHASE_B_YX == YX_PHASE_DROP_VOL_RESTORE)
                                setYxValue(&yx, 0, yxTable.RECORD_TS);
                            break;
                        case POINT_C_PRESSURE_LOSS:
                            if(yxTable.PHASE_C_YX == YX_PHASE_DROP_VOL)
                                setYxValue(&yx, 1, yxTable.RECORD_TS);
                            else if(yxTable.PHASE_C_YX == YX_PHASE_DROP_VOL_RESTORE)
                                setYxValue(&yx, 0, yxTable.RECORD_TS);
                            break;
                        case POINT_A_PHASE_VOLTAGE:
                            if(yxTable.PHASE_A_YX == YX_PHASE_UNDER_VOL)
                                setYxValue(&yx, 1, yxTable.RECORD_TS);
                            else if(yxTable.PHASE_A_YX == YX_PHASE_UNDER_VOL_RESTORE)
                                setYxValue(&yx, 0, yxTable.RECORD_TS);
                            break;
                        case POINT_B_PHASE_VOLTAGE:
                            if(yxTable.PHASE_B_YX == YX_PHASE_UNDER_VOL)
                                setYxValue(&yx, 1, yxTable.RECORD_TS);
                            else if(yxTable.PHASE_B_YX == YX_PHASE_UNDER_VOL_RESTORE)
                                setYxValue(&yx, 0, yxTable.RECORD_TS);
                            break;
                        case POINT_C_PHASE_VOLTAGE:
                            if(yxTable.PHASE_C_YX == YX_PHASE_UNDER_VOL)
                                setYxValue(&yx, 1, yxTable.RECORD_TS);
                            else if(yxTable.PHASE_C_YX == YX_PHASE_UNDER_VOL_RESTORE)
                                setYxValue(&yx, 0, yxTable.RECORD_TS);
                            break;
                        case POINT_A_PHASE_OVERVOLTAGE:
                            if(yxTable.PHASE_A_YX == YX_PHASE_OVER_VOL)
                                setYxValue(&yx, 1, yxTable.RECORD_TS);
                            else if(yxTable.PHASE_A_YX == YX_PHASE_OVER_VOL_RESTORE)
                                setYxValue(&yx, 0, yxTable.RECORD_TS);
                            break;
                        case POINT_B_PHASE_OVERVOLTAGE:
                            if(yxTable.PHASE_B_YX == YX_PHASE_OVER_VOL)
                                setYxValue(&yx, 1, yxTable.RECORD_TS);
                            else if(yxTable.PHASE_B_YX == YX_PHASE_OVER_VOL_RESTORE)
                                setYxValue(&yx, 0, yxTable.RECORD_TS);
                            break;
                        case POINT_C_PHASE_OVERVOLTAGE:
                            if(yxTable.PHASE_C_YX == YX_PHASE_OVER_VOL)
                                setYxValue(&yx, 1, yxTable.RECORD_TS);
                            else if(yxTable.PHASE_C_YX == YX_PHASE_OVER_VOL_RESTORE)
                                setYxValue(&yx, 0, yxTable.RECORD_TS);
                            break;
                        case POINT_A_PHASE_FLOW:
                            if(yxTable.PHASE_A_YX == YX_PHASE_OVER_CURRENT)
                                setYxValue(&yx, 1, yxTable.RECORD_TS);
                            else if(yxTable.PHASE_A_YX == YX_PHASE_OVER_CURRENT_RESTORE)
                                setYxValue(&yx, 0, yxTable.RECORD_TS);
                            break;
                        case POINT_B_PHASE_FLOW:
                            if(yxTable.PHASE_B_YX == YX_PHASE_OVER_CURRENT)
                                setYxValue(&yx, 1, yxTable.RECORD_TS);
                            else if(yxTable.PHASE_B_YX == YX_PHASE_OVER_CURRENT_RESTORE)
                                setYxValue(&yx, 0, yxTable.RECORD_TS);
                            break;
                        case POINT_C_PHASE_FLOW:
                            if(yxTable.PHASE_C_YX == YX_PHASE_OVER_CURRENT)
                                setYxValue(&yx, 1, yxTable.RECORD_TS);
                            else if(yxTable.PHASE_C_YX == YX_PHASE_OVER_CURRENT_RESTORE)
                                setYxValue(&yx, 0, yxTable.RECORD_TS);
                            break;
                        case POINT_A_PHASE_OVERLOADED:
                            if(yxTable.PHASE_A_YX == YX_OVERLOAD)
                                setYxValue(&yx, 1, yxTable.RECORD_TS);
                            else if(yxTable.PHASE_A_YX == YX_OVERLOAD_RESTORE)
                                setYxValue(&yx, 0, yxTable.RECORD_TS);
                            break;
                        case POINT_B_PHASE_OVERLOADED:
                            if(yxTable.PHASE_B_YX == YX_OVERLOAD)
                                setYxValue(&yx, 1, yxTable.RECORD_TS);
                            else if(yxTable.PHASE_B_YX == YX_OVERLOAD_RESTORE)
                                setYxValue(&yx, 0, yxTable.RECORD_TS);
                            break;
                        case POINT_C_PHASE_OVERLOADED:
                            if(yxTable.PHASE_C_YX == YX_OVERLOAD)
                                setYxValue(&yx, 1, yxTable.RECORD_TS);
                            else if(yxTable.PHASE_C_YX == YX_OVERLOAD_RESTORE)
                                setYxValue(&yx, 0, yxTable.RECORD_TS);
                            break;
                        case POINT_SWITCH_OUTAGES:
                            if(yxTable.PHASE_A_YX == YX_POWER_DOWN || yxTable.PHASE_B_YX == YX_POWER_DOWN || yxTable.PHASE_C_YX == YX_POWER_DOWN)
                                setYxValue(&yx, 1, yxTable.RECORD_TS);
                            else if(yxTable.PHASE_A_YX == YX_POWER_UP || yxTable.PHASE_B_YX == YX_POWER_UP || yxTable.PHASE_C_YX == YX_POWER_UP)
                                setYxValue(&yx, 0, yxTable.RECORD_TS);
                            break;
                        case POINT_VOLTAGE_IMBALANCE:
                            if(yxTable.PHASE_A_YX == YX_SWITCH_VOLTAGE_UNBLANCE || yxTable.PHASE_B_YX == YX_SWITCH_VOLTAGE_UNBLANCE || yxTable.PHASE_C_YX == YX_SWITCH_VOLTAGE_UNBLANCE)
                                setYxValue(&yx, 1, yxTable.RECORD_TS);
                            else if(yxTable.PHASE_A_YX == YX_SWITCH_VOLTAGE_UNBLANCE_RESTORE || yxTable.PHASE_B_YX == YX_SWITCH_VOLTAGE_UNBLANCE_RESTORE || yxTable.PHASE_C_YX == YX_SWITCH_VOLTAGE_UNBLANCE_RESTORE)
                                setYxValue(&yx, 0, yxTable.RECORD_TS);
                            break;
                        case POINT_CURRENT_IMBALANCE:
                            if(yxTable.PHASE_A_YX == YX_SWITCH_CURRENT_UNBLANCE || yxTable.PHASE_B_YX == YX_SWITCH_CURRENT_UNBLANCE || yxTable.PHASE_C_YX == YX_SWITCH_CURRENT_UNBLANCE)
                                setYxValue(&yx, 1, yxTable.RECORD_TS);
                            else if(yxTable.PHASE_A_YX == YX_SWITCH_CURRENT_UNBLANCE_RESTORE || yxTable.PHASE_B_YX == YX_SWITCH_CURRENT_UNBLANCE_RESTORE || yxTable.PHASE_C_YX == YX_SWITCH_CURRENT_UNBLANCE_RESTORE)
                                setYxValue(&yx, 0, yxTable.RECORD_TS);
                            break;
                        case POINT_SWITCH_TEMPERATURE:
                            if(yxTable.PHASE_A_YX == YX_SWITCH_HOOKDOWN_TEMPOVER || yxTable.PHASE_A_YX == YX_SWITCH_HOOKDOWN_HIGHTEMP)
                                setYxValue(&yx, 1, yxTable.RECORD_TS);
                            else if(yxTable.PHASE_A_YX == YX_SWITCH_HOOKDOWN_TEMPOVER_RESTORE || yxTable.PHASE_A_YX == YX_SWITCH_HOOKDOWN_HIGHTEMP_RESTORE)
                                setYxValue(&yx, 0, yxTable.RECORD_TS);
                            break;
                        case POINT_PHASE_INFO:
                                setYxValue(&yx, yxTable.PHASE_INFO, yxTable.RECORD_TS);
                            break;
                        case POINT_ZERO_SEQUENCE:
                            if (yxTable.PHASE_A_YX == YX_SWITCH_ZERO_PROTECT || yxTable.PHASE_B_YX == YX_SWITCH_ZERO_PROTECT || yxTable.PHASE_C_YX == YX_SWITCH_ZERO_PROTECT)
                                setYxValue(&yx, 1, yxTable.RECORD_TS);
                        default:
                            setYxValue(&yx, yx.VALUE, yxTable.RECORD_TS);
                            break;
                    }
//                }
//                 单相
/*                else if(yxTable.PHASE_INFO == YX_PHASE_A || yxTable.PHASE_INFO == YX_PHASE_B || yxTable.PHASE_INFO == YX_PHASE_C)
                {
                    switch (yx.POINT_TYPE)
                    {
                        case POINT_POWER_OUTAGES:
                            if(yxTable.PHASE_A_YX == YX_POWER_DOWN || yxTable.PHASE_B_YX == YX_POWER_DOWN || yxTable.PHASE_C_YX == YX_POWER_DOWN)
                                setYxValue(&yx, 1, yxTable.RECORD_TS);
                            else
                                setYxValue(&yx, 0, yxTable.RECORD_TS);
                            break;
                        case POINT_ALL_PRESSURE_LOSS:
                            if(yxTable.PHASE_A_YX == YX_PHASE_DROP_VOL && yxTable.PHASE_B_YX == YX_PHASE_DROP_VOL && yxTable.PHASE_C_YX == YX_PHASE_DROP_VOL)
                                setYxValue(&yx, 1, yxTable.RECORD_TS);
                            else
                                setYxValue(&yx, 0, yxTable.RECORD_TS);
                            break;
                        case POINT_A_PRESSURE_LOSS:
                            if(yxTable.PHASE_A_YX == YX_PHASE_DROP_VOL)
                                setYxValue(&yx, 1, yxTable.RECORD_TS);
                            else
                                setYxValue(&yx, 0, yxTable.RECORD_TS);
                            break;
                        case POINT_B_PRESSURE_LOSS:
                            if(yxTable.PHASE_B_YX == YX_PHASE_DROP_VOL)
                                setYxValue(&yx, 1, yxTable.RECORD_TS);
                            else
                                setYxValue(&yx, 0, yxTable.RECORD_TS);
                            break;
                        case POINT_C_PRESSURE_LOSS:
                            if(yxTable.PHASE_C_YX == YX_PHASE_DROP_VOL)
                                setYxValue(&yx, 1, yxTable.RECORD_TS);
                            else
                                setYxValue(&yx, 0, yxTable.RECORD_TS);
                            break;
                        case POINT_A_PHASE_VOLTAGE:
                            if(yxTable.PHASE_A_YX == YX_PHASE_UNDER_VOL)
                                setYxValue(&yx, 1, yxTable.RECORD_TS);
                            else
                                setYxValue(&yx, 0, yxTable.RECORD_TS);
                            break;
                        case POINT_B_PHASE_VOLTAGE:
                            if(yxTable.PHASE_B_YX == YX_PHASE_UNDER_VOL)
                                setYxValue(&yx, 1, yxTable.RECORD_TS);
                            else
                                setYxValue(&yx, 0, yxTable.RECORD_TS);
                            break;
                        case POINT_C_PHASE_VOLTAGE:
                            if(yxTable.PHASE_C_YX == YX_PHASE_UNDER_VOL)
                                setYxValue(&yx, 1, yxTable.RECORD_TS);
                            else
                                setYxValue(&yx, 0, yxTable.RECORD_TS);
                            break;
                        case POINT_A_PHASE_OVERVOLTAGE:
                            if(yxTable.PHASE_A_YX == YX_PHASE_OVER_VOL)
                                setYxValue(&yx, 1, yxTable.RECORD_TS);
                            else
                                setYxValue(&yx, 0, yxTable.RECORD_TS);
                            break;
                        case POINT_B_PHASE_OVERVOLTAGE:
                            if(yxTable.PHASE_B_YX == YX_PHASE_OVER_VOL)
                                setYxValue(&yx, 1, yxTable.RECORD_TS);
                            else
                                setYxValue(&yx, 0, yxTable.RECORD_TS);
                            break;
                        case POINT_C_PHASE_OVERVOLTAGE:
                            if(yxTable.PHASE_C_YX == YX_PHASE_OVER_VOL)
                                setYxValue(&yx, 1, yxTable.RECORD_TS);
                            else
                                setYxValue(&yx, 0, yxTable.RECORD_TS);
                            break;
                        case POINT_A_PHASE_FLOW:
                            if(yxTable.PHASE_A_YX == YX_PHASE_OVER_CURRENT)
                                setYxValue(&yx, 1, yxTable.RECORD_TS);
                            else
                                setYxValue(&yx, 0, yxTable.RECORD_TS);
                            break;
                        case POINT_B_PHASE_FLOW:
                            if(yxTable.PHASE_B_YX == YX_PHASE_OVER_CURRENT)
                                setYxValue(&yx, 1, yxTable.RECORD_TS);
                            else
                                setYxValue(&yx, 0, yxTable.RECORD_TS);
                            break;
                        case POINT_C_PHASE_FLOW:
                            if(yxTable.PHASE_C_YX == YX_PHASE_OVER_CURRENT)
                                setYxValue(&yx, 1, yxTable.RECORD_TS);
                            else
                                setYxValue(&yx, 0, yxTable.RECORD_TS);
                            break;
                        case POINT_A_PHASE_OVERLOADED:
                            if(yxTable.PHASE_A_YX == YX_OVERLOAD)
                                setYxValue(&yx, 1, yxTable.RECORD_TS);
                            else
                                setYxValue(&yx, 0, yxTable.RECORD_TS);
                            break;
                        case POINT_B_PHASE_OVERLOADED:
                            if(yxTable.PHASE_B_YX == YX_OVERLOAD)
                                setYxValue(&yx, 1, yxTable.RECORD_TS);
                            else
                                setYxValue(&yx, 0, yxTable.RECORD_TS);
                            break;
                        case POINT_C_PHASE_OVERLOADED:
                            if(yxTable.PHASE_C_YX == YX_OVERLOAD)
                                setYxValue(&yx, 1, yxTable.RECORD_TS);
                            else
                                setYxValue(&yx, 0, yxTable.RECORD_TS);
                            break;
                        default:
                            break;
                    }
*/
//                }
            }
        }
    }
    // refresh yc
    for (int i = 0; i < f_ycPoints.size(); ++i)
    {
        YcPoint &yc = f_ycPoints.at(i);
        if (yc.DATABASE_NAME == sDatabaseName && yc.TABLE_NAME == sTableName)
        {
            if (yc.DEVICE_ATTR == DEVICE_TABLE_ATTR_PHASE_A_VOLT) { setYcValue(&yc, yxTable.PHASE_A_VOLT, yxTable.RECORD_TS);}
            if (yc.DEVICE_ATTR == DEVICE_TABLE_ATTR_PHASE_B_VOLT) { setYcValue(&yc, yxTable.PHASE_B_VOLT, yxTable.RECORD_TS);}
            if (yc.DEVICE_ATTR == DEVICE_TABLE_ATTR_PHASE_C_VOLT) { setYcValue(&yc, yxTable.PHASE_C_VOLT, yxTable.RECORD_TS);}
            if (yc.DEVICE_ATTR == DEVICE_TABLE_ATTR_PHASE_A_ELEC) { setYcValue(&yc, yxTable.PHASE_A_ELEC, yxTable.RECORD_TS);}
            if (yc.DEVICE_ATTR == DEVICE_TABLE_ATTR_PHASE_B_ELEC) { setYcValue(&yc, yxTable.PHASE_B_ELEC, yxTable.RECORD_TS);}
            if (yc.DEVICE_ATTR == DEVICE_TABLE_ATTR_PHASE_C_ELEC) { setYcValue(&yc, yxTable.PHASE_C_ELEC, yxTable.RECORD_TS);}
            if (yc.DEVICE_ATTR == DEVICE_TABLE_ATTR_RECORD_TS) { setYcValue(&yc, (yxTable.RECORD_TS - SEPOCH_2022) / 60, yxTable.RECORD_TS); }
        }
    }
    return 0;
}

int PointManager::refreshAREADAILYTablePointValue(const std::string &sDatabaseName, const std::string &sTableName, AREADAILYTable & areadailyTable)
{
    // refresh yx
//    for (int i = 0; i < f_yxPoints.size(); ++i)
//    {
//        YxPoint &yx = f_yxPoints.at(i);
//        if (yx.DATABASE_NAME == sDatabaseName && yx.TABLE_NAME == sTableName)
//        {
//            if (yx.DEVICE_ATTR == DEVICE_TABLE_ATTR_SWITCH_ON_OFF) { setYxValue(&yx, switchTable.SWITCH_ON_OFF, switchTable.SWITCH_TS);}
//        }
//    }

    // refresh yc
    for(int i = 0; i < f_ycPoints.size(); ++i)
    {
        YcPoint &yc = f_ycPoints.at(i);
        if (yc.DATABASE_NAME == sDatabaseName && yc.TABLE_NAME == sTableName)
        {
            if (yc.DEVICE_ATTR == DEVICE_TABLE_ATTR_DAILY_LINE_LOSS_IN_AREA) {setYcValue(&yc, areadailyTable.DAILY_LINE_LOSS_IN_AREA, areadailyTable.RECORD_TS);}
            if (yc.DEVICE_ATTR == DEVICE_TABLE_ATTR_THREE_PHASE_UNBALANCE_OF_DAILY_CURRENT) {setYcValue(&yc, areadailyTable.THREE_PHASE_UNBALANCE_OF_DAILY_CURRENT, areadailyTable.RECORD_TS);}
            if (yc.DEVICE_ATTR == DEVICE_TABLE_ATTR_OVER_LIMIT_RATE_OF_DAILY_VOLTAGE) {setYcValue(&yc, areadailyTable.OVER_LIMIT_RATE_OF_DAILY_VOLTAGE, areadailyTable.RECORD_TS);}
            if (yc.DEVICE_ATTR == DEVICE_TABLE_ATTR_LOWER_LIMIT_RATE_OF_DAILY_VOLTAGE) {setYcValue(&yc, areadailyTable.LOWER_LIMIT_RATE_OF_DAILY_VOLTAGE, areadailyTable.RECORD_TS);}
            if (yc.DEVICE_ATTR == DEVICE_TABLE_ATTR_QUALIFIED_RATE_OF_DAILY_VOLTAGE) {setYcValue(&yc, areadailyTable.QUALIFIED_RATE_OF_DAILY_VOLTAGE, areadailyTable.RECORD_TS);}
            if (yc.DEVICE_ATTR == DEVICE_TABLE_ATTR_DAILY_LOAD_RATE) {setYcValue(&yc, areadailyTable.DAILY_LOAD_RATE, areadailyTable.RECORD_TS);}
            if (yc.DEVICE_ATTR == DEVICE_TABLE_ATTR_RECORD_TS) { setYcValue(&yc, (areadailyTable.RECORD_TS - SEPOCH_2022) / 60, areadailyTable.RECORD_TS); }
        }
    }
    return 0;
}

int PointManager::refreshAREAMONTHLYTablePointValue(const std::string &sDatabaseName, const std::string &sTableName, AREAMONTHLYTable & areamonthlyTable)
{
    for(int i = 0; i < f_ycPoints.size(); ++i)
    {
        YcPoint &yc = f_ycPoints.at(i);
        if(yc.DATABASE_NAME == sDatabaseName && yc.TABLE_NAME == sTableName)
        {
            if (yc.DEVICE_ATTR == DEVICE_TABLE_ATTR_MONTHLY_LINE_LOSS) {setYcValue(&yc, areamonthlyTable.MONTHLY_LINE_LOSS, areamonthlyTable.RECORD_TS);}
            if (yc.DEVICE_ATTR == DEVICE_TABLE_ATTR_THREE_PHASE_UNBALANCE_OF_MONTHLY_CURRENT) {setYcValue(&yc, areamonthlyTable.THREE_PHASE_UNBALANCE_OF_MONTHLY_CURRENT, areamonthlyTable.RECORD_TS);}
            if (yc.DEVICE_ATTR == DEVICE_TABLE_ATTR_OVER_LIMIT_RATE_OF_MONTHLY_VOLTAGE) {setYcValue(&yc, areamonthlyTable.OVER_LIMIT_RATE_OF_MONTHLY_VOLTAGE, areamonthlyTable.RECORD_TS);}
            if (yc.DEVICE_ATTR == DEVICE_TABLE_ATTR_LOWER_LIMIT_RATE_OF_MONTHLY_VOLTAGE) {setYcValue(&yc, areamonthlyTable.LOWER_LIMIT_RATE_OF_MONTHLY_VOLTAGE, areamonthlyTable.RECORD_TS);}
            if (yc.DEVICE_ATTR == DEVICE_TABLE_ATTR_MONTHLY_VOLTAGE_QUALIFICATION_RATE) {setYcValue(&yc, areamonthlyTable.MONTHLY_VOLTAGE_QUALIFICATION_RATE, areamonthlyTable.RECORD_TS);}
            if (yc.DEVICE_ATTR == DEVICE_TABLE_ATTR_MONTHLY_LOAD_RATE) {setYcValue(&yc, areamonthlyTable.MONTHLY_LOAD_RATE, areamonthlyTable.RECORD_TS);}
            if (yc.DEVICE_ATTR == DEVICE_TABLE_ATTR_RECORD_TS) { setYcValue(&yc, (areamonthlyTable.RECORD_TS - SEPOCH_2022) / 60, areamonthlyTable.RECORD_TS); }
        }
    }
    return 0;
}

int PointManager::refreshPOWERLOSSTablePointValue(const std::string &sDatabaseName, const std::string &sTableName, POWERLOSSTable & powerlossTable)
{
    for(int i = 0; i < f_yxPoints.size(); ++i)
    {
        YxPoint &yx = f_yxPoints.at(i);
        if(yx.DATABASE_NAME == sDatabaseName && yx.TABLE_NAME == sTableName)
        {
            if (yx.DEVICE_ATTR == DEVICE_TABLE_ATTR_AREABUGTYPE)
            {
                switch (yx.POINT_TYPE)
                {
                    case PINT_LOW_VOLTAGE:
                        if (powerlossTable.AREABUGSTAA == YX_PHASE_DROP_VOL || powerlossTable.AREABUGSTAB == YX_PHASE_DROP_VOL || powerlossTable.AREABUGSTAC == YX_PHASE_DROP_VOL)
                            setYxValue(&yx, 1, powerlossTable.RECORD_TS);
                        else
                            setYxValue(&yx, 0, powerlossTable.RECORD_TS);
                        break;
                    case POINT_OVERLOADING:
                        if (powerlossTable.AREABUGSTAA == YX_OVERLOAD || powerlossTable.AREABUGSTAB == YX_OVERLOAD || powerlossTable.AREABUGSTAC == YX_OVERLOAD)
                            setYxValue(&yx, 1, powerlossTable.RECORD_TS);
                        else
                            setYxValue(&yx, 0, powerlossTable.RECORD_TS);
                        break;
                    case POINT_POWER_OUTAGES:
                        if (powerlossTable.AREABUGSTAA == YX_POWER_DOWN || powerlossTable.AREABUGSTAB == YX_POWER_DOWN || powerlossTable.AREABUGSTAC == YX_POWER_DOWN)
                            setYxValue(&yx, 1, powerlossTable.RECORD_TS);
                        else
                            setYxValue(&yx, 0, powerlossTable.RECORD_TS);
                        break;
                    case POINT_CURRENT_UNBLANCE:
                        if (powerlossTable.AREABUGSTAA == YX_SWITCH_CURRENT_UNBLANCE || powerlossTable.AREABUGSTAB == YX_SWITCH_CURRENT_UNBLANCE || powerlossTable.AREABUGSTAC == YX_SWITCH_CURRENT_UNBLANCE)
                            setYxValue(&yx, 1, powerlossTable.RECORD_TS);
                        else
                            setYxValue(&yx, 0, powerlossTable.RECORD_TS);
                        break;
                    case POINT_CURRENT_PHASE_DROP:
                        if (powerlossTable.AREABUGSTAA == YX_SWITCH_CURRENT_PHASE_DROP || powerlossTable.AREABUGSTAB == YX_SWITCH_CURRENT_PHASE_DROP || powerlossTable.AREABUGSTAC == YX_SWITCH_CURRENT_PHASE_DROP)
                            setYxValue(&yx, 1, powerlossTable.RECORD_TS);
                        else
                            setYxValue(&yx, 0, powerlossTable.RECORD_TS);
                        break;
                    default:
                        setYxValue(&yx, yx.VALUE, powerlossTable.RECORD_TS);
                        break;
                }
                setYxValue(&yx, powerlossTable.AREABUGTYPE, powerlossTable.RECORD_TS);
            }
            if (yx.DEVICE_ATTR == DEVICE_TABLE_ATTR_AREABUGSTAA) {setYxValue(&yx, powerlossTable.AREABUGSTAA, powerlossTable.RECORD_TS);}
            if (yx.DEVICE_ATTR == DEVICE_TABLE_ATTR_AREABUGSTAB) {setYxValue(&yx, powerlossTable.AREABUGSTAB, powerlossTable.RECORD_TS);}
            if (yx.DEVICE_ATTR == DEVICE_TABLE_ATTR_AREABUGSTAC) {setYxValue(&yx, powerlossTable.AREABUGSTAC, powerlossTable.RECORD_TS);}
            //if (yx.DEVICE_ATTR == DEVICE_TABLE_ATTR_BRANCHDEVICE) {setYcValue(&yx, powerlossTable.BRANCHDEVICE, powerlossTable.RECORD_TS);}
        }
    }
    return 0;
}

int PointManager::refreshAREAANNUALTablePointValue(const std::string &sDatabaseName, const std::string &sTableName, AREAMETERYEARSTable & areameteryearsTable)
{
    for(int i = 0; i < f_ycPoints.size(); ++i)
    {
        YcPoint &yc = f_ycPoints.at(i);
        if(yc.DATABASE_NAME == sDatabaseName && yc.TABLE_NAME == sTableName)
        {
            if (yc.DEVICE_ATTR == DEVICE_TABLE_ATTR_THREE_PHASE_UNBALANCE_OF_ANNUAL_CURRENT) {setYcValue(&yc, areameteryearsTable.THREE_PHASE_UNBALANCE_OF_ANNUAL_CURRENT, areameteryearsTable.RECORD_TS);}
            if (yc.DEVICE_ATTR == DEVICE_TABLE_ATTR_ANNUAL_VOLTAGE_OVER_LIMIT_RATE) {setYcValue(&yc, areameteryearsTable.ANNUAL_VOLTAGE_OVER_LIMIT_RATE, areameteryearsTable.RECORD_TS);}
            if (yc.DEVICE_ATTR == DEVICE_TABLE_ATTR_ANNUAL_VOLTAGE_LOWER_LIMIT_RATE) {setYcValue(&yc, areameteryearsTable.ANNUAL_VOLTAGE_LOWER_LIMIT_RATE, areameteryearsTable.RECORD_TS);}
            if (yc.DEVICE_ATTR == DEVICE_TABLE_ATTR_ANNUAL_VOLTAGE_QUALIFICATION_RATE) {setYcValue(&yc, areameteryearsTable.ANNUAL_VOLTAGE_QUALIFICATION_RATE, areameteryearsTable.RECORD_TS);}
            if (yc.DEVICE_ATTR == DEVICE_TABLE_ATTR_ANNUAL_LOAD_RATE) {setYcValue(&yc, areameteryearsTable.ANNUAL_LOAD_RATE, areameteryearsTable.RECORD_TS);}
            if (yc.DEVICE_ATTR == DEVICE_TABLE_ATTR_RECORD_TS) {setYcValue(&yc, (areameteryearsTable.RECORD_TS - SEPOCH_2022) / 60, areameteryearsTable.RECORD_TS);}
        }
    }
    return 0;
}

int PointManager::refreshCHARGETablePointValue(const std::string &sDatabaseName, const std::string &sTableName, CHARGETable & chargeTable)
{
    for(int i = 0; i < f_ycPoints.size(); ++i)
    {
        YcPoint &yc = f_ycPoints.at(i);
        if(yc.DATABASE_NAME == sDatabaseName && yc.TABLE_NAME == sTableName)
        {
//            if (yc.DEVICE_ATTR == DEVICE_TABLE_ATTR_CHARGEID) {setYcValue(&yc, chargeTable.CHARGEID, chargeTable.RECORD_TS);}
            if (yc.DEVICE_ATTR == DEVICE_TABLE_ATTR_CHARGEVOLTAGE) {setYcValue(&yc, chargeTable.CHARGEVOLTAGE, chargeTable.RECORD_TS);}
            if (yc.DEVICE_ATTR == DEVICE_TABLE_ATTR_CHARGECURRENT) {setYcValue(&yc, chargeTable.CHARGECURRENT, chargeTable.RECORD_TS);}
            if (yc.DEVICE_ATTR == DEVICE_TABLE_ATTR_CHARGEPOWER) {setYcValue(&yc, chargeTable.CHARGEPOWER, chargeTable.RECORD_TS);}
            if (yc.DEVICE_ATTR == DEVICE_TABLE_ATTR_CHARGEENERGETOTAL) {setYcValue(&yc, chargeTable.CHARGEENERGETOTAL, chargeTable.RECORD_TS);}
            if (yc.DEVICE_ATTR == DEVICE_TABLE_ATTR_CHARGEENERGETIP) {setYcValue(&yc, chargeTable.CHARGEENERGETIP, chargeTable.RECORD_TS);}
            if (yc.DEVICE_ATTR == DEVICE_TABLE_ATTR_CHARGEENERGEPEAK) {setYcValue(&yc, chargeTable.CHARGEENERGEPEAK, chargeTable.RECORD_TS);}
            if (yc.DEVICE_ATTR == DEVICE_TABLE_ATTR_CHARGEENERGEFLAT) {setYcValue(&yc, chargeTable.CHARGEENERGEFLAT, chargeTable.RECORD_TS);}
            if (yc.DEVICE_ATTR == DEVICE_TABLE_ATTR_CHARGEENERGEBOTTOM) {setYcValue(&yc, chargeTable.CHARGEENERGEBOTTOM, chargeTable.RECORD_TS);}
            if (yc.DEVICE_ATTR == DEVICE_TABLE_ATTR_CHARGETEMPTURE) {setYcValue(&yc, chargeTable.CHARGETEMPTURE, chargeTable.RECORD_TS);}
            if (yc.DEVICE_ATTR == DEVICE_TABLE_ATTR_RECORD_TS) { setYcValue(&yc, (chargeTable.RECORD_TS - SEPOCH_2022) / 60, chargeTable.RECORD_TS); }
        }
    }
    return 0;
}

int PointManager::refreshBRANCHTablePointValue(const std::string &sDatabaseName, const std::string &sTableName, BRANCHTable & branchTable)
{
    for(int i = 0; i < f_ycPoints.size(); ++i)
    {
        YcPoint &yc = f_ycPoints.at(i);
        if(yc.DATABASE_NAME == sDatabaseName && yc.TABLE_NAME == sTableName)
        {
//            if (yc.DEVICE_ATTR == DEVICE_TABLE_ATTR_BRANCHDEVICE) {setYcValue(&yc, branchTable.BRANCHDEVICE, branchTable.RECORD_TS);}
            if (yc.DEVICE_ATTR == DEVICE_TABLE_ATTR_LINELOSS){setYcValue(&yc, branchTable.LINELOSS, branchTable.RECORD_TS);}
            if (yc.DEVICE_ATTR == DEVICE_TABLE_ATTR_LINELOSS_MONTH){setYcValue(&yc, branchTable.LINELOSS_MONTH, branchTable.RECORD_TS);}
            if (yc.DEVICE_ATTR == DEVICE_TABLE_ATTR_LINELOSSA) {setYcValue(&yc, branchTable.LINELOSSA, branchTable.RECORD_TS);}
            if (yc.DEVICE_ATTR == DEVICE_TABLE_ATTR_LINELOSSB) {setYcValue(&yc, branchTable.LINELOSSB, branchTable.RECORD_TS);}
            if (yc.DEVICE_ATTR == DEVICE_TABLE_ATTR_LINELOSSC) {setYcValue(&yc, branchTable.LINELOSSC, branchTable.RECORD_TS);}
            if (yc.DEVICE_ATTR == DEVICE_TABLE_ATTR_FROZTP) {setYcValue(&yc, branchTable.FROZTP, branchTable.RECORD_TS);}
            if (yc.DEVICE_ATTR == DEVICE_TABLE_ATTR_RECORD_TS) {setYcValue(&yc, (branchTable.RECORD_TS - SEPOCH_2022) / 60, branchTable.RECORD_TS); }
        }
    }
    return 0;
}

int PointManager::refreshPHASEBLANCETablePointValue(const std::string &sDatabaseName, const std::string &sTableName, PHASEBLANCETable & phaseblanceTable)
{
    for (int i = 0; i < f_ycPoints.size(); ++i)
    {
        YcPoint &yc = f_ycPoints.at(i);
        if (yc.DATABASE_NAME == sDatabaseName && yc.TABLE_NAME == sTableName)
        {
//            if (yc.DEVICE_ATTR == DEVICE_TABLE_ATTR_DEVICEID){setYcValue(&yc, phaseblanceTable.DEVICEID, phaseblanceTable.RECORD_TS);}
            if (yc.DEVICE_ATTR == DEVICE_TABLE_ATTR_CURBALANCERATE) {setYcValue(&yc, phaseblanceTable.CURBALANCERATE, phaseblanceTable.RECORD_TS);}
            if (yc.DEVICE_ATTR == DEVICE_TABLE_ATTR_RECORD_TS) { setYcValue(&yc, (phaseblanceTable.RECORD_TS - SEPOCH_2022) / 60, phaseblanceTable.RECORD_TS); }
        }
    }
    return 0;
}

int PointManager::refreshCHARGEYXTablePointValue(const std::string &sDatabaseName, const std::string &sTableName, CHARGEYXTable & chargeyxTable)
{
    for (int i = 0; i < f_ycPoints.size(); ++i)
    {
        YcPoint &yc = f_ycPoints.at(i);
        if (yc.DATABASE_NAME == sDatabaseName && yc.TABLE_NAME == sTableName)
        {
//            if (yx.DEVICE_ATTR == DEVICE_TABLE_ATTR_CHARGEID) {setYxValue(&yx, chargeyxTable.CHARGEID, chargeyxTable.RECORD_TS);}
            if (yc.DEVICE_ATTR == DEVICE_TABLE_ATTR_CHARGESTATUS){setYcValue(&yc, chargeyxTable.CHARGESTATUS, chargeyxTable.RECORD_TS);}
            if (yc.DEVICE_ATTR == DEVICE_TABLE_ATTR_CHARGEHOOKSTATUS) {setYcValue(&yc, chargeyxTable.CHARGEHOOKSTATUS, chargeyxTable.RECORD_TS);}
            if (yc.DEVICE_ATTR == DEVICE_TABLE_ATTR_RECORD_TS) { setYcValue(&yc, (chargeyxTable.RECORD_TS - SEPOCH_2022) / 60, chargeyxTable.RECORD_TS); }
        }
    }
    for (int i = 0; i < f_yxPoints.size(); ++i)
    {
        YxPoint &yx = f_yxPoints.at(i);
        if (yx.DATABASE_NAME == sDatabaseName && yx.TABLE_NAME == sTableName)
        {
//            if (yx.DEVICE_ATTR == DEVICE_TABLE_ATTR_CHARGEID) {setYxValue(&yx, chargeyxTable.CHARGEID, chargeyxTable.RECORD_TS);}
            if (yx.DEVICE_ATTR == DEVICE_TABLE_ATTR_CHARGESTATUS){setYxValue(&yx, chargeyxTable.CHARGESTATUS, chargeyxTable.RECORD_TS);}
            if (yx.DEVICE_ATTR == DEVICE_TABLE_ATTR_CHARGEHOOKSTATUS) {setYxValue(&yx, chargeyxTable.CHARGEHOOKSTATUS, chargeyxTable.RECORD_TS);}
        }
    }
    return 0;
}

int PointManager::refreshTHTablePointValue(const std::string &sDatabaseName, const std::string &sTableName, THTable & thTable)
{
    for (int i = 0; i < f_ycPoints.size(); ++i)
    {
        YcPoint &yc = f_ycPoints.at(i);
        if (yc.DATABASE_NAME == sDatabaseName && yc.TABLE_NAME == sTableName)
        {
            if (yc.DEVICE_ATTR == DEVICE_TABLE_ATTR_TEMPERATURE){setYcValue(&yc, thTable.TEMPERATURE, thTable.RECORD_TS);}
            if (yc.DEVICE_ATTR == DEVICE_TABLE_ATTR_HUMIDITY) {setYcValue(&yc, thTable.HUMIDITY, thTable.RECORD_TS);}
            if (yc.DEVICE_ATTR == DEVICE_TABLE_ATTR_RECORD_TS) { setYcValue(&yc, (thTable.RECORD_TS - SEPOCH_2022) / 60, thTable.RECORD_TS); }
        }
    }
    return 0;
}

// MayBe SMOKESTATE Is YC or YX
int PointManager::refreshSENSORSMOKETablePointValue(const std::string &sDatabaseName, const std::string &sTableName, SENSORSMOKETable & sensorsmokeTable)
{
    for (int i = 0; i < f_ycPoints.size(); ++i)
    {
        YcPoint &yc = f_ycPoints.at(i);
        if (yc.DATABASE_NAME == sDatabaseName && yc.TABLE_NAME == sTableName)
        {
            if (yc.DEVICE_ATTR == DEVICE_TABLE_ATTR_SMOKE_STATE){setYcValue(&yc, sensorsmokeTable.SMOKESTATE, sensorsmokeTable.RECORD_TS);}
            if (yc.DEVICE_ATTR == DEVICE_TABLE_ATTR_RECORD_TS) { setYcValue(&yc, (sensorsmokeTable.RECORD_TS - SEPOCH_2022) / 60, sensorsmokeTable.RECORD_TS); }
        }
    }
    for (int i = 0; i < f_yxPoints.size(); ++i)
    {
        YxPoint &yx = f_yxPoints.at(i);
        if (yx.DATABASE_NAME == sDatabaseName && yx.TABLE_NAME == sTableName)
        {
            if (yx.DEVICE_ATTR == DEVICE_TABLE_ATTR_SMOKE_STATE){setYxValue(&yx, sensorsmokeTable.SMOKESTATE, sensorsmokeTable.RECORD_TS);}
        }
    }
    return 0;
}

void PointManager::setYxValue(YxPoint *yx, int value, msepoch_t t, bool tag)
{
    yx->VALUE = value;
    yx->QUALITY = QUALITY_VALID;
    msepoch_t dt = 0;
    if (t > 1000000000000 && t < 9999999999999) {
        dt = t;
    } else if (t > 1000000000 && t < 9999999999) {
        dt = t * 1000;
    }
    yx->TIME_MS = dt;
    yx->SINGLE_VALUE = toSingleValue(value);
    yx->DOUBLE_VALUE = toDoublePointValue(value);
    yx->TIME24 = toCP24Time2a(dt);
    yx->TIME56 = toCP56Time2a(dt);
    yx->Tag = tag;
}

void PointManager::setYxValue(YxPoint *yx, int value, msepoch_t t)
{
    setYxValue(yx, value, t, false);
}

void PointManager::setYcValue(YcPoint *yc, double value, msepoch_t t, bool tag)
{
    yc->VALUE = value;
    yc->QUALITY = QUALITY_VALID;
    msepoch_t dt = 0;
    if (t > 1000000000000 && t < 9999999999999) {
        dt = t;
    } else if (t > 1000000000 && t < 9999999999) {
        dt = t * 1000;
    }
    yc->TIME_MS = dt;
    yc->SHORT_VALUE = (float)value;
    yc->NORMALIZED_VALUE = toNormalizedValue(value, yc->NORMALIZED_MAX);
    yc->SCALED_VALUE = toScaledValue(value, yc->SCALED_FACTOR);
    yc->TIME24 = toCP24Time2a(dt);
    yc->TIME56 = toCP56Time2a(dt);
    yc->Tag = tag;
}

void PointManager::setYcValue(YcPoint *yc, double value, msepoch_t t)
{
    yc->VALUE = value;
    yc->QUALITY = QUALITY_VALID;
    msepoch_t dt = 0;
    if (t > 1000000000000 && t < 9999999999999) {
        dt = t;
    } else if (t > 1000000000 && t < 9999999999) {
        dt = t * 1000;
    }
    yc->TIME_MS = dt;
    yc->SHORT_VALUE = (float)value;
    yc->NORMALIZED_VALUE = toNormalizedValue(value, yc->NORMALIZED_MAX);
    yc->SCALED_VALUE = toScaledValue(value, yc->SCALED_FACTOR);
    yc->TIME24 = toCP24Time2a(dt);
    yc->TIME56 = toCP56Time2a(dt);
}

void PointManager::resetYxPoint(YxPoint *yx)
{
    msepoch_t dt = CxTime::currentMsepoch();
    yx->VALUE = 0;
    yx->OldVALUE = 0;
//    yx->QUALITY = QUALITY_INVALID;
    yx->QUALITY = QUALITY_VALID;
    yx->TIME_MS = dt;
    yx->SINGLE_VALUE = false;
    yx->DOUBLE_VALUE = toDoublePointValue(-1);
    yx->TIME24 = toCP24Time2a(dt);
    yx->TIME56 = toCP56Time2a(dt);
}

void PointManager::resetYcPoint(YcPoint *yc)
{
    msepoch_t dt = CxTime::currentMsepoch();
    yc->VALUE = 0;
    yc->OldVALUE = 0;
    yc->QUALITY = QUALITY_INVALID;
    yc->TIME_MS = dt;
    yc->SHORT_VALUE = 0;
    yc->NORMALIZED_VALUE = 0;
    yc->SCALED_VALUE = 0;
    yc->TIME24 = toCP24Time2a(dt);
    yc->TIME56 = toCP56Time2a(dt);
}

void PointManager::resetIotDevice(IotDevice *device)
{
    msepoch_t t = CxTime::currentMsepoch();
    device->FW_VERSION = -1;
    device->SW_VERSION = -1;
    device->IOT_VERSION = -1;
    device->WRITE_TIME = -1;
    device->STATUS = -1;
}

bool PointManager::toSingleValue(int value)
{
    return value != 0;
}

float PointManager::toNormalizedValue(double value, double max)
{
    return (float)(value / max);
}

int PointManager::toScaledValue(double value, double scaled)
{
    return int (value * scaled);
}

std::vector<const YxPoint*> PointManager::getYxsByDeviceTBName(const std::string &dbName, const std::string &tbName)
{
    std::vector<const YxPoint*> r;
    for (int i = 0; i < f_yxPoints.size(); ++i)
    {
        const YxPoint &yx = f_yxPoints.at(i);
        if ((yx.DATABASE_NAME == dbName && yx.TABLE_NAME == tbName))
        {
            const YxPoint *oYx = &yx;
            r.push_back(oYx);
        }
    }
    return r;
}

std::vector<const YxPoint*> PointManager::getYxsByDevice2(const std::string &deviceId, const std::string &orDeviceId)
{
    std::vector<const YxPoint*> r;
    for (int i = 0; i < f_yxPoints.size(); ++i)
    {
        YxPoint &yx = f_yxPoints.at(i);
        if (yx.DeviceId == deviceId || yx.DeviceId == orDeviceId)
        {
            const YxPoint *oYx = &yx;
            r.push_back(oYx);
        }
    }
    return r;
}

std::vector<const YxPoint*> PointManager::getYxsByDeviceTagTrue(const std::string &deviceId)
{
    std::vector<const YxPoint*> r;
    for (int i = 0; i < f_yxPoints.size(); ++i)
    {
        YxPoint &yx = f_yxPoints.at(i);
        if (yx.DeviceId == deviceId && yx.Tag == true)
        {
            yx.Tag = false;
            const YxPoint *oYx = &yx;
            r.push_back(oYx);
        }
    }
    return r;
}

std::vector<const YcPoint*> PointManager::getYcsByDevice(const std::string &dbName, const std::string &tbName)
{
    std::vector<const YcPoint*> r;
    for (int i = 0; i < f_ycPoints.size(); ++i)
    {
        const YcPoint &yc = f_ycPoints.at(i);
        if (yc.DATABASE_NAME == dbName && yc.TABLE_NAME == tbName && yc.Tag == true)
        {
//            r.push_back(&yc);
            const YcPoint *oYc = &yc;
            r.push_back(oYc);
        }
    }
    return r;
}

void PointManager::setPhaseinfo(std::string &deviceId, PHASE_NOTIFY &data)
{
    std::vector<YcPoint> r;
    for (int i = 0; i < f_ycPoints.size(); ++i)
    {
        YcPoint &yc = f_ycPoints.at(i);
        int posYc = yc.TABLE_NAME.find_last_of('_');
        std::string ycTbName = yc.TABLE_NAME.substr(posYc+1);
        if (ycTbName == deviceId)
        {
            switch (data.PHASE_INFO)
            {
                case 0:
                    yc.Tag = true;
                    break;
                case 1:
                    if (yc.DEVICE_ATTR == DEVICE_TABLE_ATTR_METER_PHASE_A_VOLT || yc.DEVICE_ATTR == DEVICE_TABLE_ATTR_METER_PHASE_A_ELEC ||    \
                        yc.DEVICE_ATTR == DEVICE_TABLE_ATTR_METER_PHASE_A_FORTH_HAVE_POWER || yc.DEVICE_ATTR == DEVICE_TABLE_ATTR_METER_PHASE_A_BACK_HAVE_POWER || \
                        yc.DEVICE_ATTR == DEVICE_TABLE_ATTR_METER_POWER_FATOR_A || yc.DEVICE_ATTR == DEVICE_TABLE_ATTR_METER_PHASE_A_ENERGE)
                    {
                        yc.Tag = true;
                    }
                    else if (yc.DEVICE_ATTR == DEVICE_TABLE_ATTR_METER_PHASE_B_VOLT || yc.DEVICE_ATTR == DEVICE_TABLE_ATTR_METER_PHASE_B_ELEC ||    \
                        yc.DEVICE_ATTR == DEVICE_TABLE_ATTR_METER_PHASE_B_FORTH_HAVE_POWER || yc.DEVICE_ATTR == DEVICE_TABLE_ATTR_METER_PHASE_B_BACK_HAVE_POWER || \
                        yc.DEVICE_ATTR == DEVICE_TABLE_ATTR_METER_POWER_FATOR_B || yc.DEVICE_ATTR == DEVICE_TABLE_ATTR_METER_PHASE_B_ENERGE)
                    {
                        yc.Tag = false;
                    }
                    else if (yc.DEVICE_ATTR == DEVICE_TABLE_ATTR_METER_PHASE_C_VOLT || yc.DEVICE_ATTR == DEVICE_TABLE_ATTR_METER_PHASE_C_ELEC ||    \
                        yc.DEVICE_ATTR == DEVICE_TABLE_ATTR_METER_PHASE_C_FORTH_HAVE_POWER || yc.DEVICE_ATTR == DEVICE_TABLE_ATTR_METER_PHASE_C_BACK_HAVE_POWER || \
                        yc.DEVICE_ATTR == DEVICE_TABLE_ATTR_METER_POWER_FATOR_C || yc.DEVICE_ATTR == DEVICE_TABLE_ATTR_METER_PHASE_C_ENERGE)
                    {
                        yc.Tag = false;
                    }
                    else
                        yc.Tag = true;
                    break;
                case 2:
                    if (yc.DEVICE_ATTR == DEVICE_TABLE_ATTR_METER_PHASE_A_VOLT || yc.DEVICE_ATTR == DEVICE_TABLE_ATTR_METER_PHASE_A_ELEC ||    \
                        yc.DEVICE_ATTR == DEVICE_TABLE_ATTR_METER_PHASE_A_FORTH_HAVE_POWER || yc.DEVICE_ATTR == DEVICE_TABLE_ATTR_METER_PHASE_A_BACK_HAVE_POWER || \
                        yc.DEVICE_ATTR == DEVICE_TABLE_ATTR_METER_POWER_FATOR_A || yc.DEVICE_ATTR == DEVICE_TABLE_ATTR_METER_PHASE_A_ENERGE)
                    {
                        yc.Tag = false;
                    }
                    else if (yc.DEVICE_ATTR == DEVICE_TABLE_ATTR_METER_PHASE_B_VOLT || yc.DEVICE_ATTR == DEVICE_TABLE_ATTR_METER_PHASE_B_ELEC ||    \
                        yc.DEVICE_ATTR == DEVICE_TABLE_ATTR_METER_PHASE_B_FORTH_HAVE_POWER || yc.DEVICE_ATTR == DEVICE_TABLE_ATTR_METER_PHASE_B_BACK_HAVE_POWER || \
                        yc.DEVICE_ATTR == DEVICE_TABLE_ATTR_METER_POWER_FATOR_B || yc.DEVICE_ATTR == DEVICE_TABLE_ATTR_METER_PHASE_B_ENERGE)
                    {
                        yc.Tag = true;
                    }
                    else if (yc.DEVICE_ATTR == DEVICE_TABLE_ATTR_METER_PHASE_C_VOLT || yc.DEVICE_ATTR == DEVICE_TABLE_ATTR_METER_PHASE_C_ELEC ||    \
                        yc.DEVICE_ATTR == DEVICE_TABLE_ATTR_METER_PHASE_C_FORTH_HAVE_POWER || yc.DEVICE_ATTR == DEVICE_TABLE_ATTR_METER_PHASE_C_BACK_HAVE_POWER || \
                        yc.DEVICE_ATTR == DEVICE_TABLE_ATTR_METER_POWER_FATOR_C || yc.DEVICE_ATTR == DEVICE_TABLE_ATTR_METER_PHASE_C_ENERGE)
                    {
                        yc.Tag = false;
                    }
                    else
                        yc.Tag = true;
                    break;
                case 3:
                    if (yc.DEVICE_ATTR == DEVICE_TABLE_ATTR_METER_PHASE_A_VOLT || yc.DEVICE_ATTR == DEVICE_TABLE_ATTR_METER_PHASE_A_ELEC ||    \
                        yc.DEVICE_ATTR == DEVICE_TABLE_ATTR_METER_PHASE_A_FORTH_HAVE_POWER || yc.DEVICE_ATTR == DEVICE_TABLE_ATTR_METER_PHASE_A_BACK_HAVE_POWER || \
                        yc.DEVICE_ATTR == DEVICE_TABLE_ATTR_METER_POWER_FATOR_A || yc.DEVICE_ATTR == DEVICE_TABLE_ATTR_METER_PHASE_A_ENERGE)
                    {
                        yc.Tag = false;
                    }
                    else if (yc.DEVICE_ATTR == DEVICE_TABLE_ATTR_METER_PHASE_B_VOLT || yc.DEVICE_ATTR == DEVICE_TABLE_ATTR_METER_PHASE_B_ELEC ||    \
                        yc.DEVICE_ATTR == DEVICE_TABLE_ATTR_METER_PHASE_B_FORTH_HAVE_POWER || yc.DEVICE_ATTR == DEVICE_TABLE_ATTR_METER_PHASE_B_BACK_HAVE_POWER || \
                        yc.DEVICE_ATTR == DEVICE_TABLE_ATTR_METER_POWER_FATOR_B || yc.DEVICE_ATTR == DEVICE_TABLE_ATTR_METER_PHASE_B_ENERGE)
                    {
                        yc.Tag = false;
                    }
                    else if (yc.DEVICE_ATTR == DEVICE_TABLE_ATTR_METER_PHASE_C_VOLT || yc.DEVICE_ATTR == DEVICE_TABLE_ATTR_METER_PHASE_C_ELEC ||    \
                        yc.DEVICE_ATTR == DEVICE_TABLE_ATTR_METER_PHASE_C_FORTH_HAVE_POWER || yc.DEVICE_ATTR == DEVICE_TABLE_ATTR_METER_PHASE_C_BACK_HAVE_POWER || \
                        yc.DEVICE_ATTR == DEVICE_TABLE_ATTR_METER_POWER_FATOR_C || yc.DEVICE_ATTR == DEVICE_TABLE_ATTR_METER_PHASE_C_ENERGE)
                    {
                        yc.Tag = true;
                    }
                    else
                        yc.Tag = true;
                    break;
                default:
                    break;
            }
        }
    }
}

int PointManager::refreshStandbyPointValue(const std::string &sDatabaseName)
{
    for(int i = 0; i < f_ycPoints.size(); ++i)
    {
        YcPoint &yc = f_ycPoints.at(i);
        if (yc.DATABASE_NAME == sDatabaseName)
        {
            setYcValue(&yc, 0, 0);
        }
    }

    for (int i = 0; i < f_yxPoints.size(); ++i)
    {
        YxPoint &yx = f_yxPoints.at(i);
        if (yx.DATABASE_NAME == sDatabaseName)
        {
            setYxValue(&yx,0, 0);
        }
    }

    return 0;
}

int PointManager::refreshAllPointOldValue()
{
    for (int i = 0; i < f_yxPoints.size(); ++i)
    {
        YxPoint &yx = f_yxPoints.at(i);
        yx.OldVALUE = yx.VALUE;
    }
    for (int i = 0; i < f_ycPoints.size(); ++i)
    {
        YcPoint &yc = f_ycPoints.at(i);
        yc.OldVALUE = yc.VALUE;
    }
    return 0;
}

int PointManager::refreshIRTablePointValue(const std::string &sDatabaseName, const std::string &sTableName,IRTable &irTable)
{
    for (int i = 0; i < f_yxPoints.size(); ++i)
    {
        YxPoint &yx = f_yxPoints.at(i);
        if (yx.DATABASE_NAME == sDatabaseName && yx.TABLE_NAME == sTableName)
        {
            if (yx.DEVICE_ATTR == DEVICE_TABLE_ATTR_IRSTATE){setYxValue(&yx, irTable.IRSTATE, irTable.RECORD_TS);}
            if (yx.DEVICE_ATTR == DEVICE_TABLE_ATTR_IRDELAYTIME) {setYxValue(&yx, irTable.IRDELAYTIME, irTable.RECORD_TS);}
        }
    }

    for (int i = 0; i < f_ycPoints.size(); ++i)
    {
        YcPoint &yc = f_ycPoints.at(i);
        YxPoint &yx = f_yxPoints.at(i);
        if (yc.DATABASE_NAME == sDatabaseName && yc.TABLE_NAME == sTableName)
        {
            if (yc.DEVICE_ATTR == DEVICE_TABLE_ATTR_IRSTATE){setYcValue(&yc, irTable.IRSTATE, irTable.RECORD_TS);}
            if (yc.DEVICE_ATTR == DEVICE_TABLE_ATTR_IRDELAYTIME) {setYcValue(&yc, irTable.IRDELAYTIME, irTable.RECORD_TS);}
        }
    }
    return 0;
}

int PointManager::refreshWATERDEEPTablePointValue(const std::string &sDatabaseName, const std::string &sTableName,WATERDEEPTable &waterdeepTable)
{
    for (int i = 0; i < f_ycPoints.size(); ++i)
    {
        YcPoint &yc = f_ycPoints.at(i);
        if (yc.DATABASE_NAME == sDatabaseName && yc.TABLE_NAME == sTableName)
        {
            if (yc.DEVICE_ATTR == DEVICE_TABLE_ATTR_DEEP){setYcValue(&yc, waterdeepTable.DEEP, waterdeepTable.RECORD_TS);}
        }
    }
    return 0;
}

int PointManager::refreshSENSORDOORTablePointValue(const std::string &sDatabaseName, const std::string &sTableName,SENSORDOORTable &sensordoorTable)
{
    for (int i = 0; i < f_yxPoints.size(); ++i)
    {
        YxPoint &yx = f_yxPoints.at(i);
        if (yx.DATABASE_NAME == sDatabaseName && yx.TABLE_NAME == sTableName)
        {
            if (yx.DEVICE_ATTR == DEVICE_TABLE_ATTR_DOORSTATE){setYxValue(&yx, sensordoorTable.DOORSTATE, sensordoorTable.RECORD_TS);}
        }
    }
    for (int i = 0; i < f_ycPoints.size(); ++i)
    {
        YcPoint &yc = f_ycPoints.at(i);
        if (yc.DATABASE_NAME == sDatabaseName && yc.TABLE_NAME == sTableName)
        {
            if (yc.DEVICE_ATTR == DEVICE_TABLE_ATTR_DOORSTATE){setYcValue(&yc, sensordoorTable.DOORSTATE, sensordoorTable.RECORD_TS);}
        }
    }
    return 0;
}

int PointManager::refreshRELAYPLCTablePointValue(const std::string &sDatabaseName, const std::string &sTableName, SENSORDOORTable &sensordoorTable)
{
    for (int i = 0; i < f_yxPoints.size(); ++i)
    {
        YxPoint &yx = f_yxPoints.at(i);
        if (yx.DATABASE_NAME == sDatabaseName && yx.TABLE_NAME == sTableName)
        {
            if (yx.DEVICE_ATTR == DEVICE_TABLE_ATTR_RELAYPLCSTATE){setYxValue(&yx, sensordoorTable.DOORSTATE, sensordoorTable.RECORD_TS);}
        }
    }
    return 0;
}

int PointManager::refreshBOPOTablePointValue(const std::string &sDatabaseName, const std::string &sTableName, SENSORDOORTable &sensordoorTable)
{
    for (int i = 0; i < f_yxPoints.size(); ++i)
    {
        YxPoint &yx = f_yxPoints.at(i);
        if (yx.DATABASE_NAME == sDatabaseName && yx.TABLE_NAME == sTableName)
        {
            if (yx.DEVICE_ATTR == DEVICE_TABLE_ATTR_BOPOSTATE){setYxValue(&yx, sensordoorTable.DOORSTATE, sensordoorTable.RECORD_TS);}
        }
    }
    return 0;
}

const std::vector<YxPoint> &PointManager::getYxs()
{
    return f_yxPoints;
}

const std::vector<YcPoint> &PointManager::getYcs()
{
    return f_ycPoints;
}

void PointManager::preCheck()
{
    fnDebug().out(".preCheck begin ... ... ... ... ... ... ... ... ...");
    //
    fnDebug().out(".preCheck Check DataBases And Tables Exist ...");
    map<string, sqlite3*> dbs;
    for(map<string, map<string, msepoch_t> >::const_iterator it = f_deviceTables.begin(); it != f_deviceTables.end(); ++it)
    {
        // databases
        const string &dbName = it->first;
        const map<string, msepoch_t> &tbTimes = it->second;
        sqlite3 *db = NULL;
        map<string, sqlite3*>::const_iterator itDb = dbs.find(dbName);
        if (itDb != dbs.end())
        {
            db = itDb->second;
        }
        else
        {
            string sFilePath = DBManager::getDbFilePath(dbName);
            if (CxFileSystem::isExist(sFilePath))
            {
                db = DBManager::openSqlite(sFilePath);
                if (db == NULL)
                {
                    fnError().out(".preCheck DataBase [%s] open ERROR!", dbName.c_str());
                    continue;
                }
                dbs[dbName] = db;
            }
            else
            {
                fnError().out(".preCheck DataBase [%s] do not exist!", dbName.c_str());
                continue;
            }
        }
        // tables
        for (map<string, msepoch_t>::const_iterator it2 = tbTimes.begin(); it2 != tbTimes.end(); ++it2)
        {
            const string &tbName = it2->first;
            if(CxString::beginWith(tbName, "YX_TABLE_tajiao-gz"))
            {
                continue;
            }
            if(CxString::beginWith(tbName, "POWERLOSS_TABLE_"))
            {
                continue;
            }
            if(CxString::beginWith(tbName, "BRANCH_TABLE_"))
            {
                continue;
            }
            if(CxString::beginWith(tbName, "PHASE_BLANCE_TABLE_"))
            {
                continue;
            }
            if(! DBManager::sqliteTableExist(db, (char *)tbName.c_str()))
            {
                fnError().out(".preCheck DataBase [%s] TableName [%s] do not exist!", dbName.c_str(), tbName.c_str());
            }
        }
    }
    //
    fnDebug().out(".preCheck Check YxPoint ModelEventParams ...");
    for (int i = 0; i < f_yxPoints.size(); ++i)
    {
        const YxPoint &yxPoint = f_yxPoints[i];
        const map<string, string> &params = yxPoint.ModelEventParams;
        if (params.size() <= 0)
        {
            fnDebug().out(".preCheck - ModelEventParams [TABLE_NAME: %s] [DEVICE_ATTR: %s] ModelEventParams Is Empty .", yxPoint.TABLE_NAME.c_str(), yxPoint.DEVICE_ATTR.c_str());
            continue;
        }
        int count = 0;
        for (map<string, string>::const_iterator it = params.begin(); it != params.end(); ++it)
        {
            const std::string &attrName = it->first;
            const std::string &modelAttr = it->second;
            if (modelAttr != "value")
            {
                const YcPoint *ycPoint = PointManager::getYcByDeviceModelAttr(yxPoint.DeviceId, modelAttr);
                if (ycPoint == NULL)
                {
                    fnDebug().out(".preCheck - ModelEventParams [MODEL: %s] [TABLE_NAME: %s] [MODEL_ATTR: %s] Can Not Find ModelEventParam [ %s ] .", yxPoint.MODEL.c_str(), yxPoint.TABLE_NAME.c_str(), yxPoint.DEVICE_ATTR.c_str(), modelAttr.c_str());
                }
                else
                {
                    ++count;
                }
            }
            else
            {
                ++count;
            }
        }
        if (count == 0)
        {
            fnDebug().out(".preCheck - ModelEventParams [MODEL: %s] [TABLE_NAME: %s] [DEVICE_ATTR: %s] ModelEventParams - Count Of Valid Param = 0 ", yxPoint.MODEL.c_str(), yxPoint.TABLE_NAME.c_str(), yxPoint.DEVICE_ATTR.c_str());
        }
    }
    fnDebug().out(".preCheck end--- --- --- --- --- --- --- --- ---");
}

msepoch_t PointManager::getRefreshTime(const string &sDbName, const string &sTbName)
{
    map<string, map<string, msepoch_t> >::const_iterator it = f_deviceTables.find(sDbName);
    if (it != f_deviceTables.end())
    {
        const map<string, msepoch_t> &tbTimes = it->second;
        const map<string, msepoch_t>::const_iterator it2 = tbTimes.find(sTbName);
        if (it2 != tbTimes.end())
        {
            return it2->second;
        }
    }
    return 0;
}

void PointManager::setRefreshTime(const string &sDbName, const string &sTbName)
{
    map<string, map<string, msepoch_t> >::iterator it = f_deviceTables.find(sDbName);
    if (it != f_deviceTables.end())
    {
        map<string, msepoch_t> &tbTimes = it->second;
        map<string, msepoch_t>::iterator it2 = tbTimes.find(sTbName);
        if (it2 != tbTimes.end())
        {
            it2->second = CxTime::currentMsepoch();
        }
    }
}

void PointManager::refreshOldValue(const YxPoint *yxPoint)
{
    YxPoint *yx = (YxPoint *)yxPoint;
    yx->OldVALUE = yx->VALUE;
}


typedef struct BB_PvInverter
{
    double		PVINVERTER_SYSSTATE;		//运行状态
    double		PVINVERTER_GENERATIONTIME_TOTAL;		//总发电时间
    double		PVINVERTER_SERVICETIME_TOTAL;		//总运行时间
    double		PVINVERTER_FREQUENCY_GRID;		//电网频率
    double		PVINVERTER_ACTIVEPOWER_OUTPUT_TOTAL;		//总有功功率。放电为正，充电为负
    double		PVINVERTER_REACTIVEPOWER_OUTPUT_TOTAL;		//总无功功率。逆变器端超前为正，滞后为负
    double		PVINVERTER_APPARENTPOWER_OUTPUT_TOTAL;		//总视在功率。放电为正，充电为负
    double		PVINVERTER_ACTIVEPOWER_PCC_TOTAL;		//总PCC有功功率。卖电为正，买电为负
    double		PVINVERTER_REACTIVEPOWER_PCC_TOTAL;		//总PCC无功功率。逆变器端超前为正，滞后为负
    double		PVINVERTER_APPARENTPOWER_PCC_TOTAL;		//总PCC视在功率。卖电为正，买电为负
    double		PVINVERTER_VOLTAGE_PHASE_R;		//R相电网电压
    double		PVINVERTER_CURRENT_OUTPUT_R;		//R相逆变器输出电流
    double		PVINVERTER_ACTIVEPOWER_OUTPUT_R;		//R相逆变器输出有功功率。放电为正，充电为负
    double		PVINVERTER_REACTIVEPOWER_OUTPUT_R;		//R相逆变器输出无功功率。逆变器端超前为正，滞后为负
    double		PVINVERTER_POWERFACTOR_OUTPUT_R;		//R相功率因数。逆变器端超前为正，滞后为负
    double		PVINVERTER_CURRENT_PCC_R;			//R相PCC电流
    double		PVINVERTER_ACTIVEPOWER_PCC_R;		//R相PCC有功功率
    double		PVINVERTER_REACTIVEPOWER_PCC_R;		//R相PCC无功功率。逆变器端超前为正，滞后为负
    double		PVINVERTER_POWERFACTOR_PCC_R;		//R相PCC功率因数。逆变器端超前为正，滞后为负
    double		PVINVERTER_VOLTAGE_PHASE_S;		//S相电网电压
    double		PVINVERTER_CURRENT_OUTPUT_S;		//S相逆变器输出电流
    double		PVINVERTER_ACTIVEPOWER_OUTPUT_S;		//S相逆变器输出有功功率。放电为正，充电为负
    double		PVINVERTER_REACTIVEPOWER_OUTPUT_S;		//S相逆变器输出无功功率。逆变器端超前为正，滞后为负
    double		PVINVERTER_POWERFACTOR_OUTPUT_S;		//S相功率因数。逆变器端超前为正，滞后为负
    double		PVINVERTER_CURRENT_PCC_S;			//S相PCC电流
    double		PVINVERTER_ACTIVEPOWER_PCC_S;		//S相PCC有功功率
    double		PVINVERTER_REACTIVEPOWER_PCC_S;		//S相PCC无功功率。逆变器端超前为正，滞后为负
    double		PVINVERTER_POWERFACTOR_PCC_S;		//S相PCC功率因数。逆变器端超前为正，滞后为负
    double		PVINVERTER_VOLTAGE_PHASE_T;		//T相电网电压
    double		PVINVERTER_CURRENT_OUTPUT_T;		//T相逆变器输出电流
    double		PVINVERTER_ACTIVEPOWER_OUTPUT_T;		//T相逆变器输出有功功率。放电为正，充电为负
    double		PVINVERTER_REACTIVEPOWER_OUTPUT_T;		//T相逆变器输出无功功率。逆变器端超前为正，滞后为负
    double		PVINVERTER_POWERFACTOR_OUTPUT_T;		//T相功率因数。逆变器端超前为正，滞后为负
    double		PVINVERTER_CURRENT_PCC_T;			//T相PCC电流
    double		PVINVERTER_ACTIVEPOWER_PCC_T;		//T相PCC有功功率
    double		PVINVERTER_REACTIVEPOWER_PCC_T;		//T相PCC无功功率。逆变器端超前为正，滞后为负
    double		PVINVERTER_POWERFACTOR_PCC_T;		//T相PCC功率因数。逆变器端超前为正，滞后为负
    double		PVINVERTER_ACTIVEPOWER_PV_EXT;		//外部发电功率
    double		PVINVERTER_ACTIVEPOWER_LOAD_SYS;		//系统总负载功率
    double		PVINVERTER_ACTIVEPOWER_LOAD_TOTAL;		//负载有功功率。负载消耗为正，回馈为负。
    double		PVINVERTER_REACTIVEPOWER_LOAD_TOTAL;		//负载无功功率
    double		PVINVERTER_APPARENTPOWER_LOAD_TOTAL;		//负载视在功率。负载消耗为正，回馈为负。
    double		PVINVERTER_FREQUENCY_OUTPUT;			//输出电压频率
    double		PVINVERTER_VOLTAGE_OUTPUT_R;		//R相逆变器输出电压
    double		PVINVERTER_CURRENT_LOAD_R;		//R相负载电流
    double		PVINVERTER_ACTIVEPOWER_LOAD_R;		//R相负载有功功率。负载消耗为正，回馈为负。
    double		PVINVERTER_REACTIVEPOWER_LOAD_R;		//R相负载无功功率。逆变器端超前为正，滞后为负
    double		PVINVERTER_APPARENTPOWER_LOAD_R;		//R相负载视在功率。负载消耗为正，回馈为负。
    double		PVINVERTER_LOADPEAKRATIO_R;			//R相负载峰值比
    double		PVINVERTER_VOLTAGE_OUTPUT_S;		//S相逆变器输出电压
    double		PVINVERTER_CURRENT_LOAD_S;		//S相负载电流
    double		PVINVERTER_ACTIVEPOWER_LOAD_S;		//S相负载有功功率。负载消耗为正，回馈为负。
    double		PVINVERTER_REACTIVEPOWER_LOAD_S;		//S相负载无功功率。逆变器端超前为正，滞后为负
    double		PVINVERTER_APPARENTPOWER_LOAD_S;		//S相负载视在功率。负载消耗为正，回馈为负。
    double		PVINVERTER_LOADPEAKRATIO_S;			//S相负载峰值比
    double		PVINVERTER_VOLTAGE_OUTPUT_T;		//T相逆变器输出电压
    double		PVINVERTER_CURRENT_LOAD_T;		//T相负载电流
    double		PVINVERTER_ACTIVEPOWER_LOAD_T;		//T相负载有功功率。负载消耗为正，回馈为负。
    double		PVINVERTER_REACTIVEPOWER_LOAD_T;		//T相负载无功功率。逆变器端超前为正，滞后为负
    double		PVINVERTER_APPARENTPOWER_LOAD_T;		//T相负载视在功率。负载消耗为正，回馈为负。
    double		PVINVERTER_LOADPEAKRATIO_T;			//T相负载峰值比
    double		PVINVERTER_VOLTAGE_PV1;			//第1路PV电压
    double		PVINVERTER_CURRENT_PV1;			//第1路PV电流
    double		PVINVERTER_POWER_PV1;			//第1路PV功率
    double		PVINVERTER_VOLTAGE_PV2;			//第2路PV电压
    double		PVINVERTER_CURRENT_PV2;			//第2路PV电流
    double		PVINVERTER_POWER_PV2;			//第2路PV功率
    double		PVINVERTER_VOLTAGE_PV3;			//第3路PV电压
    double		PVINVERTER_CURRENT_PV3;			//第3路PV电流
    double		PVINVERTER_POWER_PV3;			//第3路PV功率
    double		PVINVERTER_VOLTAGE_PV4;			//第4路PV电压
    double		PVINVERTER_CURRENT_PV4;			//第4路PV电流
    double		PVINVERTER_POWER_PV4;			//第4路PV功率
    double		PVINVERTER_VOLTAGE_PV5;			//第5路PV电压
    double		PVINVERTER_CURRENT_PV5;			//第5路PV电流
    double		PVINVERTER_POWER_PV5;			//第5路PV功率
    double		PVINVERTER_VOLTAGE_PV6;			//第6路PV电压
    double		PVINVERTER_CURRENT_PV6;			//第6路PV电流
    double		PVINVERTER_POWER_PV6;			//第6路PV功率
    double		PVINVERTER_VOLTAGE_PV7;			//第7路PV电压
    double		PVINVERTER_CURRENT_PV7;			//第7路PV电流
    double		PVINVERTER_POWER_PV7;			//第7路PV功率
    double		PVINVERTER_VOLTAGE_PV8;			//第8路PV电压
    double		PVINVERTER_CURRENT_PV8;			//第8路PV电流
    double		PVINVERTER_POWER_PV8;			//第8路PV功率
    double		PVINVERTER_VOLTAGE_PV9;			//第9路PV电压
    double		PVINVERTER_CURRENT_PV9;			//第9路PV电流
    double		PVINVERTER_POWER_PV9;			//第9路PV功率
    double		PVINVERTER_VOLTAGE_PV10;			//第10路PV电压
    double		PVINVERTER_CURRENT_PV10;			//第10路PV电流
    double		PVINVERTER_POWER_PV10;			//第10路PV功率
    double		PVINVERTER_VOLTAGE_PV11;			//第11路PV电压
    double		PVINVERTER_CURRENT_PV11;			//第11路PV电流
    double		PVINVERTER_POWER_PV11;			//第11路PV功率
    double		PVINVERTER_VOLTAGE_PV12;			//第12路PV电压
    double		PVINVERTER_CURRENT_PV12;			//第12路PV电流
    double		PVINVERTER_POWER_PV12;			//第12路PV功率
    double		PVINVERTER_VOLTAGE_PV13;			//第13路PV电压
    double		PVINVERTER_CURRENT_PV13;			//第13路PV电流
    double		PVINVERTER_POWER_PV13;			//第13路PV功率
    double		PVINVERTER_VOLTAGE_PV14;			//第14路PV电压
    double		PVINVERTER_CURRENT_PV14;			//第14路PV电流
    double		PVINVERTER_POWER_PV14;			//第14路PV功率
    double		PVINVERTER_VOLTAGE_PV15;			//第15路PV电压
    double		PVINVERTER_CURRENT_PV15;			//第15路PV电流
    double		PVINVERTER_POWER_PV15;			//第15路PV功率
    double		PVINVERTER_VOLTAGE_PV16;			//第16路PV电压
    double		PVINVERTER_CURRENT_PV16;			//第16路PV电流
    double		PVINVERTER_POWER_PV16;			//第16路PV功率
    double		PVINVERTER_GENERATIONTIME_TODAY;		//当日发电时间
    double			PVINVERTER_PV_GENERATION_TODAY;		//当日发电量
    double			PVINVERTER_PV_GENERATION_TOTAL;		//总发电量
    double			PVINVERTER_LOAD_CONSUMPTION_TODAY;		//当日负载耗电量
    double			PVINVERTER_LOAD_CONSUMPTION_TOTAL;		//总负载耗电量
    double			PVINVERTER_ENERGY_PURCHASE_TODAY;		//当日买电量
    double			PVINVERTER_ENERGY_PURCHASE_TOTAL;		//总买电量
    double			PVINVERTER_ENERGY_SELLING_TODAY;		//当日卖电量
    double			PVINVERTER_ENERGY_SELLING_TOTAL;		//总卖电量
    double		PVINVERTER_REMOTE_ON_OFF_CONTROL;		//远程开关机。	0x0000：关机	0x0001：开机
} BBPvInverter;

static BBPvInverter f_bbPvInverter;

int PointManager::refreshPVINVERTERTablePointValue(const string &sDatabaseName, const string &sTableName,
                                                   PVINVERTERTable &pvInverterTable)
{
    for (int i = 0; i < f_ycPoints.size(); ++i)
    {
        YcPoint &yc = f_ycPoints.at(i);
        if (yc.DATABASE_NAME == sDatabaseName && yc.TABLE_NAME == sTableName)
        {
            if (yc.DEVICE_ATTR == PVINVERTER_TABLE_ATTR_PVINVERTER_SYSSTATE){setYcValue(&yc, pvInverterTable.PVINVERTER_SYSSTATE * f_bbPvInverter.PVINVERTER_SYSSTATE, pvInverterTable.RECORD_TS);}
            if (yc.DEVICE_ATTR == PVINVERTER_TABLE_ATTR_PVINVERTER_GENERATIONTIME_TOTAL){setYcValue(&yc, pvInverterTable.PVINVERTER_GENERATIONTIME_TOTAL * f_bbPvInverter.PVINVERTER_GENERATIONTIME_TOTAL, pvInverterTable.RECORD_TS);}
            if (yc.DEVICE_ATTR == PVINVERTER_TABLE_ATTR_PVINVERTER_SERVICETIME_TOTAL){setYcValue(&yc, pvInverterTable.PVINVERTER_SERVICETIME_TOTAL * f_bbPvInverter.PVINVERTER_SERVICETIME_TOTAL, pvInverterTable.RECORD_TS);}
            if (yc.DEVICE_ATTR == PVINVERTER_TABLE_ATTR_PVINVERTER_FREQUENCY_GRID){setYcValue(&yc, pvInverterTable.PVINVERTER_FREQUENCY_GRID * f_bbPvInverter.PVINVERTER_FREQUENCY_GRID, pvInverterTable.RECORD_TS);}
            if (yc.DEVICE_ATTR == PVINVERTER_TABLE_ATTR_PVINVERTER_ACTIVEPOWER_OUTPUT_TOTAL){setYcValue(&yc, pvInverterTable.PVINVERTER_ACTIVEPOWER_OUTPUT_TOTAL * f_bbPvInverter.PVINVERTER_ACTIVEPOWER_OUTPUT_TOTAL, pvInverterTable.RECORD_TS);}
            if (yc.DEVICE_ATTR == PVINVERTER_TABLE_ATTR_PVINVERTER_REACTIVEPOWER_OUTPUT_TOTAL){setYcValue(&yc, pvInverterTable.PVINVERTER_REACTIVEPOWER_OUTPUT_TOTAL * f_bbPvInverter.PVINVERTER_REACTIVEPOWER_OUTPUT_TOTAL, pvInverterTable.RECORD_TS);}
            if (yc.DEVICE_ATTR == PVINVERTER_TABLE_ATTR_PVINVERTER_APPARENTPOWER_OUTPUT_TOTAL){setYcValue(&yc, pvInverterTable.PVINVERTER_APPARENTPOWER_OUTPUT_TOTAL * f_bbPvInverter.PVINVERTER_APPARENTPOWER_OUTPUT_TOTAL, pvInverterTable.RECORD_TS);}
            if (yc.DEVICE_ATTR == PVINVERTER_TABLE_ATTR_PVINVERTER_ACTIVEPOWER_PCC_TOTAL){setYcValue(&yc, pvInverterTable.PVINVERTER_ACTIVEPOWER_PCC_TOTAL * f_bbPvInverter.PVINVERTER_ACTIVEPOWER_PCC_TOTAL, pvInverterTable.RECORD_TS);}
            if (yc.DEVICE_ATTR == PVINVERTER_TABLE_ATTR_PVINVERTER_REACTIVEPOWER_PCC_TOTAL){setYcValue(&yc, pvInverterTable.PVINVERTER_REACTIVEPOWER_PCC_TOTAL * f_bbPvInverter.PVINVERTER_REACTIVEPOWER_PCC_TOTAL, pvInverterTable.RECORD_TS);}
            if (yc.DEVICE_ATTR == PVINVERTER_TABLE_ATTR_PVINVERTER_APPARENTPOWER_PCC_TOTAL){setYcValue(&yc, pvInverterTable.PVINVERTER_APPARENTPOWER_PCC_TOTAL * f_bbPvInverter.PVINVERTER_APPARENTPOWER_PCC_TOTAL, pvInverterTable.RECORD_TS);}
            if (yc.DEVICE_ATTR == PVINVERTER_TABLE_ATTR_PVINVERTER_VOLTAGE_PHASE_R){setYcValue(&yc, pvInverterTable.PVINVERTER_VOLTAGE_PHASE_R * f_bbPvInverter.PVINVERTER_VOLTAGE_PHASE_R, pvInverterTable.RECORD_TS);}
            if (yc.DEVICE_ATTR == PVINVERTER_TABLE_ATTR_PVINVERTER_CURRENT_OUTPUT_R){setYcValue(&yc, pvInverterTable.PVINVERTER_CURRENT_OUTPUT_R * f_bbPvInverter.PVINVERTER_CURRENT_OUTPUT_R, pvInverterTable.RECORD_TS);}
            if (yc.DEVICE_ATTR == PVINVERTER_TABLE_ATTR_PVINVERTER_ACTIVEPOWER_OUTPUT_R){setYcValue(&yc, pvInverterTable.PVINVERTER_ACTIVEPOWER_OUTPUT_R * f_bbPvInverter.PVINVERTER_ACTIVEPOWER_OUTPUT_R, pvInverterTable.RECORD_TS);}
            if (yc.DEVICE_ATTR == PVINVERTER_TABLE_ATTR_PVINVERTER_REACTIVEPOWER_OUTPUT_R){setYcValue(&yc, pvInverterTable.PVINVERTER_REACTIVEPOWER_OUTPUT_R * f_bbPvInverter.PVINVERTER_REACTIVEPOWER_OUTPUT_R, pvInverterTable.RECORD_TS);}
            if (yc.DEVICE_ATTR == PVINVERTER_TABLE_ATTR_PVINVERTER_POWERFACTOR_OUTPUT_R){setYcValue(&yc, pvInverterTable.PVINVERTER_POWERFACTOR_OUTPUT_R * f_bbPvInverter.PVINVERTER_POWERFACTOR_OUTPUT_R, pvInverterTable.RECORD_TS);}
            if (yc.DEVICE_ATTR == PVINVERTER_TABLE_ATTR_PVINVERTER_CURRENT_PCC_R){setYcValue(&yc, pvInverterTable.PVINVERTER_CURRENT_PCC_R * f_bbPvInverter.PVINVERTER_CURRENT_PCC_R, pvInverterTable.RECORD_TS);}
            if (yc.DEVICE_ATTR == PVINVERTER_TABLE_ATTR_PVINVERTER_ACTIVEPOWER_PCC_R){setYcValue(&yc, pvInverterTable.PVINVERTER_ACTIVEPOWER_PCC_R * f_bbPvInverter.PVINVERTER_ACTIVEPOWER_PCC_R, pvInverterTable.RECORD_TS);}
            if (yc.DEVICE_ATTR == PVINVERTER_TABLE_ATTR_PVINVERTER_REACTIVEPOWER_PCC_R){setYcValue(&yc, pvInverterTable.PVINVERTER_REACTIVEPOWER_PCC_R * f_bbPvInverter.PVINVERTER_REACTIVEPOWER_PCC_R, pvInverterTable.RECORD_TS);}
            if (yc.DEVICE_ATTR == PVINVERTER_TABLE_ATTR_PVINVERTER_POWERFACTOR_PCC_R){setYcValue(&yc, pvInverterTable.PVINVERTER_POWERFACTOR_PCC_R * f_bbPvInverter.PVINVERTER_POWERFACTOR_PCC_R, pvInverterTable.RECORD_TS);}
            if (yc.DEVICE_ATTR == PVINVERTER_TABLE_ATTR_PVINVERTER_VOLTAGE_PHASE_S){setYcValue(&yc, pvInverterTable.PVINVERTER_VOLTAGE_PHASE_S * f_bbPvInverter.PVINVERTER_VOLTAGE_PHASE_S, pvInverterTable.RECORD_TS);}
            if (yc.DEVICE_ATTR == PVINVERTER_TABLE_ATTR_PVINVERTER_CURRENT_OUTPUT_S){setYcValue(&yc, pvInverterTable.PVINVERTER_CURRENT_OUTPUT_S * f_bbPvInverter.PVINVERTER_CURRENT_OUTPUT_S, pvInverterTable.RECORD_TS);}
            if (yc.DEVICE_ATTR == PVINVERTER_TABLE_ATTR_PVINVERTER_ACTIVEPOWER_OUTPUT_S){setYcValue(&yc, pvInverterTable.PVINVERTER_ACTIVEPOWER_OUTPUT_S * f_bbPvInverter.PVINVERTER_ACTIVEPOWER_OUTPUT_S, pvInverterTable.RECORD_TS);}
            if (yc.DEVICE_ATTR == PVINVERTER_TABLE_ATTR_PVINVERTER_REACTIVEPOWER_OUTPUT_S){setYcValue(&yc, pvInverterTable.PVINVERTER_REACTIVEPOWER_OUTPUT_S * f_bbPvInverter.PVINVERTER_REACTIVEPOWER_OUTPUT_S, pvInverterTable.RECORD_TS);}
            if (yc.DEVICE_ATTR == PVINVERTER_TABLE_ATTR_PVINVERTER_POWERFACTOR_OUTPUT_S){setYcValue(&yc, pvInverterTable.PVINVERTER_POWERFACTOR_OUTPUT_S * f_bbPvInverter.PVINVERTER_POWERFACTOR_OUTPUT_S, pvInverterTable.RECORD_TS);}
            if (yc.DEVICE_ATTR == PVINVERTER_TABLE_ATTR_PVINVERTER_CURRENT_PCC_S){setYcValue(&yc, pvInverterTable.PVINVERTER_CURRENT_PCC_S * f_bbPvInverter.PVINVERTER_CURRENT_PCC_S, pvInverterTable.RECORD_TS);}
            if (yc.DEVICE_ATTR == PVINVERTER_TABLE_ATTR_PVINVERTER_ACTIVEPOWER_PCC_S){setYcValue(&yc, pvInverterTable.PVINVERTER_ACTIVEPOWER_PCC_S * f_bbPvInverter.PVINVERTER_ACTIVEPOWER_PCC_S, pvInverterTable.RECORD_TS);}
            if (yc.DEVICE_ATTR == PVINVERTER_TABLE_ATTR_PVINVERTER_REACTIVEPOWER_PCC_S){setYcValue(&yc, pvInverterTable.PVINVERTER_REACTIVEPOWER_PCC_S * f_bbPvInverter.PVINVERTER_REACTIVEPOWER_PCC_S, pvInverterTable.RECORD_TS);}
            if (yc.DEVICE_ATTR == PVINVERTER_TABLE_ATTR_PVINVERTER_POWERFACTOR_PCC_S){setYcValue(&yc, pvInverterTable.PVINVERTER_POWERFACTOR_PCC_S * f_bbPvInverter.PVINVERTER_POWERFACTOR_PCC_S, pvInverterTable.RECORD_TS);}
            if (yc.DEVICE_ATTR == PVINVERTER_TABLE_ATTR_PVINVERTER_VOLTAGE_PHASE_T){setYcValue(&yc, pvInverterTable.PVINVERTER_VOLTAGE_PHASE_T * f_bbPvInverter.PVINVERTER_VOLTAGE_PHASE_T, pvInverterTable.RECORD_TS);}
            if (yc.DEVICE_ATTR == PVINVERTER_TABLE_ATTR_PVINVERTER_CURRENT_OUTPUT_T){setYcValue(&yc, pvInverterTable.PVINVERTER_CURRENT_OUTPUT_T * f_bbPvInverter.PVINVERTER_CURRENT_OUTPUT_T, pvInverterTable.RECORD_TS);}
            if (yc.DEVICE_ATTR == PVINVERTER_TABLE_ATTR_PVINVERTER_ACTIVEPOWER_OUTPUT_T){setYcValue(&yc, pvInverterTable.PVINVERTER_ACTIVEPOWER_OUTPUT_T * f_bbPvInverter.PVINVERTER_ACTIVEPOWER_OUTPUT_T, pvInverterTable.RECORD_TS);}
            if (yc.DEVICE_ATTR == PVINVERTER_TABLE_ATTR_PVINVERTER_REACTIVEPOWER_OUTPUT_T){setYcValue(&yc, pvInverterTable.PVINVERTER_REACTIVEPOWER_OUTPUT_T * f_bbPvInverter.PVINVERTER_REACTIVEPOWER_OUTPUT_T, pvInverterTable.RECORD_TS);}
            if (yc.DEVICE_ATTR == PVINVERTER_TABLE_ATTR_PVINVERTER_POWERFACTOR_OUTPUT_T){setYcValue(&yc, pvInverterTable.PVINVERTER_POWERFACTOR_OUTPUT_T * f_bbPvInverter.PVINVERTER_POWERFACTOR_OUTPUT_T, pvInverterTable.RECORD_TS);}
            if (yc.DEVICE_ATTR == PVINVERTER_TABLE_ATTR_PVINVERTER_CURRENT_PCC_T){setYcValue(&yc, pvInverterTable.PVINVERTER_CURRENT_PCC_T * f_bbPvInverter.PVINVERTER_CURRENT_PCC_T, pvInverterTable.RECORD_TS);}
            if (yc.DEVICE_ATTR == PVINVERTER_TABLE_ATTR_PVINVERTER_ACTIVEPOWER_PCC_T){setYcValue(&yc, pvInverterTable.PVINVERTER_ACTIVEPOWER_PCC_T * f_bbPvInverter.PVINVERTER_ACTIVEPOWER_PCC_T, pvInverterTable.RECORD_TS);}
            if (yc.DEVICE_ATTR == PVINVERTER_TABLE_ATTR_PVINVERTER_REACTIVEPOWER_PCC_T){setYcValue(&yc, pvInverterTable.PVINVERTER_REACTIVEPOWER_PCC_T * f_bbPvInverter.PVINVERTER_REACTIVEPOWER_PCC_T, pvInverterTable.RECORD_TS);}
            if (yc.DEVICE_ATTR == PVINVERTER_TABLE_ATTR_PVINVERTER_POWERFACTOR_PCC_T){setYcValue(&yc, pvInverterTable.PVINVERTER_POWERFACTOR_PCC_T * f_bbPvInverter.PVINVERTER_POWERFACTOR_PCC_T, pvInverterTable.RECORD_TS);}
            if (yc.DEVICE_ATTR == PVINVERTER_TABLE_ATTR_PVINVERTER_ACTIVEPOWER_PV_EXT){setYcValue(&yc, pvInverterTable.PVINVERTER_ACTIVEPOWER_PV_EXT * f_bbPvInverter.PVINVERTER_ACTIVEPOWER_PV_EXT, pvInverterTable.RECORD_TS);}
            if (yc.DEVICE_ATTR == PVINVERTER_TABLE_ATTR_PVINVERTER_ACTIVEPOWER_LOAD_SYS){setYcValue(&yc, pvInverterTable.PVINVERTER_ACTIVEPOWER_LOAD_SYS * f_bbPvInverter.PVINVERTER_ACTIVEPOWER_LOAD_SYS, pvInverterTable.RECORD_TS);}
            if (yc.DEVICE_ATTR == PVINVERTER_TABLE_ATTR_PVINVERTER_ACTIVEPOWER_LOAD_TOTAL){setYcValue(&yc, pvInverterTable.PVINVERTER_ACTIVEPOWER_LOAD_TOTAL * f_bbPvInverter.PVINVERTER_ACTIVEPOWER_LOAD_TOTAL, pvInverterTable.RECORD_TS);}
            if (yc.DEVICE_ATTR == PVINVERTER_TABLE_ATTR_PVINVERTER_REACTIVEPOWER_LOAD_TOTAL){setYcValue(&yc, pvInverterTable.PVINVERTER_REACTIVEPOWER_LOAD_TOTAL * f_bbPvInverter.PVINVERTER_REACTIVEPOWER_LOAD_TOTAL, pvInverterTable.RECORD_TS);}
            if (yc.DEVICE_ATTR == PVINVERTER_TABLE_ATTR_PVINVERTER_APPARENTPOWER_LOAD_TOTAL){setYcValue(&yc, pvInverterTable.PVINVERTER_APPARENTPOWER_LOAD_TOTAL * f_bbPvInverter.PVINVERTER_APPARENTPOWER_LOAD_TOTAL, pvInverterTable.RECORD_TS);}
            if (yc.DEVICE_ATTR == PVINVERTER_TABLE_ATTR_PVINVERTER_FREQUENCY_OUTPUT){setYcValue(&yc, pvInverterTable.PVINVERTER_FREQUENCY_OUTPUT * f_bbPvInverter.PVINVERTER_FREQUENCY_OUTPUT, pvInverterTable.RECORD_TS);}
            if (yc.DEVICE_ATTR == PVINVERTER_TABLE_ATTR_PVINVERTER_VOLTAGE_OUTPUT_R){setYcValue(&yc, pvInverterTable.PVINVERTER_VOLTAGE_OUTPUT_R * f_bbPvInverter.PVINVERTER_VOLTAGE_OUTPUT_R, pvInverterTable.RECORD_TS);}
            if (yc.DEVICE_ATTR == PVINVERTER_TABLE_ATTR_PVINVERTER_CURRENT_LOAD_R){setYcValue(&yc, pvInverterTable.PVINVERTER_CURRENT_LOAD_R * f_bbPvInverter.PVINVERTER_CURRENT_LOAD_R, pvInverterTable.RECORD_TS);}
            if (yc.DEVICE_ATTR == PVINVERTER_TABLE_ATTR_PVINVERTER_ACTIVEPOWER_LOAD_R){setYcValue(&yc, pvInverterTable.PVINVERTER_ACTIVEPOWER_LOAD_R * f_bbPvInverter.PVINVERTER_ACTIVEPOWER_LOAD_R, pvInverterTable.RECORD_TS);}
            if (yc.DEVICE_ATTR == PVINVERTER_TABLE_ATTR_PVINVERTER_REACTIVEPOWER_LOAD_R){setYcValue(&yc, pvInverterTable.PVINVERTER_REACTIVEPOWER_LOAD_R * f_bbPvInverter.PVINVERTER_REACTIVEPOWER_LOAD_R, pvInverterTable.RECORD_TS);}
            if (yc.DEVICE_ATTR == PVINVERTER_TABLE_ATTR_PVINVERTER_APPARENTPOWER_LOAD_R){setYcValue(&yc, pvInverterTable.PVINVERTER_APPARENTPOWER_LOAD_R * f_bbPvInverter.PVINVERTER_APPARENTPOWER_LOAD_R, pvInverterTable.RECORD_TS);}
            if (yc.DEVICE_ATTR == PVINVERTER_TABLE_ATTR_PVINVERTER_LOADPEAKRATIO_R){setYcValue(&yc, pvInverterTable.PVINVERTER_LOADPEAKRATIO_R * f_bbPvInverter.PVINVERTER_LOADPEAKRATIO_R, pvInverterTable.RECORD_TS);}
            if (yc.DEVICE_ATTR == PVINVERTER_TABLE_ATTR_PVINVERTER_VOLTAGE_OUTPUT_S){setYcValue(&yc, pvInverterTable.PVINVERTER_VOLTAGE_OUTPUT_S * f_bbPvInverter.PVINVERTER_VOLTAGE_OUTPUT_S, pvInverterTable.RECORD_TS);}
            if (yc.DEVICE_ATTR == PVINVERTER_TABLE_ATTR_PVINVERTER_CURRENT_LOAD_S){setYcValue(&yc, pvInverterTable.PVINVERTER_CURRENT_LOAD_S * f_bbPvInverter.PVINVERTER_CURRENT_LOAD_S, pvInverterTable.RECORD_TS);}
            if (yc.DEVICE_ATTR == PVINVERTER_TABLE_ATTR_PVINVERTER_ACTIVEPOWER_LOAD_S){setYcValue(&yc, pvInverterTable.PVINVERTER_ACTIVEPOWER_LOAD_S * f_bbPvInverter.PVINVERTER_ACTIVEPOWER_LOAD_S, pvInverterTable.RECORD_TS);}
            if (yc.DEVICE_ATTR == PVINVERTER_TABLE_ATTR_PVINVERTER_REACTIVEPOWER_LOAD_S){setYcValue(&yc, pvInverterTable.PVINVERTER_REACTIVEPOWER_LOAD_S * f_bbPvInverter.PVINVERTER_REACTIVEPOWER_LOAD_S, pvInverterTable.RECORD_TS);}
            if (yc.DEVICE_ATTR == PVINVERTER_TABLE_ATTR_PVINVERTER_APPARENTPOWER_LOAD_S){setYcValue(&yc, pvInverterTable.PVINVERTER_APPARENTPOWER_LOAD_S * f_bbPvInverter.PVINVERTER_APPARENTPOWER_LOAD_S, pvInverterTable.RECORD_TS);}
            if (yc.DEVICE_ATTR == PVINVERTER_TABLE_ATTR_PVINVERTER_LOADPEAKRATIO_S){setYcValue(&yc, pvInverterTable.PVINVERTER_LOADPEAKRATIO_S * f_bbPvInverter.PVINVERTER_LOADPEAKRATIO_S, pvInverterTable.RECORD_TS);}
            if (yc.DEVICE_ATTR == PVINVERTER_TABLE_ATTR_PVINVERTER_VOLTAGE_OUTPUT_T){setYcValue(&yc, pvInverterTable.PVINVERTER_VOLTAGE_OUTPUT_T * f_bbPvInverter.PVINVERTER_VOLTAGE_OUTPUT_T, pvInverterTable.RECORD_TS);}
            if (yc.DEVICE_ATTR == PVINVERTER_TABLE_ATTR_PVINVERTER_CURRENT_LOAD_T){setYcValue(&yc, pvInverterTable.PVINVERTER_CURRENT_LOAD_T * f_bbPvInverter.PVINVERTER_CURRENT_LOAD_T, pvInverterTable.RECORD_TS);}
            if (yc.DEVICE_ATTR == PVINVERTER_TABLE_ATTR_PVINVERTER_ACTIVEPOWER_LOAD_T){setYcValue(&yc, pvInverterTable.PVINVERTER_ACTIVEPOWER_LOAD_T * f_bbPvInverter.PVINVERTER_ACTIVEPOWER_LOAD_T, pvInverterTable.RECORD_TS);}
            if (yc.DEVICE_ATTR == PVINVERTER_TABLE_ATTR_PVINVERTER_REACTIVEPOWER_LOAD_T){setYcValue(&yc, pvInverterTable.PVINVERTER_REACTIVEPOWER_LOAD_T * f_bbPvInverter.PVINVERTER_REACTIVEPOWER_LOAD_T, pvInverterTable.RECORD_TS);}
            if (yc.DEVICE_ATTR == PVINVERTER_TABLE_ATTR_PVINVERTER_APPARENTPOWER_LOAD_T){setYcValue(&yc, pvInverterTable.PVINVERTER_APPARENTPOWER_LOAD_T * f_bbPvInverter.PVINVERTER_APPARENTPOWER_LOAD_T, pvInverterTable.RECORD_TS);}
            if (yc.DEVICE_ATTR == PVINVERTER_TABLE_ATTR_PVINVERTER_LOADPEAKRATIO_T){setYcValue(&yc, pvInverterTable.PVINVERTER_LOADPEAKRATIO_T * f_bbPvInverter.PVINVERTER_LOADPEAKRATIO_T, pvInverterTable.RECORD_TS);}
            if (yc.DEVICE_ATTR == PVINVERTER_TABLE_ATTR_PVINVERTER_VOLTAGE_PV1){setYcValue(&yc, pvInverterTable.PVINVERTER_VOLTAGE_PV1 * f_bbPvInverter.PVINVERTER_VOLTAGE_PV1, pvInverterTable.RECORD_TS);}
            if (yc.DEVICE_ATTR == PVINVERTER_TABLE_ATTR_PVINVERTER_CURRENT_PV1){setYcValue(&yc, pvInverterTable.PVINVERTER_CURRENT_PV1 * f_bbPvInverter.PVINVERTER_CURRENT_PV1, pvInverterTable.RECORD_TS);}
            if (yc.DEVICE_ATTR == PVINVERTER_TABLE_ATTR_PVINVERTER_POWER_PV1){setYcValue(&yc, pvInverterTable.PVINVERTER_POWER_PV1 * f_bbPvInverter.PVINVERTER_POWER_PV1, pvInverterTable.RECORD_TS);}
            if (yc.DEVICE_ATTR == PVINVERTER_TABLE_ATTR_PVINVERTER_VOLTAGE_PV2){setYcValue(&yc, pvInverterTable.PVINVERTER_VOLTAGE_PV2 * f_bbPvInverter.PVINVERTER_VOLTAGE_PV2, pvInverterTable.RECORD_TS);}
            if (yc.DEVICE_ATTR == PVINVERTER_TABLE_ATTR_PVINVERTER_CURRENT_PV2){setYcValue(&yc, pvInverterTable.PVINVERTER_CURRENT_PV2 * f_bbPvInverter.PVINVERTER_CURRENT_PV2, pvInverterTable.RECORD_TS);}
            if (yc.DEVICE_ATTR == PVINVERTER_TABLE_ATTR_PVINVERTER_POWER_PV2){setYcValue(&yc, pvInverterTable.PVINVERTER_POWER_PV2 * f_bbPvInverter.PVINVERTER_POWER_PV2, pvInverterTable.RECORD_TS);}
            if (yc.DEVICE_ATTR == PVINVERTER_TABLE_ATTR_PVINVERTER_VOLTAGE_PV3){setYcValue(&yc, pvInverterTable.PVINVERTER_VOLTAGE_PV3 * f_bbPvInverter.PVINVERTER_VOLTAGE_PV3, pvInverterTable.RECORD_TS);}
            if (yc.DEVICE_ATTR == PVINVERTER_TABLE_ATTR_PVINVERTER_CURRENT_PV3){setYcValue(&yc, pvInverterTable.PVINVERTER_CURRENT_PV3 * f_bbPvInverter.PVINVERTER_CURRENT_PV3, pvInverterTable.RECORD_TS);}
            if (yc.DEVICE_ATTR == PVINVERTER_TABLE_ATTR_PVINVERTER_POWER_PV3){setYcValue(&yc, pvInverterTable.PVINVERTER_POWER_PV3 * f_bbPvInverter.PVINVERTER_POWER_PV3, pvInverterTable.RECORD_TS);}
            if (yc.DEVICE_ATTR == PVINVERTER_TABLE_ATTR_PVINVERTER_VOLTAGE_PV4){setYcValue(&yc, pvInverterTable.PVINVERTER_VOLTAGE_PV4 * f_bbPvInverter.PVINVERTER_VOLTAGE_PV4, pvInverterTable.RECORD_TS);}
            if (yc.DEVICE_ATTR == PVINVERTER_TABLE_ATTR_PVINVERTER_CURRENT_PV4){setYcValue(&yc, pvInverterTable.PVINVERTER_CURRENT_PV4 * f_bbPvInverter.PVINVERTER_CURRENT_PV4, pvInverterTable.RECORD_TS);}
            if (yc.DEVICE_ATTR == PVINVERTER_TABLE_ATTR_PVINVERTER_POWER_PV4){setYcValue(&yc, pvInverterTable.PVINVERTER_POWER_PV4 * f_bbPvInverter.PVINVERTER_POWER_PV4, pvInverterTable.RECORD_TS);}
            if (yc.DEVICE_ATTR == PVINVERTER_TABLE_ATTR_PVINVERTER_VOLTAGE_PV5){setYcValue(&yc, pvInverterTable.PVINVERTER_VOLTAGE_PV5 * f_bbPvInverter.PVINVERTER_VOLTAGE_PV5, pvInverterTable.RECORD_TS);}
            if (yc.DEVICE_ATTR == PVINVERTER_TABLE_ATTR_PVINVERTER_CURRENT_PV5){setYcValue(&yc, pvInverterTable.PVINVERTER_CURRENT_PV5 * f_bbPvInverter.PVINVERTER_CURRENT_PV5, pvInverterTable.RECORD_TS);}
            if (yc.DEVICE_ATTR == PVINVERTER_TABLE_ATTR_PVINVERTER_POWER_PV5){setYcValue(&yc, pvInverterTable.PVINVERTER_POWER_PV5 * f_bbPvInverter.PVINVERTER_POWER_PV5, pvInverterTable.RECORD_TS);}
            if (yc.DEVICE_ATTR == PVINVERTER_TABLE_ATTR_PVINVERTER_VOLTAGE_PV6){setYcValue(&yc, pvInverterTable.PVINVERTER_VOLTAGE_PV6 * f_bbPvInverter.PVINVERTER_VOLTAGE_PV6, pvInverterTable.RECORD_TS);}
            if (yc.DEVICE_ATTR == PVINVERTER_TABLE_ATTR_PVINVERTER_CURRENT_PV6){setYcValue(&yc, pvInverterTable.PVINVERTER_CURRENT_PV6 * f_bbPvInverter.PVINVERTER_CURRENT_PV6, pvInverterTable.RECORD_TS);}
            if (yc.DEVICE_ATTR == PVINVERTER_TABLE_ATTR_PVINVERTER_POWER_PV6){setYcValue(&yc, pvInverterTable.PVINVERTER_POWER_PV6 * f_bbPvInverter.PVINVERTER_POWER_PV6, pvInverterTable.RECORD_TS);}
            if (yc.DEVICE_ATTR == PVINVERTER_TABLE_ATTR_PVINVERTER_VOLTAGE_PV7){setYcValue(&yc, pvInverterTable.PVINVERTER_VOLTAGE_PV7 * f_bbPvInverter.PVINVERTER_VOLTAGE_PV7, pvInverterTable.RECORD_TS);}
            if (yc.DEVICE_ATTR == PVINVERTER_TABLE_ATTR_PVINVERTER_CURRENT_PV7){setYcValue(&yc, pvInverterTable.PVINVERTER_CURRENT_PV7 * f_bbPvInverter.PVINVERTER_CURRENT_PV7, pvInverterTable.RECORD_TS);}
            if (yc.DEVICE_ATTR == PVINVERTER_TABLE_ATTR_PVINVERTER_POWER_PV7){setYcValue(&yc, pvInverterTable.PVINVERTER_POWER_PV7 * f_bbPvInverter.PVINVERTER_POWER_PV7, pvInverterTable.RECORD_TS);}
            if (yc.DEVICE_ATTR == PVINVERTER_TABLE_ATTR_PVINVERTER_VOLTAGE_PV8){setYcValue(&yc, pvInverterTable.PVINVERTER_VOLTAGE_PV8 * f_bbPvInverter.PVINVERTER_VOLTAGE_PV8, pvInverterTable.RECORD_TS);}
            if (yc.DEVICE_ATTR == PVINVERTER_TABLE_ATTR_PVINVERTER_CURRENT_PV8){setYcValue(&yc, pvInverterTable.PVINVERTER_CURRENT_PV8 * f_bbPvInverter.PVINVERTER_CURRENT_PV8, pvInverterTable.RECORD_TS);}
            if (yc.DEVICE_ATTR == PVINVERTER_TABLE_ATTR_PVINVERTER_POWER_PV8){setYcValue(&yc, pvInverterTable.PVINVERTER_POWER_PV8 * f_bbPvInverter.PVINVERTER_POWER_PV8, pvInverterTable.RECORD_TS);}
            if (yc.DEVICE_ATTR == PVINVERTER_TABLE_ATTR_PVINVERTER_VOLTAGE_PV9){setYcValue(&yc, pvInverterTable.PVINVERTER_VOLTAGE_PV9 * f_bbPvInverter.PVINVERTER_VOLTAGE_PV9, pvInverterTable.RECORD_TS);}
            if (yc.DEVICE_ATTR == PVINVERTER_TABLE_ATTR_PVINVERTER_CURRENT_PV9){setYcValue(&yc, pvInverterTable.PVINVERTER_CURRENT_PV9 * f_bbPvInverter.PVINVERTER_CURRENT_PV9, pvInverterTable.RECORD_TS);}
            if (yc.DEVICE_ATTR == PVINVERTER_TABLE_ATTR_PVINVERTER_POWER_PV9){setYcValue(&yc, pvInverterTable.PVINVERTER_POWER_PV9 * f_bbPvInverter.PVINVERTER_POWER_PV9, pvInverterTable.RECORD_TS);}
            if (yc.DEVICE_ATTR == PVINVERTER_TABLE_ATTR_PVINVERTER_VOLTAGE_PV10){setYcValue(&yc, pvInverterTable.PVINVERTER_VOLTAGE_PV10 * f_bbPvInverter.PVINVERTER_VOLTAGE_PV10, pvInverterTable.RECORD_TS);}
            if (yc.DEVICE_ATTR == PVINVERTER_TABLE_ATTR_PVINVERTER_CURRENT_PV10){setYcValue(&yc, pvInverterTable.PVINVERTER_CURRENT_PV10 * f_bbPvInverter.PVINVERTER_CURRENT_PV10, pvInverterTable.RECORD_TS);}
            if (yc.DEVICE_ATTR == PVINVERTER_TABLE_ATTR_PVINVERTER_POWER_PV10){setYcValue(&yc, pvInverterTable.PVINVERTER_POWER_PV10 * f_bbPvInverter.PVINVERTER_POWER_PV10, pvInverterTable.RECORD_TS);}
            if (yc.DEVICE_ATTR == PVINVERTER_TABLE_ATTR_PVINVERTER_VOLTAGE_PV11){setYcValue(&yc, pvInverterTable.PVINVERTER_VOLTAGE_PV11 * f_bbPvInverter.PVINVERTER_VOLTAGE_PV11, pvInverterTable.RECORD_TS);}
            if (yc.DEVICE_ATTR == PVINVERTER_TABLE_ATTR_PVINVERTER_CURRENT_PV11){setYcValue(&yc, pvInverterTable.PVINVERTER_CURRENT_PV11 * f_bbPvInverter.PVINVERTER_CURRENT_PV11, pvInverterTable.RECORD_TS);}
            if (yc.DEVICE_ATTR == PVINVERTER_TABLE_ATTR_PVINVERTER_POWER_PV11){setYcValue(&yc, pvInverterTable.PVINVERTER_POWER_PV11 * f_bbPvInverter.PVINVERTER_POWER_PV11, pvInverterTable.RECORD_TS);}
            if (yc.DEVICE_ATTR == PVINVERTER_TABLE_ATTR_PVINVERTER_VOLTAGE_PV12){setYcValue(&yc, pvInverterTable.PVINVERTER_VOLTAGE_PV12 * f_bbPvInverter.PVINVERTER_VOLTAGE_PV12, pvInverterTable.RECORD_TS);}
            if (yc.DEVICE_ATTR == PVINVERTER_TABLE_ATTR_PVINVERTER_CURRENT_PV12){setYcValue(&yc, pvInverterTable.PVINVERTER_CURRENT_PV12 * f_bbPvInverter.PVINVERTER_CURRENT_PV12, pvInverterTable.RECORD_TS);}
            if (yc.DEVICE_ATTR == PVINVERTER_TABLE_ATTR_PVINVERTER_POWER_PV12){setYcValue(&yc, pvInverterTable.PVINVERTER_POWER_PV12 * f_bbPvInverter.PVINVERTER_POWER_PV12, pvInverterTable.RECORD_TS);}
            if (yc.DEVICE_ATTR == PVINVERTER_TABLE_ATTR_PVINVERTER_VOLTAGE_PV13){setYcValue(&yc, pvInverterTable.PVINVERTER_VOLTAGE_PV13 * f_bbPvInverter.PVINVERTER_VOLTAGE_PV13, pvInverterTable.RECORD_TS);}
            if (yc.DEVICE_ATTR == PVINVERTER_TABLE_ATTR_PVINVERTER_CURRENT_PV13){setYcValue(&yc, pvInverterTable.PVINVERTER_CURRENT_PV13 * f_bbPvInverter.PVINVERTER_CURRENT_PV13, pvInverterTable.RECORD_TS);}
            if (yc.DEVICE_ATTR == PVINVERTER_TABLE_ATTR_PVINVERTER_POWER_PV13){setYcValue(&yc, pvInverterTable.PVINVERTER_POWER_PV13 * f_bbPvInverter.PVINVERTER_POWER_PV13, pvInverterTable.RECORD_TS);}
            if (yc.DEVICE_ATTR == PVINVERTER_TABLE_ATTR_PVINVERTER_VOLTAGE_PV14){setYcValue(&yc, pvInverterTable.PVINVERTER_VOLTAGE_PV14 * f_bbPvInverter.PVINVERTER_VOLTAGE_PV14, pvInverterTable.RECORD_TS);}
            if (yc.DEVICE_ATTR == PVINVERTER_TABLE_ATTR_PVINVERTER_CURRENT_PV14){setYcValue(&yc, pvInverterTable.PVINVERTER_CURRENT_PV14 * f_bbPvInverter.PVINVERTER_CURRENT_PV14, pvInverterTable.RECORD_TS);}
            if (yc.DEVICE_ATTR == PVINVERTER_TABLE_ATTR_PVINVERTER_POWER_PV14){setYcValue(&yc, pvInverterTable.PVINVERTER_POWER_PV14 * f_bbPvInverter.PVINVERTER_POWER_PV14, pvInverterTable.RECORD_TS);}
            if (yc.DEVICE_ATTR == PVINVERTER_TABLE_ATTR_PVINVERTER_VOLTAGE_PV15){setYcValue(&yc, pvInverterTable.PVINVERTER_VOLTAGE_PV15 * f_bbPvInverter.PVINVERTER_VOLTAGE_PV15, pvInverterTable.RECORD_TS);}
            if (yc.DEVICE_ATTR == PVINVERTER_TABLE_ATTR_PVINVERTER_CURRENT_PV15){setYcValue(&yc, pvInverterTable.PVINVERTER_CURRENT_PV15 * f_bbPvInverter.PVINVERTER_CURRENT_PV15, pvInverterTable.RECORD_TS);}
            if (yc.DEVICE_ATTR == PVINVERTER_TABLE_ATTR_PVINVERTER_POWER_PV15){setYcValue(&yc, pvInverterTable.PVINVERTER_POWER_PV15 * f_bbPvInverter.PVINVERTER_POWER_PV15, pvInverterTable.RECORD_TS);}
            if (yc.DEVICE_ATTR == PVINVERTER_TABLE_ATTR_PVINVERTER_VOLTAGE_PV16){setYcValue(&yc, pvInverterTable.PVINVERTER_VOLTAGE_PV16 * f_bbPvInverter.PVINVERTER_VOLTAGE_PV16, pvInverterTable.RECORD_TS);}
            if (yc.DEVICE_ATTR == PVINVERTER_TABLE_ATTR_PVINVERTER_CURRENT_PV16){setYcValue(&yc, pvInverterTable.PVINVERTER_CURRENT_PV16 * f_bbPvInverter.PVINVERTER_CURRENT_PV16, pvInverterTable.RECORD_TS);}
            if (yc.DEVICE_ATTR == PVINVERTER_TABLE_ATTR_PVINVERTER_POWER_PV16){setYcValue(&yc, pvInverterTable.PVINVERTER_POWER_PV16 * f_bbPvInverter.PVINVERTER_POWER_PV16, pvInverterTable.RECORD_TS);}
            if (yc.DEVICE_ATTR == PVINVERTER_TABLE_ATTR_PVINVERTER_GENERATIONTIME_TODAY){setYcValue(&yc, pvInverterTable.PVINVERTER_GENERATIONTIME_TODAY * f_bbPvInverter.PVINVERTER_GENERATIONTIME_TODAY, pvInverterTable.RECORD_TS);}
            if (yc.DEVICE_ATTR == PVINVERTER_TABLE_ATTR_PVINVERTER_PV_GENERATION_TODAY){setYcValue(&yc, pvInverterTable.PVINVERTER_PV_GENERATION_TODAY * f_bbPvInverter.PVINVERTER_PV_GENERATION_TODAY, pvInverterTable.RECORD_TS);}
            if (yc.DEVICE_ATTR == PVINVERTER_TABLE_ATTR_PVINVERTER_PV_GENERATION_TOTAL){setYcValue(&yc, pvInverterTable.PVINVERTER_PV_GENERATION_TOTAL * f_bbPvInverter.PVINVERTER_PV_GENERATION_TOTAL, pvInverterTable.RECORD_TS);}
            if (yc.DEVICE_ATTR == PVINVERTER_TABLE_ATTR_PVINVERTER_LOAD_CONSUMPTION_TODAY){setYcValue(&yc, pvInverterTable.PVINVERTER_LOAD_CONSUMPTION_TODAY * f_bbPvInverter.PVINVERTER_LOAD_CONSUMPTION_TODAY, pvInverterTable.RECORD_TS);}
            if (yc.DEVICE_ATTR == PVINVERTER_TABLE_ATTR_PVINVERTER_LOAD_CONSUMPTION_TOTAL){setYcValue(&yc, pvInverterTable.PVINVERTER_LOAD_CONSUMPTION_TOTAL * f_bbPvInverter.PVINVERTER_LOAD_CONSUMPTION_TOTAL, pvInverterTable.RECORD_TS);}
            if (yc.DEVICE_ATTR == PVINVERTER_TABLE_ATTR_PVINVERTER_ENERGY_PURCHASE_TODAY){setYcValue(&yc, pvInverterTable.PVINVERTER_ENERGY_PURCHASE_TODAY * f_bbPvInverter.PVINVERTER_ENERGY_PURCHASE_TODAY, pvInverterTable.RECORD_TS);}
            if (yc.DEVICE_ATTR == PVINVERTER_TABLE_ATTR_PVINVERTER_ENERGY_PURCHASE_TOTAL){setYcValue(&yc, pvInverterTable.PVINVERTER_ENERGY_PURCHASE_TOTAL * f_bbPvInverter.PVINVERTER_ENERGY_PURCHASE_TOTAL, pvInverterTable.RECORD_TS);}
            if (yc.DEVICE_ATTR == PVINVERTER_TABLE_ATTR_PVINVERTER_ENERGY_SELLING_TODAY){setYcValue(&yc, pvInverterTable.PVINVERTER_ENERGY_SELLING_TODAY * f_bbPvInverter.PVINVERTER_ENERGY_SELLING_TODAY, pvInverterTable.RECORD_TS);}
            if (yc.DEVICE_ATTR == PVINVERTER_TABLE_ATTR_PVINVERTER_ENERGY_SELLING_TOTAL){setYcValue(&yc, pvInverterTable.PVINVERTER_ENERGY_SELLING_TOTAL * f_bbPvInverter.PVINVERTER_ENERGY_SELLING_TOTAL, pvInverterTable.RECORD_TS);}
            if (yc.DEVICE_ATTR == PVINVERTER_TABLE_ATTR_PVINVERTER_REMOTE_ON_OFF_CONTROL){setYcValue(&yc, pvInverterTable.PVINVERTER_REMOTE_ON_OFF_CONTROL * f_bbPvInverter.PVINVERTER_REMOTE_ON_OFF_CONTROL, pvInverterTable.RECORD_TS);}
        }
    }
    return 0;
}

int PointManager::refreshP485METERTablePointValue(const string &sDatabaseName, const string &sTableName,
                                                  P485METERTable &p485MeterTable)
{
    for (int i = 0; i < f_ycPoints.size(); ++i)
    {
        YcPoint &yc = f_ycPoints.at(i);
        if (yc.DATABASE_NAME == sDatabaseName && yc.TABLE_NAME == sTableName)
        {
            if (yc.DEVICE_ATTR == P485METER_TABLE_ATTR_P485METER_UA) { setYcValue(&yc, p485MeterTable.P485METER_UA, p485MeterTable.RECORD_TS);}
            if (yc.DEVICE_ATTR == P485METER_TABLE_ATTR_P485METER_UB) { setYcValue(&yc, p485MeterTable.P485METER_UB, p485MeterTable.RECORD_TS);}
            if (yc.DEVICE_ATTR == P485METER_TABLE_ATTR_P485METER_UC) { setYcValue(&yc, p485MeterTable.P485METER_UC, p485MeterTable.RECORD_TS);}
            if (yc.DEVICE_ATTR == P485METER_TABLE_ATTR_P485METER_IA) { setYcValue(&yc, p485MeterTable.P485METER_IA, p485MeterTable.RECORD_TS);}
            if (yc.DEVICE_ATTR == P485METER_TABLE_ATTR_P485METER_IB) { setYcValue(&yc, p485MeterTable.P485METER_IB, p485MeterTable.RECORD_TS);}
            if (yc.DEVICE_ATTR == P485METER_TABLE_ATTR_P485METER_IC) { setYcValue(&yc, p485MeterTable.P485METER_IC, p485MeterTable.RECORD_TS);}
            if (yc.DEVICE_ATTR == P485METER_TABLE_ATTR_P485METER_ACTIVEPOWER_TOTAL) { setYcValue(&yc, p485MeterTable.P485METER_ACTIVEPOWER_TOTAL, p485MeterTable.RECORD_TS);}
            if (yc.DEVICE_ATTR == P485METER_TABLE_ATTR_P485METER_APPARENTPOWER_TOTAL) { setYcValue(&yc, p485MeterTable.P485METER_APPARENTPOWER_TOTAL, p485MeterTable.RECORD_TS);}
            if (yc.DEVICE_ATTR == P485METER_TABLE_ATTR_P485METER_REACTIVEPOWER_TOTAL) { setYcValue(&yc, p485MeterTable.P485METER_REACTIVEPOWER_TOTAL, p485MeterTable.RECORD_TS);}
            if (yc.DEVICE_ATTR == P485METER_TABLE_ATTR_P485METER_POWERFACTOR_TOTAL) { setYcValue(&yc, p485MeterTable.P485METER_POWERFACTOR_TOTAL, p485MeterTable.RECORD_TS);}
            if (yc.DEVICE_ATTR == P485METER_TABLE_ATTR_P485METER_ACTIVEPOWERDEMAND_TOTAL) { setYcValue(&yc, p485MeterTable.P485METER_ACTIVEPOWERDEMAND_TOTAL, p485MeterTable.RECORD_TS);}
            if (yc.DEVICE_ATTR == P485METER_TABLE_ATTR_P485METER_UAB) { setYcValue(&yc, p485MeterTable.P485METER_UAB, p485MeterTable.RECORD_TS);}
            if (yc.DEVICE_ATTR == P485METER_TABLE_ATTR_P485METER_UBC) { setYcValue(&yc, p485MeterTable.P485METER_UBC, p485MeterTable.RECORD_TS);}
            if (yc.DEVICE_ATTR == P485METER_TABLE_ATTR_P485METER_UCA) { setYcValue(&yc, p485MeterTable.P485METER_UCA, p485MeterTable.RECORD_TS);}
            if (yc.DEVICE_ATTR == P485METER_TABLE_ATTR_P485METER_CURRENTIMBALANCE) { setYcValue(&yc, p485MeterTable.P485METER_CURRENTIMBALANCE, p485MeterTable.RECORD_TS);}
            if (yc.DEVICE_ATTR == P485METER_TABLE_ATTR_P485METER_ZEROSEQUENCECURRENT) { setYcValue(&yc, p485MeterTable.P485METER_ZEROSEQUENCECURRENT, p485MeterTable.RECORD_TS);}
            if (yc.DEVICE_ATTR == P485METER_TABLE_ATTR_P485METER_ACTIVEPOWER_A) { setYcValue(&yc, p485MeterTable.P485METER_ACTIVEPOWER_A, p485MeterTable.RECORD_TS);}
            if (yc.DEVICE_ATTR == P485METER_TABLE_ATTR_P485METER_ACTIVEPOWER_B) { setYcValue(&yc, p485MeterTable.P485METER_ACTIVEPOWER_B, p485MeterTable.RECORD_TS);}
            if (yc.DEVICE_ATTR == P485METER_TABLE_ATTR_P485METER_ACTIVEPOWER_C) { setYcValue(&yc, p485MeterTable.P485METER_ACTIVEPOWER_C, p485MeterTable.RECORD_TS);}
            if (yc.DEVICE_ATTR == P485METER_TABLE_ATTR_P485METER_REACTIVEPOWER_A) { setYcValue(&yc, p485MeterTable.P485METER_REACTIVEPOWER_A, p485MeterTable.RECORD_TS);}
            if (yc.DEVICE_ATTR == P485METER_TABLE_ATTR_P485METER_REACTIVEPOWER_B) { setYcValue(&yc, p485MeterTable.P485METER_REACTIVEPOWER_B, p485MeterTable.RECORD_TS);}
            if (yc.DEVICE_ATTR == P485METER_TABLE_ATTR_P485METER_REACTIVEPOWER_C) { setYcValue(&yc, p485MeterTable.P485METER_REACTIVEPOWER_C, p485MeterTable.RECORD_TS);}
            if (yc.DEVICE_ATTR == P485METER_TABLE_ATTR_P485METER_APPARENTPOWER_A) { setYcValue(&yc, p485MeterTable.P485METER_APPARENTPOWER_A, p485MeterTable.RECORD_TS);}
            if (yc.DEVICE_ATTR == P485METER_TABLE_ATTR_P485METER_APPARENTPOWER_B) { setYcValue(&yc, p485MeterTable.P485METER_APPARENTPOWER_B, p485MeterTable.RECORD_TS);}
            if (yc.DEVICE_ATTR == P485METER_TABLE_ATTR_P485METER_APPARENTPOWER_C) { setYcValue(&yc, p485MeterTable.P485METER_APPARENTPOWER_C, p485MeterTable.RECORD_TS);}
            if (yc.DEVICE_ATTR == P485METER_TABLE_ATTR_P485METER_POWERFACTOR_A) { setYcValue(&yc, p485MeterTable.P485METER_POWERFACTOR_A, p485MeterTable.RECORD_TS);}
            if (yc.DEVICE_ATTR == P485METER_TABLE_ATTR_P485METER_POWERFACTOR_B) { setYcValue(&yc, p485MeterTable.P485METER_POWERFACTOR_B, p485MeterTable.RECORD_TS);}
            if (yc.DEVICE_ATTR == P485METER_TABLE_ATTR_P485METER_POWERFACTOR_C) { setYcValue(&yc, p485MeterTable.P485METER_POWERFACTOR_C, p485MeterTable.RECORD_TS);}
            if (yc.DEVICE_ATTR == P485METER_TABLE_ATTR_P485METER_FREQUENCY) { setYcValue(&yc, p485MeterTable.P485METER_FREQUENCY, p485MeterTable.RECORD_TS);}
            if (yc.DEVICE_ATTR == P485METER_TABLE_ATTR_P485METER_UATHD) { setYcValue(&yc, p485MeterTable.P485METER_UATHD, p485MeterTable.RECORD_TS);}
            if (yc.DEVICE_ATTR == P485METER_TABLE_ATTR_P485METER_UBTHD) { setYcValue(&yc, p485MeterTable.P485METER_UBTHD, p485MeterTable.RECORD_TS);}
            if (yc.DEVICE_ATTR == P485METER_TABLE_ATTR_P485METER_UCTHD) { setYcValue(&yc, p485MeterTable.P485METER_UCTHD, p485MeterTable.RECORD_TS);}
            if (yc.DEVICE_ATTR == P485METER_TABLE_ATTR_P485METER_IATHD) { setYcValue(&yc, p485MeterTable.P485METER_IATHD, p485MeterTable.RECORD_TS);}
            if (yc.DEVICE_ATTR == P485METER_TABLE_ATTR_P485METER_IBTHD) { setYcValue(&yc, p485MeterTable.P485METER_IBTHD, p485MeterTable.RECORD_TS);}
            if (yc.DEVICE_ATTR == P485METER_TABLE_ATTR_P485METER_ICTHD) { setYcValue(&yc, p485MeterTable.P485METER_ICTHD, p485MeterTable.RECORD_TS);}
            if (yc.DEVICE_ATTR == P485METER_TABLE_ATTR_P485METER_UATH_3) { setYcValue(&yc, p485MeterTable.P485METER_UATH_3, p485MeterTable.RECORD_TS);}
            if (yc.DEVICE_ATTR == P485METER_TABLE_ATTR_P485METER_UATH_5) { setYcValue(&yc, p485MeterTable.P485METER_UATH_5, p485MeterTable.RECORD_TS);}
            if (yc.DEVICE_ATTR == P485METER_TABLE_ATTR_P485METER_UATH_7) { setYcValue(&yc, p485MeterTable.P485METER_UATH_7, p485MeterTable.RECORD_TS);}
            if (yc.DEVICE_ATTR == P485METER_TABLE_ATTR_P485METER_UATH_9) { setYcValue(&yc, p485MeterTable.P485METER_UATH_9, p485MeterTable.RECORD_TS);}
            if (yc.DEVICE_ATTR == P485METER_TABLE_ATTR_P485METER_UATH_11) { setYcValue(&yc, p485MeterTable.P485METER_UATH_11, p485MeterTable.RECORD_TS);}
            if (yc.DEVICE_ATTR == P485METER_TABLE_ATTR_P485METER_UATH_13) { setYcValue(&yc, p485MeterTable.P485METER_UATH_13, p485MeterTable.RECORD_TS);}
            if (yc.DEVICE_ATTR == P485METER_TABLE_ATTR_P485METER_UATH_15) { setYcValue(&yc, p485MeterTable.P485METER_UATH_15, p485MeterTable.RECORD_TS);}
            if (yc.DEVICE_ATTR == P485METER_TABLE_ATTR_P485METER_UATH_17) { setYcValue(&yc, p485MeterTable.P485METER_UATH_17, p485MeterTable.RECORD_TS);}
            if (yc.DEVICE_ATTR == P485METER_TABLE_ATTR_P485METER_UBTH_3) { setYcValue(&yc, p485MeterTable.P485METER_UBTH_3, p485MeterTable.RECORD_TS);}
            if (yc.DEVICE_ATTR == P485METER_TABLE_ATTR_P485METER_UBTH_5) { setYcValue(&yc, p485MeterTable.P485METER_UBTH_5, p485MeterTable.RECORD_TS);}
            if (yc.DEVICE_ATTR == P485METER_TABLE_ATTR_P485METER_UBTH_7) { setYcValue(&yc, p485MeterTable.P485METER_UBTH_7, p485MeterTable.RECORD_TS);}
            if (yc.DEVICE_ATTR == P485METER_TABLE_ATTR_P485METER_UBTH_9) { setYcValue(&yc, p485MeterTable.P485METER_UBTH_9, p485MeterTable.RECORD_TS);}
            if (yc.DEVICE_ATTR == P485METER_TABLE_ATTR_P485METER_UBTH_11) { setYcValue(&yc, p485MeterTable.P485METER_UBTH_11, p485MeterTable.RECORD_TS);}
            if (yc.DEVICE_ATTR == P485METER_TABLE_ATTR_P485METER_UBTH_13) { setYcValue(&yc, p485MeterTable.P485METER_UBTH_13, p485MeterTable.RECORD_TS);}
            if (yc.DEVICE_ATTR == P485METER_TABLE_ATTR_P485METER_UBTH_15) { setYcValue(&yc, p485MeterTable.P485METER_UBTH_15, p485MeterTable.RECORD_TS);}
            if (yc.DEVICE_ATTR == P485METER_TABLE_ATTR_P485METER_UBTH_17) { setYcValue(&yc, p485MeterTable.P485METER_UBTH_17, p485MeterTable.RECORD_TS);}
            if (yc.DEVICE_ATTR == P485METER_TABLE_ATTR_P485METER_UCTH_3) { setYcValue(&yc, p485MeterTable.P485METER_UCTH_3, p485MeterTable.RECORD_TS);}
            if (yc.DEVICE_ATTR == P485METER_TABLE_ATTR_P485METER_UCTH_5) { setYcValue(&yc, p485MeterTable.P485METER_UCTH_5, p485MeterTable.RECORD_TS);}
            if (yc.DEVICE_ATTR == P485METER_TABLE_ATTR_P485METER_UCTH_7) { setYcValue(&yc, p485MeterTable.P485METER_UCTH_7, p485MeterTable.RECORD_TS);}
            if (yc.DEVICE_ATTR == P485METER_TABLE_ATTR_P485METER_UCTH_9) { setYcValue(&yc, p485MeterTable.P485METER_UCTH_9, p485MeterTable.RECORD_TS);}
            if (yc.DEVICE_ATTR == P485METER_TABLE_ATTR_P485METER_UCTH_11) { setYcValue(&yc, p485MeterTable.P485METER_UCTH_11, p485MeterTable.RECORD_TS);}
            if (yc.DEVICE_ATTR == P485METER_TABLE_ATTR_P485METER_UCTH_13) { setYcValue(&yc, p485MeterTable.P485METER_UCTH_13, p485MeterTable.RECORD_TS);}
            if (yc.DEVICE_ATTR == P485METER_TABLE_ATTR_P485METER_UCTH_15) { setYcValue(&yc, p485MeterTable.P485METER_UCTH_15, p485MeterTable.RECORD_TS);}
            if (yc.DEVICE_ATTR == P485METER_TABLE_ATTR_P485METER_UCTH_17) { setYcValue(&yc, p485MeterTable.P485METER_UCTH_17, p485MeterTable.RECORD_TS);}
            if (yc.DEVICE_ATTR == P485METER_TABLE_ATTR_P485METER_IATH_3) { setYcValue(&yc, p485MeterTable.P485METER_IATH_3, p485MeterTable.RECORD_TS);}
            if (yc.DEVICE_ATTR == P485METER_TABLE_ATTR_P485METER_IATH_5) { setYcValue(&yc, p485MeterTable.P485METER_IATH_5, p485MeterTable.RECORD_TS);}
            if (yc.DEVICE_ATTR == P485METER_TABLE_ATTR_P485METER_IATH_7) { setYcValue(&yc, p485MeterTable.P485METER_IATH_7, p485MeterTable.RECORD_TS);}
            if (yc.DEVICE_ATTR == P485METER_TABLE_ATTR_P485METER_IATH_9) { setYcValue(&yc, p485MeterTable.P485METER_IATH_9, p485MeterTable.RECORD_TS);}
            if (yc.DEVICE_ATTR == P485METER_TABLE_ATTR_P485METER_IATH_11) { setYcValue(&yc, p485MeterTable.P485METER_IATH_11, p485MeterTable.RECORD_TS);}
            if (yc.DEVICE_ATTR == P485METER_TABLE_ATTR_P485METER_IATH_13) { setYcValue(&yc, p485MeterTable.P485METER_IATH_13, p485MeterTable.RECORD_TS);}
            if (yc.DEVICE_ATTR == P485METER_TABLE_ATTR_P485METER_IATH_15) { setYcValue(&yc, p485MeterTable.P485METER_IATH_15, p485MeterTable.RECORD_TS);}
            if (yc.DEVICE_ATTR == P485METER_TABLE_ATTR_P485METER_IATH_17) { setYcValue(&yc, p485MeterTable.P485METER_IATH_17, p485MeterTable.RECORD_TS);}
            if (yc.DEVICE_ATTR == P485METER_TABLE_ATTR_P485METER_IBTH_3) { setYcValue(&yc, p485MeterTable.P485METER_IBTH_3, p485MeterTable.RECORD_TS);}
            if (yc.DEVICE_ATTR == P485METER_TABLE_ATTR_P485METER_IBTH_5) { setYcValue(&yc, p485MeterTable.P485METER_IBTH_5, p485MeterTable.RECORD_TS);}
            if (yc.DEVICE_ATTR == P485METER_TABLE_ATTR_P485METER_IBTH_7) { setYcValue(&yc, p485MeterTable.P485METER_IBTH_7, p485MeterTable.RECORD_TS);}
            if (yc.DEVICE_ATTR == P485METER_TABLE_ATTR_P485METER_IBTH_9) { setYcValue(&yc, p485MeterTable.P485METER_IBTH_9, p485MeterTable.RECORD_TS);}
            if (yc.DEVICE_ATTR == P485METER_TABLE_ATTR_P485METER_IBTH_11) { setYcValue(&yc, p485MeterTable.P485METER_IBTH_11, p485MeterTable.RECORD_TS);}
            if (yc.DEVICE_ATTR == P485METER_TABLE_ATTR_P485METER_IBTH_13) { setYcValue(&yc, p485MeterTable.P485METER_IBTH_13, p485MeterTable.RECORD_TS);}
            if (yc.DEVICE_ATTR == P485METER_TABLE_ATTR_P485METER_IBTH_15) { setYcValue(&yc, p485MeterTable.P485METER_IBTH_15, p485MeterTable.RECORD_TS);}
            if (yc.DEVICE_ATTR == P485METER_TABLE_ATTR_P485METER_IBTH_17) { setYcValue(&yc, p485MeterTable.P485METER_IBTH_17, p485MeterTable.RECORD_TS);}
            if (yc.DEVICE_ATTR == P485METER_TABLE_ATTR_P485METER_ICTH_3) { setYcValue(&yc, p485MeterTable.P485METER_ICTH_3, p485MeterTable.RECORD_TS);}
            if (yc.DEVICE_ATTR == P485METER_TABLE_ATTR_P485METER_ICTH_5) { setYcValue(&yc, p485MeterTable.P485METER_ICTH_5, p485MeterTable.RECORD_TS);}
            if (yc.DEVICE_ATTR == P485METER_TABLE_ATTR_P485METER_ICTH_7) { setYcValue(&yc, p485MeterTable.P485METER_ICTH_7, p485MeterTable.RECORD_TS);}
            if (yc.DEVICE_ATTR == P485METER_TABLE_ATTR_P485METER_ICTH_9) { setYcValue(&yc, p485MeterTable.P485METER_ICTH_9, p485MeterTable.RECORD_TS);}
            if (yc.DEVICE_ATTR == P485METER_TABLE_ATTR_P485METER_ICTH_11) { setYcValue(&yc, p485MeterTable.P485METER_ICTH_11, p485MeterTable.RECORD_TS);}
            if (yc.DEVICE_ATTR == P485METER_TABLE_ATTR_P485METER_ICTH_13) { setYcValue(&yc, p485MeterTable.P485METER_ICTH_13, p485MeterTable.RECORD_TS);}
            if (yc.DEVICE_ATTR == P485METER_TABLE_ATTR_P485METER_ICTH_15) { setYcValue(&yc, p485MeterTable.P485METER_ICTH_15, p485MeterTable.RECORD_TS);}
            if (yc.DEVICE_ATTR == P485METER_TABLE_ATTR_P485METER_ICTH_17) { setYcValue(&yc, p485MeterTable.P485METER_ICTH_17, p485MeterTable.RECORD_TS);}
            if (yc.DEVICE_ATTR == P485METER_TABLE_ATTR_P485METER_VOLTAGE_IMBALANCE) { setYcValue(&yc, p485MeterTable.P485METER_VOLTAGE_IMBALANCE, p485MeterTable.RECORD_TS);}
            if (yc.DEVICE_ATTR == P485METER_TABLE_ATTR_P485METER_ACTIVEENERGY_TOTAL) { setYcValue(&yc, p485MeterTable.P485METER_ACTIVEENERGY_TOTAL, p485MeterTable.RECORD_TS);}
            if (yc.DEVICE_ATTR == P485METER_TABLE_ATTR_P485METER_REACTIVEENERGY_TOTAL) { setYcValue(&yc, p485MeterTable.P485METER_REACTIVEENERGY_TOTAL, p485MeterTable.RECORD_TS);}
        }
    }
    return 0;
}

void PointManager::setBBPvInverter(const string &bb)
{
    BBPvInverter bbPvInverter;
    {
        bbPvInverter.PVINVERTER_SYSSTATE=1;
        bbPvInverter.PVINVERTER_GENERATIONTIME_TOTAL=1;
        bbPvInverter.PVINVERTER_SERVICETIME_TOTAL=1;
        bbPvInverter.PVINVERTER_FREQUENCY_GRID=1;
        bbPvInverter.PVINVERTER_ACTIVEPOWER_OUTPUT_TOTAL=1;
        bbPvInverter.PVINVERTER_REACTIVEPOWER_OUTPUT_TOTAL=1;
        bbPvInverter.PVINVERTER_APPARENTPOWER_OUTPUT_TOTAL=1;
        bbPvInverter.PVINVERTER_ACTIVEPOWER_PCC_TOTAL=1;
        bbPvInverter.PVINVERTER_REACTIVEPOWER_PCC_TOTAL=1;
        bbPvInverter.PVINVERTER_APPARENTPOWER_PCC_TOTAL=1;
        bbPvInverter.PVINVERTER_VOLTAGE_PHASE_R=1;
        bbPvInverter.PVINVERTER_CURRENT_OUTPUT_R=1;
        bbPvInverter.PVINVERTER_ACTIVEPOWER_OUTPUT_R=1;
        bbPvInverter.PVINVERTER_REACTIVEPOWER_OUTPUT_R=1;
        bbPvInverter.PVINVERTER_POWERFACTOR_OUTPUT_R=1;
        bbPvInverter.PVINVERTER_CURRENT_PCC_R=1;
        bbPvInverter.PVINVERTER_ACTIVEPOWER_PCC_R=1;
        bbPvInverter.PVINVERTER_REACTIVEPOWER_PCC_R=1;
        bbPvInverter.PVINVERTER_POWERFACTOR_PCC_R=1;
        bbPvInverter.PVINVERTER_VOLTAGE_PHASE_S=1;
        bbPvInverter.PVINVERTER_CURRENT_OUTPUT_S=1;
        bbPvInverter.PVINVERTER_ACTIVEPOWER_OUTPUT_S=1;
        bbPvInverter.PVINVERTER_REACTIVEPOWER_OUTPUT_S=1;
        bbPvInverter.PVINVERTER_POWERFACTOR_OUTPUT_S=1;
        bbPvInverter.PVINVERTER_CURRENT_PCC_S=1;
        bbPvInverter.PVINVERTER_ACTIVEPOWER_PCC_S=1;
        bbPvInverter.PVINVERTER_REACTIVEPOWER_PCC_S=1;
        bbPvInverter.PVINVERTER_POWERFACTOR_PCC_S=1;
        bbPvInverter.PVINVERTER_VOLTAGE_PHASE_T=1;
        bbPvInverter.PVINVERTER_CURRENT_OUTPUT_T=1;
        bbPvInverter.PVINVERTER_ACTIVEPOWER_OUTPUT_T=1;
        bbPvInverter.PVINVERTER_REACTIVEPOWER_OUTPUT_T=1;
        bbPvInverter.PVINVERTER_POWERFACTOR_OUTPUT_T=1;
        bbPvInverter.PVINVERTER_CURRENT_PCC_T=1;
        bbPvInverter.PVINVERTER_ACTIVEPOWER_PCC_T=1;
        bbPvInverter.PVINVERTER_REACTIVEPOWER_PCC_T=1;
        bbPvInverter.PVINVERTER_POWERFACTOR_PCC_T=1;
        bbPvInverter.PVINVERTER_ACTIVEPOWER_PV_EXT=1;
        bbPvInverter.PVINVERTER_ACTIVEPOWER_LOAD_SYS=1;
        bbPvInverter.PVINVERTER_ACTIVEPOWER_LOAD_TOTAL=1;
        bbPvInverter.PVINVERTER_REACTIVEPOWER_LOAD_TOTAL=1;
        bbPvInverter.PVINVERTER_APPARENTPOWER_LOAD_TOTAL=1;
        bbPvInverter.PVINVERTER_FREQUENCY_OUTPUT=1;
        bbPvInverter.PVINVERTER_VOLTAGE_OUTPUT_R=1;
        bbPvInverter.PVINVERTER_CURRENT_LOAD_R=1;
        bbPvInverter.PVINVERTER_ACTIVEPOWER_LOAD_R=1;
        bbPvInverter.PVINVERTER_REACTIVEPOWER_LOAD_R=1;
        bbPvInverter.PVINVERTER_APPARENTPOWER_LOAD_R=1;
        bbPvInverter.PVINVERTER_LOADPEAKRATIO_R=1;
        bbPvInverter.PVINVERTER_VOLTAGE_OUTPUT_S=1;
        bbPvInverter.PVINVERTER_CURRENT_LOAD_S=1;
        bbPvInverter.PVINVERTER_ACTIVEPOWER_LOAD_S=1;
        bbPvInverter.PVINVERTER_REACTIVEPOWER_LOAD_S=1;
        bbPvInverter.PVINVERTER_APPARENTPOWER_LOAD_S=1;
        bbPvInverter.PVINVERTER_LOADPEAKRATIO_S=1;
        bbPvInverter.PVINVERTER_VOLTAGE_OUTPUT_T=1;
        bbPvInverter.PVINVERTER_CURRENT_LOAD_T=1;
        bbPvInverter.PVINVERTER_ACTIVEPOWER_LOAD_T=1;
        bbPvInverter.PVINVERTER_REACTIVEPOWER_LOAD_T=1;
        bbPvInverter.PVINVERTER_APPARENTPOWER_LOAD_T=1;
        bbPvInverter.PVINVERTER_LOADPEAKRATIO_T=1;
        bbPvInverter.PVINVERTER_VOLTAGE_PV1=1;
        bbPvInverter.PVINVERTER_CURRENT_PV1=1;
        bbPvInverter.PVINVERTER_POWER_PV1=1;
        bbPvInverter.PVINVERTER_VOLTAGE_PV2=1;
        bbPvInverter.PVINVERTER_CURRENT_PV2=1;
        bbPvInverter.PVINVERTER_POWER_PV2=1;
        bbPvInverter.PVINVERTER_VOLTAGE_PV3=1;
        bbPvInverter.PVINVERTER_CURRENT_PV3=1;
        bbPvInverter.PVINVERTER_POWER_PV3=1;
        bbPvInverter.PVINVERTER_VOLTAGE_PV4=1;
        bbPvInverter.PVINVERTER_CURRENT_PV4=1;
        bbPvInverter.PVINVERTER_POWER_PV4=1;
        bbPvInverter.PVINVERTER_VOLTAGE_PV5=1;
        bbPvInverter.PVINVERTER_CURRENT_PV5=1;
        bbPvInverter.PVINVERTER_POWER_PV5=1;
        bbPvInverter.PVINVERTER_VOLTAGE_PV6=1;
        bbPvInverter.PVINVERTER_CURRENT_PV6=1;
        bbPvInverter.PVINVERTER_POWER_PV6=1;
        bbPvInverter.PVINVERTER_VOLTAGE_PV7=1;
        bbPvInverter.PVINVERTER_CURRENT_PV7=1;
        bbPvInverter.PVINVERTER_POWER_PV7=1;
        bbPvInverter.PVINVERTER_VOLTAGE_PV8=1;
        bbPvInverter.PVINVERTER_CURRENT_PV8=1;
        bbPvInverter.PVINVERTER_POWER_PV8=1;
        bbPvInverter.PVINVERTER_VOLTAGE_PV9=1;
        bbPvInverter.PVINVERTER_CURRENT_PV9=1;
        bbPvInverter.PVINVERTER_POWER_PV9=1;
        bbPvInverter.PVINVERTER_VOLTAGE_PV10=1;
        bbPvInverter.PVINVERTER_CURRENT_PV10=1;
        bbPvInverter.PVINVERTER_POWER_PV10=1;
        bbPvInverter.PVINVERTER_VOLTAGE_PV11=1;
        bbPvInverter.PVINVERTER_CURRENT_PV11=1;
        bbPvInverter.PVINVERTER_POWER_PV11=1;
        bbPvInverter.PVINVERTER_VOLTAGE_PV12=1;
        bbPvInverter.PVINVERTER_CURRENT_PV12=1;
        bbPvInverter.PVINVERTER_POWER_PV12=1;
        bbPvInverter.PVINVERTER_VOLTAGE_PV13=1;
        bbPvInverter.PVINVERTER_CURRENT_PV13=1;
        bbPvInverter.PVINVERTER_POWER_PV13=1;
        bbPvInverter.PVINVERTER_VOLTAGE_PV14=1;
        bbPvInverter.PVINVERTER_CURRENT_PV14=1;
        bbPvInverter.PVINVERTER_POWER_PV14=1;
        bbPvInverter.PVINVERTER_VOLTAGE_PV15=1;
        bbPvInverter.PVINVERTER_CURRENT_PV15=1;
        bbPvInverter.PVINVERTER_POWER_PV15=1;
        bbPvInverter.PVINVERTER_VOLTAGE_PV16=1;
        bbPvInverter.PVINVERTER_CURRENT_PV16=1;
        bbPvInverter.PVINVERTER_POWER_PV16=1;
        bbPvInverter.PVINVERTER_GENERATIONTIME_TODAY=1;
        bbPvInverter.PVINVERTER_PV_GENERATION_TODAY=1;
        bbPvInverter.PVINVERTER_PV_GENERATION_TOTAL=1;
        bbPvInverter.PVINVERTER_LOAD_CONSUMPTION_TODAY=1;
        bbPvInverter.PVINVERTER_LOAD_CONSUMPTION_TOTAL=1;
        bbPvInverter.PVINVERTER_ENERGY_PURCHASE_TODAY=1;
        bbPvInverter.PVINVERTER_ENERGY_PURCHASE_TOTAL=1;
        bbPvInverter.PVINVERTER_ENERGY_SELLING_TODAY=1;
        bbPvInverter.PVINVERTER_ENERGY_SELLING_TOTAL=1;
        bbPvInverter.PVINVERTER_REMOTE_ON_OFF_CONTROL=1;
    }
    map<string, string> bbs = CxString::splitToMap(bb, '=', '\n');
    for(map<string,string>::const_iterator it = bbs.begin(); it != bbs.end(); ++it)
    {
        const std::string &sAttr = it->first;
        const std::string &sBB = it->second;
        if (sAttr ==PVINVERTER_TABLE_ATTR_PVINVERTER_SYSSTATE){bbPvInverter.PVINVERTER_SYSSTATE=CxString::toDouble(sBB);}
        if (sAttr ==PVINVERTER_TABLE_ATTR_PVINVERTER_GENERATIONTIME_TOTAL){bbPvInverter.PVINVERTER_GENERATIONTIME_TOTAL=CxString::toDouble(sBB);}
        if (sAttr ==PVINVERTER_TABLE_ATTR_PVINVERTER_SERVICETIME_TOTAL){bbPvInverter.PVINVERTER_SERVICETIME_TOTAL=CxString::toDouble(sBB);}
        if (sAttr ==PVINVERTER_TABLE_ATTR_PVINVERTER_FREQUENCY_GRID){bbPvInverter.PVINVERTER_FREQUENCY_GRID=CxString::toDouble(sBB);}
        if (sAttr ==PVINVERTER_TABLE_ATTR_PVINVERTER_ACTIVEPOWER_OUTPUT_TOTAL){bbPvInverter.PVINVERTER_ACTIVEPOWER_OUTPUT_TOTAL=CxString::toDouble(sBB);}
        if (sAttr ==PVINVERTER_TABLE_ATTR_PVINVERTER_REACTIVEPOWER_OUTPUT_TOTAL){bbPvInverter.PVINVERTER_REACTIVEPOWER_OUTPUT_TOTAL=CxString::toDouble(sBB);}
        if (sAttr ==PVINVERTER_TABLE_ATTR_PVINVERTER_APPARENTPOWER_OUTPUT_TOTAL){bbPvInverter.PVINVERTER_APPARENTPOWER_OUTPUT_TOTAL=CxString::toDouble(sBB);}
        if (sAttr ==PVINVERTER_TABLE_ATTR_PVINVERTER_ACTIVEPOWER_PCC_TOTAL){bbPvInverter.PVINVERTER_ACTIVEPOWER_PCC_TOTAL=CxString::toDouble(sBB);}
        if (sAttr ==PVINVERTER_TABLE_ATTR_PVINVERTER_REACTIVEPOWER_PCC_TOTAL){bbPvInverter.PVINVERTER_REACTIVEPOWER_PCC_TOTAL=CxString::toDouble(sBB);}
        if (sAttr ==PVINVERTER_TABLE_ATTR_PVINVERTER_APPARENTPOWER_PCC_TOTAL){bbPvInverter.PVINVERTER_APPARENTPOWER_PCC_TOTAL=CxString::toDouble(sBB);}
        if (sAttr ==PVINVERTER_TABLE_ATTR_PVINVERTER_VOLTAGE_PHASE_R){bbPvInverter.PVINVERTER_VOLTAGE_PHASE_R=CxString::toDouble(sBB);}
        if (sAttr ==PVINVERTER_TABLE_ATTR_PVINVERTER_CURRENT_OUTPUT_R){bbPvInverter.PVINVERTER_CURRENT_OUTPUT_R=CxString::toDouble(sBB);}
        if (sAttr ==PVINVERTER_TABLE_ATTR_PVINVERTER_ACTIVEPOWER_OUTPUT_R){bbPvInverter.PVINVERTER_ACTIVEPOWER_OUTPUT_R=CxString::toDouble(sBB);}
        if (sAttr ==PVINVERTER_TABLE_ATTR_PVINVERTER_REACTIVEPOWER_OUTPUT_R){bbPvInverter.PVINVERTER_REACTIVEPOWER_OUTPUT_R=CxString::toDouble(sBB);}
        if (sAttr ==PVINVERTER_TABLE_ATTR_PVINVERTER_POWERFACTOR_OUTPUT_R){bbPvInverter.PVINVERTER_POWERFACTOR_OUTPUT_R=CxString::toDouble(sBB);}
        if (sAttr ==PVINVERTER_TABLE_ATTR_PVINVERTER_CURRENT_PCC_R){bbPvInverter.PVINVERTER_CURRENT_PCC_R=CxString::toDouble(sBB);}
        if (sAttr ==PVINVERTER_TABLE_ATTR_PVINVERTER_ACTIVEPOWER_PCC_R){bbPvInverter.PVINVERTER_ACTIVEPOWER_PCC_R=CxString::toDouble(sBB);}
        if (sAttr ==PVINVERTER_TABLE_ATTR_PVINVERTER_REACTIVEPOWER_PCC_R){bbPvInverter.PVINVERTER_REACTIVEPOWER_PCC_R=CxString::toDouble(sBB);}
        if (sAttr ==PVINVERTER_TABLE_ATTR_PVINVERTER_POWERFACTOR_PCC_R){bbPvInverter.PVINVERTER_POWERFACTOR_PCC_R=CxString::toDouble(sBB);}
        if (sAttr ==PVINVERTER_TABLE_ATTR_PVINVERTER_VOLTAGE_PHASE_S){bbPvInverter.PVINVERTER_VOLTAGE_PHASE_S=CxString::toDouble(sBB);}
        if (sAttr ==PVINVERTER_TABLE_ATTR_PVINVERTER_CURRENT_OUTPUT_S){bbPvInverter.PVINVERTER_CURRENT_OUTPUT_S=CxString::toDouble(sBB);}
        if (sAttr ==PVINVERTER_TABLE_ATTR_PVINVERTER_ACTIVEPOWER_OUTPUT_S){bbPvInverter.PVINVERTER_ACTIVEPOWER_OUTPUT_S=CxString::toDouble(sBB);}
        if (sAttr ==PVINVERTER_TABLE_ATTR_PVINVERTER_REACTIVEPOWER_OUTPUT_S){bbPvInverter.PVINVERTER_REACTIVEPOWER_OUTPUT_S=CxString::toDouble(sBB);}
        if (sAttr ==PVINVERTER_TABLE_ATTR_PVINVERTER_POWERFACTOR_OUTPUT_S){bbPvInverter.PVINVERTER_POWERFACTOR_OUTPUT_S=CxString::toDouble(sBB);}
        if (sAttr ==PVINVERTER_TABLE_ATTR_PVINVERTER_CURRENT_PCC_S){bbPvInverter.PVINVERTER_CURRENT_PCC_S=CxString::toDouble(sBB);}
        if (sAttr ==PVINVERTER_TABLE_ATTR_PVINVERTER_ACTIVEPOWER_PCC_S){bbPvInverter.PVINVERTER_ACTIVEPOWER_PCC_S=CxString::toDouble(sBB);}
        if (sAttr ==PVINVERTER_TABLE_ATTR_PVINVERTER_REACTIVEPOWER_PCC_S){bbPvInverter.PVINVERTER_REACTIVEPOWER_PCC_S=CxString::toDouble(sBB);}
        if (sAttr ==PVINVERTER_TABLE_ATTR_PVINVERTER_POWERFACTOR_PCC_S){bbPvInverter.PVINVERTER_POWERFACTOR_PCC_S=CxString::toDouble(sBB);}
        if (sAttr ==PVINVERTER_TABLE_ATTR_PVINVERTER_VOLTAGE_PHASE_T){bbPvInverter.PVINVERTER_VOLTAGE_PHASE_T=CxString::toDouble(sBB);}
        if (sAttr ==PVINVERTER_TABLE_ATTR_PVINVERTER_CURRENT_OUTPUT_T){bbPvInverter.PVINVERTER_CURRENT_OUTPUT_T=CxString::toDouble(sBB);}
        if (sAttr ==PVINVERTER_TABLE_ATTR_PVINVERTER_ACTIVEPOWER_OUTPUT_T){bbPvInverter.PVINVERTER_ACTIVEPOWER_OUTPUT_T=CxString::toDouble(sBB);}
        if (sAttr ==PVINVERTER_TABLE_ATTR_PVINVERTER_REACTIVEPOWER_OUTPUT_T){bbPvInverter.PVINVERTER_REACTIVEPOWER_OUTPUT_T=CxString::toDouble(sBB);}
        if (sAttr ==PVINVERTER_TABLE_ATTR_PVINVERTER_POWERFACTOR_OUTPUT_T){bbPvInverter.PVINVERTER_POWERFACTOR_OUTPUT_T=CxString::toDouble(sBB);}
        if (sAttr ==PVINVERTER_TABLE_ATTR_PVINVERTER_CURRENT_PCC_T){bbPvInverter.PVINVERTER_CURRENT_PCC_T=CxString::toDouble(sBB);}
        if (sAttr ==PVINVERTER_TABLE_ATTR_PVINVERTER_ACTIVEPOWER_PCC_T){bbPvInverter.PVINVERTER_ACTIVEPOWER_PCC_T=CxString::toDouble(sBB);}
        if (sAttr ==PVINVERTER_TABLE_ATTR_PVINVERTER_REACTIVEPOWER_PCC_T){bbPvInverter.PVINVERTER_REACTIVEPOWER_PCC_T=CxString::toDouble(sBB);}
        if (sAttr ==PVINVERTER_TABLE_ATTR_PVINVERTER_POWERFACTOR_PCC_T){bbPvInverter.PVINVERTER_POWERFACTOR_PCC_T=CxString::toDouble(sBB);}
        if (sAttr ==PVINVERTER_TABLE_ATTR_PVINVERTER_ACTIVEPOWER_PV_EXT){bbPvInverter.PVINVERTER_ACTIVEPOWER_PV_EXT=CxString::toDouble(sBB);}
        if (sAttr ==PVINVERTER_TABLE_ATTR_PVINVERTER_ACTIVEPOWER_LOAD_SYS){bbPvInverter.PVINVERTER_ACTIVEPOWER_LOAD_SYS=CxString::toDouble(sBB);}
        if (sAttr ==PVINVERTER_TABLE_ATTR_PVINVERTER_ACTIVEPOWER_LOAD_TOTAL){bbPvInverter.PVINVERTER_ACTIVEPOWER_LOAD_TOTAL=CxString::toDouble(sBB);}
        if (sAttr ==PVINVERTER_TABLE_ATTR_PVINVERTER_REACTIVEPOWER_LOAD_TOTAL){bbPvInverter.PVINVERTER_REACTIVEPOWER_LOAD_TOTAL=CxString::toDouble(sBB);}
        if (sAttr ==PVINVERTER_TABLE_ATTR_PVINVERTER_APPARENTPOWER_LOAD_TOTAL){bbPvInverter.PVINVERTER_APPARENTPOWER_LOAD_TOTAL=CxString::toDouble(sBB);}
        if (sAttr ==PVINVERTER_TABLE_ATTR_PVINVERTER_FREQUENCY_OUTPUT){bbPvInverter.PVINVERTER_FREQUENCY_OUTPUT=CxString::toDouble(sBB);}
        if (sAttr ==PVINVERTER_TABLE_ATTR_PVINVERTER_VOLTAGE_OUTPUT_R){bbPvInverter.PVINVERTER_VOLTAGE_OUTPUT_R=CxString::toDouble(sBB);}
        if (sAttr ==PVINVERTER_TABLE_ATTR_PVINVERTER_CURRENT_LOAD_R){bbPvInverter.PVINVERTER_CURRENT_LOAD_R=CxString::toDouble(sBB);}
        if (sAttr ==PVINVERTER_TABLE_ATTR_PVINVERTER_ACTIVEPOWER_LOAD_R){bbPvInverter.PVINVERTER_ACTIVEPOWER_LOAD_R=CxString::toDouble(sBB);}
        if (sAttr ==PVINVERTER_TABLE_ATTR_PVINVERTER_REACTIVEPOWER_LOAD_R){bbPvInverter.PVINVERTER_REACTIVEPOWER_LOAD_R=CxString::toDouble(sBB);}
        if (sAttr ==PVINVERTER_TABLE_ATTR_PVINVERTER_APPARENTPOWER_LOAD_R){bbPvInverter.PVINVERTER_APPARENTPOWER_LOAD_R=CxString::toDouble(sBB);}
        if (sAttr ==PVINVERTER_TABLE_ATTR_PVINVERTER_LOADPEAKRATIO_R){bbPvInverter.PVINVERTER_LOADPEAKRATIO_R=CxString::toDouble(sBB);}
        if (sAttr ==PVINVERTER_TABLE_ATTR_PVINVERTER_VOLTAGE_OUTPUT_S){bbPvInverter.PVINVERTER_VOLTAGE_OUTPUT_S=CxString::toDouble(sBB);}
        if (sAttr ==PVINVERTER_TABLE_ATTR_PVINVERTER_CURRENT_LOAD_S){bbPvInverter.PVINVERTER_CURRENT_LOAD_S=CxString::toDouble(sBB);}
        if (sAttr ==PVINVERTER_TABLE_ATTR_PVINVERTER_ACTIVEPOWER_LOAD_S){bbPvInverter.PVINVERTER_ACTIVEPOWER_LOAD_S=CxString::toDouble(sBB);}
        if (sAttr ==PVINVERTER_TABLE_ATTR_PVINVERTER_REACTIVEPOWER_LOAD_S){bbPvInverter.PVINVERTER_REACTIVEPOWER_LOAD_S=CxString::toDouble(sBB);}
        if (sAttr ==PVINVERTER_TABLE_ATTR_PVINVERTER_APPARENTPOWER_LOAD_S){bbPvInverter.PVINVERTER_APPARENTPOWER_LOAD_S=CxString::toDouble(sBB);}
        if (sAttr ==PVINVERTER_TABLE_ATTR_PVINVERTER_LOADPEAKRATIO_S){bbPvInverter.PVINVERTER_LOADPEAKRATIO_S=CxString::toDouble(sBB);}
        if (sAttr ==PVINVERTER_TABLE_ATTR_PVINVERTER_VOLTAGE_OUTPUT_T){bbPvInverter.PVINVERTER_VOLTAGE_OUTPUT_T=CxString::toDouble(sBB);}
        if (sAttr ==PVINVERTER_TABLE_ATTR_PVINVERTER_CURRENT_LOAD_T){bbPvInverter.PVINVERTER_CURRENT_LOAD_T=CxString::toDouble(sBB);}
        if (sAttr ==PVINVERTER_TABLE_ATTR_PVINVERTER_ACTIVEPOWER_LOAD_T){bbPvInverter.PVINVERTER_ACTIVEPOWER_LOAD_T=CxString::toDouble(sBB);}
        if (sAttr ==PVINVERTER_TABLE_ATTR_PVINVERTER_REACTIVEPOWER_LOAD_T){bbPvInverter.PVINVERTER_REACTIVEPOWER_LOAD_T=CxString::toDouble(sBB);}
        if (sAttr ==PVINVERTER_TABLE_ATTR_PVINVERTER_APPARENTPOWER_LOAD_T){bbPvInverter.PVINVERTER_APPARENTPOWER_LOAD_T=CxString::toDouble(sBB);}
        if (sAttr ==PVINVERTER_TABLE_ATTR_PVINVERTER_LOADPEAKRATIO_T){bbPvInverter.PVINVERTER_LOADPEAKRATIO_T=CxString::toDouble(sBB);}
        if (sAttr ==PVINVERTER_TABLE_ATTR_PVINVERTER_VOLTAGE_PV1){bbPvInverter.PVINVERTER_VOLTAGE_PV1=CxString::toDouble(sBB);}
        if (sAttr ==PVINVERTER_TABLE_ATTR_PVINVERTER_CURRENT_PV1){bbPvInverter.PVINVERTER_CURRENT_PV1=CxString::toDouble(sBB);}
        if (sAttr ==PVINVERTER_TABLE_ATTR_PVINVERTER_POWER_PV1){bbPvInverter.PVINVERTER_POWER_PV1=CxString::toDouble(sBB);}
        if (sAttr ==PVINVERTER_TABLE_ATTR_PVINVERTER_VOLTAGE_PV2){bbPvInverter.PVINVERTER_VOLTAGE_PV2=CxString::toDouble(sBB);}
        if (sAttr ==PVINVERTER_TABLE_ATTR_PVINVERTER_CURRENT_PV2){bbPvInverter.PVINVERTER_CURRENT_PV2=CxString::toDouble(sBB);}
        if (sAttr ==PVINVERTER_TABLE_ATTR_PVINVERTER_POWER_PV2){bbPvInverter.PVINVERTER_POWER_PV2=CxString::toDouble(sBB);}
        if (sAttr ==PVINVERTER_TABLE_ATTR_PVINVERTER_VOLTAGE_PV3){bbPvInverter.PVINVERTER_VOLTAGE_PV3=CxString::toDouble(sBB);}
        if (sAttr ==PVINVERTER_TABLE_ATTR_PVINVERTER_CURRENT_PV3){bbPvInverter.PVINVERTER_CURRENT_PV3=CxString::toDouble(sBB);}
        if (sAttr ==PVINVERTER_TABLE_ATTR_PVINVERTER_POWER_PV3){bbPvInverter.PVINVERTER_POWER_PV3=CxString::toDouble(sBB);}
        if (sAttr ==PVINVERTER_TABLE_ATTR_PVINVERTER_VOLTAGE_PV4){bbPvInverter.PVINVERTER_VOLTAGE_PV4=CxString::toDouble(sBB);}
        if (sAttr ==PVINVERTER_TABLE_ATTR_PVINVERTER_CURRENT_PV4){bbPvInverter.PVINVERTER_CURRENT_PV4=CxString::toDouble(sBB);}
        if (sAttr ==PVINVERTER_TABLE_ATTR_PVINVERTER_POWER_PV4){bbPvInverter.PVINVERTER_POWER_PV4=CxString::toDouble(sBB);}
        if (sAttr ==PVINVERTER_TABLE_ATTR_PVINVERTER_VOLTAGE_PV5){bbPvInverter.PVINVERTER_VOLTAGE_PV5=CxString::toDouble(sBB);}
        if (sAttr ==PVINVERTER_TABLE_ATTR_PVINVERTER_CURRENT_PV5){bbPvInverter.PVINVERTER_CURRENT_PV5=CxString::toDouble(sBB);}
        if (sAttr ==PVINVERTER_TABLE_ATTR_PVINVERTER_POWER_PV5){bbPvInverter.PVINVERTER_POWER_PV5=CxString::toDouble(sBB);}
        if (sAttr ==PVINVERTER_TABLE_ATTR_PVINVERTER_VOLTAGE_PV6){bbPvInverter.PVINVERTER_VOLTAGE_PV6=CxString::toDouble(sBB);}
        if (sAttr ==PVINVERTER_TABLE_ATTR_PVINVERTER_CURRENT_PV6){bbPvInverter.PVINVERTER_CURRENT_PV6=CxString::toDouble(sBB);}
        if (sAttr ==PVINVERTER_TABLE_ATTR_PVINVERTER_POWER_PV6){bbPvInverter.PVINVERTER_POWER_PV6=CxString::toDouble(sBB);}
        if (sAttr ==PVINVERTER_TABLE_ATTR_PVINVERTER_VOLTAGE_PV7){bbPvInverter.PVINVERTER_VOLTAGE_PV7=CxString::toDouble(sBB);}
        if (sAttr ==PVINVERTER_TABLE_ATTR_PVINVERTER_CURRENT_PV7){bbPvInverter.PVINVERTER_CURRENT_PV7=CxString::toDouble(sBB);}
        if (sAttr ==PVINVERTER_TABLE_ATTR_PVINVERTER_POWER_PV7){bbPvInverter.PVINVERTER_POWER_PV7=CxString::toDouble(sBB);}
        if (sAttr ==PVINVERTER_TABLE_ATTR_PVINVERTER_VOLTAGE_PV8){bbPvInverter.PVINVERTER_VOLTAGE_PV8=CxString::toDouble(sBB);}
        if (sAttr ==PVINVERTER_TABLE_ATTR_PVINVERTER_CURRENT_PV8){bbPvInverter.PVINVERTER_CURRENT_PV8=CxString::toDouble(sBB);}
        if (sAttr ==PVINVERTER_TABLE_ATTR_PVINVERTER_POWER_PV8){bbPvInverter.PVINVERTER_POWER_PV8=CxString::toDouble(sBB);}
        if (sAttr ==PVINVERTER_TABLE_ATTR_PVINVERTER_VOLTAGE_PV9){bbPvInverter.PVINVERTER_VOLTAGE_PV9=CxString::toDouble(sBB);}
        if (sAttr ==PVINVERTER_TABLE_ATTR_PVINVERTER_CURRENT_PV9){bbPvInverter.PVINVERTER_CURRENT_PV9=CxString::toDouble(sBB);}
        if (sAttr ==PVINVERTER_TABLE_ATTR_PVINVERTER_POWER_PV9){bbPvInverter.PVINVERTER_POWER_PV9=CxString::toDouble(sBB);}
        if (sAttr ==PVINVERTER_TABLE_ATTR_PVINVERTER_VOLTAGE_PV10){bbPvInverter.PVINVERTER_VOLTAGE_PV10=CxString::toDouble(sBB);}
        if (sAttr ==PVINVERTER_TABLE_ATTR_PVINVERTER_CURRENT_PV10){bbPvInverter.PVINVERTER_CURRENT_PV10=CxString::toDouble(sBB);}
        if (sAttr ==PVINVERTER_TABLE_ATTR_PVINVERTER_POWER_PV10){bbPvInverter.PVINVERTER_POWER_PV10=CxString::toDouble(sBB);}
        if (sAttr ==PVINVERTER_TABLE_ATTR_PVINVERTER_VOLTAGE_PV11){bbPvInverter.PVINVERTER_VOLTAGE_PV11=CxString::toDouble(sBB);}
        if (sAttr ==PVINVERTER_TABLE_ATTR_PVINVERTER_CURRENT_PV11){bbPvInverter.PVINVERTER_CURRENT_PV11=CxString::toDouble(sBB);}
        if (sAttr ==PVINVERTER_TABLE_ATTR_PVINVERTER_POWER_PV11){bbPvInverter.PVINVERTER_POWER_PV11=CxString::toDouble(sBB);}
        if (sAttr ==PVINVERTER_TABLE_ATTR_PVINVERTER_VOLTAGE_PV12){bbPvInverter.PVINVERTER_VOLTAGE_PV12=CxString::toDouble(sBB);}
        if (sAttr ==PVINVERTER_TABLE_ATTR_PVINVERTER_CURRENT_PV12){bbPvInverter.PVINVERTER_CURRENT_PV12=CxString::toDouble(sBB);}
        if (sAttr ==PVINVERTER_TABLE_ATTR_PVINVERTER_POWER_PV12){bbPvInverter.PVINVERTER_POWER_PV12=CxString::toDouble(sBB);}
        if (sAttr ==PVINVERTER_TABLE_ATTR_PVINVERTER_VOLTAGE_PV13){bbPvInverter.PVINVERTER_VOLTAGE_PV13=CxString::toDouble(sBB);}
        if (sAttr ==PVINVERTER_TABLE_ATTR_PVINVERTER_CURRENT_PV13){bbPvInverter.PVINVERTER_CURRENT_PV13=CxString::toDouble(sBB);}
        if (sAttr ==PVINVERTER_TABLE_ATTR_PVINVERTER_POWER_PV13){bbPvInverter.PVINVERTER_POWER_PV13=CxString::toDouble(sBB);}
        if (sAttr ==PVINVERTER_TABLE_ATTR_PVINVERTER_VOLTAGE_PV14){bbPvInverter.PVINVERTER_VOLTAGE_PV14=CxString::toDouble(sBB);}
        if (sAttr ==PVINVERTER_TABLE_ATTR_PVINVERTER_CURRENT_PV14){bbPvInverter.PVINVERTER_CURRENT_PV14=CxString::toDouble(sBB);}
        if (sAttr ==PVINVERTER_TABLE_ATTR_PVINVERTER_POWER_PV14){bbPvInverter.PVINVERTER_POWER_PV14=CxString::toDouble(sBB);}
        if (sAttr ==PVINVERTER_TABLE_ATTR_PVINVERTER_VOLTAGE_PV15){bbPvInverter.PVINVERTER_VOLTAGE_PV15=CxString::toDouble(sBB);}
        if (sAttr ==PVINVERTER_TABLE_ATTR_PVINVERTER_CURRENT_PV15){bbPvInverter.PVINVERTER_CURRENT_PV15=CxString::toDouble(sBB);}
        if (sAttr ==PVINVERTER_TABLE_ATTR_PVINVERTER_POWER_PV15){bbPvInverter.PVINVERTER_POWER_PV15=CxString::toDouble(sBB);}
        if (sAttr ==PVINVERTER_TABLE_ATTR_PVINVERTER_VOLTAGE_PV16){bbPvInverter.PVINVERTER_VOLTAGE_PV16=CxString::toDouble(sBB);}
        if (sAttr ==PVINVERTER_TABLE_ATTR_PVINVERTER_CURRENT_PV16){bbPvInverter.PVINVERTER_CURRENT_PV16=CxString::toDouble(sBB);}
        if (sAttr ==PVINVERTER_TABLE_ATTR_PVINVERTER_POWER_PV16){bbPvInverter.PVINVERTER_POWER_PV16=CxString::toDouble(sBB);}
        if (sAttr ==PVINVERTER_TABLE_ATTR_PVINVERTER_GENERATIONTIME_TODAY){bbPvInverter.PVINVERTER_GENERATIONTIME_TODAY=CxString::toDouble(sBB);}
        if (sAttr ==PVINVERTER_TABLE_ATTR_PVINVERTER_PV_GENERATION_TODAY){bbPvInverter.PVINVERTER_PV_GENERATION_TODAY=CxString::toDouble(sBB);}
        if (sAttr ==PVINVERTER_TABLE_ATTR_PVINVERTER_PV_GENERATION_TOTAL){bbPvInverter.PVINVERTER_PV_GENERATION_TOTAL=CxString::toDouble(sBB);}
        if (sAttr ==PVINVERTER_TABLE_ATTR_PVINVERTER_LOAD_CONSUMPTION_TODAY){bbPvInverter.PVINVERTER_LOAD_CONSUMPTION_TODAY=CxString::toDouble(sBB);}
        if (sAttr ==PVINVERTER_TABLE_ATTR_PVINVERTER_LOAD_CONSUMPTION_TOTAL){bbPvInverter.PVINVERTER_LOAD_CONSUMPTION_TOTAL=CxString::toDouble(sBB);}
        if (sAttr ==PVINVERTER_TABLE_ATTR_PVINVERTER_ENERGY_PURCHASE_TODAY){bbPvInverter.PVINVERTER_ENERGY_PURCHASE_TODAY=CxString::toDouble(sBB);}
        if (sAttr ==PVINVERTER_TABLE_ATTR_PVINVERTER_ENERGY_PURCHASE_TOTAL){bbPvInverter.PVINVERTER_ENERGY_PURCHASE_TOTAL=CxString::toDouble(sBB);}
        if (sAttr ==PVINVERTER_TABLE_ATTR_PVINVERTER_ENERGY_SELLING_TODAY){bbPvInverter.PVINVERTER_ENERGY_SELLING_TODAY=CxString::toDouble(sBB);}
        if (sAttr ==PVINVERTER_TABLE_ATTR_PVINVERTER_ENERGY_SELLING_TOTAL){bbPvInverter.PVINVERTER_ENERGY_SELLING_TOTAL=CxString::toDouble(sBB);}
        if (sAttr ==PVINVERTER_TABLE_ATTR_PVINVERTER_REMOTE_ON_OFF_CONTROL){bbPvInverter.PVINVERTER_REMOTE_ON_OFF_CONTROL=CxString::toDouble(sBB);}
    }
    f_bbPvInverter = bbPvInverter;
}

int PointManager::refreshCNS_DEVINFOTablePointValue(const string &sDatabaseName, const string &sTableName,
                                                    CNS_DEVINFOTable &cnsDevinfoTable)
{
    for (int i = 0; i < f_ycPoints.size(); ++i)
    {
        YcPoint &yc = f_ycPoints.at(i);
        if (yc.DATABASE_NAME == sDatabaseName && yc.TABLE_NAME == sTableName)
        {
            if (yc.DEVICE_ATTR == CNS_DEVINFO_TABLE_ATTR_MODULE_ADDR){setYcString(&yc, cnsDevinfoTable.MODULE_ADDR, cnsDevinfoTable.RECORD_TS);}
            if (yc.DEVICE_ATTR == CNS_DEVINFO_TABLE_ATTR_MAC){setYcString(&yc, cnsDevinfoTable.MAC, cnsDevinfoTable.RECORD_TS);}
            if (yc.DEVICE_ATTR == CNS_DEVINFO_TABLE_ATTR_PRODUCT_TYPE){setYcValue(&yc, cnsDevinfoTable.PRODUCT_TYPE, cnsDevinfoTable.RECORD_TS);}
            if (yc.DEVICE_ATTR == CNS_DEVINFO_TABLE_ATTR_DEVICE_TYPE){setYcValue(&yc, cnsDevinfoTable.DEVICE_TYPE, cnsDevinfoTable.RECORD_TS);}
            if (yc.DEVICE_ATTR == CNS_DEVINFO_TABLE_ATTR_ROLE){setYcValue(&yc, cnsDevinfoTable.ROLE, cnsDevinfoTable.RECORD_TS);}
            if (yc.DEVICE_ATTR == CNS_DEVINFO_TABLE_ATTR_TEI){setYcValue(&yc, cnsDevinfoTable.TEI, cnsDevinfoTable.RECORD_TS);}
            if (yc.DEVICE_ATTR == CNS_DEVINFO_TABLE_ATTR_PROXY_TEI){setYcValue(&yc, cnsDevinfoTable.PROXY_TEI, cnsDevinfoTable.RECORD_TS);}
            if (yc.DEVICE_ATTR == CNS_DEVINFO_TABLE_ATTR_SZCHIPCODE){setYcString(&yc, cnsDevinfoTable.SZCHIPCODE, cnsDevinfoTable.RECORD_TS);}
            if (yc.DEVICE_ATTR == CNS_DEVINFO_TABLE_ATTR_SZMANUCODE){setYcString(&yc, cnsDevinfoTable.SZMANUCODE, cnsDevinfoTable.RECORD_TS);}
            if (yc.DEVICE_ATTR == CNS_DEVINFO_TABLE_ATTR_UCBOOTVER){setYcValue(&yc, cnsDevinfoTable.UCBOOTVER, cnsDevinfoTable.RECORD_TS);}
            if (yc.DEVICE_ATTR == CNS_DEVINFO_TABLE_ATTR_VRCVERSION){setYcString(&yc, cnsDevinfoTable.VRCVERSION, cnsDevinfoTable.RECORD_TS);}
            if (yc.DEVICE_ATTR == CNS_DEVINFO_TABLE_ATTR_us_year){setYcValue(&yc, cnsDevinfoTable.us_year, cnsDevinfoTable.RECORD_TS);}
            if (yc.DEVICE_ATTR == CNS_DEVINFO_TABLE_ATTR_us_month){setYcValue(&yc, cnsDevinfoTable.us_month, cnsDevinfoTable.RECORD_TS);}
            if (yc.DEVICE_ATTR == CNS_DEVINFO_TABLE_ATTR_us_day){setYcValue(&yc, cnsDevinfoTable.us_day, cnsDevinfoTable.RECORD_TS);}
            if (yc.DEVICE_ATTR == CNS_DEVINFO_TABLE_ATTR_PHASE_STATUS){setYcValue(&yc, cnsDevinfoTable.PHASE_STATUS, cnsDevinfoTable.RECORD_TS);}
            if (yc.DEVICE_ATTR == CNS_DEVINFO_TABLE_ATTR_PHASE_RESULT){setYcValue(&yc, cnsDevinfoTable.PHASE_RESULT, cnsDevinfoTable.RECORD_TS);}
            if (yc.DEVICE_ATTR == CNS_DEVINFO_TABLE_ATTR_PHASE_RESULT_B){setYcValue(&yc, cnsDevinfoTable.PHASE_RESULT_B, cnsDevinfoTable.RECORD_TS);}
            if (yc.DEVICE_ATTR == CNS_DEVINFO_TABLE_ATTR_PHASE_RESULT_C){setYcValue(&yc, cnsDevinfoTable.PHASE_RESULT_C, cnsDevinfoTable.RECORD_TS);}
            if (yc.DEVICE_ATTR == CNS_DEVINFO_TABLE_ATTR_phase_flag){setYcValue(&yc, cnsDevinfoTable.phase_flag, cnsDevinfoTable.RECORD_TS);}
            if (yc.DEVICE_ATTR == CNS_DEVINFO_TABLE_ATTR_STATUS){setYcValue(&yc, cnsDevinfoTable.STATUS, cnsDevinfoTable.RECORD_TS);}
            if (yc.DEVICE_ATTR == CNS_DEVINFO_TABLE_ATTR_LOCATION){setYcString(&yc, cnsDevinfoTable.LOCATION, cnsDevinfoTable.RECORD_TS);}
            if (yc.DEVICE_ATTR == CNS_DEVINFO_TABLE_ATTR_PROXY_CHANGE_CNT){setYcValue(&yc, cnsDevinfoTable.PROXY_CHANGE_CNT, cnsDevinfoTable.RECORD_TS);}
            if (yc.DEVICE_ATTR == CNS_DEVINFO_TABLE_ATTR_LEAVE_CNT){setYcValue(&yc, cnsDevinfoTable.LEAVE_CNT, cnsDevinfoTable.RECORD_TS);}
            if (yc.DEVICE_ATTR == CNS_DEVINFO_TABLE_ATTR_LEAVE_TOTAL_TIME){setYcValue(&yc, cnsDevinfoTable.LEAVE_TOTAL_TIME, cnsDevinfoTable.RECORD_TS);}
            if (yc.DEVICE_ATTR == CNS_DEVINFO_TABLE_ATTR_JOIN_NET_TIME){setYcValue(&yc, cnsDevinfoTable.JOIN_NET_TIME, cnsDevinfoTable.RECORD_TS);}
        }
    }
    return 0;
}

int PointManager::refreshCNS_DEV_STATUSTablePointValue(const string &sDatabaseName, const string &sTableName,
                                                     CNS_DEV_STATUSTable &cnsDevStatusTable)
{
    for (int i = 0; i < f_ycPoints.size(); ++i)
    {
        YcPoint &yc = f_ycPoints.at(i);
        if (yc.DATABASE_NAME == sDatabaseName && yc.TABLE_NAME == sTableName)
        {
            //            if (yc.DEVICE_ATTR == CNS_DEVINFO_TABLE_ATTR_MAC){setYcValue(&yc, cnsDevinfoTable.MAC, cnsDevinfoTable.RECORD_TS);}
            if (yc.DEVICE_ATTR == CNS_DEV_STATUS_TABLE_ATTR_ROLE){setYcValue(&yc, cnsDevStatusTable.ROLE, cnsDevStatusTable.RECORD_TS);}
            if (yc.DEVICE_ATTR == CNS_DEV_STATUS_TABLE_ATTR_STATUS){setYcValue(&yc, cnsDevStatusTable.STATUS, cnsDevStatusTable.RECORD_TS);}
        }
    }
    return 0;
}

int PointManager::refreshCNS_TOPOTablePointValue(const string &sDatabaseName, const string &sTableName,
                                                 CNS_TOPOTable &cnsTOPOTable)
{
    for (int i = 0; i < f_ycPoints.size(); ++i)
    {
        YcPoint &yc = f_ycPoints.at(i);
        if (yc.DATABASE_NAME == sDatabaseName && yc.TABLE_NAME == sTableName)
        {
            if (yc.DEVICE_ATTR == CNS_TOPO_TABLE_ATTR_ROLE){setYcValue(&yc, cnsTOPOTable.ROLE, cnsTOPOTable.RECORD_TS);}
            if (yc.DEVICE_ATTR == CNS_TOPO_TABLE_ATTR_LEVEL){setYcValue(&yc, cnsTOPOTable.LEVEL, cnsTOPOTable.RECORD_TS);}
            if (yc.DEVICE_ATTR == CNS_TOPO_TABLE_ATTR_TEI){setYcValue(&yc, cnsTOPOTable.TEI, cnsTOPOTable.RECORD_TS);}
            if (yc.DEVICE_ATTR == CNS_TOPO_TABLE_ATTR_PROXY_TEI){setYcValue(&yc, cnsTOPOTable.PROXY_TEI, cnsTOPOTable.RECORD_TS);}
        }
    }
    return 0;
}

int PointManager::refreshCNS_CNS_SNR_PARTablePointValue(const string &sDatabaseName, const string &sTableName,
                                                        CNS_CNS_SNR_PARTable &cnsCnsSnrParTable)
{
    for (int i = 0; i < f_ycPoints.size(); ++i)
    {
        YcPoint &yc = f_ycPoints.at(i);
        if (yc.DATABASE_NAME == sDatabaseName && yc.TABLE_NAME == sTableName)
        {
            //            if (yc.DEVICE_ATTR == CNS_DEVINFO_TABLE_ATTR_MAC){setYcValue(&yc, cnsDevinfoTable.MAC, cnsDevinfoTable.RECORD_TS);}
            if (yc.DEVICE_ATTR == CNS_CNS_SNR_PAR_TABLE_ATTR_SNR){setYcValue(&yc, cnsCnsSnrParTable.SNR, cnsCnsSnrParTable.RECORD_TS);}
            if (yc.DEVICE_ATTR == CNS_CNS_SNR_PAR_TABLE_ATTR_PAR){setYcValue(&yc, cnsCnsSnrParTable.PAR, cnsCnsSnrParTable.RECORD_TS);}
        }
    }
    return 0;
}

int PointManager::refreshVOCTablePointValue(const string &sDatabaseName, const string &sTableName, TVOC_table &tvocTable)
{
    for (int i = 0; i < f_ycPoints.size(); ++i)
    {
        YcPoint &yc = f_ycPoints.at(i);
        if (yc.DATABASE_NAME == sDatabaseName && yc.TABLE_NAME == sTableName)
        {
            if (yc.DEVICE_ATTR == C_TVOC_TABLE_ATTR_TVOC){setYcValue(&yc, tvocTable.TVOC, tvocTable.RECORD_TS);}
        }
    }
    return 0;
}

int PointManager::refreshSENSORLEAKTablePointValue(const string &sDatabaseName, const string &sTableName,
                                                   SENSORLEAK_table &sensorleakTable)
{
    for (int i = 0; i < f_ycPoints.size(); ++i)
    {
        YcPoint &yc = f_ycPoints.at(i);
        if (yc.DATABASE_NAME == sDatabaseName && yc.TABLE_NAME == sTableName)
        {
            if (yc.DEVICE_ATTR == C_SENSORLEAK_TABLE_ATTR_LEAKSTATE){setYcValue(&yc, sensorleakTable.LEAKSTATE, sensorleakTable.RECORD_TS);}
        }
    }
    return 0;
}

int PointManager::refreshNOISETablePointValue(const string &sDatabaseName, const string &sTableName,
                                              NOISE_table &noiseTable)
{
    for (int i = 0; i < f_ycPoints.size(); ++i)
    {
        YcPoint &yc = f_ycPoints.at(i);
        if (yc.DATABASE_NAME == sDatabaseName && yc.TABLE_NAME == sTableName)
        {
            if (yc.DEVICE_ATTR == C_NOISE_TABLE_ATTR_NOISE){setYcValue(&yc, noiseTable.NOISE, noiseTable.RECORD_TS);}
        }
    }
    return 0;
}

int PointManager::refreshO2TablePointValue(const string &sDatabaseName, const string &sTableName,
                                              O2_table &o2Table)
{
    for (int i = 0; i < f_ycPoints.size(); ++i)
    {
        YcPoint &yc = f_ycPoints.at(i);
        if (yc.DATABASE_NAME == sDatabaseName && yc.TABLE_NAME == sTableName)
        {
            if (yc.DEVICE_ATTR == C_O2_TABLE_ATTR_O2){setYcValue(&yc, o2Table.O2, o2Table.RECORD_TS);}
        }
    }
    return 0;
}

int PointManager::refreshO3TablePointValue(const string &sDatabaseName, const string &sTableName,
                                              O3_table &o3Table)
{
    for (int i = 0; i < f_ycPoints.size(); ++i)
    {
        YcPoint &yc = f_ycPoints.at(i);
        if (yc.DATABASE_NAME == sDatabaseName && yc.TABLE_NAME == sTableName)
        {
            if (yc.DEVICE_ATTR == C_O3_TABLE_ATTR_O3){setYcValue(&yc, o3Table.O3, o3Table.RECORD_TS);}
        }
    }
    return 0;
}

int PointManager::refreshQXTablePointValue(const string &sDatabaseName, const string &sTableName,
                                              QX_table &qxTable)
{
    for (int i = 0; i < f_ycPoints.size(); ++i)
    {
        YcPoint &yc = f_ycPoints.at(i);
        if (yc.DATABASE_NAME == sDatabaseName && yc.TABLE_NAME == sTableName)
        {
            if (yc.DEVICE_ATTR == C_QX_TABLE_ATTR_TEMPERATURE){setYcValue(&yc, qxTable.TEMPERATURE, qxTable.RECORD_TS);}
            if (yc.DEVICE_ATTR == C_QX_TABLE_ATTR_HUMIDITY){setYcValue(&yc, qxTable.HUMIDITY, qxTable.RECORD_TS);}
            if (yc.DEVICE_ATTR == C_QX_TABLE_ATTR_NAP){setYcValue(&yc, qxTable.NAP, qxTable.RECORD_TS);}
            if (yc.DEVICE_ATTR == C_QX_TABLE_ATTR_WIND_SPEED){setYcValue(&yc, qxTable.WIND_SPEED, qxTable.RECORD_TS);}
            if (yc.DEVICE_ATTR == C_QX_TABLE_ATTR_WIND_DIRECTION){setYcValue(&yc, qxTable.WIND_DIRECTION, qxTable.RECORD_TS);}
            if (yc.DEVICE_ATTR == C_QX_TABLE_ATTR_RAINFULL){setYcValue(&yc, qxTable.RAINFULL, qxTable.RECORD_TS);}
        }
    }
    return 0;
}

int PointManager::refreshSF6TablePointValue(const string &sDatabaseName, const string &sTableName,
                                           SF6_table &sf6Table)
{
    for (int i = 0; i < f_ycPoints.size(); ++i)
    {
        YcPoint &yc = f_ycPoints.at(i);
        if (yc.DATABASE_NAME == sDatabaseName && yc.TABLE_NAME == sTableName)
        {
            if (yc.DEVICE_ATTR == C_SF6_TABLE_ATTR_SF6){setYcValue(&yc, sf6Table.SF6, sf6Table.RECORD_TS);}
        }
    }
    return 0;
}

int PointManager::refreshOILTablePointValue(const string &sDatabaseName, const string &sTableName,
                                           OIL_table &oilTable)
{
    for (int i = 0; i < f_ycPoints.size(); ++i)
    {
        YcPoint &yc = f_ycPoints.at(i);
        if (yc.DATABASE_NAME == sDatabaseName && yc.TABLE_NAME == sTableName)
        {
            if (yc.DEVICE_ATTR == C_OIL_TABLE_ATTR_R){setYcValue(&yc, oilTable.R, oilTable.RECORD_TS);}
            if (yc.DEVICE_ATTR == C_OIL_TABLE_ATTR_G){setYcValue(&yc, oilTable.G, oilTable.RECORD_TS);}
            if (yc.DEVICE_ATTR == C_OIL_TABLE_ATTR_B){setYcValue(&yc, oilTable.B, oilTable.RECORD_TS);}
            if (yc.DEVICE_ATTR == C_OIL_TABLE_ATTR_state){setYcValue(&yc, oilTable.state, oilTable.RECORD_TS);}
        }
    }
    return 0;
}

int PointManager::refreshSENSORGUZHITablePointValue(const std::string &sDatabaseName, const string &sTableName, SENSORGUZHI_table &sensorguzhiTable)
{
    for (int i = 0; i < f_ycPoints.size(); ++i)
    {
        YcPoint &yc = f_ycPoints.at(i);
        if (yc.DATABASE_NAME == sDatabaseName && yc.TABLE_NAME == sTableName)
        {
            if (yc.DEVICE_ATTR == DEVICE_TABLE_ATTR_GUZHI_PHASE_A_VOLT) { setYcValue(&yc, sensorguzhiTable.GUZHI_PHASE_A_VOLT, sensorguzhiTable.RECORD_TS);}
            if (yc.DEVICE_ATTR == DEVICE_TABLE_ATTR_GUZHI_PHASE_B_VOLT) { setYcValue(&yc, sensorguzhiTable.GUZHI_PHASE_B_VOLT, sensorguzhiTable.RECORD_TS);}
            if (yc.DEVICE_ATTR == DEVICE_TABLE_ATTR_GUZHI_PHASE_C_VOLT) { setYcValue(&yc, sensorguzhiTable.GUZHI_PHASE_C_VOLT, sensorguzhiTable.RECORD_TS);}
            if (yc.DEVICE_ATTR == DEVICE_TABLE_ATTR_GUZHI_PHASE_A_ELEC) { setYcValue(&yc, sensorguzhiTable.GUZHI_PHASE_A_ELEC, sensorguzhiTable.RECORD_TS);}
            if (yc.DEVICE_ATTR == DEVICE_TABLE_ATTR_GUZHI_PHASE_B_ELEC) { setYcValue(&yc, sensorguzhiTable.GUZHI_PHASE_B_ELEC, sensorguzhiTable.RECORD_TS);}
            if (yc.DEVICE_ATTR == DEVICE_TABLE_ATTR_GUZHI_PHASE_C_ELEC) { setYcValue(&yc, sensorguzhiTable.GUZHI_PHASE_C_ELEC, sensorguzhiTable.RECORD_TS);}
            if (yc.DEVICE_ATTR == DEVICE_TABLE_ATTR_RECORD_TS) { setYcValue(&yc, (sensorguzhiTable.RECORD_TS - SEPOCH_2022) / 60, sensorguzhiTable.RECORD_TS); }
        }
    }
    return 0;
}

int PointManager::refreshRFIDTablePointValue(const string &sDatabaseName, const string &sTableName,
                                            RFID_table &rfidTable)
{
    for (int i = 0; i < f_ycPoints.size(); ++i)
    {
        YcPoint &yc = f_ycPoints.at(i);
        if (yc.DATABASE_NAME == sDatabaseName && yc.TABLE_NAME == sTableName)
        {
            if (yc.DEVICE_ATTR == C_RFID_TABLE_ATTR_RFID_0){setYcValue(&yc, rfidTable.RFID_0, rfidTable.RECORD_TS);}
            if (yc.DEVICE_ATTR == C_RFID_TABLE_ATTR_RFID_1){setYcValue(&yc, rfidTable.RFID_1, rfidTable.RECORD_TS);}
            if (yc.DEVICE_ATTR == C_RFID_TABLE_ATTR_RFID_2){setYcValue(&yc, rfidTable.RFID_2, rfidTable.RECORD_TS);}
            if (yc.DEVICE_ATTR == C_RFID_TABLE_ATTR_RFID_3){setYcValue(&yc, rfidTable.RFID_3, rfidTable.RECORD_TS);}
            if (yc.DEVICE_ATTR == C_RFID_TABLE_ATTR_RFID_4){setYcValue(&yc, rfidTable.RFID_4, rfidTable.RECORD_TS);}
            if (yc.DEVICE_ATTR == C_RFID_TABLE_ATTR_RFID_5){setYcValue(&yc, rfidTable.RFID_5, rfidTable.RECORD_TS);}
            if (yc.DEVICE_ATTR == C_RFID_TABLE_ATTR_RFID_6){setYcValue(&yc, rfidTable.RFID_6, rfidTable.RECORD_TS);}
            if (yc.DEVICE_ATTR == C_RFID_TABLE_ATTR_RFID_7){setYcValue(&yc, rfidTable.RFID_7, rfidTable.RECORD_TS);}
            if (yc.DEVICE_ATTR == C_RFID_TABLE_ATTR_RFID_8){setYcValue(&yc, rfidTable.RFID_8, rfidTable.RECORD_TS);}
        }
    }
    return 0;
}

void PointManager::setYcString(YcPoint *yc, const string &value, msepoch_t t)
{
    yc->VString = value;
    yc->QUALITY = QUALITY_VALID;
    msepoch_t dt = 0;
    if (t > 1000000000000 && t < 9999999999999) {
        dt = t;
    } else if (t > 1000000000 && t < 9999999999) {
        dt = t * 1000;
    }
    yc->TIME_MS = dt;
    yc->TIME24 = toCP24Time2a(dt);
    yc->TIME56 = toCP56Time2a(dt);
}

const YcPoint * PointManager::refreshABCByModelAttr(const std::string &sModelGuid)
{
    YcPoint *abc = NULL;
    double av=0, ai=0, bv=0, bi=0, cv=0, ci=0;
    msepoch_t dtABC = 0;
    for (int j = 0; j < f_ycPoints.size(); ++j)
    {
        YcPoint &yc = f_ycPoints.at(j);
        if (yc.MODEL_GUID == sModelGuid)
        {
            if (yc.DEVICE_ATTR == DEVICE_TABLE_ATTR_METER_PHASE_A_VOLT) { av = yc.VALUE; if (av > 0) dtABC = yc.TIME_MS; }
            if (yc.DEVICE_ATTR == DEVICE_TABLE_ATTR_METER_PHASE_B_VOLT) { bv = yc.VALUE; if (bv > 0) dtABC = yc.TIME_MS; }
            if (yc.DEVICE_ATTR == DEVICE_TABLE_ATTR_METER_PHASE_C_VOLT) { cv = yc.VALUE; if (cv > 0) dtABC = yc.TIME_MS; }
            if (yc.DEVICE_ATTR == DEVICE_TABLE_ATTR_METER_PHASE_A_ELEC) { ai = yc.VALUE; if (av > 0) dtABC = yc.TIME_MS; }
            if (yc.DEVICE_ATTR == DEVICE_TABLE_ATTR_METER_PHASE_B_ELEC) { bi = yc.VALUE; if (bv > 0) dtABC = yc.TIME_MS; }
            if (yc.DEVICE_ATTR == DEVICE_TABLE_ATTR_METER_PHASE_C_ELEC) { ci = yc.VALUE; if (cv > 0) dtABC = yc.TIME_MS; }
            if (yc.MODEL_ATTR == "ABC") abc = &yc;
        }
    }
    if (abc != NULL)
    {
        if ((av > 0 || ai > 0) && (bv > 0 || bi > 0) && (cv > 0 || ci > 0))
        {
            setYcString(abc, "ABC", dtABC);
        }
        else if (av > 0 || ai > 0)
        {
            setYcString(abc, "A", dtABC);
        }
        else if (bv > 0 || bi > 0)
        {
            setYcString(abc, "B", dtABC);
        }
        else if (cv > 0 || ci > 0)
        {
            setYcString(abc, "C", dtABC);
        }
        else
        {
            setYcString(abc, "RST", CxTime::currentMsepoch());
        }
    }
    return abc;
}

int PointManager::refreshDAILYTablePointValue(const string &sDatabaseName, const string &sTableName,
                                              DAILY_Table &dailyTable)
{
    for (int i = 0; i < f_ycPoints.size(); ++i)
    {
        YcPoint &yc = f_ycPoints.at(i);
        if (yc.DATABASE_NAME == sDatabaseName && yc.TABLE_NAME == sTableName)
        {
            if (yc.DEVICE_ATTR == C_DAILY_TABLE_ATTR_PHASE_INFO){setYcValue(&yc, dailyTable.PHASE_INFO, dailyTable.RECORD_TS);}
            if (yc.DEVICE_ATTR == C_DAILY_TABLE_ATTR_POWER_METER_ENERGE){setYcValue(&yc, dailyTable.POWER_METER_ENERGE, dailyTable.RECORD_TS);}
        }
    }
    return 0;
}

// 22/06/07/11
static msepoch_t ONTIME_FREEZEN2(const std::string &sPOWER_METER_ENERGE)
{
    if (sPOWER_METER_ENERGE.size() < 11)
    {
        return 0;
    }
    string sDate(sPOWER_METER_ENERGE.c_str());
    sDate.resize(8);
    string sHour = sPOWER_METER_ENERGE.substr(9, 2);
    string sMinute = sPOWER_METER_ENERGE.size() >= 14 ? sPOWER_METER_ENERGE.substr(12, 2) : "00";
    string sDt = "20" + sDate + " " + sHour + ":" + sMinute + ":00:000";
    return CxTime::fromString(sDt);
}

int PointManager::refreshONTIMETablePointValue(const string &sDatabaseName, const string &sTableName,
                                              ONTIME_Table &dailyTable)
{
    msepoch_t dtOnTime = ONTIME_FREEZEN2(dailyTable.ONTIME_FREEZEN);
    for (int i = 0; i < f_ycPoints.size(); ++i)
    {
        YcPoint &yc = f_ycPoints.at(i);
        if (yc.DATABASE_NAME == sDatabaseName && yc.TABLE_NAME == sTableName)
        {
            if (yc.DEVICE_ATTR == C_ONTIME_TABLE_ATTR_PHASE_INFO){setYcValue(&yc, dailyTable.PHASE_INFO, dailyTable.RECORD_TS);}
            if (yc.DEVICE_ATTR == C_ONTIME_TABLE_ATTR_ONTIME_POSITIVE_ENERGEY){setYcValue(&yc, dailyTable.ONTIME_POSITIVE_ENERGEY, dtOnTime);}
            if (yc.DEVICE_ATTR == C_ONTIME_TABLE_ATTR_ONTIME_REACTIVE_ENERGEY){setYcValue(&yc, dailyTable.ONTIME_REACTIVE_ENERGEY, dtOnTime);}
            if (yc.DEVICE_ATTR == C_ONTIME_TABLE_ATTR_ONTIME_FREEZEN){ setYcValue(&yc, (double)dtOnTime, dailyTable.RECORD_TS);}
        }
    }
    return 0;
}

std::string PointManager::getCharLastString(const std::string &str, const char ch)
{
    int pos = str.find_last_of(ch);
    return str.substr(pos+1);
}

std::string PointManager::ycToString()
{
    std::string res;

    std::map<std::string, std::string> deviceDatas;

    for (int i = 0; i < f_ycPoints.size(); i++)
    {
        YcPoint &currentYC = f_ycPoints.at(i);
        std::string currentDeviceID = getCharLastString(currentYC.TABLE_NAME, '_');

        fnInfo().out(".YCToString - currentDeviceID: [%s]", currentDeviceID.c_str());

        if (currentDeviceID != "")
        {
            deviceDatas[currentDeviceID] = "";
        }
    }

    for (std::map<std::string, std::string>::iterator iter = deviceDatas.begin(); iter != deviceDatas.end(); iter++)
    {
        std::string tmp;
        for (int i = 0; i < f_ycPoints.size(); ++i)
        {
            YcPoint &currentYC = f_ycPoints.at(i);
            std::string currentDeviceID = getCharLastString(currentYC.TABLE_NAME, '_');
            if (iter->first == currentDeviceID)
            {
                tmp += CxString::format("%0.3f,", currentYC.VALUE);
            }
        }
        tmp = tmp.substr(0, tmp.length()-1);
        res += CxString::format("%s,%s\r\n", iter->first.c_str(), tmp.c_str());
    }

    fnInfo().out(".YCToString - nextDeviceID: [%s]", res.c_str());
    return res;
}
