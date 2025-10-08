#ifndef MQTT_MODEL_DEVICE_H
#define MQTT_MODEL_DEVICE_H


#include "point_manager.h"

#define MATT_ATTR_INVALID_VALUE (-1)

class MqttModelDevice
{
public:
    static void start();

    static const std::vector<IotDevice> &getIotDevices();

    static std::vector<std::string> getDeviceCodes(const std::vector<std::string> &sDeviceIds);

    static const IotDevice * getIotDeviceByCode(const std::string &sDeviceCode);

    static const std::map<std::string, std::vector<YcPoint>> &getModelYcPoint();

    static const std::map<std::string, std::vector<YxPoint>> &getModelYxPoint();

    static std::vector<YcPoint> loadAllYcPoint();

    static std::vector<YxPoint> loadAllYxPoint();

private:
    static std::string tableNameByCode(const std::string &prefix, const std::string &code);

    static void pushModelYcPoints(const std::string &model, const YcPoint &yc);

    static void pushModelYxPoints(const std::string &model, const YxPoint &yx);

};


#endif //MQTT_MODEL_DEVICE_H
