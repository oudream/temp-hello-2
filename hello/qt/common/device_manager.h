#ifndef HELLO_IEC104_DEVICE_MANAGER_H
#define HELLO_IEC104_DEVICE_MANAGER_H

#include "db_manager.h"

#include "device_ddl.h"

class DeviceManager
{
public:
    static void start();

    static std::string getDeviceTableName(int iDeviceType, const std::string &sDeviceId);

    static bool getLastGUZHITable(sqlite3 *sqlite, const std::string &sTableName, GUZHITable *r);

    static bool getLastMETERTable(sqlite3 *sqlite, const std::string &sTableName, METERTable *r);

    static bool getLastSWITCHTable(sqlite3 *sqlite, const std::string &sTableName, SWITCHTable *r);

    static bool getLastYXTABLE(sqlite3 *sqlite, const std::string &sTableName, YXTABLE *r);

    static bool getLastAREADAILYTable(sqlite3 *sqlite, const std::string &sTableName, AREADAILYTable *r);

    static bool getLastAREAMONTHLYTable(sqlite3 *sqlite, const std::string &sTableName, AREAMONTHLYTable *r);

    static bool getLastPOWERLOSSTable(sqlite3 *sqlite, const std::string &sTableName, POWERLOSSTable *r);

    static bool getLastAREAANNUALTable(sqlite3 *sqlite, const std::string &sTableName, AREAMETERYEARSTable *r);

    static bool getLastCHARGETable(sqlite3 *sqlite, const std::string &sTableName, CHARGETable *r);

    static bool getLastBRANCHTable(sqlite3 *sqlite, const std::string &sTableName, BRANCHTable *r);

    static bool getLastPHASEBLANCETable(sqlite3 *sqlite, const std::string &sTableName, PHASEBLANCETable *r);

    static bool getLastCHARGEYXTable(sqlite3 *sqlite, const std::string &sTableName, CHARGEYXTable *r);

    static bool getLastBRANCHMONTH(sqlite3 *sqlite, const std::string &sTableName, BRANCHTable *r);

    static bool getLastTHTABLE(sqlite3 *sqlite, const std::string &sTableName, THTable *r);

    static bool getLastSENSORSMOKETABLE(sqlite3 *sqlite, const std::string &sTableName, SENSORSMOKETable *r);

    static bool getLastJIEXI_MQTT_METER_TABLE_test(sqlite3 *sqlite, const std::string &sTableName, JIEXI_MQTT_METER_TABLE_test *r);

    static int insertJIEXI_MQTT_METER_TABLE_test(sqlite3 *sqlite, const std::string &sTableName, const JIEXI_MQTT_METER_TABLE_test &r);

    static bool getLastIRTABLE(sqlite3 *sqlite, const std::string &sTableName, IRTable *r);

    static bool getLastWATERDEEPTABLE(sqlite3 *sqlite, const std::string &sTableName, WATERDEEPTable *r);

    static bool getLastSENSORDOORTABLE(sqlite3 *sqlite, const std::string &sTableName, SENSORDOORTable *r);

    static bool getLastSENSORLEAKTABLE(sqlite3 *sqlite, const std::string &sTableName, SENSORDOORTable *r);

    static bool getLastRELAYPLCTABLE(sqlite3 *sqlite, const std::string &sTableName, SENSORDOORTable *r);

    static bool getLastBOPOTABLE(sqlite3 *sqlite, const std::string &sTableName, SENSORDOORTable *r);

    static bool getLastPVINVERTERTable(sqlite3 *sqlite, const std::string &sTableName, PVINVERTERTable *r);

    static bool getLastP485METERTable(sqlite3 *sqlite, const std::string &sTableName, P485METERTable *r);

    static bool getLastCNSDevInfoTable(sqlite3 *sqlite, const std::string &sTableName, CNS_DEVINFOTable *r);

    static bool getLastCNSDevStatusTable(sqlite3 *sqlite, const std::string &sTableName, CNS_DEV_STATUSTable *r);

    static bool getLastCNSTOPOTable(sqlite3 *sqlite, const std::string &sTableName, CNS_TOPOTable *r);

    static bool getLastCNSSnrParTable(sqlite3 *sqlite, const std::string &sTableName, CNS_CNS_SNR_PARTable *r);

    static bool getLastVOCTable(sqlite3 *sqlite, const std::string &sTableName, TVOC_table *r);

    static bool getLastSENSORLEAKTable(sqlite3 *sqlite, const std::string &sTableName, SENSORLEAK_table *r);

    static bool getLastNOISETable(sqlite3 *sqlite, const std::string &sTableName, NOISE_table *r);

    static bool getLastDAILYTable(sqlite3 *sqlite, const std::string &sTableName, DAILY_Table *r);

    static bool getLastONTIMETable(sqlite3 *sqlite, const std::string &sTableName, ONTIME_Table *r);

    static bool getLastO2Table(sqlite3 *sqlite, const std::string &sTableName, O2_table *r);
    
    static bool getLastO3Table(sqlite3 *sqlite, const std::string &sTableName, O3_table *r);

    static bool getLastQXTable(sqlite3 *sqlite, const std::string &sTableName, QX_table *r);

    static bool getLastSF6Table(sqlite3 *sqlite, const std::string &sTableName, SF6_table *r);
    
    static bool getLastOILTable(sqlite3 *sqlite, const std::string &sTableName, OIL_table *r);

    static bool getLastSENSORGUZHITable(sqlite3 *sqlite, const std::string &sTableName, SENSORGUZHI_table *r);

    static bool getLastRFIDTable(sqlite3 *sqlite, const std::string &sTableName, RFID_table *r);

};

#endif //HELLO_IEC104_DEVICE_MANAGER_H
