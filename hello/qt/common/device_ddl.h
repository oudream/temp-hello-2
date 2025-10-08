#ifndef DEVICE_DDL_H
#define DEVICE_DDL_H

#include "sqlite3/sqlite3.h"
#include <string>

// --- --- ---
// --- DEVICE_TABLE
// --- --- ---

typedef enum DeviceTypeEnum
{
    DEVICE_TYPE_T_METER,
    DEVICE_TYPE_TEMP_HUMI,
    DEVICE_TYPE_TEMPCONTROLLER,
    DEVICE_TYPE_SENSORSMOKE,
    DEVICE_TYPE_SENSORLEAK,
    DEVICE_TYPE_SENSORLEAK485,
    DEVICE_TYPE_SENSORDOOR,
    DEVICE_TYPE_WIRELESSTEMPTURE,
    DEVICE_TYPE_POWER_METER,
    DEVICE_TYPE_SWITCH,
    DEVICE_TYPE_GUZHI,
    DEVICE_TYPE_GATEWAY,
    DEVICE_TYPE_CHARGE_KELU,    // 科陆充电设备
    DEVICE_TYPE_WATER_DEEP,  // 水深
    DEVICE_TYPE_IR,    // 红外
    DEVICE_TYPE_BOPO,    // 玻璃破碎传感器
    DEVICE_TYPE_METER_645,    // 台区645设备，直接通过645协议读取数据
    DEVICE_TYPE_LIGHT_TABLE,    // 光照传感器 17
    DEVICE_TYPE_RELAYPLC_TABLE,    // 载波开关，deviceNo编号0xb0~bf;
    DEVICE_TYPE_PVINVERTER_TABLE,    // 光伏逆变器,deviceNo编号0xc0~cf;
    DEVICE_TYPE_P485METER_TABLE,    // 485型导轨表,deviceNo编号0xd0~df;
    DEVICE_TYPE_MAX_DEVICE_TABLE
}DEVICE_TYPE_ENUM;

typedef struct DeviceTable
{
    int ID;
    std::string DATABASE_NAME;
    int DEVICE_TYPE;
    std::string DEVICE_ID;
    std::string TABLE_NAME;
    std::string DEVICE_MANUFACTOR;
    std::string DEVICE_SW_VER;
    std::string DEVICE_HW_VER;
    int DEVICE_MEM;
    std::string DEVICE_CPU;
} DeviceTable;
//
#define C_DEVICE_TABLE_SELECT \
    "SELECT `ID`, `DATABASE_NAME`, `DEVICE_TYPE`, `DEVICE_ID`, `DEVICE_MANUFACTOR`, `DEVICE_SW_VER`, `DEVICE_HW_VER`, `DEVICE_MEM`, `DEVICE_CPU`" \
    " FROM `DEVICE_TABLE`"


// --- --- ---
// --- GUZHI_TABLE
// --- --- ---
#define C_GUZHI_TABLE_PREFIX "GUZHI_TABLE"
#define C_GUZHI_TABLE_CREATE \
    "CREATE TABLE `%s                                                                        \n" \
    "(                                                                                       \n" \
    "	 `ID`         					   		 integer NOT NULL PRIMARY KEY AUTOINCREMENT, \n" \
    "    `GUZHI_CPU_OC`                          real    default 0,                          \n" \
    "    `GUZHI_PHASE_A_VOLT`                    real    default 0,                          \n" \
    "    `GUZHI_PHASE_B_VOLT`                    real    default 0,                          \n" \
    "    `GUZHI_PHASE_C_VOLT`                    real    default 0,                          \n" \
    "    `GUZHI_PHASE_A_ELEC`                    real    default 0,                          \n" \
    "    `GUZHI_PHASE_B_ELEC`                    real    default 0,                          \n" \
    "    `GUZHI_PHASE_C_ELEC`                    real    default 0,                          \n" \
    "    `GUZHI_POSTIVE_ENERGE`                  real   default 0,                           \n" \
    "    `GUZHI_REVERSE_ENERGE`                  real   default 0,                           \n" \
    "    `RECORD_TS`                             integer default -1                          \n" \
    ");                                                                                      "
//
#define C_GUZHI_TABLE_INSERT \
		"INSERT INTO `%s`(`GUZHI_CPU_OC`, `GUZHI_PHASE_A_VOLT`, `GUZHI_PHASE_B_VOLT`, `GUZHI_PHASE_C_VOLT`," \
		" `GUZHI_PHASE_A_ELEC`, `GUZHI_PHASE_B_ELEC`, `GUZHI_PHASE_C_ELEC`," \
		" `GUZHI_POSTIVE_ENERGE`, `GUZHI_REVERSE_ENERGE`, `RECORD_TS`)" \
		" VALUES" \
		" (?, ?, ?, ?, ?, ?, ?, ?, ?, ?);"
//
#define C_GUZHI_TABLE_SELECT \
		"SELECT `ID`, `GUZHI_CPU_OC`, `GUZHI_PHASE_A_VOLT`, `GUZHI_PHASE_B_VOLT`, `GUZHI_PHASE_C_VOLT`," \
		" `GUZHI_PHASE_A_ELEC`, `GUZHI_PHASE_B_ELEC`, `GUZHI_PHASE_C_ELEC`," \
		" `GUZHI_POSTIVE_ENERGE`, `GUZHI_REVERSE_ENERGE`, `RECORD_TS`" \
		" FROM `%s`" \
		" WHERE `ID` = (SELECT MAX(`ID`) FROM `%s`)"
//
#define DEVICE_TABLE_ATTR_ID "ID"
#define DEVICE_TABLE_ATTR_GUZHI_CPU_OC "GUZHI_CPU_OC"
#define DEVICE_TABLE_ATTR_GUZHI_PHASE_A_VOLT "GUZHI_PHASE_A_VOLT"
#define DEVICE_TABLE_ATTR_GUZHI_PHASE_B_VOLT "GUZHI_PHASE_B_VOLT"
#define DEVICE_TABLE_ATTR_GUZHI_PHASE_C_VOLT "GUZHI_PHASE_C_VOLT"
#define DEVICE_TABLE_ATTR_GUZHI_PHASE_A_ELEC "GUZHI_PHASE_A_ELEC"
#define DEVICE_TABLE_ATTR_GUZHI_PHASE_B_ELEC "GUZHI_PHASE_B_ELEC"
#define DEVICE_TABLE_ATTR_GUZHI_PHASE_C_ELEC "GUZHI_PHASE_C_ELEC"
#define DEVICE_TABLE_ATTR_GUZHI_POSTIVE_ENERGE "GUZHI_POSTIVE_ENERGE"
#define DEVICE_TABLE_ATTR_GUZHI_REVERSE_ENERGE "GUZHI_REVERSE_ENERGE"
#define DEVICE_TABLE_ATTR_RECORD_TS "RECORD_TS"
//
typedef struct GUZHITable
{
    int ID;
    double GUZHI_CPU_OC;
    double GUZHI_PHASE_A_VOLT;
    double GUZHI_PHASE_B_VOLT;
    double GUZHI_PHASE_C_VOLT;
    double GUZHI_PHASE_A_ELEC;
    double GUZHI_PHASE_B_ELEC;
    double GUZHI_PHASE_C_ELEC;
    double GUZHI_POSTIVE_ENERGE;
    double GUZHI_REVERSE_ENERGE;
    sqlite3_int64 RECORD_TS;
} GUZHITable;


// --- --- ---
// --- METER_TABLE
// --- --- ---
#define C_METER_TABLE_PREFIX "METER_TABLE"
#define C_METER_TABLE_CREATE \
    "CREATE TABLE `%s`                                                                            \n" \
    "(                                                                                            \n" \
    "	 `ID`         					   			   integer NOT NULL PRIMARY KEY AUTOINCREMENT,\n" \
    "    `POWER_METER_CPU_OC`                          real    default 0,                         \n" \
    "    `POWER_METER_ENERGE`                          real    default 0,                         \n" \
    "    `POWER_METER_FORTH_HAVE_POWER_TOTAL`          real    default 0,                         \n" \
    "    `POWER_METER_BACK_HAVE_POWER_TOTAL`           real    default 0,                         \n" \
    "    `POWER_METER_PHASE_A_FORTH_HAVE_POWER`        real    default 0,                         \n" \
    "    `POWER_METER_PHASE_A_BACK_HAVE_POWER`         real    default 0,                         \n" \
    "    `POWER_METER_PHASE_B_FORTH_HAVE_POWER`        real    default 0,                         \n" \
    "    `POWER_METER_PHASE_B_BACK_HAVE_POWER`         real    default 0,                         \n" \
    "    `POWER_METER_PHASE_C_FORTH_HAVE_POWER`        real    default 0,                         \n" \
    "    `POWER_METER_PHASE_C_BACK_HAVE_POWER`         real    default 0,                         \n" \
    "    `POWER_METER_PHASE_A_VOLT`                    real    default 0,                         \n" \
    "    `POWER_METER_PHASE_B_VOLT`                    real    default 0,                         \n" \
    "    `POWER_METER_PHASE_C_VOLT`                    real    default 0,                         \n" \
    "    `POWER_METER_PHASE_A_ELEC`                    real    default 0,                         \n" \
    "    `POWER_METER_PHASE_B_ELEC`                    real    default 0,                         \n" \
    "    `POWER_METER_PHASE_C_ELEC`                    real    default 0,                         \n" \
    "    `POWER_METER_PHASE_A_ENERGE` 				   real    default 0,                         \n" \
    "    `POWER_METER_PHASE_B_ENERGE` 				   real    default 0,                         \n" \
    "    `POWER_METER_PHASE_C_ENERGE` 				   real    default 0,                         \n" \
    "    `POWER_METER_POWER_FATOR_A`                   real    default 0,                         \n" \
    "    `POWER_METER_POWER_FATOR_B`                   real    default 0,                         \n" \
    "    `POWER_METER_POWER_FATOR_C`                   real    default 0,                         \n" \
    "    `POWER_METER_POWER_FATOR`                     real    default 0,                         \n" \
    "    `RECORD_TS`                              	   integer default -1                         \n" \
    ");                                                                                     "
//
#define C_METER_TABLE_INSERT \
		"INSERT INTO `%s`(`POWER_METER_CPU_OC`, `POWER_METER_ENERGE`," \
		" `POWER_METER_FORTH_HAVE_POWER_TOTAL`, `POWER_METER_BACK_HAVE_POWER_TOTAL`, `POWER_METER_PHASE_A_FORTH_HAVE_POWER`," \
		" `POWER_METER_PHASE_A_BACK_HAVE_POWER`, `POWER_METER_PHASE_B_FORTH_HAVE_POWER`, `POWER_METER_PHASE_B_BACK_HAVE_POWER`," \
		" `POWER_METER_PHASE_C_FORTH_HAVE_POWER`, `POWER_METER_PHASE_C_BACK_HAVE_POWER`, `POWER_METER_PHASE_A_VOLT`, `POWER_METER_PHASE_B_VOLT`," \
		" `POWER_METER_PHASE_C_VOLT`, `POWER_METER_PHASE_A_ELEC`, `POWER_METER_PHASE_B_ELEC`, `POWER_METER_PHASE_C_ELEC`,`POWER_METER_PHASE_A_ENERGE`," \
		" `POWER_METER_PHASE_B_ENERGE`, `POWER_METER_PHASE_C_ENERGE`,`POWER_METER_POWER_FATOR_A`," \
		" `POWER_METER_POWER_FATOR_B`,`POWER_METER_POWER_FATOR_C`,`POWER_METER_POWER_FATOR`, `RECORD_TS`)" \
		" VALUES" \
		" (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?);"
//
#define C_METER_TABLE_SELECT \
			"SELECT `ID`, `POWER_METER_CPU_OC`, `POWER_METER_ENERGE`, `POWER_METER_BACK_ENERGE`," \
			" `POWER_METER_FORTH_HAVE_POWER_TOTAL`, `POWER_METER_BACK_HAVE_POWER_TOTAL`, `POWER_METER_PHASE_A_FORTH_HAVE_POWER`," \
			" `POWER_METER_PHASE_A_BACK_HAVE_POWER`, `POWER_METER_PHASE_B_FORTH_HAVE_POWER`, `POWER_METER_PHASE_B_BACK_HAVE_POWER`," \
			" `POWER_METER_PHASE_C_FORTH_HAVE_POWER`, `POWER_METER_PHASE_C_BACK_HAVE_POWER`, `POWER_METER_PHASE_A_VOLT`, `POWER_METER_PHASE_B_VOLT`," \
			" `POWER_METER_PHASE_C_VOLT`, `POWER_METER_PHASE_A_ELEC`, `POWER_METER_PHASE_B_ELEC`, `POWER_METER_PHASE_C_ELEC`,`POWER_METER_PHASE_A_ENERGE`," \
			" `POWER_METER_PHASE_B_ENERGE`, `POWER_METER_PHASE_C_ENERGE`,`POWER_METER_POWER_FATOR_A`," \
			" `POWER_METER_POWER_FATOR_B`,`POWER_METER_POWER_FATOR_C`,`POWER_METER_POWER_FATOR`, `RECORD_TS`" \
		" FROM `%s`" \
		" WHERE `ID` = (SELECT MAX(`ID`) FROM `%s`)"
//
#define DEVICE_TABLE_ATTR_ID "ID"
#define DEVICE_TABLE_ATTR_METER_CPU_OC "POWER_METER_CPU_OC"
#define DEVICE_TABLE_ATTR_METER_ENERGE "POWER_METER_ENERGE"
#define DEVICE_TABLE_ATTR_METER_BACK_ENERGE "POWER_METER_BACK_ENERGE"
#define DEVICE_TABLE_ATTR_METER_FORTH_HAVE_POWER_TOTAL "POWER_METER_FORTH_HAVE_POWER_TOTAL"
#define DEVICE_TABLE_ATTR_METER_BACK_HAVE_POWER_TOTAL "POWER_METER_BACK_HAVE_POWER_TOTAL"
#define DEVICE_TABLE_ATTR_METER_PHASE_A_FORTH_HAVE_POWER "POWER_METER_PHASE_A_FORTH_HAVE_POWER"
#define DEVICE_TABLE_ATTR_METER_PHASE_A_BACK_HAVE_POWER "POWER_METER_PHASE_A_BACK_HAVE_POWER"
#define DEVICE_TABLE_ATTR_METER_PHASE_B_FORTH_HAVE_POWER "POWER_METER_PHASE_B_FORTH_HAVE_POWER"
#define DEVICE_TABLE_ATTR_METER_PHASE_B_BACK_HAVE_POWER "POWER_METER_PHASE_B_BACK_HAVE_POWER"
#define DEVICE_TABLE_ATTR_METER_PHASE_C_FORTH_HAVE_POWER "POWER_METER_PHASE_C_FORTH_HAVE_POWER"
#define DEVICE_TABLE_ATTR_METER_PHASE_C_BACK_HAVE_POWER "POWER_METER_PHASE_C_BACK_HAVE_POWER"
#define DEVICE_TABLE_ATTR_METER_PHASE_A_VOLT "POWER_METER_PHASE_A_VOLT"
#define DEVICE_TABLE_ATTR_METER_PHASE_B_VOLT "POWER_METER_PHASE_B_VOLT"
#define DEVICE_TABLE_ATTR_METER_PHASE_C_VOLT "POWER_METER_PHASE_C_VOLT"
#define DEVICE_TABLE_ATTR_METER_PHASE_A_ELEC "POWER_METER_PHASE_A_ELEC"
#define DEVICE_TABLE_ATTR_METER_PHASE_B_ELEC "POWER_METER_PHASE_B_ELEC"
#define DEVICE_TABLE_ATTR_METER_PHASE_C_ELEC "POWER_METER_PHASE_C_ELEC"
#define DEVICE_TABLE_ATTR_METER_PHASE_A_ENERGE "POWER_METER_PHASE_A_ENERGE"
#define DEVICE_TABLE_ATTR_METER_PHASE_B_ENERGE "POWER_METER_PHASE_B_ENERGE"
#define DEVICE_TABLE_ATTR_METER_PHASE_C_ENERGE "POWER_METER_PHASE_C_ENERGE"
#define DEVICE_TABLE_ATTR_METER_POWER_FATOR_A  "POWER_METER_POWER_FATOR_A"
#define DEVICE_TABLE_ATTR_METER_POWER_FATOR_B  "POWER_METER_POWER_FATOR_B"
#define DEVICE_TABLE_ATTR_METER_POWER_FATOR_C  "POWER_METER_POWER_FATOR_C"
#define DEVICE_TABLE_ATTR_METER_POWER_FATOR "POWER_METER_POWER_FATOR"
#define DEVICE_TABLE_ATTR_RECORD_TS "RECORD_TS"
//
typedef struct METERTable
{
    int ID;
    double POWER_METER_CPU_OC;
    double POWER_METER_ENERGE;
    double POWER_METER_BACK_ENERGE;
    double POWER_METER_FORTH_HAVE_POWER_TOTAL;          // 总有功
    double POWER_METER_BACK_HAVE_POWER_TOTAL;           // 总无功
    double POWER_METER_PHASE_A_FORTH_HAVE_POWER;        // A相有功
    double POWER_METER_PHASE_A_BACK_HAVE_POWER;         // A相无功
    double POWER_METER_PHASE_B_FORTH_HAVE_POWER;        // B相有功
    double POWER_METER_PHASE_B_BACK_HAVE_POWER;         // B相无功
    double POWER_METER_PHASE_C_FORTH_HAVE_POWER;        // C相有功
    double POWER_METER_PHASE_C_BACK_HAVE_POWER;         // C相无功
    double POWER_METER_PHASE_A_VOLT;
    double POWER_METER_PHASE_B_VOLT;
    double POWER_METER_PHASE_C_VOLT;
    double POWER_METER_PHASE_A_ELEC;
    double POWER_METER_PHASE_B_ELEC;
    double POWER_METER_PHASE_C_ELEC;
    double POWER_METER_PHASE_A_ENERGE;
    double POWER_METER_PHASE_B_ENERGE;
    double POWER_METER_PHASE_C_ENERGE;
    double POWER_METER_POWER_FATOR_A;
    double POWER_METER_POWER_FATOR_B;
    double POWER_METER_POWER_FATOR_C;
    double POWER_METER_POWER_FATOR;
    sqlite3_int64 POWER_METER_TS;                // 时间戳
} METERTable;

typedef struct powerMeterStru
{
    float		POWER_METER_CPU_OC;
    float 		POWER_METER_ENERGE;					//正向有功总电能
    float 		POWER_METER_BACK_ENERGE;			//反向有功总电能
    float 		POWER_METER_FORTH_HAVE_POWER_TOTAL;	//瞬时总有功功率
    float 		POWER_METER_BACK_HAVE_POWER_TOTAL;	//瞬时总无功功率
    float 		POWER_METER_PHASE_TEMP_FORTH_HAVE_POWER;
    float 		POWER_METER_PHASE_TEMP_BACK_HAVE_POWER;
    float 		POWER_METER_PHASE_A_FORTH_HAVE_POWER;	//瞬时A相有功功率
    float 		POWER_METER_PHASE_A_BACK_HAVE_POWER;	//瞬时A相无功功率
    float 		POWER_METER_PHASE_B_FORTH_HAVE_POWER;
    float 		POWER_METER_PHASE_B_BACK_HAVE_POWER;
    float 		POWER_METER_PHASE_C_FORTH_HAVE_POWER;
    float 		POWER_METER_PHASE_C_BACK_HAVE_POWER;
    float 		POWER_METER_PHASE_TEMP_VOLT;			//相位未确认时填充（单相表有效）
    float 		POWER_METER_PHASE_A_VOLT;
    float 		POWER_METER_PHASE_B_VOLT;
    float 		POWER_METER_PHASE_C_VOLT;
    float 		POWER_METER_PHASE_TEMP_ELEC;
    float 		POWER_METER_PHASE_A_ELEC;
    float 		POWER_METER_PHASE_B_ELEC;
    float 		POWER_METER_PHASE_C_ELEC;
    float 		POWER_METER_PHASE_TEMP_ENERGE;
    float 		POWER_METER_PHASE_A_ENERGE;						//A相正向有功电能
    float 		POWER_METER_PHASE_B_ENERGE;
    float 		POWER_METER_PHASE_C_ENERGE;
    float 		POWER_METER_PHASE_TEMP_BACK_ENERGE;
    float 		POWER_METER_PHASE_A_BACK_ENERGE;				//A相反向有功电能
    float 		POWER_METER_PHASE_B_BACK_ENERGE;
    float 		POWER_METER_PHASE_C_BACK_ENERGE;
    float 		POWER_METER_POWER_FATOR_TEMP;
    float 		POWER_METER_POWER_FATOR_A;						//功率因数
    float 		POWER_METER_POWER_FATOR_B;
    float 		POWER_METER_POWER_FATOR_C;
    float 		POWER_METER_POWER_FATOR;
    float 		POWER_METER_FRE;								//频率
    float 		POWER_METER_NULLLINE_ELEC;	//wang add 零线电流
    float 		POWER_METER_A_ANGLE;		//wang add  A相相角
    float 		POWER_METER_B_ANGLE;		//wang add  B相相角
    float 		POWER_METER_C_ANGLE;		//wang add  C相相角
    long long 	POWER_METER_TS;
} POWER_METER_Table;

