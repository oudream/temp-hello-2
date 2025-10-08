#include "mqtt_model_device.h"

#include <ccxx/cxstring.h>
#include <ccxx/cxinterinfo.h>
#include <ccxx/cxinterinfo_group_define.h>


using namespace std;


CXOUTINFO_DEFINE(IIG_NORMAL, IIG_NORMAL, "MQTT-MODEL-DEVICE")


#define CS_YC_POINT_TABLE_SELECT \
    "SELECT `ID`, `DATABASE_NAME`, `TABLE_NAME`, `DEVICE_ATTR`, `CA`, `IOA`, `TYPE_ID`, `NORMALIZED_MAX`, `SCALED_FACTOR`, `PRECISION`, `MODEL`, `MODEL_ATTR`, `MODEL_ATTR_TYPE`, `MODEL_GUID`, `MODEL_DEAD_ZONE`, `MODEL_RATIO`, `MODEL_IS_REPORT`, `MODEL_USER_DEFINE`, `RES1`, `RES2`, `RES3` FROM `IOT_YC_POINT_TABLE`;"

#define CS_YX_POINT_TABLE_SELECT \
    "SELECT `ID`, `DATABASE_NAME`, `TABLE_NAME`, `DEVICE_ATTR`, `POINT_TYPE`, `CA`, `IOA`, `TYPE_ID`, `EVENT_TYPE`, `MODEL`, `MODEL_ATTR`, `MODEL_ATTR_TYPE`, `MODEL_EVENT_TYPE`, `MODEL_EVENT_PARAM`, `MODEL_GUID`, `RES1`, `RES2`, `RES3` FROM `IOT_YX_POINT_TABLE`;"

#define CS_IOT_DEVICE_TABLE_SELECT \
    "SELECT `ID`, `PARENT_DEVICE_ID`, `NODE_ID`, `DEVICE_ID`, `TOKEN`, `NAME`, `CODE`, `DESCRIPTION`, `MANUFACTURER_ID`, `MODEL`, `PRODUCT_ID`, `FW_VERSION`, `SW_VERSION`, `IOT_VERSION`, `WRITE_TIME`, `STATUS`, `EXTENSION_INFO`, `APP_NAME`, `DEVICE_ADDRESS`, `ONLINE_FLAG`, `SLAVE`, `MANUFACTURER_NAME`, `DEVICE_PORT`, `DEV`, `PROTOCOL_TYPE`, `IS_REPORT` FROM `IOT_DEVICE_TABLE`;"


static std::vector<IotDevice> f_iotDevices;

static std::map<std::string, std::vector<YcPoint>> f_modelYcPoints;

static std::map<std::string, std::vector<YxPoint>> f_modelYxPoints;


