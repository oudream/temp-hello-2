#ifndef MQTT_UTIL_H
#define MQTT_UTIL_H


class MqttUtil {
public:
    static int GetEncryptedPassword(const char *password, char **timestamp, char **encryptedPwd);

    // return: 2022022816
    static char* GetClientTimesStamp();

};


#endif //MQTT_UTIL_H