// --- --- ---
// --- SWITCH_TABLE
// --- --- ---
#define C_SWITCH_TABLE_PREFIX "SWITCH_TABLE"
#define C_SWITCH_TABLE_CREATE \
    "CREATE TABLE `%s                                                                        \n" \
    "(                                                                                       \n" \
    "	 `ID`         					   		  integer NOT NULL PRIMARY KEY AUTOINCREMENT,\n" \
    "    `SWITCH_CPU_OC`                          real    default 0,                         \n" \
    "    `SWITCH_TEMPERATURE`                     real    default 0,                         \n" \
    "    `SWITCH_VOLTAGE`                         real    default 0,                         \n" \
    "    `SWITCH_CURRENT`                         real    default 0,                         \n" \
    "    `SWITCH_ENERGE`                          real    default 0,                         \n" \
    "    `SWITCH_FORTH_HAVE_POWER_TOTAL`          real    default 0,                         \n" \
    "    `SWITCH_BACK_HAVE_POWER_TOTAL`           real    default 0,                         \n" \
    "    `SWITCH_PHASE_A_FORTH_HAVE_POWER`        real    default 0,                         \n" \
    "    `SWITCH_PHASE_A_BACK_HAVE_POWER`         real    default 0,                         \n" \
    "    `SWITCH_PHASE_B_FORTH_HAVE_POWER`        real    default 0,                         \n" \
    "    `SWITCH_PHASE_B_BACK_HAVE_POWER`         real    default 0,                         \n" \
    "    `SWITCH_PHASE_C_FORTH_HAVE_POWER`        real    default 0,                         \n" \
    "    `SWITCH_PHASE_C_BACK_HAVE_POWER`         real    default 0,                         \n" \
    "    `SWITCH_PHASE_A_VOLT`                    real    default 0,                         \n" \
    "    `SWITCH_PHASE_B_VOLT`                    real    default 0,                         \n" \
    "    `SWITCH_PHASE_C_VOLT`                    real    default 0,                         \n" \
    "    `SWITCH_PHASE_A_ELEC`                    real    default 0,                         \n" \
    "    `SWITCH_PHASE_B_ELEC`                    real    default 0,                         \n" \
    "    `SWITCH_PHASE_C_ELEC`                    real    default 0,                         \n" \
    "    `SWITCH_ACTIVE_POWER`                    real    default 0,                         \n" \
    "    `SWITCH_ON_OFF`                          integer default -1,                        \n" \
    "    `SWITCH_RESIDUAL_CURRENT`                real    default 0,                         \n" \
    "    `SWITCH_ZERO_SEQUENCE_CURRENT`           real    default 0,                         \n" \
    "    `SWITCH_POWER_FATOR`                     real    default 0,                         \n" \
    "    `SWITCH_INPUT_A_TEMP`                    real    default 0,                         \n" \
    "    `SWITCH_INPUT_B_TEMP`                    real    default 0,                         \n" \
    "    `SWITCH_INPUT_C_TEMP`                    real    default 0,                         \n" \
    "    `SWITCH_OUTPUT_A_TEMP`                   real    default 0,                         \n" \
    "    `SWITCH_OUTPUT_B_TEMP`                   real    default 0,                         \n" \
    "    `SWITCH_OUTPUT_C_TEMP`                   real    default 0,                         \n" \
    "    `SWITCH_TEMP`                            real    default 0,                         \n" \
    "    `RECORD_TS`                              integer default -1                         \n" \
    ");                                                                                      "
//
#define C_SWITCH_TABLE_INSERT \
		"INSERT INTO `%s`(`SWITCH_CPU_OC`, `SWITCH_TEMPERATURE`, `SWITCH_VOLTAGE`, `SWITCH_CURRENT`," \
		" `SWITCH_ENERGE`, `SWITCH_FORTH_HAVE_POWER_TOTAL`, `SWITCH_BACK_HAVE_POWER_TOTAL`," \
		" `SWITCH_PHASE_A_FORTH_HAVE_POWER`, `SWITCH_PHASE_A_BACK_HAVE_POWER`, `SWITCH_PHASE_B_FORTH_HAVE_POWER`," \
		" `SWITCH_PHASE_B_BACK_HAVE_POWER`, `SWITCH_PHASE_C_FORTH_HAVE_POWER`, `SWITCH_PHASE_C_BACK_HAVE_POWER`, `SWITCH_PHASE_A_VOLT`," \
		" `SWITCH_PHASE_B_VOLT`, `SWITCH_PHASE_C_VOLT`, `SWITCH_PHASE_A_ELEC`, `SWITCH_PHASE_B_ELEC`, `SWITCH_PHASE_C_ELEC`," \
		" `SWITCH_ACTIVE_POWER`, `SWITCH_ON_OFF`," \
		" `SWITCH_RESIDUAL_CURRENT`, `SWITCH_POWER_FATOR`," \
		" `SWITCH_INPUT_A_TEMP`, `SWITCH_INPUT_B_TEMP`, `SWITCH_INPUT_C_TEMP`," \
		" `SWITCH_OUTPUT_A_TEMP`, `SWITCH_OUTPUT_B_TEMP`, `SWITCH_OUTPUT_C_TEMP`, `SWITCH_TEMP`, `RECORD_TS`)" \
		" VALUES" \
		" (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?);"
//
#define C_SWITCH_TABLE_SELECT \
		"SELECT `ID`, `SWITCH_CPU_OC`, `SWITCH_TEMPERATURE`, `SWITCH_VOLTAGE`, `SWITCH_CURRENT`," \
			" `SWITCH_ENERGE`, `SWITCH_FORTH_HAVE_POWER_TOTAL`, `SWITCH_BACK_HAVE_POWER_TOTAL`," \
			" `SWITCH_PHASE_A_FORTH_HAVE_POWER`, `SWITCH_PHASE_A_BACK_HAVE_POWER`, `SWITCH_PHASE_B_FORTH_HAVE_POWER`," \
			" `SWITCH_PHASE_B_BACK_HAVE_POWER`, `SWITCH_PHASE_C_FORTH_HAVE_POWER`, `SWITCH_PHASE_C_BACK_HAVE_POWER`, `SWITCH_PHASE_A_VOLT`," \
			" `SWITCH_PHASE_B_VOLT`, `SWITCH_PHASE_C_VOLT`, `SWITCH_PHASE_A_ELEC`, `SWITCH_PHASE_B_ELEC`, `SWITCH_PHASE_C_ELEC`," \
			" `SWITCH_ACTIVE_POWER`, `SWITCH_ON_OFF`, `SWITCH_RESIDUAL_CURRENT`, `SWITCH_ZERO_SEQUENCE_CURRENT`, `SWITCH_POWER_FATOR`," \
			" `SWITCH_INPUT_A_TEMP`, `SWITCH_INPUT_B_TEMP`, `SWITCH_INPUT_C_TEMP`, `SWITCH_OUTPUT_A_TEMP`, `SWITCH_OUTPUT_B_TEMP`, "   \
			" `SWITCH_OUTPUT_C_TEMP`, `SWITCH_TEMP`, `RECORD_TS`" \
		" FROM `%s`" \
		" WHERE `ID` = (SELECT MAX(`ID`) FROM `%s`)"
//
//
#define DEVICE_TABLE_ATTR_ID "ID"
#define DEVICE_TABLE_ATTR_SWITCH_CPU_OC "SWITCH_CPU_OC"
#define DEVICE_TABLE_ATTR_SWITCH_TEMPERATURE "SWITCH_TEMPERATURE"
#define DEVICE_TABLE_ATTR_SWITCH_VOLTAGE "SWITCH_VOLTAGE"
#define DEVICE_TABLE_ATTR_SWITCH_CURRENT "SWITCH_CURRENT"
#define DEVICE_TABLE_ATTR_SWITCH_ENERGE "SWITCH_ENERGE"
#define DEVICE_TABLE_ATTR_SWITCH_FORTH_HAVE_POWER_TOTAL "SWITCH_FORTH_HAVE_POWER_TOTAL"
#define DEVICE_TABLE_ATTR_SWITCH_BACK_HAVE_POWER_TOTAL "SWITCH_BACK_HAVE_POWER_TOTAL"
#define DEVICE_TABLE_ATTR_SWITCH_PHASE_A_FORTH_HAVE_POWER "SWITCH_PHASE_A_FORTH_HAVE_POWER"
#define DEVICE_TABLE_ATTR_SWITCH_PHASE_A_BACK_HAVE_POWER "SWITCH_PHASE_A_BACK_HAVE_POWER"
#define DEVICE_TABLE_ATTR_SWITCH_PHASE_B_FORTH_HAVE_POWER "SWITCH_PHASE_B_FORTH_HAVE_POWER"
#define DEVICE_TABLE_ATTR_SWITCH_PHASE_B_BACK_HAVE_POWER "SWITCH_PHASE_B_BACK_HAVE_POWER"
#define DEVICE_TABLE_ATTR_SWITCH_PHASE_C_FORTH_HAVE_POWER "SWITCH_PHASE_C_FORTH_HAVE_POWER"
#define DEVICE_TABLE_ATTR_SWITCH_PHASE_C_BACK_HAVE_POWER "SWITCH_PHASE_C_BACK_HAVE_POWER"
#define DEVICE_TABLE_ATTR_SWITCH_PHASE_A_VOLT "SWITCH_PHASE_A_VOLT"
#define DEVICE_TABLE_ATTR_SWITCH_PHASE_B_VOLT "SWITCH_PHASE_B_VOLT"
#define DEVICE_TABLE_ATTR_SWITCH_PHASE_C_VOLT "SWITCH_PHASE_C_VOLT"
#define DEVICE_TABLE_ATTR_SWITCH_PHASE_A_ELEC "SWITCH_PHASE_A_ELEC"
#define DEVICE_TABLE_ATTR_SWITCH_PHASE_B_ELEC "SWITCH_PHASE_B_ELEC"
#define DEVICE_TABLE_ATTR_SWITCH_PHASE_C_ELEC "SWITCH_PHASE_C_ELEC"
#define DEVICE_TABLE_ATTR_SWITCH_ACTIVE_POWER "SWITCH_ACTIVE_POWER"
#define DEVICE_TABLE_ATTR_SWITCH_ON_OFF "SWITCH_ON_OFF"
#define DEVICE_TABLE_ATTR_SWITCH_RESIDUAL_CURRENT "SWITCH_RESIDUAL_CURRENT"
#define DEVICE_TABLE_ATTR_SWITCH_ZERO_SEQUENCE_CURRENT "SWITCH_ZERO_SEQUENCE_CURRENT"
#define DEVICE_TABLE_ATTR_SWITCH_POWER_FATOR "SWITCH_POWER_FATOR"
#define DEVICE_TABLE_ATTR_SWITCH_INPUT_A_TEMP "SWITCH_INPUT_A_TEMP"
#define DEVICE_TABLE_ATTR_SWITCH_INPUT_B_TEMP "SWITCH_INPUT_B_TEMP"
#define DEVICE_TABLE_ATTR_SWITCH_INPUT_C_TEMP "SWITCH_INPUT_C_TEMP"
#define DEVICE_TABLE_ATTR_SWITCH_OUTPUT_A_TEMP "SWITCH_OUTPUT_A_TEMP"
#define DEVICE_TABLE_ATTR_SWITCH_OUTPUT_B_TEMP "SWITCH_OUTPUT_B_TEMP"
#define DEVICE_TABLE_ATTR_SWITCH_OUTPUT_C_TEMP "SWITCH_OUTPUT_C_TEMP"
#define DEVICE_TABLE_ATTR_SWITCH_TEMP "SWITCH_TEMP"
#define DEVICE_TABLE_ATTR_SWITCH_TS "SWITCH_TS"
//
typedef struct SWITCHTable
{
    int ID;
    double SWITCH_CPU_OC;
    double SWITCH_TEMPERATURE;
    double SWITCH_VOLTAGE;
    double SWITCH_CURRENT;
    double SWITCH_ENERGE;
    double SWITCH_FORTH_HAVE_POWER_TOTAL;
    double SWITCH_BACK_HAVE_POWER_TOTAL;
    double SWITCH_PHASE_A_FORTH_HAVE_POWER;
    double SWITCH_PHASE_A_BACK_HAVE_POWER;
    double SWITCH_PHASE_B_FORTH_HAVE_POWER;
    double SWITCH_PHASE_B_BACK_HAVE_POWER;
    double SWITCH_PHASE_C_FORTH_HAVE_POWER;
    double SWITCH_PHASE_C_BACK_HAVE_POWER;
    double SWITCH_PHASE_A_VOLT;
    double SWITCH_PHASE_B_VOLT;
    double SWITCH_PHASE_C_VOLT;
    double SWITCH_PHASE_A_ELEC;
    double SWITCH_PHASE_B_ELEC;
    double SWITCH_PHASE_C_ELEC;
    double SWITCH_ACTIVE_POWER;
    int SWITCH_ON_OFF;
    double SWITCH_RESIDUAL_CURRENT;
    double SWITCH_ZERO_SEQUENCE_CURRENT;
    double SWITCH_POWER_FATOR;
    int SWITCH_INPUT_A_TEMP;
    int SWITCH_INPUT_B_TEMP;
    int SWITCH_INPUT_C_TEMP;
    int SWITCH_OUTPUT_A_TEMP;
    int SWITCH_OUTPUT_B_TEMP;
    int SWITCH_OUTPUT_C_TEMP;
    int SWITCH_TEMP;
    sqlite3_int64 SWITCH_TS;
} SWITCHTable;


// --- --- ---
// --- YX_TABLE
// --- --- ---
#define C_YX_TABLE_PREFIX "YX_TABLE"
#define C_YX_TABLE_CREATE \
    "CREATE TABLE `%s`                                                                 \n" \
    "(                                                                                 \n" \
    "	 `ID`         					integer NOT NULL PRIMARY KEY AUTOINCREMENT,    \n" \
    "    `PHASE_A_YX`           		integer    default 0,                          \n" \
    "    `PHASE_B_YX`           		integer    default 0,                          \n" \
    "    `PHASE_C_YX`           		integer    default 0,                          \n" \
    "    `PHASE_INFO`           		integer    default 0,                          \n" \
	"    `PHASE_A_VOLT`                 real       default 0,                          \n" \
    "    `PHASE_B_VOLT`                 real       default 0,                          \n" \
    "    `PHASE_C_VOLT`                 real       default 0,                          \n" \
    "    `PHASE_A_ELEC`                 real       default 0,                          \n" \
    "    `PHASE_B_ELEC`                 real       default 0,                          \n" \
    "    `PHASE_C_ELEC`                 real       default 0,                          \n" \
    "    `RECORD_TS`            		integer    default -1                          \n" \
    ");                                                                                  "
//
#define C_YX_TABLE_INSERT \
		"INSERT INTO `%s`(`PHASE_A_YX`, `PHASE_B_YX`," \
		" `PHASE_C_YX`,`RECORD_TS`)" \
		" VALUES" \
		" (?, ?, ?, ?);"
//
#define C_YX_TABLE_SELECT \
    "SELECT `ID`, `PHASE_A_YX`, `PHASE_B_YX`, `PHASE_C_YX`,"\
    "`PHASE_INFO`, `PHASE_A_VOLT`, `PHASE_B_VOLT`, `PHASE_C_VOLT`,"\
    "`PHASE_A_ELEC`, `PHASE_B_ELEC`, `PHASE_C_ELEC`, `RECORD_TS`" \
    " FROM `%s`" \
    " WHERE `ID` = (SELECT MAX(`ID`) FROM `%s`)"
//
#define DEVICE_TABLE_ATTR_ID "ID"
#define DEVICE_TABLE_ATTR_PHASE_A_YX "PHASE_A_YX"
#define DEVICE_TABLE_ATTR_PHASE_B_YX "PHASE_B_YX"
#define DEVICE_TABLE_ATTR_PHASE_C_YX "PHASE_C_YX"
#define DEVICE_TABLE_ATTR_PHASE_INFO "PHASE_INFO"
#define DEVICE_TABLE_ATTR_PHASE_A_VOLT "PHASE_A_VOLT"
#define DEVICE_TABLE_ATTR_PHASE_B_VOLT "PHASE_B_VOLT"
#define DEVICE_TABLE_ATTR_PHASE_C_VOLT "PHASE_C_VOLT"
#define DEVICE_TABLE_ATTR_PHASE_A_ELEC "PHASE_A_ELEC"
#define DEVICE_TABLE_ATTR_PHASE_B_ELEC "PHASE_B_ELEC"
#define DEVICE_TABLE_ATTR_PHASE_C_ELEC "PHASE_C_ELEC"
#define DEVICE_TABLE_ATTR_RECORD_TS "RECORD_TS"
//
typedef struct YXTABLE
{
    int ID;
    int PHASE_A_YX;
    int PHASE_B_YX;
    int PHASE_C_YX;
    int PHASE_INFO;
    double PHASE_A_VOLT;
    double PHASE_B_VOLT;
    double PHASE_C_VOLT;
    double PHASE_A_ELEC;
    double PHASE_B_ELEC;
    double PHASE_C_ELEC;
    sqlite3_int64 RECORD_TS;
} YXTABLE;

/**
 *  AREA_DAILY_TABLE
**/
#define C_AREA_DAILY_TABLE_PREFIX "AREA_DAILY_TABLE"
#define C_AREA_DAILY_TABLE_CREATE 															 		\
 "CREATE TABLE `%s`                                                                      			\n" \
 "(                                                                                      			\n" \
 " `ID`              							integer NOT NULL PRIMARY KEY AUTOINCREMENT,			\n" \
 " `DAILY_LINE_LOSS_IN_AREA`      				real    default 0,       							\n" \
 " `THREE_PHASE_UNBALANCE_OF_DAILY_CURRENT`  	real    default 0,       					 		\n" \
 " `OVER_LIMIT_RATE_OF_DAILY_VOLTAGE`    		real    default 0,       						 	\n" \
 " `LOWER_LIMIT_RATE_OF_DAILY_VOLTAGE`    		real    default 0,       						 	\n" \
 " `QUALIFIED_RATE_OF_DAILY_VOLTAGE`    		real    default 0,       						 	\n" \
 " `DAILY_LOAD_RATE`                            real    default 0,                                  \n" \
 " `RECORD_TS`              					integer default -1                         			\n" \
 ");                                                                                                  "

#define C_AREA_DAILY_TABLE_INSERT 																	\
   "INSERT INTO `%s`(`DAILY_LINE_LOSS_IN_AREA`,`THREE_PHASE_UNBALANCE_OF_DAILY_CURRENT`,			\
   `OVER_LIMIT_RATE_OF_DAILY_VOLTAGE`,`LOWER_LIMIT_RATE_OF_DAILY_VOLTAGE`,							\
   `QUALIFIED_RATE_OF_DAILY_VOLTAGE`, `DAILY_LOAD_RATE`, `RECORD_TS`)" 													\
   " VALUES" 																						\
   " (?, ?, ?, ?, ?, ?, ?);"