void MqttModelDevice::start()
{
    sqlite3 *sqliteDb = DBManager::openSqlite(DBManager::getDbFilePath(DB_NAME_GW_IOT));
    SQL::Con db(sqliteDb);
    // IotDevice
    {
        fnInfo().out(".start - LOAD IOT Device ... ");
        vector<IotDevice> vs;
        {
            auto a = db.bindnquery<int, string, string, string, string, string, string, string, string, string, string, int, int, int, int, int, string, string, string, string, string, string, string, string, string, string>(CS_IOT_DEVICE_TABLE_SELECT);
            for (auto x:a)
            {
                IotDevice m;
                PointManager::resetIotDevice(&m);
                m.ID = get<0>(x);
                m.PARENT_DEVICE_ID = get<1>(x);
                m.NODE_ID = get<2>(x);
                m.DEVICE_ID = get<3>(x);
                m.TOKEN = get<4>(x);
                m.NAME = get<5>(x);
                m.CODE = get<6>(x);
                m.DESCRIPTION = get<7>(x);
                m.MANUFACTURER_ID = get<8>(x);
                m.MODEL = get<9>(x);
                m.PRODUCT_ID = get<10>(x);
                m.FW_VERSION = get<11>(x);
                m.SW_VERSION = get<12>(x);
                m.IOT_VERSION = get<13>(x);
                m.WRITE_TIME = get<14>(x);
                m.STATUS = get<15>(x);
                m.EXTENSION_INFO = get<16>(x);
                m.APP_NAME = get<17>(x);
                m.DEVICE_ADDRESS = get<18>(x);
                m.ONLINE_FLAG = get<19>(x);
                m.SLAVE = get<20>(x);
                m.MANUFACTURER_NAME = get<21>(x);
                m.DEVICE_PORT = get<22>(x);
                m.DEV = get<23>(x);
                m.PROTOCOL_TYPE = get<24>(x);
                m.IS_REPORT = get<25>(x);
                vs.push_back(m);
                fnInfo().out(".start - LOAD IOT Device [MODEL: %s] [CODE: %s] [DEVICE_ID: %s]", m.MODEL.c_str(), m.CODE.c_str(), m.DEVICE_ID.c_str());
            }
        }
        f_iotDevices = vs;
        fnInfo().out(".start - LOAD IOT Device Count[%d] Complete.", vs.size());
    }
    {
        auto a = db.bindnquery<int, string, string, string, int, int, int, double, double, int, string, string, string, string, string, string, string, string, string, string, string, string>(CS_YC_POINT_TABLE_SELECT);
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
            m.CA = get<4>(x);
            m.IOA = get<5>(x);
            // IEC60870_5_TypeID
            m.TYPE_ID = get<6>(x);
            m.NORMALIZED_MAX = get<7>(x);
            m.SCALED_FACTOR = get<8>(x);
            m.PRECISION = get<9>(x);
            // mqtt
            m.MODEL = get<10>(x);
            m.MODEL_ATTR = get<11>(x);
            m.MODEL_ATTR_TYPE = get<12>(x);
            m.MODEL_GUID = get<13>(x);
            m.MODEL_DEAD_ZONE = get<14>(x);
            m.MODEL_RATIO = get<15>(x);
            m.MODEL_IS_REPORT = get<16>(x);
            m.MODEL_USER_DEFINE = get<17>(x);
            m.RES1 = get<18>(x);
            m.RES2 = get<19>(x);
            m.MODEL_UNIT = get<20>(x);
            m.Tag = true;
            pushModelYcPoints(m.MODEL, m);
        }
    }
    {
        auto a = db.bindnquery<int, string, string, string, int, int, int, int, int, string, string, string, string, string, string, string, string, string>(CS_YX_POINT_TABLE_SELECT);
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
            pushModelYxPoints(m.MODEL, m);
        }
    }
    DBManager::closeSqlite(sqliteDb);
}

void MqttModelDevice::pushModelYcPoints(const string & model, const YcPoint& yc)
{
    map<string, vector<YcPoint>>::iterator it = f_modelYcPoints.find(model);
    if (it != f_modelYcPoints.end())
    {
        vector<YcPoint> & ycs = it->second;
        ycs.push_back(yc);
        fnInfo().out(".start - LOAD MODEL YC- [MODEL... %s %s] [DB... %s %s %s]", yc.MODEL.c_str(), yc.MODEL_ATTR.c_str(), yc.DATABASE_NAME.c_str(), yc.TABLE_NAME.c_str(), yc.DEVICE_ATTR.c_str());
    }
    else
    {
        vector<YcPoint> ycs;
        ycs.push_back(yc);
        f_modelYcPoints[model] = ycs;
        fnInfo().out(".start - LOAD MODEL YC* [MODEL... %s %s] [DB... %s %s %s]", yc.MODEL.c_str(), yc.MODEL_ATTR.c_str(), yc.DATABASE_NAME.c_str(), yc.TABLE_NAME.c_str(), yc.DEVICE_ATTR.c_str());
    }
}

