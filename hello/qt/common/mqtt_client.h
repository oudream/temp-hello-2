#ifndef MQTT_JIEXI_H
#define MQTT_JIEXI_H


#include <ccxx/cxglobal.h>


typedef void (*fn_mqtt_connected_t)(const std::string &cause);
typedef void (*fn_mqtt_received_payload_t)(const std::string &topic, const std::string &payload);
typedef void (*fn_mqtt_received_token_t)(const std::string &sFunName, int iToken, const std::string &topic);


class MqttClient
{
public:
    // sub thread
    static void registConnected(fn_mqtt_connected_t fn);

    // sub thread
    static void registConnectionLost(fn_mqtt_connected_t fn);

    // sub thread
    static void registReceivedPayload(fn_mqtt_received_payload_t fn);

    // sub thread
    static void registReceivedToken(fn_mqtt_received_token_t fn);

    static void init(const char *ip, int port, const char *clientId, const char *username = NULL, const char *password = NULL);

    static void setUsername(const char *username);

    static void setPassword(const char *password);

    static void checkConnect(msepoch_t dtNow, const char *ip, int port, const char *clientId, const char *username = NULL, const char *password = NULL);

    static void unInit();

    static bool isConnected();

    static void subscribe(const std::string &topic);

    static int publish(const std::string &topic, const std::string &payload, int timeout = 0);

};


#endif //MQTT_JIEXI_H