#define C_AREA_DAILY_TABLE_SELECT \
    "SELECT `ID`, `DAILY_LINE_LOSS_IN_AREA`, `THREE_PHASE_UNBALANCE_OF_DAILY_CURRENT`, `OVER_LIMIT_RATE_OF_DAILY_VOLTAGE`,"      \
    "`LOWER_LIMIT_RATE_OF_DAILY_VOLTAGE`, `QUALIFIED_RATE_OF_DAILY_VOLTAGE`, `DAILY_LOAD_RATE`, `RECORD_TS`"                                        \
    "FROM `%s` WHERE `ID` = (SELECT MAX(`ID`) FROM `%s`)"

#define DEVICE_TABLE_ATTR_ID                                                              "ID"
#define DEVICE_TABLE_ATTR_DAILY_LINE_LOSS_IN_AREA                                         "DAILY_LINE_LOSS_IN_AREA"
#define DEVICE_TABLE_ATTR_THREE_PHASE_UNBALANCE_OF_DAILY_CURRENT                          "THREE_PHASE_UNBALANCE_OF_DAILY_CURRENT"
#define DEVICE_TABLE_ATTR_OVER_LIMIT_RATE_OF_DAILY_VOLTAGE                                "OVER_LIMIT_RATE_OF_DAILY_VOLTAGE"
#define DEVICE_TABLE_ATTR_LOWER_LIMIT_RATE_OF_DAILY_VOLTAGE                               "LOWER_LIMIT_RATE_OF_DAILY_VOLTAGE"
#define DEVICE_TABLE_ATTR_QUALIFIED_RATE_OF_DAILY_VOLTAGE                                 "QUALIFIED_RATE_OF_DAILY_VOLTAGE"
#define DEVICE_TABLE_ATTR_DAILY_LOAD_RATE                                                 "DAILY_LOAD_RATE"
#define DEVICE_TABLE_ATTR_RECORD_TS                                                       "RECORD_TS"

typedef struct area_daily_table
{
    int ID;
    double DAILY_LINE_LOSS_IN_AREA;
    double THREE_PHASE_UNBALANCE_OF_DAILY_CURRENT;
    double OVER_LIMIT_RATE_OF_DAILY_VOLTAGE;
    double LOWER_LIMIT_RATE_OF_DAILY_VOLTAGE;
    double QUALIFIED_RATE_OF_DAILY_VOLTAGE;
    double DAILY_LOAD_RATE;
    sqlite3_int64 RECORD_TS;
}AREADAILYTable;


/**
 *  AREA_MONTHLY_TABLE
**/
#define C_AREA_MONTHLY_TABLE_PREFIX "AREA_MONTHLY_TABLE"
#define C_AREA_MONTHLY_TABLE_CREATE 																\
	"CREATE TABLE `%s`                                                                      		\n" \
    "(                                                                                      		\n" \
    " `ID`              							integer NOT NULL PRIMARY KEY AUTOINCREMENT,		\n" \
	" `MONTHLY_LINE_LOSS`        					double  default 0,       						\n" \
	" `THREE_PHASE_UNBALANCE_OF_MONTHLY_CURRENT`  	double  default 0,       						\n" \
	" `OVER_LIMIT_RATE_OF_MONTHLY_VOLTAGE`   		double  default 0,       						\n" \
	" `LOWER_LIMIT_RATE_OF_MONTHLY_VOLTAGE`   		double  default 0,       						\n" \
	" `MONTHLY_VOLTAGE_QUALIFICATION_RATE`   		double  default 0,       						\n" \
    " `MONTHLY_LOAD_RATE`                           double  default 0,                              \n" \
	" `RECORD_TS`              						integer default -1                         		\n" \
    ");                                                                                               "

#define C_AREA_MONTHLY_TABLE_INSERT 																\
   "INSERT INTO `%s`(`MONTHLY_LINE_LOSS`,`THREE_PHASE_UNBALANCE_OF_MONTHLY_CURRENT`,			    \
   `OVER_LIMIT_RATE_OF_MONTHLY_VOLTAGE`,`LOWER_LIMIT_RATE_OF_MONTHLY_VOLTAGE`,						\
   `MONTHLY_VOLTAGE_QUALIFICATION_RATE`, `MONTHLY_LOAD_RATE`, `RECORD_TS`)" 												\
   " VALUES" 																						\
   " (?, ?, ?, ?, ?, ?, ?);"

#define C_AREA_MONTHLY_TABLE_SELECT \
    "SELECT `ID`, `MONTHLY_LINE_LOSS`, `THREE_PHASE_UNBALANCE_OF_MONTHLY_CURRENT`, `OVER_LIMIT_RATE_OF_MONTHLY_VOLTAGE`,"             \
    "`LOWER_LIMIT_RATE_OF_MONTHLY_VOLTAGE`, `MONTHLY_VOLTAGE_QUALIFICATION_RATE`, `MONTHLY_LOAD_RATE`, `RECORD_TS`"                                        \
    "FROM `%s` WHERE `ID` = (SELECT MAX(`ID`) FROM `%s`)"

#define DEVICE_TABLE_ATTR_ID                                            "ID"
#define DEVICE_TABLE_ATTR_MONTHLY_LINE_LOSS                             "MONTHLY_LINE_LOSS"
#define DEVICE_TABLE_ATTR_THREE_PHASE_UNBALANCE_OF_MONTHLY_CURRENT      "THREE_PHASE_UNBALANCE_OF_MONTHLY_CURRENT"
#define DEVICE_TABLE_ATTR_OVER_LIMIT_RATE_OF_MONTHLY_VOLTAGE            "OVER_LIMIT_RATE_OF_MONTHLY_VOLTAGE"
#define DEVICE_TABLE_ATTR_LOWER_LIMIT_RATE_OF_MONTHLY_VOLTAGE           "LOWER_LIMIT_RATE_OF_MONTHLY_VOLTAGE"
#define DEVICE_TABLE_ATTR_MONTHLY_VOLTAGE_QUALIFICATION_RATE            "MONTHLY_VOLTAGE_QUALIFICATION_RATE"
#define DEVICE_TABLE_ATTR_MONTHLY_LOAD_RATE                             "MONTHLY_LOAD_RATE"
#define DEVICE_TABLE_ATTR_RECORD_TS                                     "RECORD_TS"

typedef struct area_monthly_table
{
    int ID;
    double MONTHLY_LINE_LOSS;
    double THREE_PHASE_UNBALANCE_OF_MONTHLY_CURRENT;
    double OVER_LIMIT_RATE_OF_MONTHLY_VOLTAGE;
    double LOWER_LIMIT_RATE_OF_MONTHLY_VOLTAGE;
    double MONTHLY_VOLTAGE_QUALIFICATION_RATE;
    double MONTHLY_LOAD_RATE;
    sqlite3_int64 RECORD_TS;
}AREAMONTHLYTable;

/* *
 *  POWERLOSS_TABLE
* */
#define C_POWERLOSS_TABLE_DBNAME "gw_sys"
#define C_POWERLOSS_TABLE_PREFIX "POWERLOSS_TABLE"
#define C_POWERLOSS_TABLE_CREATE                                            \
    "CREATE TABLE `%s`                                                  \n" \
    "(                                                                  \n" \
    "   `ID`        integer NOT NULL PRIMARY KEY AUTOINCREMENT,         \n" \
    "   `AREABUGTYPE`   integer default -1,                             \n" \
    "   `AREABUGSTAA`   integer default 0,                              \n" \
    "   `AREABUGSTAB`   integer default 0,                              \n" \
    "   `AREABUGSTAC`   integer default 0,                              \n" \
    "   `BRANCHDEVICE`  text    default NULL,                           \n" \
    "   `RECORD_TS`     integer default -1                              \n" \
    ");                                                                   "

#define C_POWERLOSS_TABLE_INSERT                                            \
    "INSERT INTO `%s`( `AREABUGTYPE`, `AREABUGSTAA`, `AREABUGSTAB`,         \
    `AREABUGSTAC`, `BRANCHDEVICE`, `RECORD_TS`)"                            \
    "VALUES" 																\
    "(?, ?, ?, ?, ?, ?);"

#define C_POWERLOSS_TABLE_SELECT \
    "SELECT `ID`, `AREABUGTYPE`, `AREABUGSTAA`, `AREABUGSTAB`,"             \
    "`AREABUGSTAC`, `BRANCHDEVICE`, `RECORD_TS`"                            \
    "FROM `%s` WHERE `ID` = (SELECT MAX(`ID`) FROM `%s` WHERE `BRANCHDEVICE` = '%s')"

#define DEVICE_TABLE_ATTR_ID                                            "ID"
#define DEVICE_TABLE_ATTR_AREABUGTYPE                                   "AREABUGTYPE"
#define DEVICE_TABLE_ATTR_AREABUGSTAA                                   "AREABUGSTAA"
#define DEVICE_TABLE_ATTR_AREABUGSTAB                                   "AREABUGSTAB"
#define DEVICE_TABLE_ATTR_AREABUGSTAC                                   "AREABUGSTAC"
#define DEVICE_TABLE_ATTR_BRANCHDEVICE                                  "BRANCHDEVICE"
#define DEVICE_TABLE_ATTR_RECORD_TS                                     "RECORD_TS"

typedef struct powerloss_table
{
    int ID;
    int AREABUGTYPE;
    int AREABUGSTAA;
    int AREABUGSTAB;
    int AREABUGSTAC;
    std::string BRANCHDEVICE;
    sqlite3_int64 RECORD_TS;
}POWERLOSSTable;

typedef struct areaBug_yx
{
    int areaBugType;
    int areaBugStaA;
    int areaBugStaB;
    int areaBugStaC;
    char branchDevice[16];
    long recordTs;
}AREABUG_YX;

typedef struct phaseStru
{
    char deviceId[16];
    int   PHASE_INFO;
} PHASE_NOTIFY;

/* *
 *      AREA_ANNUAL_TABLE
* */

#define C_AREA_ANNUAL_TABLE_PREFIX "AREA_ANNUAL_TABLE"
#define C_AREA_ANNUAL_TABLE_CREATE                                                                              \
    "CREATE TABLE `%s`                                                                                      \n" \
    "(                                                                                                      \n" \
    "   `ID`                                            integer NOT NULL PRIMARY KEY AUTOINCREMENT,         \n" \
    "   `THREE_PHASE_UNBALANCE_OF_ANNUAL_CURRENT`       real    default 0,                                  \n" \
    "   `ANNUAL_VOLTAGE_OVER_LIMIT_RATE`                real    default 0,                                  \n" \
    "   `ANNUAL_VOLTAGE_LOWER_LIMIT_RATE`               real    default 0,                                  \n" \
    "   `ANNUAL_VOLTAGE_QUALIFICATION_RATE`             real    default 0,                                  \n" \
    "   `ANNUAL_LOAD_RATE`                              real    default 0,                                  \n" \
    "   `RECORD_TS`                                     integer default -1                                  \n" \
    ");                                                                                                       "

#define C_AREA_ANNUAL_TABLE_INSERT                                            \
    "INSERT INTO `%s`( `THREE_PHASE_UNBALANCE_OF_ANNUAL_CURRENT`, `ANNUAL_VOLTAGE_OVER_LIMIT_RATE`, `ANNUAL_VOLTAGE_LOWER_LIMIT_RATE`,         \
    `ANNUAL_VOLTAGE_QUALIFICATION_RATE`, `ANNUAL_LOAD_RATE`, `RECORD_TS`)"                            \
    "VALUES" 																\
    "(?, ?, ?, ?, ?, ?);"

#define C_AREA_ANNUAL_TABLE_SELECT \
    "SELECT `ID`, `THREE_PHASE_UNBALANCE_OF_ANNUAL_CURRENT`, `ANNUAL_VOLTAGE_OVER_LIMIT_RATE`, `ANNUAL_VOLTAGE_LOWER_LIMIT_RATE`,"             \
    "`ANNUAL_VOLTAGE_QUALIFICATION_RATE`, `ANNUAL_LOAD_RATE`, `RECORD_TS`"                            \
    "FROM `%s` WHERE `ID` = (SELECT MAX(`ID`) FROM `%s`)"

#define DEVICE_TABLE_ATTR_ID                                            "ID"
#define DEVICE_TABLE_ATTR_THREE_PHASE_UNBALANCE_OF_ANNUAL_CURRENT       "THREE_PHASE_UNBALANCE_OF_ANNUAL_CURRENT"
#define DEVICE_TABLE_ATTR_ANNUAL_VOLTAGE_OVER_LIMIT_RATE                "ANNUAL_VOLTAGE_OVER_LIMIT_RATE"
#define DEVICE_TABLE_ATTR_ANNUAL_VOLTAGE_LOWER_LIMIT_RATE               "ANNUAL_VOLTAGE_LOWER_LIMIT_RATE"
#define DEVICE_TABLE_ATTR_ANNUAL_VOLTAGE_QUALIFICATION_RATE             "ANNUAL_VOLTAGE_QUALIFICATION_RATE"
#define DEVICE_TABLE_ATTR_ANNUAL_LOAD_RATE                              "ANNUAL_LOAD_RATE"
#define DEVICE_TABLE_ATTR_RECORD_TS                                     "RECORD_TS"

typedef struct area_meter_year_table
{
    int ID;
    double THREE_PHASE_UNBALANCE_OF_ANNUAL_CURRENT;
    double ANNUAL_VOLTAGE_OVER_LIMIT_RATE;
    double ANNUAL_VOLTAGE_LOWER_LIMIT_RATE;
    double ANNUAL_VOLTAGE_QUALIFICATION_RATE;
    double ANNUAL_LOAD_RATE;
    sqlite3_int64 RECORD_TS;
}AREAMETERYEARSTable;

/* *
 *      CHARGE_TABLE
* */

#define C_CHARGE_TABLE_PREFIX "CHARGE_KELU_TABLE"
#define C_CHARGE_TABLE_CREATE                                                                              \
    "CREATE TABLE `%s`                                                                                      \n" \
    "(                                                                                                      \n" \
    "   `ID`                                            integer NOT NULL PRIMARY KEY AUTOINCREMENT,         \n" \
    "   `CHARGEID`                                      text    default NULL,                               \n" \
    "   `CHARGEVOLTAGE`                                 real    default 0,                                  \n" \
    "   `CHARGECURRENT`                                 real    default 0,                                  \n" \
    "   `CHARGEPOWER`                                   real    default 0,                                  \n" \
    "   `CHARGEENERGETOTAL`                             real    default 0,                                  \n" \
    "   `CHARGEENERGETIP`                               real    default 0,                                  \n" \
    "   `CHARGEENERGEPEAK`                              real    default 0,                                  \n" \
    "   `CHARGEENERGEFLAT`                              real    default 0,                                  \n" \
    "   `CHARGEENERGEBOTTOM`                            real    default 0,                                  \n" \
    "   `CHARGETEMPTURE`                                real    default 0,                                  \n" \
    "   `RECORD_TS`                                     integer default -1                                  \n" \
    ");                                                                                                       "

#define C_CHARGE_TABLE_INSERT                                            \
    "INSERT INTO `%s`( `CHARGEID`, `CHARGEVOLTAGE`, `CHARGECURRENT`, `CHARGEPOWER`, `CHARGEENERGETOTAL`,       \
    `CHARGEENERGETIP`, `CHARGEENERGEPEAK`, `CHARGEENERGEFLAT`, `CHARGEENERGEBOTTOM`, `CHARGETEMPTURE`, `RECORD_TS`)"                            \
    "VALUES" 																\
    "(?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?);"

#define C_CHARGE_TABLE_SELECT \
    "SELECT `ID`, `CHARGEID`, `CHARGEVOLTAGE`, `CHARGECURRENT`, `CHARGEPOWER`, `CHARGEENERGETOTAL`,"             \
    "`CHARGEENERGETIP`, `CHARGEENERGEPEAK`, `CHARGEENERGEFLAT`, `CHARGEENERGEBOTTOM`, `CHARGETEMPTURE`, `RECORD_TS`"                            \
    "FROM `%s` WHERE `ID` = (SELECT MAX(`ID`) FROM `%s`)"

#define DEVICE_TABLE_ATTR_ID                                            "ID"
#define DEVICE_TABLE_ATTR_CHARGEID                                      "CHARGEID"
#define DEVICE_TABLE_ATTR_CHARGEVOLTAGE                                 "CHARGEVOLTAGE"
#define DEVICE_TABLE_ATTR_CHARGECURRENT                                 "CHARGECURRENT"
#define DEVICE_TABLE_ATTR_CHARGEPOWER                                   "CHARGEPOWER"
#define DEVICE_TABLE_ATTR_CHARGEENERGETOTAL                             "CHARGEENERGETOTAL"
#define DEVICE_TABLE_ATTR_CHARGEENERGETIP                               "CHARGEENERGETIP"
#define DEVICE_TABLE_ATTR_CHARGEENERGEPEAK                              "CHARGEENERGEPEAK"
#define DEVICE_TABLE_ATTR_CHARGEENERGEFLAT                              "CHARGEENERGEFLAT"
#define DEVICE_TABLE_ATTR_CHARGEENERGEBOTTOM                            "CHARGEENERGEBOTTOM"
#define DEVICE_TABLE_ATTR_CHARGETEMPTURE                                "CHARGETEMPTURE"
#define DEVICE_TABLE_ATTR_RECORD_TS                                     "RECORD_TS"

typedef struct charge_table
{
    int ID;
    std::string CHARGEID;
    double CHARGEVOLTAGE;
    double CHARGECURRENT;
    double CHARGEPOWER;
    double CHARGEENERGETOTAL;
    double CHARGEENERGETIP;
    double CHARGEENERGEPEAK;
    double CHARGEENERGEFLAT;
    double CHARGEENERGEBOTTOM;
    double CHARGETEMPTURE;
    sqlite3_int64 RECORD_TS;
}CHARGETable;

/* *
 *      BRANCH_TABLE
* */
#define C_BRANCH_TABLE_PREFIX "BRANCH_TABLE"
#define C_BRANCH_TABLE_CREATE                                                                              \
    "CREATE TABLE `%s`                                                                                      \n" \
    "(                                                                                                      \n" \
    "   `ID`                                            integer NOT NULL PRIMARY KEY AUTOINCREMENT,         \n" \
    "   `BRANCHDEVICE`                                  text    default NULL,                               \n" \
    "   `LINELOSS`                                      real    default 0,                                  \n" \
    "   `LINELOSSA`                                     real    default 0,                                  \n" \
    "   `LINELOSSB`                                     real    default 0,                                  \n" \
    "   `LINELOSSC`                                     real    default 0,                                  \n" \
    "   `FROZTP`                                        real    default 0,                                  \n" \
    "   `RECORD_TS`                                     integer default -1                                  \n" \
    ");                                                                                                       "

#define C_BRANCH_TABLE_INSERT                                            \
    "INSERT INTO `%s`( `BRANCHDEVICE`, `LINELOSS`, `LINELOSSA`, `LINELOSSB`, `LINELOSSC`,       \
    `FROZTP`, `RECORD_TS`)"                            \
    "VALUES" 																\
    "(?, ?, ?, ?, ?, ?, ?);"

#define C_BRANCH_TABLE_SELECT \
    "SELECT `ID`, `BRANCHDEVICE`, `LINELOSS`, `LINELOSSA`, `LINELOSSB`, `LINELOSSC`,"             \
    "`FROZTP`, `RECORD_TS`"                            \
    "FROM `%s` WHERE `ID` = (SELECT MAX(`ID`) FROM `%s` WHERE `BRANCHDEVICE` = '%s')"


#define C_BRANCH_TABLE_MONTH_SELECT \
    "SELECT `ID`, `BRANCHDEVICE`, `LINELOSS`, `LINELOSSA`, `LINELOSSB`, `LINELOSSC`,"             \
    "`FROZTP`, `RECORD_TS`"                                                                       \
    "FROM `%s` WHERE `ID` = (SELECT MAX(`ID`) FROM `%s` WHERE `BRANCHDEVICE` = '%s' "             \
    "AND (`RECORD_TS` > %d AND `RECORD_TS` < %d))"