void MqttModelDevice::pushModelYxPoints(const string &model, const YxPoint &yx)
{
    map<string, vector<YxPoint>>::iterator it = f_modelYxPoints.find(model);
    if (it != f_modelYxPoints.end())
    {
        vector<YxPoint> & yxs = it->second;
        yxs.push_back(yx);
        fnInfo().out(".start - LOAD MODEL YX- [MODEL... %s %s %s %s] [DB... %s %s %s]", yx.MODEL.c_str(), yx.MODEL_ATTR.c_str(), yx.MODEL_ATTR_TYPE.c_str(), yx.MODEL_EVENT_TYPE.c_str(), yx.DATABASE_NAME.c_str(), yx.TABLE_NAME.c_str(), yx.DEVICE_ATTR.c_str());
    }
    else
    {
        vector<YxPoint> yxs;
        yxs.push_back(yx);
        f_modelYxPoints[model] = yxs;
        fnInfo().out(".start - LOAD MODEL YX* [MODEL... %s %s %s %s] [DB... %s %s %s]", yx.MODEL.c_str(), yx.MODEL_ATTR.c_str(), yx.MODEL_ATTR_TYPE.c_str(), yx.MODEL_EVENT_TYPE.c_str(), yx.DATABASE_NAME.c_str(), yx.TABLE_NAME.c_str(), yx.DEVICE_ATTR.c_str());
    }
}

const std::vector<IotDevice> &MqttModelDevice::getIotDevices()
{
    return f_iotDevices;
}

const std::map<std::string, std::vector<YcPoint>> &MqttModelDevice::getModelYcPoint()
{
    return f_modelYcPoints;
}

const std::map<std::string, std::vector<YxPoint>> &MqttModelDevice::getModelYxPoint()
{
    return f_modelYxPoints;
}

std::vector<YcPoint> MqttModelDevice::loadAllYcPoint()
{
    std::vector<YcPoint> r;
    fnDebug().out(".loadAllYcPoint - [ MODEL * DEVICES : %d * %d ]", f_modelYcPoints.size(), f_iotDevices.size());
    for (int i = 0; i < f_iotDevices.size(); ++i)
    {
        const IotDevice &device = f_iotDevices[i];
        if (device.CODE.empty() || device.DEVICE_ID.empty())
        {
            fnWarn().out(".loadAllYcPoint - IOT Device [Code: %s] [DEVICE_ID: %s] Can Not Find Model Yc Point", device.CODE.c_str(), device.DEVICE_ID.c_str());
            continue;
        }
        map<string, vector<YcPoint>>::const_iterator it = f_modelYcPoints.find(device.MODEL);
        if (it != f_modelYcPoints.end())
        {
            const std::string &model = it->first;
            const vector<YcPoint> &modelYcs = it->second;
            vector<string> attrs;
            for (int j = 0; j < modelYcs.size(); ++j)
            {
                const YcPoint &modelYc = modelYcs[j];
                YcPoint yc = modelYc;
                yc.TABLE_NAME = tableNameByCode(modelYc.TABLE_NAME, device.CODE);
                string::size_type posDeviceId = yc.TABLE_NAME.find_last_of('_');
                if (posDeviceId != string::npos)
                {
                    yc.DeviceId = yc.TABLE_NAME.substr(posDeviceId+1);
                }
                yc.MODEL_GUID = device.DEVICE_ID;
                yc.VALUE = MATT_ATTR_INVALID_VALUE;
                yc.OldVALUE = MATT_ATTR_INVALID_VALUE;
                yc.QUALITY = QUALITY_INVALID;
                yc.TIME_MS = 0;
                yc.SHORT_VALUE = MATT_ATTR_INVALID_VALUE;
                yc.NORMALIZED_VALUE = MATT_ATTR_INVALID_VALUE;
                yc.SCALED_VALUE = 0;
                yc.TIME24 = {0};
                yc.TIME56 = {0};
                r.push_back(yc);
                attrs.push_back(modelYc.MODEL_ATTR);
            }
            string sAttrs = CxString::join(attrs, ',');
            fnInfo().out(".loadAllYcPoint - YcPoint [MODEL & DEVICE: %s %s] [MODEL_ATTRS: %s]", model.c_str(), device.DEVICE_ID.c_str(), sAttrs.c_str());
        }
        else
        {
            fnError().out(".loadAllYcPoint - YcPoint [MODEL & DEVICE: %s %s], Can Not Find Model", device.MODEL.c_str(), device.DEVICE_ID.c_str());
        }
     }
    fnInfo().out(".loadAllYcPoint -- COMPLETE. Load Yc Point [%d] .", r.size());
    return r;
}

