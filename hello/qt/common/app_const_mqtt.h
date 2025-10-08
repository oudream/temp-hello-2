#ifndef APP_CONST_MQTT_H
#define APP_CONST_MQTT_H

#include "app_const.h"

//
#define APP_CONST_TABLE_NAME "MQTT_CONST_TABLE"

//
#define APP_CONST_JIEXI_MQTT_IP "JIEXI_MQTT_IP"
#define APP_CONST_JIEXI_MQTT_PORT "JIEXI_MQTT_PORT"
#define APP_CONST_JIEXI_TEST_SOE_TIMEOUT "JIEXI_TEST_SOE_TIMEOUT"
#define APP_CONST_JIEXI_TEST_SOE_DB_TABLES "JIEXI_TEST_SOE_DB_TABLES"
#define APP_CONST_JIEXI_MQTT_MODEL_SINGLE_NAME "JIEXI_MQTT_MODEL_SINGLE_NAME"
#define APP_CONST_JIEXI_MQTT_MODEL_SINGLE_CONTENT "JIEXI_MQTT_MODEL_SINGLE_CONTENT"

//
#define DEFAULT_JIEXI_MQTT_MODEL_NAME "Meter_single"
#define DEFAULT_JIEXI_MQTT_MODEL_CONTENT "{\"token\":\"%lld\",\"timestamp\":\"%s\",\"model\":\"Meter_single\",\"body\":[{\"name\":\"tgP\",\"type\":\"float\",\"unit\":\"kW\",\"deadzone\":\"0.01\",\"ratio\":\"1\",\"isReport\":\"1\",\"userdefine\":\"总有功功率\"},{\"name\":\"tgPa\",\"type\":\"float\",\"unit\":\"kW\",\"deadzone\":\"0.01\",\"ratio\":\"1\",\"isReport\":\"1\",\"userdefine\":\"A相有功功率 \"},{\"name\":\"tgPb\",\"type\":\"float\",\"unit\":\"kW\",\"deadzone\":\"0.01\",\"ratio\":\"1\",\"isReport\":\"1\",\"userdefine\":\"B相有功功率 \"},{\"name\":\"tgPc\",\"type\":\"float\",\"unit\":\"kW\",\"deadzone\":\"0.01\",\"ratio\":\"1\",\"isReport\":\"1\",\"userdefine\":\"C相有功功率 \"},{\"name\":\"tgUa\",\"type\":\"float\",\"unit\":\"V\",\"deadzone\":\"0.01\",\"ratio\":\"1\",\"isReport\":\"1\",\"userdefine\":\"A相电压\"},{\"name\":\"tgUb\",\"type\":\"float\",\"unit\":\"V\",\"deadzone\":\"0.01\",\"ratio\":\"1\",\"isReport\":\"1\",\"userdefine\":\"B相电压\"},{\"name\":\"tgUc\",\"type\":\"float\",\"unit\":\"V\",\"deadzone\":\"0.01\",\"ratio\":\"1\",\"isReport\":\"1\",\"userdefine\":\"C相电压\"},{\"name\":\"tgIa\",\"type\":\"float\",\"unit\":\"A\",\"deadzone\":\"0.01\",\"ratio\":\"1\",\"isReport\":\"1\",\"userdefine\":\"A相电流\"},{\"name\":\"tgIb\",\"type\":\"float\",\"unit\":\"A\",\"deadzone\":\"0.01\",\"ratio\":\"1\",\"isReport\":\"1\",\"userdefine\":\"B相电流\"},{\"name\":\"tgIc\",\"type\":\"float\",\"unit\":\"A\",\"deadzone\":\"0.01\",\"ratio\":\"1\",\"isReport\":\"1\",\"userdefine\":\"C相电流\"},{\"name\":\"tgI0\",\"type\":\"float\",\"unit\":\"A\",\"deadzone\":\"0.01\",\"ratio\":\"1\",\"isReport\":\"1\",\"userdefine\":\"零线电流\"},{\"name\":\"tgSupWh\",\"type\":\"float\",\"unit\":\"kWh\",\"deadzone\":\"0.01\",\"ratio\":\"1\",\"isReport\":\"1\",\"userdefine\":\"正向有功总电能\"},{\"name\":\"tgSupWhA\",\"type\":\"float\",\"unit\":\"kWh\",\"deadzone\":\"0.01\",\"ratio\":\"1\",\"isReport\":\"1\",\"userdefine\":\"A相正向有功电能\"},{\"name\":\"tgSupWhB\",\"type\":\"float\",\"unit\":\"kWh\",\"deadzone\":\"0.01\",\"ratio\":\"1\",\"isReport\":\"1\",\"userdefine\":\"B相正向有功电能\"},{\"name\":\"tgSupWhC\",\"type\":\"float\",\"unit\":\"kWh\",\"deadzone\":\"0.01\",\"ratio\":\"1\",\"isReport\":\"1\",\"userdefine\":\"C相正向有功电能\"}]}"

#endif //APP_CONST_MQTT_H