#define DEVICE_TABLE_ATTR_ID                                            "ID"
#define DEVICE_TABLE_ATTR_BRANCHDEVICE                                  "BRANCHDEVICE"
#define DEVICE_TABLE_ATTR_LINELOSS                                      "LINELOSS"
#define DEVICE_TABLE_ATTR_LINELOSS_MONTH                                "LINELOSS_MONTH"
#define DEVICE_TABLE_ATTR_LINELOSSA                                     "LINELOSSA"
#define DEVICE_TABLE_ATTR_LINELOSSB                                     "LINELOSSB"
#define DEVICE_TABLE_ATTR_LINELOSSC                                     "LINELOSSC"
#define DEVICE_TABLE_ATTR_FROZTP                                        "FROZTP"
#define DEVICE_TABLE_ATTR_RECORD_TS                                     "RECORD_TS"

typedef struct branch_table
{
    int ID;
    std::string BRANCHDEVICE;
    double LINELOSS;
    double LINELOSS_MONTH;
    double LINELOSSA;
    double LINELOSSB;
    double LINELOSSC;
    sqlite3_int64 FROZTP;
    sqlite3_int64 RECORD_TS;
}BRANCHTable;

/* *
 *  PHASE_BLANCE_TABLE
* */
#define C_PHASE_BLANCE_TABLE_PREFIX "PHASE_BLANCE_TABLE"
#define C_PHASE_BLANCE_TABLE_CREATE                                                                              \
    "CREATE TABLE `%s`                                                                                      \n" \
    "(                                                                                                      \n" \
    "   `ID`                                            integer NOT NULL PRIMARY KEY AUTOINCREMENT,         \n" \
    "   `DEVICE`                                        text    default NULL,                               \n" \
    "   `CURBALANCERATE`                                real    default 0,                                  \n" \
    "   `RECORD_TS`                                     integer default -1                                  \n" \
    ");                                                                                                       "

#define C_PHASE_BLANCE_TABLE_INSERT                                            \
    "INSERT INTO `%s`( `DEVICEID`, `CURBALANCERATE`,        \
    `RECORD_TS`)"                            \
    "VALUES" 																\
    "(?, ?, ?);"

#define C_PHASE_BLANCE_TABLE_SELECT \
    "SELECT `ID`, `DEVICE`, `CURBALANCERATE`,  "             \
    "`RECORD_TS`"                            \
    "FROM `%s` WHERE `ID` = (SELECT MAX(`ID`) FROM `%s` WHERE `DEVICE` = '%s')"

#define DEVICE_TABLE_ATTR_ID                                            "ID"
#define DEVICE_TABLE_ATTR_DEVICEID                                      "DEVICEID"
#define DEVICE_TABLE_ATTR_CURBALANCERATE                                "CURBALANCERATE"
#define DEVICE_TABLE_ATTR_RECORD_TS                                     "RECORD_TS"

typedef struct phase_blance_table
{
    int ID;
    std::string DEVICEID;
    double CURBALANCERATE;
    sqlite3_int64 RECORD_TS;
}PHASEBLANCETable;

/* *
 *      CHARGE_YX_TABLE
* */
#define C_CHARGE_YX_TABLE_PREFIX "CHARGE_YX_TABLE"
#define C_CHARGE_YX_TABLE_CREATE                                                                              \
    "CREATE TABLE `%s`                                                                                      \n" \
    "(                                                                                                      \n" \
    "   `ID`                                            integer NOT NULL PRIMARY KEY AUTOINCREMENT,         \n" \
    "   `CHARGEID`                                      text    default NULL,                               \n" \
    "   `CHARGESTATUS`                                  integer default -1,                                 \n" \
    "   `CHARGEHOOKSTATUS`                              integer default -1,                                 \n" \
    "   `RECORD_TS`                                     integer default -1                                  \n" \
    ");                                                                                                       "

#define C_CHARGE_YX_TABLE_INSERT                                            \
    "INSERT INTO `%s`( `CHARGEID`, `CHARGESTATUS`, `CHARGEHOOKSTATUS`,        \
    `RECORD_TS`)"                                                             \
    "VALUES" 																  \
    "(?, ?, ?, ?);"

#define C_CHARGE_YX_TABLE_SELECT \
    "SELECT `ID`, `CHARGEID`, `CHARGESTATUS`, `CHARGEHOOKSTATUS`,"             \
    "`RECORD_TS`"                                                              \
    "FROM `%s` WHERE `ID` = (SELECT MAX(`ID`) FROM `%s`)"

#define DEVICE_TABLE_ATTR_ID                                            "ID"
#define DEVICE_TABLE_ATTR_CHARGEID                                      "CHARGEID"
#define DEVICE_TABLE_ATTR_CHARGESTATUS                                  "CHARGESTATUS"
#define DEVICE_TABLE_ATTR_CHARGEHOOKSTATUS                              "CHARGEHOOKSTATUS"
#define DEVICE_TABLE_ATTR_RECORD_TS                                     "RECORD_TS"

typedef struct charge_yx_table
{
    int ID;
    std::string CHARGEID;
    int CHARGESTATUS;
    int CHARGEHOOKSTATUS;
    sqlite3_int64 RECORD_TS;
}CHARGEYXTable;

/*
 * TH_TABLE
 */
#define C_TH_TABLE_PREFIX "TH_TABLE"
#define C_TH_TABLE_CREATE                                                                                       \
    "CREATE TABLE `%s`                                                                                      \n" \
    "(                                                                                                      \n" \
    "   `ID`                                            integer NOT NULL PRIMARY KEY AUTOINCREMENT,         \n" \
    "   `TEMPERATURE`                                   real    default 0,                                  \n" \
    "   `HUMIDITY`                              real    default 0,                                  \n" \
    "   `RECORD_TS`                                     integer default -1                                  \n" \
    ");                                                                                                       "

#define C_TH_TABLE_INSERT                                            \
    "INSERT INTO `%s`( `TEMPERATURE`, `HUMIDITY`,                    \
    `RECORD_TS`)"                                                             \
    "VALUES" 																  \
    "(?, ?, ?);"

#define C_TH_TABLE_SELECT                                                      \
    "SELECT `ID`, `TEMPERATURE`, `HUMIDITY`,            "             \
    "`RECORD_TS`"                                                              \
    "FROM `%s` WHERE `ID` = (SELECT MAX(`ID`) FROM `%s`)"

#define DEVICE_TABLE_ATTR_ID                                            "ID"
#define DEVICE_TABLE_ATTR_TEMPERATURE                                   "TEMPERATURE"
#define DEVICE_TABLE_ATTR_HUMIDITY                                      "HUMIDITY"
#define DEVICE_TABLE_ATTR_RECORD_TS                                     "RECORD_TS"

typedef struct th_table
{
    int ID;
    double TEMPERATURE;
    double HUMIDITY;
    sqlite3_int64 RECORD_TS;
}THTable;

/* *
 *  SENSORSMOKE_TABLE
* */
#define C_SENSORSMOKE_TABLE_PREFIX "SENSORSMOKE_TABLE"
#define C_SENSORSMOKE_TABLE_CREATE                                                                                       \
    "CREATE TABLE `%s`                                                                                      \n" \
    "(                                                                                                      \n" \
    "   `ID`                                            integer NOT NULL PRIMARY KEY AUTOINCREMENT,         \n" \
    "   `SMOKESTATE`                                    integer default 0,                                  \n" \
    "   `RECORD_TS`                                     integer default -1                                  \n" \
    ");                                                                                                       "

#define C_SENSORSMOKE_TABLE_INSERT                                            \
    "INSERT INTO `%s`( `SMOKESTATE`,                                         \
    `RECORD_TS`)"                                                             \
    "VALUES" 																  \
    "(?, ?, ?);"

#define C_SENSORSMOKE_TABLE_SELECT                                             \
    "SELECT `ID`, `SMOKE_STATE`,                                  "             \
    "`RECORD_TS`"                                                              \
    "FROM `%s` WHERE `ID` = (SELECT MAX(`ID`) FROM `%s`)"

#define DEVICE_TABLE_ATTR_ID                                            "ID"
#define DEVICE_TABLE_ATTR_SMOKE_STATE                                   "SMOKE_STATE"
#define DEVICE_TABLE_ATTR_RECORD_TS                                     "RECORD_TS"

typedef struct sensorsmoke_table
{
    int ID;
    double SMOKESTATE;
    sqlite3_int64 RECORD_TS;
}SENSORSMOKETable;

/* *
 *  JIEXI_MQTT_METER_TABLE_test
* */
#define C_TABLE_NAME_JIEXI_MQTT_METER_TABLE_test "JIEXI_MQTT_METER_TABLE_test"

#define C_JIEXI_MQTT_METER_TABLE_test_INSERT \
    "INSERT INTO `JIEXI_MQTT_METER_TABLE_test`(`POWER_METER_FORTH_HAVE_POWER_TOTAL`, `POWER_METER_PHASE_A_FORTH_HAVE_POWER`, `POWER_METER_PHASE_B_FORTH_HAVE_POWER`, `POWER_METER_PHASE_C_FORTH_HAVE_POWER`," \
    "                                          `POWER_METER_PHASE_A_VOLT`, `POWER_METER_PHASE_B_VOLT`, `POWER_METER_PHASE_C_VOLT`," \
    "                                          `POWER_METER_PHASE_A_ELEC`, `POWER_METER_PHASE_B_ELEC`, `POWER_METER_PHASE_C_ELEC`," \
    "                                          `tgI0`, `tgSupWh`, `tgSupWhA`, `tgSupWhB`, `tgSupWhC`, `RECORD_TS`)" \
    "VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?);"

#define C_JIEXI_MQTT_METER_TABLE_test_SELECT \
    "SELECT `POWER_METER_FORTH_HAVE_POWER_TOTAL`, `POWER_METER_PHASE_A_FORTH_HAVE_POWER`, `POWER_METER_PHASE_B_FORTH_HAVE_POWER`, `POWER_METER_PHASE_C_FORTH_HAVE_POWER`," \
    "       `POWER_METER_PHASE_A_VOLT`, `POWER_METER_PHASE_B_VOLT`, `POWER_METER_PHASE_C_VOLT`," \
    "       `POWER_METER_PHASE_A_ELEC`, `POWER_METER_PHASE_B_ELEC`, `POWER_METER_PHASE_C_ELEC`," \
    "       `tgI0`, `tgSupWh`, `tgSupWhA`, `tgSupWhB`, `tgSupWhC`, `RECORD_TS`" \
    "FROM `%s` WHERE `ID` = (SELECT MAX(`ID`) FROM `%s`)"

typedef struct JIEXI_MQTT_METER_TABLE_test
{
    int ID;
    double POWER_METER_FORTH_HAVE_POWER_TOTAL;          // 总有功
    double POWER_METER_PHASE_A_FORTH_HAVE_POWER;        // A相有功
    double POWER_METER_PHASE_B_FORTH_HAVE_POWER;        // B相有功
    double POWER_METER_PHASE_C_FORTH_HAVE_POWER;        // C相有功
    double POWER_METER_PHASE_A_VOLT;
    double POWER_METER_PHASE_B_VOLT;
    double POWER_METER_PHASE_C_VOLT;
    double POWER_METER_PHASE_A_ELEC;
    double POWER_METER_PHASE_B_ELEC;
    double POWER_METER_PHASE_C_ELEC;
    double tgI0;
    double tgSupWh;
    double tgSupWhA;
    double tgSupWhB;
    double tgSupWhC;
    sqlite3_int64 POWER_METER_TS;                // 时间戳
} JIEXI_MQTT_METER_TABLE_test;


/**
 *  IR_TABLE
**/
#define C_IR_TABLE_PREFIX "IR_TABLE"
#define C_IR_TABLE_CREATE                                                                                       \
    "CREATE TABLE `%s`                                                                                      \n" \
    "(                                                                                                      \n" \
    "   `ID`                                            integer NOT NULL PRIMARY KEY AUTOINCREMENT,         \n" \
    "   `IRSTATE`                                       integer default 0,                                  \n" \
    "   `IRDELAYTIME`                                   integer default 0,                                  \n" \
    "   `RECORD_TS`                                     integer default -1                                  \n" \
    ");                                                                                                       "

#define C_IR_TABLE_INSERT                                            \
    "INSERT INTO `%s`( `IRSTATE`,  `IRDELAYTIME`,                                        \
    `RECORD_TS`)"                                                             \
    "VALUES" 																  \
    "(?, ?, ?);"

#define C_IR_TABLE_SELECT                                             \
    "SELECT `ID`, `IRSTATE`, `IRDELAYTIME`,                                  "             \
    "`RECORD_TS`"                                                              \
    "FROM `%s` WHERE `ID` = (SELECT MAX(`ID`) FROM `%s`)"

#define DEVICE_TABLE_ATTR_ID                                            "ID"
#define DEVICE_TABLE_ATTR_IRSTATE                                       "IRSTATE"
#define DEVICE_TABLE_ATTR_IRDELAYTIME                                   "IRDELAYTIME"
#define DEVICE_TABLE_ATTR_RECORD_TS                                     "RECORD_TS"

typedef struct ir_table
{
    int ID;
    int IRSTATE;
    int IRDELAYTIME;
    sqlite3_int64 RECORD_TS;
}IRTable;

/**
 *  WATER_DEEP_TABLE
**/
#define C_WATER_DEEP_TABLE "WATER_DEEP_TABLE"

#define C_WATER_DEEP_TABLE_SELECT                                             \
    "SELECT `ID`, `DEEP`,                                 "             \
    "`RECORD_TS`"                                                              \
    "FROM `%s` WHERE `ID` = (SELECT MAX(`ID`) FROM `%s`)"

#define DEVICE_TABLE_ATTR_ID                                            "ID"
#define DEVICE_TABLE_ATTR_DEEP                                          "DEEP"
#define DEVICE_TABLE_ATTR_RECORD_TS                                     "RECORD_TS"

typedef struct water_deep_table
{
    int ID;
    double DEEP;
    sqlite3_int64 RECORD_TS;
}WATERDEEPTable;

/**
 *  SENSORDOOR_TABLE
**/
#define C_SENSORDOOR_TABLE "SENSORDOOR_TABLE"

#define C_SENSORDOOR_TABLE_SELECT                                             \
    "SELECT `ID`, `DOORSTATE`,                                 "             \
    "`RECORD_TS`"                                                              \
    "FROM `%s` WHERE `ID` = (SELECT MAX(`ID`) FROM `%s`)"

#define DEVICE_TABLE_ATTR_ID                                            "ID"
#define DEVICE_TABLE_ATTR_DOORSTATE                                     "DOORSTATE"
#define DEVICE_TABLE_ATTR_RECORD_TS                                     "RECORD_TS"

typedef struct sensordoor_table
{
    int ID;
    int DOORSTATE;
    sqlite3_int64 RECORD_TS;
}SENSORDOORTable;


/**
 *  RELAYPLC_TABLE
**/
#define C_RELAYPLC_TABLE "RELAYPLC_TABLE"

#define C_RELAYPLC_TABLE_SELECT                                             \
    "SELECT `ID`, `RELAYPLCSTATE`,                                 "             \
    "`RECORD_TS`"                                                              \
    "FROM `%s` WHERE `ID` = (SELECT MAX(`ID`) FROM `%s`)"

#define DEVICE_TABLE_ATTR_ID                                            "ID"
#define DEVICE_TABLE_ATTR_RELAYPLCSTATE                                 "RELAYPLCSTATE"
#define DEVICE_TABLE_ATTR_RECORD_TS                                     "RECORD_TS"

/**
 *  BOPO_TABLE
**/
#define C_BOPO_TABLE "BOPO_TABLE"

#define C_BOPO_TABLE_SELECT                                             \
    "SELECT `ID`, `BOPOSTATE`,                                 "             \
    "`RECORD_TS`"                                                              \
    "FROM `%s` WHERE `ID` = (SELECT MAX(`ID`) FROM `%s`)"

#define DEVICE_TABLE_ATTR_ID                                            "ID"
#define DEVICE_TABLE_ATTR_BOPOSTATE                                     "BOPOSTATE"
#define DEVICE_TABLE_ATTR_RECORD_TS                                     "RECORD_TS"