std::vector<YxPoint> MqttModelDevice::loadAllYxPoint()
{
    std::vector<YxPoint> r;
    fnDebug().out(".loadAllYxPoint - [ MODEL * DEVICES : %d * %d ]", f_modelYxPoints.size(), f_iotDevices.size());
    for (int i = 0; i < f_iotDevices.size(); ++i)
    {
        const IotDevice &device = f_iotDevices[i];
        if (device.CODE.empty() || device.DEVICE_ID.empty())
        {
            fnWarn().out(".loadAllYxPoint - IOT Device [Code: %s] [DEVICE_ID: %s] Can Not Find Model Yx Point", device.CODE.c_str(), device.DEVICE_ID.c_str());
            continue;
        }
        map<string, vector<YxPoint>>::const_iterator it = f_modelYxPoints.find(device.MODEL);
        if (it != f_modelYxPoints.end())
        {
            const std::string &model = it->first;
            const vector<YxPoint> &modelYxs = it->second;
            vector<string> attrs;
            for (int j = 0; j < modelYxs.size(); ++j)
            {
                const YxPoint &modelYx = modelYxs[j];
                YxPoint yx = modelYx;
                yx.TABLE_NAME = tableNameByCode(modelYx.TABLE_NAME, device.CODE);
                string::size_type posDeviceId = yx.TABLE_NAME.find_last_of('_');
                if (posDeviceId != string::npos)
                {
                    yx.DeviceId = yx.TABLE_NAME.substr(posDeviceId+1);
                }
                yx.MODEL_GUID = device.DEVICE_ID;
                yx.VALUE = MATT_ATTR_INVALID_VALUE;
                yx.OldVALUE = MATT_ATTR_INVALID_VALUE;
                yx.QUALITY = QUALITY_INVALID;
                yx.TIME_MS = 0;
                yx.SINGLE_VALUE = MATT_ATTR_INVALID_VALUE;
                yx.DOUBLE_VALUE = MATT_ATTR_INVALID_VALUE;
                yx.TIME24 = {0};
                yx.TIME56 = {0};
                r.push_back(yx);
                attrs.push_back(modelYx.MODEL_ATTR);
            }
            string sAttrs = CxString::join(attrs, ',');
            fnInfo().out(".loadAllYxPoint - YxPoint [MODEL & DEVICE: %s %s] [MODEL_ATTRS: %s]", model.c_str(), device.DEVICE_ID.c_str(), sAttrs.c_str());
        }
        else
        {
            fnError().out(".loadAllYxPoint - YxPoint [MODEL & DEVICE: %s %s], Can Not Find Model", device.MODEL.c_str(), device.DEVICE_ID.c_str());
        }
    }
    fnInfo().out(".loadAllYcPoint -- COMPLETE. Load Yx Point [%d] .", r.size());
    return r;
}

std::string MqttModelDevice::tableNameByCode(const string &prefix, const string &code)
{
    if (prefix == "AREA_DAILY_TABLE" || prefix == "AREA_MONTHLY_TABLE" || prefix == "AREA_ANNUAL_TABLE")
    {
        return prefix;
    }
    return prefix+"_"+code;
}

const IotDevice * MqttModelDevice::getIotDeviceByCode(const string &sDeviceCode)
{
    for (int i = 0; i < f_iotDevices.size(); ++i)
    {
        const IotDevice &device = f_iotDevices.at(i);
        if (device.CODE == sDeviceCode)
        {
            return &device;
        }
    }
    return NULL;
}

std::vector<std::string> MqttModelDevice::getDeviceCodes(const vector<std::string> &sDeviceIds)
{
    std::vector<std::string> r;
    for (int i = 0; i < sDeviceIds.size(); ++i)
    {
        const std::string &sDeviceId = sDeviceIds.at(i);
        for (int j = 0; j < f_iotDevices.size(); ++j)
        {
            const IotDevice &device = f_iotDevices.at(j);
            if (device.DEVICE_ID == sDeviceId)
            {
                r.push_back(device.CODE);
            }
        }

    }
    return r;
}
