
```c
// 摇控：拓扑文件启动 IOA（遥控地址）：24577
#define APP_CONST_TOPOLOGY_FILE_IOA "TOPOLOGY_FILE_IOA"
// 连接主站的IP地址：192.168.1.3
#define APP_CONST_JIEXI_MQTT_IP "JIEXI_MQTT_IP"
// 连接主站的遥信、遥测通道端口号，一般为：2404
#define APP_CONST_JIEXI_MQTT_PORT "JIEXI_MQTT_PORT"
// 连接主站的传文件通道端口号，一般为：2402
#define APP_CONST_CCS_UPLOAD_C6PORT "CCS_UPLOAD_C6PORT"
// 转发网关变化上传（SOE）功能[1:启动；0:不启动]，默认：1
#define APP_CONST_SOE_ENABLED "SOE_ENABLED"
// 发链路测试的时间间隔（如果这间隔内收到对方链路测试就主动发链路测试，为了与别厂家20秒错开，默认25秒）
#define APP_CONST_TEST_FR_TIMEOUT "TEST_FR_TIMEOUT"
// 测试：随机定时模仿网关发出SOE的DB与TABLE事件，例如：modbus=METER_TABLE_0353000050022948;modbus=SWITCH_TABLE_kaiguan1
#define APP_CONST_TEST_SOE_DB_TABLES "TEST_SOE_DB_TABLES"
```