/**
 *  PVINVERTER_TABLE
**/
#define C_PVINVERTER_TABLE "PVINVERTER_TABLE"
#define C_PVINVERTER_TABLE_SELECT \
 "SELECT `ID`, " \
 "  `PVINVERTER_SYSSTATE`, " \
 "  `PVINVERTER_GENERATIONTIME_TOTAL`, " \
 "  `PVINVERTER_SERVICETIME_TOTAL`, " \
 "  `PVINVERTER_FREQUENCY_GRID`, " \
 "  `PVINVERTER_ACTIVEPOWER_OUTPUT_TOTAL`, " \
 "  `PVINVERTER_REACTIVEPOWER_OUTPUT_TOTAL`, " \
 "  `PVINVERTER_APPARENTPOWER_OUTPUT_TOTAL`, " \
 "  `PVINVERTER_ACTIVEPOWER_PCC_TOTAL`, " \
 "  `PVINVERTER_REACTIVEPOWER_PCC_TOTAL`, " \
 "  `PVINVERTER_APPARENTPOWER_PCC_TOTAL`, " \
 "  `PVINVERTER_VOLTAGE_PHASE_R`, " \
 "  `PVINVERTER_CURRENT_OUTPUT_R`, " \
 "  `PVINVERTER_ACTIVEPOWER_OUTPUT_R`, " \
 "  `PVINVERTER_REACTIVEPOWER_OUTPUT_R`, " \
 "  `PVINVERTER_POWERFACTOR_OUTPUT_R`, " \
 "  `PVINVERTER_CURRENT_PCC_R`, " \
 "  `PVINVERTER_ACTIVEPOWER_PCC_R`, " \
 "  `PVINVERTER_REACTIVEPOWER_PCC_R`, " \
 "  `PVINVERTER_POWERFACTOR_PCC_R`, " \
 "  `PVINVERTER_VOLTAGE_PHASE_S`, " \
 "  `PVINVERTER_CURRENT_OUTPUT_S`, " \
 "  `PVINVERTER_ACTIVEPOWER_OUTPUT_S`, " \
 "  `PVINVERTER_REACTIVEPOWER_OUTPUT_S`, " \
 "  `PVINVERTER_POWERFACTOR_OUTPUT_S`, " \
 "  `PVINVERTER_CURRENT_PCC_S`, " \
 "  `PVINVERTER_ACTIVEPOWER_PCC_S`, " \
 "  `PVINVERTER_REACTIVEPOWER_PCC_S`, " \
 "  `PVINVERTER_POWERFACTOR_PCC_S`, " \
 "  `PVINVERTER_VOLTAGE_PHASE_T`, " \
 "  `PVINVERTER_CURRENT_OUTPUT_T`, " \
 "  `PVINVERTER_ACTIVEPOWER_OUTPUT_T`, " \
 "  `PVINVERTER_REACTIVEPOWER_OUTPUT_T`, " \
 "  `PVINVERTER_POWERFACTOR_OUTPUT_T`, " \
 "  `PVINVERTER_CURRENT_PCC_T`, " \
 "  `PVINVERTER_ACTIVEPOWER_PCC_T`, " \
 "  `PVINVERTER_REACTIVEPOWER_PCC_T`, " \
 "  `PVINVERTER_POWERFACTOR_PCC_T`, " \
 "  `PVINVERTER_ACTIVEPOWER_PV_EXT`, " \
 "  `PVINVERTER_ACTIVEPOWER_LOAD_SYS`, " \
 "  `PVINVERTER_ACTIVEPOWER_LOAD_TOTAL`, " \
 "  `PVINVERTER_REACTIVEPOWER_LOAD_TOTAL`, " \
 "  `PVINVERTER_APPARENTPOWER_LOAD_TOTAL`, " \
 "  `PVINVERTER_FREQUENCY_OUTPUT`, " \
 "  `PVINVERTER_VOLTAGE_OUTPUT_R`, " \
 "  `PVINVERTER_CURRENT_LOAD_R`, " \
 "  `PVINVERTER_ACTIVEPOWER_LOAD_R`, " \
 "  `PVINVERTER_REACTIVEPOWER_LOAD_R`, " \
 "  `PVINVERTER_APPARENTPOWER_LOAD_R`, " \
 "  `PVINVERTER_LOADPEAKRATIO_R`, " \
 "  `PVINVERTER_VOLTAGE_OUTPUT_S`, " \
 "  `PVINVERTER_CURRENT_LOAD_S`, " \
 "  `PVINVERTER_ACTIVEPOWER_LOAD_S`, " \
 "  `PVINVERTER_REACTIVEPOWER_LOAD_S`, " \
 "  `PVINVERTER_APPARENTPOWER_LOAD_S`, " \
 "  `PVINVERTER_LOADPEAKRATIO_S`, " \
 "  `PVINVERTER_VOLTAGE_OUTPUT_T`, " \
 "  `PVINVERTER_CURRENT_LOAD_T`, " \
 "  `PVINVERTER_ACTIVEPOWER_LOAD_T`, " \
 "  `PVINVERTER_REACTIVEPOWER_LOAD_T`, " \
 "  `PVINVERTER_APPARENTPOWER_LOAD_T`, " \
 "  `PVINVERTER_LOADPEAKRATIO_T`, " \
 "  `PVINVERTER_VOLTAGE_PV1`, " \
 "  `PVINVERTER_CURRENT_PV1`, " \
 "  `PVINVERTER_POWER_PV1`, " \
 "  `PVINVERTER_VOLTAGE_PV2`, " \
 "  `PVINVERTER_CURRENT_PV2`, " \
 "  `PVINVERTER_POWER_PV2`, " \
 "  `PVINVERTER_VOLTAGE_PV3`, " \
 "  `PVINVERTER_CURRENT_PV3`, " \
 "  `PVINVERTER_POWER_PV3`, " \
 "  `PVINVERTER_VOLTAGE_PV4`, " \
 "  `PVINVERTER_CURRENT_PV4`, " \
 "  `PVINVERTER_POWER_PV4`, " \
 "  `PVINVERTER_VOLTAGE_PV5`, " \
 "  `PVINVERTER_CURRENT_PV5`, " \
 "  `PVINVERTER_POWER_PV5`, " \
 "  `PVINVERTER_VOLTAGE_PV6`, " \
 "  `PVINVERTER_CURRENT_PV6`, " \
 "  `PVINVERTER_POWER_PV6`, " \
 "  `PVINVERTER_VOLTAGE_PV7`, " \
 "  `PVINVERTER_CURRENT_PV7`, " \
 "  `PVINVERTER_POWER_PV7`, " \
 "  `PVINVERTER_VOLTAGE_PV8`, " \
 "  `PVINVERTER_CURRENT_PV8`, " \
 "  `PVINVERTER_POWER_PV8`, " \
 "  `PVINVERTER_VOLTAGE_PV9`, " \
 "  `PVINVERTER_CURRENT_PV9`, " \
 "  `PVINVERTER_POWER_PV9`, " \
 "  `PVINVERTER_VOLTAGE_PV10`, " \
 "  `PVINVERTER_CURRENT_PV10`, " \
 "  `PVINVERTER_POWER_PV10`, " \
 "  `PVINVERTER_VOLTAGE_PV11`, " \
 "  `PVINVERTER_CURRENT_PV11`, " \
 "  `PVINVERTER_POWER_PV11`, " \
 "  `PVINVERTER_VOLTAGE_PV12`, " \
 "  `PVINVERTER_CURRENT_PV12`, " \
 "  `PVINVERTER_POWER_PV12`, " \
 "  `PVINVERTER_VOLTAGE_PV13`, " \
 "  `PVINVERTER_CURRENT_PV13`, " \
 "  `PVINVERTER_POWER_PV13`, " \
 "  `PVINVERTER_VOLTAGE_PV14`, " \
 "  `PVINVERTER_CURRENT_PV14`, " \
 "  `PVINVERTER_POWER_PV14`, " \
 "  `PVINVERTER_VOLTAGE_PV15`, " \
 "  `PVINVERTER_CURRENT_PV15`, " \
 "  `PVINVERTER_POWER_PV15`, " \
 "  `PVINVERTER_VOLTAGE_PV16`, " \
 "  `PVINVERTER_CURRENT_PV16`, " \
 "  `PVINVERTER_POWER_PV16`, " \
 "  `PVINVERTER_GENERATIONTIME_TODAY`, " \
 "  `PVINVERTER_PV_GENERATION_TODAY`, " \
 "  `PVINVERTER_PV_GENERATION_TOTAL`, " \
 "  `PVINVERTER_LOAD_CONSUMPTION_TODAY`, " \
 "  `PVINVERTER_LOAD_CONSUMPTION_TOTAL`, " \
 "  `PVINVERTER_ENERGY_PURCHASE_TODAY`, " \
 "  `PVINVERTER_ENERGY_PURCHASE_TOTAL`, " \
 "  `PVINVERTER_ENERGY_SELLING_TODAY`, " \
 "  `PVINVERTER_ENERGY_SELLING_TOTAL`, " \
 "  `PVINVERTER_REMOTE_ON_OFF_CONTROL`, " \
 "  `RECORD_TS` " \
 "FROM `%s` " \
 "WHERE `ID` = (SELECT MAX(`ID`) FROM `%s`) "

typedef struct pvInverter_table
{
    int ID;
    unsigned int		PVINVERTER_SYSSTATE;		//运行状态
    unsigned int		PVINVERTER_GENERATIONTIME_TOTAL;		//总发电时间
    unsigned int		PVINVERTER_SERVICETIME_TOTAL;		//总运行时间
    unsigned int		PVINVERTER_FREQUENCY_GRID;		//电网频率
    int		PVINVERTER_ACTIVEPOWER_OUTPUT_TOTAL;		//总有功功率。放电为正，充电为负
    int		PVINVERTER_REACTIVEPOWER_OUTPUT_TOTAL;		//总无功功率。逆变器端超前为正，滞后为负
    int		PVINVERTER_APPARENTPOWER_OUTPUT_TOTAL;		//总视在功率。放电为正，充电为负
    int		PVINVERTER_ACTIVEPOWER_PCC_TOTAL;		//总PCC有功功率。卖电为正，买电为负
    int		PVINVERTER_REACTIVEPOWER_PCC_TOTAL;		//总PCC无功功率。逆变器端超前为正，滞后为负
    int		PVINVERTER_APPARENTPOWER_PCC_TOTAL;		//总PCC视在功率。卖电为正，买电为负
    unsigned int		PVINVERTER_VOLTAGE_PHASE_R;		//R相电网电压
    unsigned int		PVINVERTER_CURRENT_OUTPUT_R;		//R相逆变器输出电流
    int		PVINVERTER_ACTIVEPOWER_OUTPUT_R;		//R相逆变器输出有功功率。放电为正，充电为负
    int		PVINVERTER_REACTIVEPOWER_OUTPUT_R;		//R相逆变器输出无功功率。逆变器端超前为正，滞后为负
    int		PVINVERTER_POWERFACTOR_OUTPUT_R;		//R相功率因数。逆变器端超前为正，滞后为负
    unsigned int		PVINVERTER_CURRENT_PCC_R;			//R相PCC电流
    int		PVINVERTER_ACTIVEPOWER_PCC_R;		//R相PCC有功功率
    int		PVINVERTER_REACTIVEPOWER_PCC_R;		//R相PCC无功功率。逆变器端超前为正，滞后为负
    int		PVINVERTER_POWERFACTOR_PCC_R;		//R相PCC功率因数。逆变器端超前为正，滞后为负

    unsigned int		PVINVERTER_VOLTAGE_PHASE_S;		//S相电网电压
    unsigned int		PVINVERTER_CURRENT_OUTPUT_S;		//S相逆变器输出电流
    int		PVINVERTER_ACTIVEPOWER_OUTPUT_S;		//S相逆变器输出有功功率。放电为正，充电为负
    int		PVINVERTER_REACTIVEPOWER_OUTPUT_S;		//S相逆变器输出无功功率。逆变器端超前为正，滞后为负
    int		PVINVERTER_POWERFACTOR_OUTPUT_S;		//S相功率因数。逆变器端超前为正，滞后为负
    unsigned int		PVINVERTER_CURRENT_PCC_S;			//S相PCC电流
    int		PVINVERTER_ACTIVEPOWER_PCC_S;		//S相PCC有功功率
    int		PVINVERTER_REACTIVEPOWER_PCC_S;		//S相PCC无功功率。逆变器端超前为正，滞后为负
    int		PVINVERTER_POWERFACTOR_PCC_S;		//S相PCC功率因数。逆变器端超前为正，滞后为负

    unsigned int		PVINVERTER_VOLTAGE_PHASE_T;		//T相电网电压
    unsigned int		PVINVERTER_CURRENT_OUTPUT_T;		//T相逆变器输出电流
    int		PVINVERTER_ACTIVEPOWER_OUTPUT_T;		//T相逆变器输出有功功率。放电为正，充电为负
    int		PVINVERTER_REACTIVEPOWER_OUTPUT_T;		//T相逆变器输出无功功率。逆变器端超前为正，滞后为负
    int		PVINVERTER_POWERFACTOR_OUTPUT_T;		//T相功率因数。逆变器端超前为正，滞后为负
    unsigned int		PVINVERTER_CURRENT_PCC_T;			//T相PCC电流
    int		PVINVERTER_ACTIVEPOWER_PCC_T;		//T相PCC有功功率
    int		PVINVERTER_REACTIVEPOWER_PCC_T;		//T相PCC无功功率。逆变器端超前为正，滞后为负
    int		PVINVERTER_POWERFACTOR_PCC_T;		//T相PCC功率因数。逆变器端超前为正，滞后为负

    unsigned int		PVINVERTER_ACTIVEPOWER_PV_EXT;		//外部发电功率
    unsigned int		PVINVERTER_ACTIVEPOWER_LOAD_SYS;		//系统总负载功率

    int		PVINVERTER_ACTIVEPOWER_LOAD_TOTAL;		//负载有功功率。负载消耗为正，回馈为负。
    int		PVINVERTER_REACTIVEPOWER_LOAD_TOTAL;		//负载无功功率
    int		PVINVERTER_APPARENTPOWER_LOAD_TOTAL;		//负载视在功率。负载消耗为正，回馈为负。
    unsigned int		PVINVERTER_FREQUENCY_OUTPUT;			//输出电压频率


    unsigned int		PVINVERTER_VOLTAGE_OUTPUT_R;		//R相逆变器输出电压
    int		PVINVERTER_CURRENT_LOAD_R;		//R相负载电流
    int		PVINVERTER_ACTIVEPOWER_LOAD_R;		//R相负载有功功率。负载消耗为正，回馈为负。
    int		PVINVERTER_REACTIVEPOWER_LOAD_R;		//R相负载无功功率。逆变器端超前为正，滞后为负
    int		PVINVERTER_APPARENTPOWER_LOAD_R;		//R相负载视在功率。负载消耗为正，回馈为负。
    unsigned int		PVINVERTER_LOADPEAKRATIO_R;			//R相负载峰值比

    unsigned int		PVINVERTER_VOLTAGE_OUTPUT_S;		//S相逆变器输出电压
    int		PVINVERTER_CURRENT_LOAD_S;		//S相负载电流
    int		PVINVERTER_ACTIVEPOWER_LOAD_S;		//S相负载有功功率。负载消耗为正，回馈为负。
    int		PVINVERTER_REACTIVEPOWER_LOAD_S;		//S相负载无功功率。逆变器端超前为正，滞后为负
    int		PVINVERTER_APPARENTPOWER_LOAD_S;		//S相负载视在功率。负载消耗为正，回馈为负。
    unsigned int		PVINVERTER_LOADPEAKRATIO_S;			//S相负载峰值比

    unsigned int		PVINVERTER_VOLTAGE_OUTPUT_T;		//T相逆变器输出电压
    int		PVINVERTER_CURRENT_LOAD_T;		//T相负载电流
    int		PVINVERTER_ACTIVEPOWER_LOAD_T;		//T相负载有功功率。负载消耗为正，回馈为负。
    int		PVINVERTER_REACTIVEPOWER_LOAD_T;		//T相负载无功功率。逆变器端超前为正，滞后为负
    int		PVINVERTER_APPARENTPOWER_LOAD_T;		//T相负载视在功率。负载消耗为正，回馈为负。
    unsigned int		PVINVERTER_LOADPEAKRATIO_T;			//T相负载峰值比

    unsigned int		PVINVERTER_VOLTAGE_PV1;			//第1路PV电压
    unsigned int		PVINVERTER_CURRENT_PV1;			//第1路PV电流
    unsigned int		PVINVERTER_POWER_PV1;			//第1路PV功率

    unsigned int		PVINVERTER_VOLTAGE_PV2;			//第2路PV电压
    unsigned int		PVINVERTER_CURRENT_PV2;			//第2路PV电流
    unsigned int		PVINVERTER_POWER_PV2;			//第2路PV功率

    unsigned int		PVINVERTER_VOLTAGE_PV3;			//第3路PV电压
    unsigned int		PVINVERTER_CURRENT_PV3;			//第3路PV电流
    unsigned int		PVINVERTER_POWER_PV3;			//第3路PV功率

    unsigned int		PVINVERTER_VOLTAGE_PV4;			//第4路PV电压
    unsigned int		PVINVERTER_CURRENT_PV4;			//第4路PV电流
    unsigned int		PVINVERTER_POWER_PV4;			//第4路PV功率

    unsigned int		PVINVERTER_VOLTAGE_PV5;			//第5路PV电压
    unsigned int		PVINVERTER_CURRENT_PV5;			//第5路PV电流
    unsigned int		PVINVERTER_POWER_PV5;			//第5路PV功率

    unsigned int		PVINVERTER_VOLTAGE_PV6;			//第6路PV电压
    unsigned int		PVINVERTER_CURRENT_PV6;			//第6路PV电流
    unsigned int		PVINVERTER_POWER_PV6;			//第6路PV功率

    unsigned int		PVINVERTER_VOLTAGE_PV7;			//第7路PV电压
    unsigned int		PVINVERTER_CURRENT_PV7;			//第7路PV电流
    unsigned int		PVINVERTER_POWER_PV7;			//第7路PV功率

    unsigned int		PVINVERTER_VOLTAGE_PV8;			//第8路PV电压
    unsigned int		PVINVERTER_CURRENT_PV8;			//第8路PV电流
    unsigned int		PVINVERTER_POWER_PV8;			//第8路PV功率

    unsigned int		PVINVERTER_VOLTAGE_PV9;			//第9路PV电压
    unsigned int		PVINVERTER_CURRENT_PV9;			//第9路PV电流
    unsigned int		PVINVERTER_POWER_PV9;			//第9路PV功率

    unsigned int		PVINVERTER_VOLTAGE_PV10;			//第10路PV电压
    unsigned int		PVINVERTER_CURRENT_PV10;			//第10路PV电流
    unsigned int		PVINVERTER_POWER_PV10;			//第10路PV功率

    unsigned int		PVINVERTER_VOLTAGE_PV11;			//第11路PV电压
    unsigned int		PVINVERTER_CURRENT_PV11;			//第11路PV电流
    unsigned int		PVINVERTER_POWER_PV11;			//第11路PV功率

    unsigned int		PVINVERTER_VOLTAGE_PV12;			//第12路PV电压
    unsigned int		PVINVERTER_CURRENT_PV12;			//第12路PV电流
    unsigned int		PVINVERTER_POWER_PV12;			//第12路PV功率

    unsigned int		PVINVERTER_VOLTAGE_PV13;			//第13路PV电压
    unsigned int		PVINVERTER_CURRENT_PV13;			//第13路PV电流
    unsigned int		PVINVERTER_POWER_PV13;			//第13路PV功率

    unsigned int		PVINVERTER_VOLTAGE_PV14;			//第14路PV电压
    unsigned int		PVINVERTER_CURRENT_PV14;			//第14路PV电流
    unsigned int		PVINVERTER_POWER_PV14;			//第14路PV功率

    unsigned int		PVINVERTER_VOLTAGE_PV15;			//第15路PV电压
    unsigned int		PVINVERTER_CURRENT_PV15;			//第15路PV电流
    unsigned int		PVINVERTER_POWER_PV15;			//第15路PV功率

    unsigned int		PVINVERTER_VOLTAGE_PV16;			//第16路PV电压
    unsigned int		PVINVERTER_CURRENT_PV16;			//第16路PV电流
    unsigned int		PVINVERTER_POWER_PV16;			//第16路PV功率

    unsigned int		PVINVERTER_GENERATIONTIME_TODAY;		//当日发电时间
    unsigned int			PVINVERTER_PV_GENERATION_TODAY;		//当日发电量
    unsigned int			PVINVERTER_PV_GENERATION_TOTAL;		//总发电量
    unsigned int			PVINVERTER_LOAD_CONSUMPTION_TODAY;		//当日负载耗电量
    unsigned int			PVINVERTER_LOAD_CONSUMPTION_TOTAL;		//总负载耗电量
    unsigned int			PVINVERTER_ENERGY_PURCHASE_TODAY;		//当日买电量
    unsigned int			PVINVERTER_ENERGY_PURCHASE_TOTAL;		//总买电量
    unsigned int			PVINVERTER_ENERGY_SELLING_TODAY;		//当日卖电量
    unsigned int			PVINVERTER_ENERGY_SELLING_TOTAL;		//总卖电量
    unsigned int		PVINVERTER_REMOTE_ON_OFF_CONTROL;		//远程开关机。	0x0000：关机	0x0001：开机
    sqlite3_int64 RECORD_TS;                // 时间戳

} PVINVERTERTable;

#define PVINVERTER_TABLE_ATTR_ID  "ID"
#define PVINVERTER_TABLE_ATTR_PVINVERTER_SYSSTATE  "PVINVERTER_SYSSTATE"
#define PVINVERTER_TABLE_ATTR_PVINVERTER_GENERATIONTIME_TOTAL  "PVINVERTER_GENERATIONTIME_TOTAL"
#define PVINVERTER_TABLE_ATTR_PVINVERTER_SERVICETIME_TOTAL  "PVINVERTER_SERVICETIME_TOTAL"
#define PVINVERTER_TABLE_ATTR_PVINVERTER_FREQUENCY_GRID  "PVINVERTER_FREQUENCY_GRID"
#define PVINVERTER_TABLE_ATTR_PVINVERTER_ACTIVEPOWER_OUTPUT_TOTAL  "PVINVERTER_ACTIVEPOWER_OUTPUT_TOTAL"
#define PVINVERTER_TABLE_ATTR_PVINVERTER_REACTIVEPOWER_OUTPUT_TOTAL  "PVINVERTER_REACTIVEPOWER_OUTPUT_TOTAL"
#define PVINVERTER_TABLE_ATTR_PVINVERTER_APPARENTPOWER_OUTPUT_TOTAL  "PVINVERTER_APPARENTPOWER_OUTPUT_TOTAL"
#define PVINVERTER_TABLE_ATTR_PVINVERTER_ACTIVEPOWER_PCC_TOTAL  "PVINVERTER_ACTIVEPOWER_PCC_TOTAL"
#define PVINVERTER_TABLE_ATTR_PVINVERTER_REACTIVEPOWER_PCC_TOTAL  "PVINVERTER_REACTIVEPOWER_PCC_TOTAL"
#define PVINVERTER_TABLE_ATTR_PVINVERTER_APPARENTPOWER_PCC_TOTAL  "PVINVERTER_APPARENTPOWER_PCC_TOTAL"
#define PVINVERTER_TABLE_ATTR_PVINVERTER_VOLTAGE_PHASE_R  "PVINVERTER_VOLTAGE_PHASE_R"
#define PVINVERTER_TABLE_ATTR_PVINVERTER_CURRENT_OUTPUT_R  "PVINVERTER_CURRENT_OUTPUT_R"
#define PVINVERTER_TABLE_ATTR_PVINVERTER_ACTIVEPOWER_OUTPUT_R  "PVINVERTER_ACTIVEPOWER_OUTPUT_R"
#define PVINVERTER_TABLE_ATTR_PVINVERTER_REACTIVEPOWER_OUTPUT_R  "PVINVERTER_REACTIVEPOWER_OUTPUT_R"
#define PVINVERTER_TABLE_ATTR_PVINVERTER_POWERFACTOR_OUTPUT_R  "PVINVERTER_POWERFACTOR_OUTPUT_R"
#define PVINVERTER_TABLE_ATTR_PVINVERTER_CURRENT_PCC_R  "PVINVERTER_CURRENT_PCC_R"
#define PVINVERTER_TABLE_ATTR_PVINVERTER_ACTIVEPOWER_PCC_R  "PVINVERTER_ACTIVEPOWER_PCC_R"
#define PVINVERTER_TABLE_ATTR_PVINVERTER_REACTIVEPOWER_PCC_R  "PVINVERTER_REACTIVEPOWER_PCC_R"
#define PVINVERTER_TABLE_ATTR_PVINVERTER_POWERFACTOR_PCC_R  "PVINVERTER_POWERFACTOR_PCC_R"
#define PVINVERTER_TABLE_ATTR_PVINVERTER_VOLTAGE_PHASE_S  "PVINVERTER_VOLTAGE_PHASE_S"
#define PVINVERTER_TABLE_ATTR_PVINVERTER_CURRENT_OUTPUT_S  "PVINVERTER_CURRENT_OUTPUT_S"
#define PVINVERTER_TABLE_ATTR_PVINVERTER_ACTIVEPOWER_OUTPUT_S  "PVINVERTER_ACTIVEPOWER_OUTPUT_S"
#define PVINVERTER_TABLE_ATTR_PVINVERTER_REACTIVEPOWER_OUTPUT_S  "PVINVERTER_REACTIVEPOWER_OUTPUT_S"
#define PVINVERTER_TABLE_ATTR_PVINVERTER_POWERFACTOR_OUTPUT_S  "PVINVERTER_POWERFACTOR_OUTPUT_S"
#define PVINVERTER_TABLE_ATTR_PVINVERTER_CURRENT_PCC_S  "PVINVERTER_CURRENT_PCC_S"
#define PVINVERTER_TABLE_ATTR_PVINVERTER_ACTIVEPOWER_PCC_S  "PVINVERTER_ACTIVEPOWER_PCC_S"
#define PVINVERTER_TABLE_ATTR_PVINVERTER_REACTIVEPOWER_PCC_S  "PVINVERTER_REACTIVEPOWER_PCC_S"
#define PVINVERTER_TABLE_ATTR_PVINVERTER_POWERFACTOR_PCC_S  "PVINVERTER_POWERFACTOR_PCC_S"
#define PVINVERTER_TABLE_ATTR_PVINVERTER_VOLTAGE_PHASE_T  "PVINVERTER_VOLTAGE_PHASE_T"
#define PVINVERTER_TABLE_ATTR_PVINVERTER_CURRENT_OUTPUT_T  "PVINVERTER_CURRENT_OUTPUT_T"
#define PVINVERTER_TABLE_ATTR_PVINVERTER_ACTIVEPOWER_OUTPUT_T  "PVINVERTER_ACTIVEPOWER_OUTPUT_T"
#define PVINVERTER_TABLE_ATTR_PVINVERTER_REACTIVEPOWER_OUTPUT_T  "PVINVERTER_REACTIVEPOWER_OUTPUT_T"
#define PVINVERTER_TABLE_ATTR_PVINVERTER_POWERFACTOR_OUTPUT_T  "PVINVERTER_POWERFACTOR_OUTPUT_T"
#define PVINVERTER_TABLE_ATTR_PVINVERTER_CURRENT_PCC_T  "PVINVERTER_CURRENT_PCC_T"
#define PVINVERTER_TABLE_ATTR_PVINVERTER_ACTIVEPOWER_PCC_T  "PVINVERTER_ACTIVEPOWER_PCC_T"
#define PVINVERTER_TABLE_ATTR_PVINVERTER_REACTIVEPOWER_PCC_T  "PVINVERTER_REACTIVEPOWER_PCC_T"
#define PVINVERTER_TABLE_ATTR_PVINVERTER_POWERFACTOR_PCC_T  "PVINVERTER_POWERFACTOR_PCC_T"
#define PVINVERTER_TABLE_ATTR_PVINVERTER_ACTIVEPOWER_PV_EXT  "PVINVERTER_ACTIVEPOWER_PV_EXT"
#define PVINVERTER_TABLE_ATTR_PVINVERTER_ACTIVEPOWER_LOAD_SYS  "PVINVERTER_ACTIVEPOWER_LOAD_SYS"
#define PVINVERTER_TABLE_ATTR_PVINVERTER_ACTIVEPOWER_LOAD_TOTAL  "PVINVERTER_ACTIVEPOWER_LOAD_TOTAL"
#define PVINVERTER_TABLE_ATTR_PVINVERTER_REACTIVEPOWER_LOAD_TOTAL  "PVINVERTER_REACTIVEPOWER_LOAD_TOTAL"
#define PVINVERTER_TABLE_ATTR_PVINVERTER_APPARENTPOWER_LOAD_TOTAL  "PVINVERTER_APPARENTPOWER_LOAD_TOTAL"
#define PVINVERTER_TABLE_ATTR_PVINVERTER_FREQUENCY_OUTPUT  "PVINVERTER_FREQUENCY_OUTPUT"
#define PVINVERTER_TABLE_ATTR_PVINVERTER_VOLTAGE_OUTPUT_R  "PVINVERTER_VOLTAGE_OUTPUT_R"
#define PVINVERTER_TABLE_ATTR_PVINVERTER_CURRENT_LOAD_R  "PVINVERTER_CURRENT_LOAD_R"
#define PVINVERTER_TABLE_ATTR_PVINVERTER_ACTIVEPOWER_LOAD_R  "PVINVERTER_ACTIVEPOWER_LOAD_R"
#define PVINVERTER_TABLE_ATTR_PVINVERTER_REACTIVEPOWER_LOAD_R  "PVINVERTER_REACTIVEPOWER_LOAD_R"
#define PVINVERTER_TABLE_ATTR_PVINVERTER_APPARENTPOWER_LOAD_R  "PVINVERTER_APPARENTPOWER_LOAD_R"
#define PVINVERTER_TABLE_ATTR_PVINVERTER_LOADPEAKRATIO_R  "PVINVERTER_LOADPEAKRATIO_R"
#define PVINVERTER_TABLE_ATTR_PVINVERTER_VOLTAGE_OUTPUT_S  "PVINVERTER_VOLTAGE_OUTPUT_S"
#define PVINVERTER_TABLE_ATTR_PVINVERTER_CURRENT_LOAD_S  "PVINVERTER_CURRENT_LOAD_S"
#define PVINVERTER_TABLE_ATTR_PVINVERTER_ACTIVEPOWER_LOAD_S  "PVINVERTER_ACTIVEPOWER_LOAD_S"
#define PVINVERTER_TABLE_ATTR_PVINVERTER_REACTIVEPOWER_LOAD_S  "PVINVERTER_REACTIVEPOWER_LOAD_S"
#define PVINVERTER_TABLE_ATTR_PVINVERTER_APPARENTPOWER_LOAD_S  "PVINVERTER_APPARENTPOWER_LOAD_S"
#define PVINVERTER_TABLE_ATTR_PVINVERTER_LOADPEAKRATIO_S  "PVINVERTER_LOADPEAKRATIO_S"
#define PVINVERTER_TABLE_ATTR_PVINVERTER_VOLTAGE_OUTPUT_T  "PVINVERTER_VOLTAGE_OUTPUT_T"
#define PVINVERTER_TABLE_ATTR_PVINVERTER_CURRENT_LOAD_T  "PVINVERTER_CURRENT_LOAD_T"
#define PVINVERTER_TABLE_ATTR_PVINVERTER_ACTIVEPOWER_LOAD_T  "PVINVERTER_ACTIVEPOWER_LOAD_T"
#define PVINVERTER_TABLE_ATTR_PVINVERTER_REACTIVEPOWER_LOAD_T  "PVINVERTER_REACTIVEPOWER_LOAD_T"
#define PVINVERTER_TABLE_ATTR_PVINVERTER_APPARENTPOWER_LOAD_T  "PVINVERTER_APPARENTPOWER_LOAD_T"
#define PVINVERTER_TABLE_ATTR_PVINVERTER_LOADPEAKRATIO_T  "PVINVERTER_LOADPEAKRATIO_T"
#define PVINVERTER_TABLE_ATTR_PVINVERTER_VOLTAGE_PV1  "PVINVERTER_VOLTAGE_PV1"
#define PVINVERTER_TABLE_ATTR_PVINVERTER_CURRENT_PV1  "PVINVERTER_CURRENT_PV1"
#define PVINVERTER_TABLE_ATTR_PVINVERTER_POWER_PV1  "PVINVERTER_POWER_PV1"
#define PVINVERTER_TABLE_ATTR_PVINVERTER_VOLTAGE_PV2  "PVINVERTER_VOLTAGE_PV2"
#define PVINVERTER_TABLE_ATTR_PVINVERTER_CURRENT_PV2  "PVINVERTER_CURRENT_PV2"
#define PVINVERTER_TABLE_ATTR_PVINVERTER_POWER_PV2  "PVINVERTER_POWER_PV2"
#define PVINVERTER_TABLE_ATTR_PVINVERTER_VOLTAGE_PV3  "PVINVERTER_VOLTAGE_PV3"
#define PVINVERTER_TABLE_ATTR_PVINVERTER_CURRENT_PV3  "PVINVERTER_CURRENT_PV3"
#define PVINVERTER_TABLE_ATTR_PVINVERTER_POWER_PV3  "PVINVERTER_POWER_PV3"
#define PVINVERTER_TABLE_ATTR_PVINVERTER_VOLTAGE_PV4  "PVINVERTER_VOLTAGE_PV4"
#define PVINVERTER_TABLE_ATTR_PVINVERTER_CURRENT_PV4  "PVINVERTER_CURRENT_PV4"
#define PVINVERTER_TABLE_ATTR_PVINVERTER_POWER_PV4  "PVINVERTER_POWER_PV4"
#define PVINVERTER_TABLE_ATTR_PVINVERTER_VOLTAGE_PV5  "PVINVERTER_VOLTAGE_PV5"
#define PVINVERTER_TABLE_ATTR_PVINVERTER_CURRENT_PV5  "PVINVERTER_CURRENT_PV5"
#define PVINVERTER_TABLE_ATTR_PVINVERTER_POWER_PV5  "PVINVERTER_POWER_PV5"
#define PVINVERTER_TABLE_ATTR_PVINVERTER_VOLTAGE_PV6  "PVINVERTER_VOLTAGE_PV6"
#define PVINVERTER_TABLE_ATTR_PVINVERTER_CURRENT_PV6  "PVINVERTER_CURRENT_PV6"
#define PVINVERTER_TABLE_ATTR_PVINVERTER_POWER_PV6  "PVINVERTER_POWER_PV6"
#define PVINVERTER_TABLE_ATTR_PVINVERTER_VOLTAGE_PV7  "PVINVERTER_VOLTAGE_PV7"
#define PVINVERTER_TABLE_ATTR_PVINVERTER_CURRENT_PV7  "PVINVERTER_CURRENT_PV7"
#define PVINVERTER_TABLE_ATTR_PVINVERTER_POWER_PV7  "PVINVERTER_POWER_PV7"
#define PVINVERTER_TABLE_ATTR_PVINVERTER_VOLTAGE_PV8  "PVINVERTER_VOLTAGE_PV8"
#define PVINVERTER_TABLE_ATTR_PVINVERTER_CURRENT_PV8  "PVINVERTER_CURRENT_PV8"
#define PVINVERTER_TABLE_ATTR_PVINVERTER_POWER_PV8  "PVINVERTER_POWER_PV8"
#define PVINVERTER_TABLE_ATTR_PVINVERTER_VOLTAGE_PV9  "PVINVERTER_VOLTAGE_PV9"
#define PVINVERTER_TABLE_ATTR_PVINVERTER_CURRENT_PV9  "PVINVERTER_CURRENT_PV9"
#define PVINVERTER_TABLE_ATTR_PVINVERTER_POWER_PV9  "PVINVERTER_POWER_PV9"
#define PVINVERTER_TABLE_ATTR_PVINVERTER_VOLTAGE_PV10  "PVINVERTER_VOLTAGE_PV10"
#define PVINVERTER_TABLE_ATTR_PVINVERTER_CURRENT_PV10  "PVINVERTER_CURRENT_PV10"
#define PVINVERTER_TABLE_ATTR_PVINVERTER_POWER_PV10  "PVINVERTER_POWER_PV10"
#define PVINVERTER_TABLE_ATTR_PVINVERTER_VOLTAGE_PV11  "PVINVERTER_VOLTAGE_PV11"
#define PVINVERTER_TABLE_ATTR_PVINVERTER_CURRENT_PV11  "PVINVERTER_CURRENT_PV11"
#define PVINVERTER_TABLE_ATTR_PVINVERTER_POWER_PV11  "PVINVERTER_POWER_PV11"
#define PVINVERTER_TABLE_ATTR_PVINVERTER_VOLTAGE_PV12  "PVINVERTER_VOLTAGE_PV12"
#define PVINVERTER_TABLE_ATTR_PVINVERTER_CURRENT_PV12  "PVINVERTER_CURRENT_PV12"
#define PVINVERTER_TABLE_ATTR_PVINVERTER_POWER_PV12  "PVINVERTER_POWER_PV12"
#define PVINVERTER_TABLE_ATTR_PVINVERTER_VOLTAGE_PV13  "PVINVERTER_VOLTAGE_PV13"
#define PVINVERTER_TABLE_ATTR_PVINVERTER_CURRENT_PV13  "PVINVERTER_CURRENT_PV13"
#define PVINVERTER_TABLE_ATTR_PVINVERTER_POWER_PV13  "PVINVERTER_POWER_PV13"
#define PVINVERTER_TABLE_ATTR_PVINVERTER_VOLTAGE_PV14  "PVINVERTER_VOLTAGE_PV14"
#define PVINVERTER_TABLE_ATTR_PVINVERTER_CURRENT_PV14  "PVINVERTER_CURRENT_PV14"
#define PVINVERTER_TABLE_ATTR_PVINVERTER_POWER_PV14  "PVINVERTER_POWER_PV14"
#define PVINVERTER_TABLE_ATTR_PVINVERTER_VOLTAGE_PV15  "PVINVERTER_VOLTAGE_PV15"
#define PVINVERTER_TABLE_ATTR_PVINVERTER_CURRENT_PV15  "PVINVERTER_CURRENT_PV15"
#define PVINVERTER_TABLE_ATTR_PVINVERTER_POWER_PV15  "PVINVERTER_POWER_PV15"
#define PVINVERTER_TABLE_ATTR_PVINVERTER_VOLTAGE_PV16  "PVINVERTER_VOLTAGE_PV16"
#define PVINVERTER_TABLE_ATTR_PVINVERTER_CURRENT_PV16  "PVINVERTER_CURRENT_PV16"
#define PVINVERTER_TABLE_ATTR_PVINVERTER_POWER_PV16  "PVINVERTER_POWER_PV16"
#define PVINVERTER_TABLE_ATTR_PVINVERTER_GENERATIONTIME_TODAY  "PVINVERTER_GENERATIONTIME_TODAY"
#define PVINVERTER_TABLE_ATTR_PVINVERTER_PV_GENERATION_TODAY  "PVINVERTER_PV_GENERATION_TODAY"
#define PVINVERTER_TABLE_ATTR_PVINVERTER_PV_GENERATION_TOTAL  "PVINVERTER_PV_GENERATION_TOTAL"
#define PVINVERTER_TABLE_ATTR_PVINVERTER_LOAD_CONSUMPTION_TODAY  "PVINVERTER_LOAD_CONSUMPTION_TODAY"
#define PVINVERTER_TABLE_ATTR_PVINVERTER_LOAD_CONSUMPTION_TOTAL  "PVINVERTER_LOAD_CONSUMPTION_TOTAL"
#define PVINVERTER_TABLE_ATTR_PVINVERTER_ENERGY_PURCHASE_TODAY  "PVINVERTER_ENERGY_PURCHASE_TODAY"
#define PVINVERTER_TABLE_ATTR_PVINVERTER_ENERGY_PURCHASE_TOTAL  "PVINVERTER_ENERGY_PURCHASE_TOTAL"
#define PVINVERTER_TABLE_ATTR_PVINVERTER_ENERGY_SELLING_TODAY  "PVINVERTER_ENERGY_SELLING_TODAY"
#define PVINVERTER_TABLE_ATTR_PVINVERTER_ENERGY_SELLING_TOTAL  "PVINVERTER_ENERGY_SELLING_TOTAL"
#define PVINVERTER_TABLE_ATTR_PVINVERTER_REMOTE_ON_OFF_CONTROL  "PVINVERTER_REMOTE_ON_OFF_CONTROL"
#define PVINVERTER_TABLE_ATTR_RECORD_TS  "RECORD_TS"



/**
 *  P485METER_TABLE
**/
#define C_P485METER_TABLE "P485METER_TABLE"
#define C_P485METER_TABLE_SELECT \
  "SELECT `ID`," \
  "`P485METER_UA`," \
  "`P485METER_UB`," \
  "`P485METER_UC`," \
  "`P485METER_IA`," \
  "`P485METER_IB`," \
  "`P485METER_IC`," \
  "`P485METER_ACTIVEPOWER_TOTAL`," \
  "`P485METER_APPARENTPOWER_TOTAL`," \
  "`P485METER_REACTIVEPOWER_TOTAL`," \
  "`P485METER_POWERFACTOR_TOTAL`," \
  "`P485METER_ACTIVEPOWERDEMAND_TOTAL`," \
  "`P485METER_UAB`," \
  "`P485METER_UBC`," \
  "`P485METER_UCA`," \
  "`P485METER_CURRENTIMBALANCE`," \
  "`P485METER_ZEROSEQUENCECURRENT`," \
  "`P485METER_ACTIVEPOWER_A`," \
  "`P485METER_ACTIVEPOWER_B`," \
  "`P485METER_ACTIVEPOWER_C`," \
  "`P485METER_REACTIVEPOWER_A`," \
  "`P485METER_REACTIVEPOWER_B`," \
  "`P485METER_REACTIVEPOWER_C`," \
  "`P485METER_APPARENTPOWER_A`," \
  "`P485METER_APPARENTPOWER_B`," \
  "`P485METER_APPARENTPOWER_C`," \
  "`P485METER_POWERFACTOR_A`," \
  "`P485METER_POWERFACTOR_B`," \
  "`P485METER_POWERFACTOR_C`," \
  "`P485METER_FREQUENCY`," \
  "`P485METER_UATHD`," \
  "`P485METER_UBTHD`," \
  "`P485METER_UCTHD`," \
  "`P485METER_IATHD`," \
  "`P485METER_IBTHD`," \
  "`P485METER_ICTHD`," \
  "`P485METER_UATH_3`," \
  "`P485METER_UATH_5`," \
  "`P485METER_UATH_7`," \
  "`P485METER_UATH_9`," \
  "`P485METER_UATH_11`," \
  "`P485METER_UATH_13`," \
  "`P485METER_UATH_15`," \
  "`P485METER_UATH_17`," \
  "`P485METER_UBTH_3`," \
  "`P485METER_UBTH_5`," \
  "`P485METER_UBTH_7`," \
  "`P485METER_UBTH_9`," \
  "`P485METER_UBTH_11`," \
  "`P485METER_UBTH_13`," \
  "`P485METER_UBTH_15`," \
  "`P485METER_UBTH_17`," \
  "`P485METER_UCTH_3`," \
  "`P485METER_UCTH_5`," \
  "`P485METER_UCTH_7`," \
  "`P485METER_UCTH_9`," \
  "`P485METER_UCTH_11`," \
  "`P485METER_UCTH_13`," \
  "`P485METER_UCTH_15`," \
  "`P485METER_UCTH_17`," \
  "`P485METER_IATH_3`," \
  "`P485METER_IATH_5`," \
  "`P485METER_IATH_7`," \
  "`P485METER_IATH_9`," \
  "`P485METER_IATH_11`," \
  "`P485METER_IATH_13`," \
  "`P485METER_IATH_15`," \
  "`P485METER_IATH_17`," \
  "`P485METER_IBTH_3`," \
  "`P485METER_IBTH_5`," \
  "`P485METER_IBTH_7`," \
  "`P485METER_IBTH_9`," \
  "`P485METER_IBTH_11`," \
  "`P485METER_IBTH_13`," \
  "`P485METER_IBTH_15`," \
  "`P485METER_IBTH_17`," \
  "`P485METER_ICTH_3`," \
  "`P485METER_ICTH_5`," \
  "`P485METER_ICTH_7`," \
  "`P485METER_ICTH_9`," \
  "`P485METER_ICTH_11`," \
  "`P485METER_ICTH_13`," \
  "`P485METER_ICTH_15`," \
  "`P485METER_ICTH_17`," \
  "`P485METER_VOLTAGE_IMBALANCE`," \
  "`P485METER_ACTIVEENERGY_TOTAL`," \
  "`P485METER_REACTIVEENERGY_TOTAL`," \
  "`RECORD_TS`" \
  "FROM `%s`" \
  "WHERE `ID` = (SELECT MAX(`ID`) FROM `%s`)"

#define P485METER_TABLE_ATTR_ID                                            "ID"
#define P485METER_TABLE_ATTR_P485METER_UA                                          "P485METER_UA"
#define P485METER_TABLE_ATTR_P485METER_UB                                          "P485METER_UB"
#define P485METER_TABLE_ATTR_P485METER_UC                                          "P485METER_UC"
#define P485METER_TABLE_ATTR_P485METER_IA                                          "P485METER_IA"
#define P485METER_TABLE_ATTR_P485METER_IB                                          "P485METER_IB"
#define P485METER_TABLE_ATTR_P485METER_IC                                          "P485METER_IC"
#define P485METER_TABLE_ATTR_P485METER_ACTIVEPOWER_TOTAL                                          "P485METER_ACTIVEPOWER_TOTAL"
#define P485METER_TABLE_ATTR_P485METER_APPARENTPOWER_TOTAL                                          "P485METER_APPARENTPOWER_TOTAL"
#define P485METER_TABLE_ATTR_P485METER_REACTIVEPOWER_TOTAL                                          "P485METER_REACTIVEPOWER_TOTAL"
#define P485METER_TABLE_ATTR_P485METER_POWERFACTOR_TOTAL                                          "P485METER_POWERFACTOR_TOTAL"
#define P485METER_TABLE_ATTR_P485METER_ACTIVEPOWERDEMAND_TOTAL                                          "P485METER_ACTIVEPOWERDEMAND_TOTAL"
#define P485METER_TABLE_ATTR_P485METER_UAB                                          "P485METER_UAB"
#define P485METER_TABLE_ATTR_P485METER_UBC                                          "P485METER_UBC"
#define P485METER_TABLE_ATTR_P485METER_UCA                                          "P485METER_UCA"
#define P485METER_TABLE_ATTR_P485METER_CURRENTIMBALANCE                                          "P485METER_CURRENTIMBALANCE"
#define P485METER_TABLE_ATTR_P485METER_ZEROSEQUENCECURRENT                                          "P485METER_ZEROSEQUENCECURRENT"
#define P485METER_TABLE_ATTR_P485METER_ACTIVEPOWER_A                                          "P485METER_ACTIVEPOWER_A"
#define P485METER_TABLE_ATTR_P485METER_ACTIVEPOWER_B                                          "P485METER_ACTIVEPOWER_B"
#define P485METER_TABLE_ATTR_P485METER_ACTIVEPOWER_C                                          "P485METER_ACTIVEPOWER_C"
#define P485METER_TABLE_ATTR_P485METER_REACTIVEPOWER_A                                          "P485METER_REACTIVEPOWER_A"
#define P485METER_TABLE_ATTR_P485METER_REACTIVEPOWER_B                                          "P485METER_REACTIVEPOWER_B"
#define P485METER_TABLE_ATTR_P485METER_REACTIVEPOWER_C                                          "P485METER_REACTIVEPOWER_C"
#define P485METER_TABLE_ATTR_P485METER_APPARENTPOWER_A                                          "P485METER_APPARENTPOWER_A"
#define P485METER_TABLE_ATTR_P485METER_APPARENTPOWER_B                                          "P485METER_APPARENTPOWER_B"
#define P485METER_TABLE_ATTR_P485METER_APPARENTPOWER_C                                          "P485METER_APPARENTPOWER_C"
#define P485METER_TABLE_ATTR_P485METER_POWERFACTOR_A                                          "P485METER_POWERFACTOR_A"
#define P485METER_TABLE_ATTR_P485METER_POWERFACTOR_B                                          "P485METER_POWERFACTOR_B"
#define P485METER_TABLE_ATTR_P485METER_POWERFACTOR_C                                          "P485METER_POWERFACTOR_C"
#define P485METER_TABLE_ATTR_P485METER_FREQUENCY                                          "P485METER_FREQUENCY"
#define P485METER_TABLE_ATTR_P485METER_UATHD                                          "P485METER_UATHD"
#define P485METER_TABLE_ATTR_P485METER_UBTHD                                          "P485METER_UBTHD"
#define P485METER_TABLE_ATTR_P485METER_UCTHD                                          "P485METER_UCTHD"
#define P485METER_TABLE_ATTR_P485METER_IATHD                                          "P485METER_IATHD"
#define P485METER_TABLE_ATTR_P485METER_IBTHD                                          "P485METER_IBTHD"
#define P485METER_TABLE_ATTR_P485METER_ICTHD                                          "P485METER_ICTHD"
#define P485METER_TABLE_ATTR_P485METER_UATH_3                                          "P485METER_UATH_3"
#define P485METER_TABLE_ATTR_P485METER_UATH_5                                          "P485METER_UATH_5"
#define P485METER_TABLE_ATTR_P485METER_UATH_7                                          "P485METER_UATH_7"
#define P485METER_TABLE_ATTR_P485METER_UATH_9                                          "P485METER_UATH_9"
#define P485METER_TABLE_ATTR_P485METER_UATH_11                                          "P485METER_UATH_11"
#define P485METER_TABLE_ATTR_P485METER_UATH_13                                          "P485METER_UATH_13"
#define P485METER_TABLE_ATTR_P485METER_UATH_15                                          "P485METER_UATH_15"
#define P485METER_TABLE_ATTR_P485METER_UATH_17                                          "P485METER_UATH_17"
#define P485METER_TABLE_ATTR_P485METER_UBTH_3                                          "P485METER_UBTH_3"
#define P485METER_TABLE_ATTR_P485METER_UBTH_5                                          "P485METER_UBTH_5"
#define P485METER_TABLE_ATTR_P485METER_UBTH_7                                          "P485METER_UBTH_7"
#define P485METER_TABLE_ATTR_P485METER_UBTH_9                                          "P485METER_UBTH_9"
#define P485METER_TABLE_ATTR_P485METER_UBTH_11                                          "P485METER_UBTH_11"
#define P485METER_TABLE_ATTR_P485METER_UBTH_13                                          "P485METER_UBTH_13"
#define P485METER_TABLE_ATTR_P485METER_UBTH_15                                          "P485METER_UBTH_15"
#define P485METER_TABLE_ATTR_P485METER_UBTH_17                                          "P485METER_UBTH_17"
#define P485METER_TABLE_ATTR_P485METER_UCTH_3                                          "P485METER_UCTH_3"
#define P485METER_TABLE_ATTR_P485METER_UCTH_5                                          "P485METER_UCTH_5"
#define P485METER_TABLE_ATTR_P485METER_UCTH_7                                          "P485METER_UCTH_7"
#define P485METER_TABLE_ATTR_P485METER_UCTH_9                                          "P485METER_UCTH_9"
#define P485METER_TABLE_ATTR_P485METER_UCTH_11                                          "P485METER_UCTH_11"
#define P485METER_TABLE_ATTR_P485METER_UCTH_13                                          "P485METER_UCTH_13"
#define P485METER_TABLE_ATTR_P485METER_UCTH_15                                          "P485METER_UCTH_15"
#define P485METER_TABLE_ATTR_P485METER_UCTH_17                                          "P485METER_UCTH_17"
#define P485METER_TABLE_ATTR_P485METER_IATH_3                                          "P485METER_IATH_3"
#define P485METER_TABLE_ATTR_P485METER_IATH_5                                          "P485METER_IATH_5"
#define P485METER_TABLE_ATTR_P485METER_IATH_7                                          "P485METER_IATH_7"
#define P485METER_TABLE_ATTR_P485METER_IATH_9                                          "P485METER_IATH_9"
#define P485METER_TABLE_ATTR_P485METER_IATH_11                                          "P485METER_IATH_11"
#define P485METER_TABLE_ATTR_P485METER_IATH_13                                          "P485METER_IATH_13"
#define P485METER_TABLE_ATTR_P485METER_IATH_15                                          "P485METER_IATH_15"
#define P485METER_TABLE_ATTR_P485METER_IATH_17                                          "P485METER_IATH_17"
#define P485METER_TABLE_ATTR_P485METER_IBTH_3                                          "P485METER_IBTH_3"
#define P485METER_TABLE_ATTR_P485METER_IBTH_5                                          "P485METER_IBTH_5"
#define P485METER_TABLE_ATTR_P485METER_IBTH_7                                          "P485METER_IBTH_7"
#define P485METER_TABLE_ATTR_P485METER_IBTH_9                                          "P485METER_IBTH_9"
#define P485METER_TABLE_ATTR_P485METER_IBTH_11                                          "P485METER_IBTH_11"
#define P485METER_TABLE_ATTR_P485METER_IBTH_13                                          "P485METER_IBTH_13"
#define P485METER_TABLE_ATTR_P485METER_IBTH_15                                          "P485METER_IBTH_15"
#define P485METER_TABLE_ATTR_P485METER_IBTH_17                                          "P485METER_IBTH_17"
#define P485METER_TABLE_ATTR_P485METER_ICTH_3                                          "P485METER_ICTH_3"
#define P485METER_TABLE_ATTR_P485METER_ICTH_5                                          "P485METER_ICTH_5"
#define P485METER_TABLE_ATTR_P485METER_ICTH_7                                          "P485METER_ICTH_7"
#define P485METER_TABLE_ATTR_P485METER_ICTH_9                                          "P485METER_ICTH_9"
#define P485METER_TABLE_ATTR_P485METER_ICTH_11                                          "P485METER_ICTH_11"
#define P485METER_TABLE_ATTR_P485METER_ICTH_13                                          "P485METER_ICTH_13"
#define P485METER_TABLE_ATTR_P485METER_ICTH_15                                          "P485METER_ICTH_15"
#define P485METER_TABLE_ATTR_P485METER_ICTH_17                                          "P485METER_ICTH_17"
#define P485METER_TABLE_ATTR_P485METER_VOLTAGE_IMBALANCE                                          "P485METER_VOLTAGE_IMBALANCE"
#define P485METER_TABLE_ATTR_P485METER_ACTIVEENERGY_TOTAL                                          "P485METER_ACTIVEENERGY_TOTAL"
#define P485METER_TABLE_ATTR_P485METER_REACTIVEENERGY_TOTAL                                          "P485METER_REACTIVEENERGY_TOTAL"
#define P485METER_TABLE_ATTR_RECORD_TS                                     "RECORD_TS"

typedef struct P485METER_table
{
    int ID;
    double		P485METER_UA;
    double		P485METER_UB;
    double		P485METER_UC;
    double		P485METER_IA;
    double		P485METER_IB;
    double		P485METER_IC;
    double		P485METER_ACTIVEPOWER_TOTAL;		//总有功功率。
    double		P485METER_APPARENTPOWER_TOTAL;		//总视在功率。
    double		P485METER_REACTIVEPOWER_TOTAL;		//总无功功率。
    double		P485METER_POWERFACTOR_TOTAL;		//功率因数。
    double		P485METER_ACTIVEPOWERDEMAND_TOTAL;		//总有功功率需量
    double		P485METER_UAB;
    double		P485METER_UBC;
    double		P485METER_UCA;
    double		P485METER_CURRENTIMBALANCE;		//电流不平衡
    double		P485METER_ZEROSEQUENCECURRENT;		//零序电流
    double		P485METER_ACTIVEPOWER_A;			//A相有功功率
    double		P485METER_ACTIVEPOWER_B;		//B相有功功率
    double		P485METER_ACTIVEPOWER_C;		//C相有功功率
    double		P485METER_REACTIVEPOWER_A;		//A相无功功率
    double		P485METER_REACTIVEPOWER_B;		//B相无功功率
    double		P485METER_REACTIVEPOWER_C;		//C相无功功率
    double		P485METER_APPARENTPOWER_A;		//A相视在功率
    double		P485METER_APPARENTPOWER_B;		//B相视在功率
    double		P485METER_APPARENTPOWER_C;			//C相视在功率
    double		P485METER_POWERFACTOR_A;		//A相功率因数
    double		P485METER_POWERFACTOR_B;		//B相功率因数
    double		P485METER_POWERFACTOR_C;		//C相功率因数
    double		P485METER_FREQUENCY;		//频率
    double		P485METER_UATHD;
    double		P485METER_UBTHD;
    double		P485METER_UCTHD;
    double		P485METER_IATHD;
    double		P485METER_IBTHD;
    double		P485METER_ICTHD;
    double		P485METER_UATH_3;
    double		P485METER_UATH_5;
    double		P485METER_UATH_7;
    double		P485METER_UATH_9;
    double		P485METER_UATH_11;
    double		P485METER_UATH_13;
    double		P485METER_UATH_15;
    double		P485METER_UATH_17;
    double		P485METER_UBTH_3;
    double		P485METER_UBTH_5;
    double		P485METER_UBTH_7;
    double		P485METER_UBTH_9;
    double		P485METER_UBTH_11;
    double		P485METER_UBTH_13;
    double		P485METER_UBTH_15;
    double		P485METER_UBTH_17;
    double		P485METER_UCTH_3;
    double		P485METER_UCTH_5;
    double		P485METER_UCTH_7;
    double		P485METER_UCTH_9;
    double		P485METER_UCTH_11;
    double		P485METER_UCTH_13;
    double		P485METER_UCTH_15;
    double		P485METER_UCTH_17;
    double		P485METER_IATH_3;
    double		P485METER_IATH_5;
    double		P485METER_IATH_7;
    double		P485METER_IATH_9;
    double		P485METER_IATH_11;
    double		P485METER_IATH_13;
    double		P485METER_IATH_15;
    double		P485METER_IATH_17;
    double		P485METER_IBTH_3;
    double		P485METER_IBTH_5;
    double		P485METER_IBTH_7;
    double		P485METER_IBTH_9;
    double		P485METER_IBTH_11;
    double		P485METER_IBTH_13;
    double		P485METER_IBTH_15;
    double		P485METER_IBTH_17;
    double		P485METER_ICTH_3;
    double		P485METER_ICTH_5;
    double		P485METER_ICTH_7;
    double		P485METER_ICTH_9;
    double		P485METER_ICTH_11;
    double		P485METER_ICTH_13;
    double		P485METER_ICTH_15;
    double		P485METER_ICTH_17;
    double		P485METER_VOLTAGE_IMBALANCE;		//电压不平衡
    double		P485METER_ACTIVEENERGY_TOTAL;		//总有功电能
    double		P485METER_REACTIVEENERGY_TOTAL;		//总无功电能
    sqlite3_int64 RECORD_TS;
}P485METERTable;


#define C_CNS_DEVINFO_TABLE_PREFIX "CNS_TABLE_DEVINFO_"
#define C_CNS_DEVINFO_TABLE_CREATE \
    "CREATE TABLE `%s`                                               \n" \
    "(                                                               \n" \
    "    `ID`               integer NOT NULL PRIMARY KEY AUTOINCREMENT,\n" \
    "    `MODULE_ADDR`      text    default 0,                         \n" \
    "    `MAC`              text    not NULL,                         \n" \
    "    `PRODUCT_TYPE`     integer    default 0,                         \n" \
    "    `DEVICE_TYPE`     integer    default 0,                         \n" \
    "    `ROLE`             integer    default 0,                         \n" \
    "    `TEI`              integer    default 0,                         \n" \
    "    `PROXY_TEI`        integer    default 0,                         \n" \
    "    `SZCHIPCODE`       text    default 0,                         \n" \
    "    `SZMANUCODE`       text    default 0,                         \n" \
    "    `UCBOOTVER`        integer    default 0,                         \n" \
    "    `VRCVERSION`       text    default 0,                         \n" \
    "    `us_year`          integer    default 0,                         \n" \
    "    `us_month`         integer    default 0,                         \n" \
    "    `us_day`           integer    default 0,                         \n" \
    "    `PHASE_STATUS`     integer    default 0,                         \n" \
    "    `PHASE_RESULT`     integer    default 0,                         \n" \
    "    `PHASE_RESULT_B`   integer    default 0,                         \n" \
    "    `PHASE_RESULT_C`   integer    default 0,                         \n" \
    "    `phase_flag`       integer    default 0,                         \n" \
    "    `STATUS`           integer    default 0,                         \n" \
    "    `LOCATION`         text       default NULL,                         \n" \
    "    `PROXY_CHANGE_CNT` integer    default 0,                         \n" \
    "    `LEAVE_CNT`        integer    default 0,                         \n" \
    "    `LEAVE_TOTAL_TIME` integer    default 0,                         \n" \
    "    `JOIN_NET_TIME`    integer    default 0,                         \n" \
    "    `RECORD_TS`        integer    default 0    \n" \
    ");"

#define C_CNS_DEVINFO_TABLE_SELECT \
    "SELECT `ID`, `MODULE_ADDR`, `MAC`, `PRODUCT_TYPE`, `DEVICE_TYPE`, `ROLE`, `TEI`, `PROXY_TEI`, `SZCHIPCODE`, `SZMANUCODE`, `UCBOOTVER`," \
    " `VRCVERSION`, `us_year`, `us_month`, `us_day`, `PHASE_STATUS`, `PHASE_RESULT`, `PHASE_RESULT_B`, `PHASE_RESULT_C`, `phase_flag`, `STATUS`, `LOCATION`, `PROXY_CHANGE_CNT`, `LEAVE_CNT`, `LEAVE_TOTAL_TIME`, `JOIN_NET_TIME`, `RECORD_TS`" \
    " FROM `%s` WHERE `ID` = (SELECT MAX(`ID`) FROM `%s`)"

#define CNS_DEVINFO_TABLE_ATTR_ID                               "ID"
#define CNS_DEVINFO_TABLE_ATTR_MODULE_ADDR                      "MODULE_ADDR"
#define CNS_DEVINFO_TABLE_ATTR_MAC                              "MAC"
#define CNS_DEVINFO_TABLE_ATTR_PRODUCT_TYPE                     "PRODUCT_TYPE"
#define CNS_DEVINFO_TABLE_ATTR_DEVICE_TYPE                     "DEVICE_TYPE"
#define CNS_DEVINFO_TABLE_ATTR_ROLE                             "ROLE"
#define CNS_DEVINFO_TABLE_ATTR_TEI                              "TEI"
#define CNS_DEVINFO_TABLE_ATTR_PROXY_TEI                        "PROXY_TEI"
#define CNS_DEVINFO_TABLE_ATTR_SZCHIPCODE                       "SZCHIPCODE"
#define CNS_DEVINFO_TABLE_ATTR_SZMANUCODE                       "SZMANUCODE"
#define CNS_DEVINFO_TABLE_ATTR_UCBOOTVER                        "UCBOOTVER"
#define CNS_DEVINFO_TABLE_ATTR_VRCVERSION                       "VRCVERSION"
#define CNS_DEVINFO_TABLE_ATTR_us_year                          "us_year"
#define CNS_DEVINFO_TABLE_ATTR_us_month                         "us_month"
#define CNS_DEVINFO_TABLE_ATTR_us_day                           "us_day"
#define CNS_DEVINFO_TABLE_ATTR_PHASE_STATUS                     "PHASE_STATUS"
#define CNS_DEVINFO_TABLE_ATTR_PHASE_RESULT                     "PHASE_RESULT"
#define CNS_DEVINFO_TABLE_ATTR_PHASE_RESULT_B                   "PHASE_RESULT_B"
#define CNS_DEVINFO_TABLE_ATTR_PHASE_RESULT_C                   "PHASE_RESULT_C"
#define CNS_DEVINFO_TABLE_ATTR_phase_flag                       "phase_flag"
#define CNS_DEVINFO_TABLE_ATTR_STATUS                           "STATUS"
#define CNS_DEVINFO_TABLE_ATTR_LOCATION                         "LOCATION"
#define CNS_DEVINFO_TABLE_ATTR_PROXY_CHANGE_CNT                 "PROXY_CHANGE_CNT"
#define CNS_DEVINFO_TABLE_ATTR_LEAVE_CNT                        "LEAVE_CNT"
#define CNS_DEVINFO_TABLE_ATTR_LEAVE_TOTAL_TIME                 "LEAVE_TOTAL_TIME"
#define CNS_DEVINFO_TABLE_ATTR_JOIN_NET_TIME                    "JOIN_NET_TIME"
#define CNS_DEVINFO_TABLE_ATTR_RECORD_TS                        "RECORD_TS"

typedef struct CNS_DEVINFO_table
{
    int ID;
    std::string    MODULE_ADDR;
    std::string     MAC;
    int  PRODUCT_TYPE;
    int  DEVICE_TYPE;
    int  ROLE;
    int  TEI;
    int  PROXY_TEI;
    std::string     SZCHIPCODE;
    std::string     SZMANUCODE;
    int  UCBOOTVER;
    std::string     VRCVERSION;
    int  us_year;
    int  us_month;
    int  us_day;
    int  PHASE_STATUS;
    int  PHASE_RESULT;
    int  PHASE_RESULT_B;
    int  PHASE_RESULT_C;
    int  phase_flag;
    int  STATUS;
    std::string     LOCATION;
    int  PROXY_CHANGE_CNT;
    int  LEAVE_CNT;
    int  LEAVE_TOTAL_TIME;
    int  JOIN_NET_TIME;
    sqlite3_int64 RECORD_TS;
}CNS_DEVINFOTable;

//设备状态
#define C_CNS_DEV_STATUS_TABLE_PREFIX "CNS_TABLE_DEV_STATUS_"
#define C_CNS_DEV_STATUS_TABLE_CREATE \
    "CREATE TABLE `%s`                                               \n" \
    "(                                                               \n" \
    "    `ID`          	  	integer NOT NULL PRIMARY KEY AUTOINCREMENT,\n" \
    "    `MAC`      		text    default 0,                         \n" \
    "    `ROLE`      		integer    default 0,                         \n" \
    "    `STATUS`     		integer    default 0,                         \n" \
    "    `RECORD_TS`     		timestamp    not null default (datetime('now','localtime'))   \n" \
    ");"

#define C_CNS_DEV_STATUS_TABLE_SELECT \
    "SELECT `ID`, `MAC`, `ROLE`, `STATUS`," \
    " `RECORD_TS`" \
    " FROM `%s` WHERE `ID` = (SELECT MAX(`ID`) FROM `%s`)"

#define CNS_DEV_STATUS_TABLE_ATTR_ID                               "ID"
#define CNS_DEV_STATUS_TABLE_ATTR_MAC                              "MAC"
#define CNS_DEV_STATUS_TABLE_ATTR_ROLE                             "ROLE"
#define CNS_DEV_STATUS_TABLE_ATTR_STATUS                           "STATUS"
#define CNS_DEV_STATUS_TABLE_ATTR_RECORD_TS                        "RECORD_TS"

typedef struct CNS_DEV_STATUS_table
{
    int ID;
    std::string     MAC;
    int  ROLE;
    int  STATUS;
    sqlite3_int64 RECORD_TS;
}CNS_DEV_STATUSTable;


//设备状态
#define C_CNS_TOPO_TABLE_PREFIX "CNS_TABLE_TOPO_"
#define C_CNS_TOPO_TABLE_CREATE \
    "CREATE TABLE `%s`                                               \n" \
    "(                                                               \n" \
    "    `ID`           integer NOT NULL PRIMARY KEY AUTOINCREMENT,\n" \
    "    `MAC`          text    default 0,                         \n" \
    "    `ROLE`         integer    default 0,                         \n" \
    "    `LEVEL`        integer    default 0,                         \n" \
    "    `TEI`          integer    default 0,    \n" \
    "    `PROXY_TEI`    integer    default 0,                         \n" \
    "    `RECORD_TS`    integer    default 0    \n" \
    ");"

#define C_CNS_TOPO_TABLE_SELECT \
    "SELECT `ID`, `MAC`, `ROLE`, `LEVEL`, `TEI`, `PROXY_TEI`," \
    " `RECORD_TS`" \
    " FROM `%s` WHERE `ID` = (SELECT MAX(`ID`) FROM `%s`)"

#define CNS_TOPO_TABLE_ATTR_ID                               "ID"
#define CNS_TOPO_TABLE_ATTR_MAC                              "MAC"
#define CNS_TOPO_TABLE_ATTR_ROLE                             "ROLE"
#define CNS_TOPO_TABLE_ATTR_LEVEL                            "LEVEL"
#define CNS_TOPO_TABLE_ATTR_TEI                              "TEI"
#define CNS_TOPO_TABLE_ATTR_PROXY_TEI                        "PROXY_TEI"
#define CNS_TOPO_TABLE_ATTR_RECORD_TS                        "RECORD_TS"

typedef struct CNS_TOPO_table
{
    int ID;
    std::string     MAC;
    int  ROLE;
    int  LEVEL;
    int  TEI;
    int  PROXY_TEI;
    sqlite3_int64 RECORD_TS;
}CNS_TOPOTable;


//衰耗 信噪比
#define C_CNS_SNR_PAR_TABLE_PREFIX "CNS_TABLE_SNR_PAR_"
#define C_CNS_SNR_PAR_TABLE_CREATE \
    "CREATE TABLE `%s`                                               \n" \
    "(                                                               \n" \
    "    `ID`          	  	integer NOT NULL PRIMARY KEY AUTOINCREMENT,\n" \
    "    `MAC`      		text    default 0,                         \n" \
    "    `SNR`      		integer    default 0,                         \n" \
    "    `PAR`     			integer    default 0,                         \n" \
    "    `RECORD_TS`     		timestamp    not null default (datetime('now','localtime'))   \n" \
    ");"

#define C_CNS_SNR_PAR_TABLE_TABLE_SELECT \
    "SELECT `ID`, `MAC`, `SNR`, `PAR`," \
    " `RECORD_TS`" \
    " FROM `%s` WHERE `ID` = (SELECT MAX(`ID`) FROM `%s`)"

#define CNS_CNS_SNR_PAR_TABLE_ATTR_ID                               "ID"
#define CNS_CNS_SNR_PAR_TABLE_ATTR_MAC                               "MAC"
#define CNS_CNS_SNR_PAR_TABLE_ATTR_SNR                               "SNR"
#define CNS_CNS_SNR_PAR_TABLE_ATTR_PAR                               "PAR"
#define CNS_CNS_SNR_PAR_TABLE_ATTR_RECORD_TS                        "RECORD_TS"

typedef struct CNS_CNS_SNR_PAR_table
{
    int ID;
    std::string     MAC;
    int  SNR;
    int  PAR;
    sqlite3_int64 RECORD_TS;
}CNS_CNS_SNR_PARTable;


//上下行成功率
#define C_CNS_COMM_RATE_TABLE_NAME "CNS_TABLE_COMM_RATE"
#define C_CNS_COMM_RATE_TABLE_CREATE \
    "CREATE TABLE `CNS_TABLE_COMM_RATE`                                               \n" \
    "(                                                               \n" \
    "    `ID`          	  	integer NOT NULL PRIMARY KEY AUTOINCREMENT,\n" \
    "    `DOWN_COMM_RATE`   integer    default 0,                         \n" \
    "    `UP_COMM_RATE`     integer    default 0,                         \n" \
    "    `COMM_RATE`     	integer    default 0,                         \n" \
    "    `RECORD_TS`     		timestamp    not null default (datetime('now','localtime'))   \n" \
    ");"



// mac表，根据该表的mac以及对应的其他属性，判断该mac对应的其他业务表是否存在
#define C_CNS_MAC_TABLE_NAME "CNS_TABLE_MAC"
#define C_CNS_MAC_TABLE_CREATE \
    "CREATE TABLE `CNS_TABLE_MAC`                                               \n" \
    "(                                                               \n" \
    "    `ID`          		integer NOT NULL PRIMARY KEY AUTOINCREMENT,\n" \
    "    `MAC`      		text    not NULL,                         \n" \
    "    `DEV_INFO_EXIST` 	integer    default -1,                         \n" \
    "    `SNR_PAR_EXIST`   integer default -1,                        \n" \
    "    `DEV_STATUS_EXIST`     integer    default -1,                         \n" \
    "    `RECORD_TS`     		timestamp    not null default (datetime('now','localtime'))   \n" \
    ");"



// tvoc
#define C_TVOC_TABLE_PREFIX "TVOC_TABLE_"

#define C_TVOC_TABLE_SELECT \
    "SELECT `ID`, `TVOC`," \
    " `RECORD_TS`" \
    " FROM `%s` WHERE `ID` = (SELECT MAX(`ID`) FROM `%s`)"

#define C_TVOC_TABLE_ATTR_TVOC                               "TVOC"

typedef struct TVOC_table
{
    int ID;
    int     TVOC;
    sqlite3_int64 RECORD_TS;
}TVOC_table;


// leak
#define C_SENSORLEAK_TABLE_PREFIX "SENSORLEAK_TABLE_"

#define C_SENSORLEAK_TABLE_SELECT \
    "SELECT `ID`, `LEAKSTATE`," \
    " `RECORD_TS`" \
    " FROM `%s` WHERE `ID` = (SELECT MAX(`ID`) FROM `%s`)"

#define C_SENSORLEAK_TABLE_ATTR_LEAKSTATE                               "LEAKSTATE"

typedef struct SENSORLEAK_table
{
    int ID;
    int     LEAKSTATE;
    sqlite3_int64 RECORD_TS;
}SENSORLEAK_table;


// leak
#define C_NOISE_TABLE_PREFIX "NOISE_TABLE_"

#define C_NOISE_TABLE_SELECT \
    "SELECT `ID`, `NOISE`," \
    " `RECORD_TS`" \
    " FROM `%s` WHERE `ID` = (SELECT MAX(`ID`) FROM `%s`)"

#define C_NOISE_TABLE_ATTR_NOISE                               "NOISE"

typedef struct NOISE_table
{
    int ID;
    double     NOISE;
    sqlite3_int64 RECORD_TS;
}NOISE_table;

// loss rate
#define C_CNS_TABLE_FRAME_LOSS_RATE_PREFIX "CNS_TABLE_FRAME_LOSS_RATE_"


// DAILY_TABLE
#define C_DAILY_TABLE_PREFIX "DAILY_TABLE_"

#define C_DAILY_TABLE_SELECT \
    "SELECT `ID`, `PHASE_INFO`, `POWER_METER_ENERGE`," \
    " `RECORD_TS`" \
    " FROM `%s` WHERE `ID` = (SELECT MAX(`ID`) FROM `%s`)"

#define C_DAILY_TABLE_ATTR_PHASE_INFO                               "PHASE_INFO"
#define C_DAILY_TABLE_ATTR_POWER_METER_ENERGE                               "POWER_METER_ENERGE"

typedef struct DAILY_Table
{
    int ID;
    int PHASE_INFO;
    double POWER_METER_ENERGE;
    sqlite3_int64 RECORD_TS;
}DAILY_Table;


// ONTIME_TABLE
#define C_ONTIME_TABLE_PREFIX "ONTIME_TABLE_"

// CREATE TABLE `ONTIME_TABLE_03001SF00000221700434770`
//(
//	 `ID`         					integer NOT NULL PRIMARY KEY AUTOINCREMENT,
//    `PHASE_INFO`           		integer    default 0,
//    `ONTIME_POSITIVE_ENERGEY`      real    default 0,
//    `ONTIME_REACTIVE_ENERGEY`      real    default 0,
//    `ONTIME_FREEZEN`               text    default 0,
//    `RECORD_TS`            		integer default 0
//);

#define C_ONTIME_TABLE_SELECT \
    "SELECT `ID`, `PHASE_INFO`, `ONTIME_POSITIVE_ENERGEY`, `ONTIME_REACTIVE_ENERGEY`, `ONTIME_FREEZEN`," \
    " `RECORD_TS`" \
    " FROM `%s` WHERE `ID` = (SELECT MAX(`ID`) FROM `%s`)"

#define C_ONTIME_TABLE_ATTR_PHASE_INFO                               "PHASE_INFO"
#define C_ONTIME_TABLE_ATTR_ONTIME_POSITIVE_ENERGEY                  "ONTIME_POSITIVE_ENERGEY"
#define C_ONTIME_TABLE_ATTR_ONTIME_REACTIVE_ENERGEY                  "ONTIME_REACTIVE_ENERGEY"
#define C_ONTIME_TABLE_ATTR_ONTIME_FREEZEN                           "ONTIME_FREEZEN"

typedef struct ONTIME_Table
{
    int ID;
    int PHASE_INFO;
    double ONTIME_POSITIVE_ENERGEY;
    double ONTIME_REACTIVE_ENERGEY;
    std::string ONTIME_FREEZEN;
    sqlite3_int64 RECORD_TS;
}ONTIME_Table;




// leak
#define C_O2_TABLE_PREFIX "O2_TABLE_"

#define C_O2_TABLE_SELECT \
    "SELECT `ID`, `O2`," \
    " `RECORD_TS`" \
    " FROM `%s` WHERE `ID` = (SELECT MAX(`ID`) FROM `%s`)"

#define C_O2_TABLE_ATTR_O2                               "O2"

typedef struct O2_table
{
    int ID;
    double     O2;
    sqlite3_int64 RECORD_TS;
}O2_table;



// leak
#define C_O3_TABLE_PREFIX "O3_TABLE_"

#define C_O3_TABLE_SELECT \
    "SELECT `ID`, `O3`," \
    " `RECORD_TS`" \
    " FROM `%s` WHERE `ID` = (SELECT MAX(`ID`) FROM `%s`)"

#define C_O3_TABLE_ATTR_O3                               "O3"

typedef struct O3_table
{
    int ID;
    double     O3;
    sqlite3_int64 RECORD_TS;
}O3_table;



// leak
#define C_SF6_TABLE_PREFIX "SF6_TABLE_"

#define C_SF6_TABLE_SELECT \
    "SELECT `ID`, `SF6`," \
    " `RECORD_TS`" \
    " FROM `%s` WHERE `ID` = (SELECT MAX(`ID`) FROM `%s`)"

#define C_SF6_TABLE_ATTR_SF6                               "SF6"

typedef struct SF6_table
{
    int ID;
    double     SF6;
    sqlite3_int64 RECORD_TS;
}SF6_table;



// leak
#define C_QX_TABLE_PREFIX "QX_TABLE_"

#define C_QX_TABLE_SELECT \
    "SELECT `ID`, `TEMPERATURE`, `HUMIDITY`, `NAP`, `WIND_SPEED`, `WIND_DIRECTION`, `RAINFULL`," \
    " `RECORD_TS`" \
    " FROM `%s` WHERE `ID` = (SELECT MAX(`ID`) FROM `%s`)"

#define C_QX_TABLE_ATTR_TEMPERATURE                               "TEMPERATURE"
#define C_QX_TABLE_ATTR_HUMIDITY                               "HUMIDITY"
#define C_QX_TABLE_ATTR_NAP                               "NAP"
#define C_QX_TABLE_ATTR_WIND_SPEED                               "WIND_SPEED"
#define C_QX_TABLE_ATTR_WIND_DIRECTION                               "WIND_DIRECTION"
#define C_QX_TABLE_ATTR_RAINFULL                               "RAINFULL"

typedef struct QX_table
{
    int ID;
    double     TEMPERATURE;
    double     HUMIDITY;
    double     NAP;
    double     WIND_SPEED;
    double     WIND_DIRECTION;
    double     RAINFULL;
    sqlite3_int64 RECORD_TS;
}QX_table;



// leak
#define C_OIL_TABLE_PREFIX "OIL_TABLE_"

#define C_OIL_TABLE_SELECT \
    "SELECT `ID`, `R`, `G`, `B`, `state`," \
    " `RECORD_TS`" \
    " FROM `%s` WHERE `ID` = (SELECT MAX(`ID`) FROM `%s`)"

#define C_OIL_TABLE_ATTR_R                               "R"
#define C_OIL_TABLE_ATTR_G                               "G"
#define C_OIL_TABLE_ATTR_B                               "B"
#define C_OIL_TABLE_ATTR_state                               "state"

typedef struct OIL_table
{
    int ID;
    int     R;
    int     G;
    int     B;
    int     state;
    sqlite3_int64 RECORD_TS;
}OIL_table;



#define C_SENSORGUZHI_TABLE_PREFIX "SENSORGUZHI_TABLE"
//
#define C_SENSORGUZHI_TABLE_SELECT \
		"SELECT `ID`, `GUZHI_PHASE_A_VOLT`, `GUZHI_PHASE_B_VOLT`, `GUZHI_PHASE_C_VOLT`," \
		" `GUZHI_PHASE_A_ELEC`, `GUZHI_PHASE_B_ELEC`, `GUZHI_PHASE_C_ELEC`," \
		" `RECORD_TS`" \
		" FROM `%s`" \
		" WHERE `ID` = (SELECT MAX(`ID`) FROM `%s`)"
//
#define DEVICE_TABLE_ATTR_SENSORGUZHI_PHASE_A_VOLT "GUZHI_PHASE_A_VOLT"
#define DEVICE_TABLE_ATTR_SENSORGUZHI_PHASE_B_VOLT "GUZHI_PHASE_B_VOLT"
#define DEVICE_TABLE_ATTR_SENSORGUZHI_PHASE_C_VOLT "GUZHI_PHASE_C_VOLT"
#define DEVICE_TABLE_ATTR_SENSORGUZHI_PHASE_A_ELEC "GUZHI_PHASE_A_ELEC"
#define DEVICE_TABLE_ATTR_SENSORGUZHI_PHASE_B_ELEC "GUZHI_PHASE_B_ELEC"
#define DEVICE_TABLE_ATTR_SENSORGUZHI_PHASE_C_ELEC "GUZHI_PHASE_C_ELEC"
//
typedef struct SENSORGUZHI_table
{
    int ID;
    double GUZHI_PHASE_A_VOLT;
    double GUZHI_PHASE_B_VOLT;
    double GUZHI_PHASE_C_VOLT;
    double GUZHI_PHASE_A_ELEC;
    double GUZHI_PHASE_B_ELEC;
    double GUZHI_PHASE_C_ELEC;
    sqlite3_int64 RECORD_TS;
} SENSORGUZHI_table;



// leak
#define C_RFID_TABLE_PREFIX "RFID_TABLE_"

#define C_RFID_TABLE_SELECT \
    "SELECT `ID`, `RFID_0`, `RFID_1`, `RFID_2`, `RFID_3`, `RFID_4`, `RFID_5`, `RFID_6`, `RFID_7`, `RFID_8`," \
    " `RECORD_TS`" \
    " FROM `%s` WHERE `ID` = (SELECT MAX(`ID`) FROM `%s`)"

#define C_RFID_TABLE_ATTR_RFID_0                               "RFID_0"
#define C_RFID_TABLE_ATTR_RFID_1                               "RFID_1"
#define C_RFID_TABLE_ATTR_RFID_2                               "RFID_2"
#define C_RFID_TABLE_ATTR_RFID_3                               "RFID_3"
#define C_RFID_TABLE_ATTR_RFID_4                               "RFID_4"
#define C_RFID_TABLE_ATTR_RFID_5                               "RFID_5"
#define C_RFID_TABLE_ATTR_RFID_6                               "RFID_6"
#define C_RFID_TABLE_ATTR_RFID_7                               "RFID_7"
#define C_RFID_TABLE_ATTR_RFID_8                               "RFID_8"

typedef struct RFID_table
{
    int ID;
    double      RFID_0;
    double      RFID_1;
    double      RFID_2;
    double      RFID_3;
    double      RFID_4;
    double      RFID_5;
    double      RFID_6;
    double      RFID_7;
    double      RFID_8;
    sqlite3_int64 RECORD_TS;
}RFID_table;



#endif //DEVICE_DDL_H


