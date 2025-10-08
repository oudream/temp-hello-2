#ifndef APP_CONST_H
#define APP_CONST_H

#include <string>
#include <map>

#include "db_manager.h"

#define CS_APP_CONST_TABLE_CREATE \
    "CREATE TABLE `%s` (" \
    "`CONST_ID` INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT," \
    "`CONST_KEY` TEXT DEFAULT ''," \
    "`CONST_VALUE` TEXT DEFAULT ''" \
    ");"

#define CS_APP_CONST_TABLE_SELECT \
    "SELECT `CONST_ID`, `CONST_KEY`, `CONST_VALUE` FROM `%s`;"

#define CS_APP_CONST_TABLE_UPDATE \
    "UPDATE `%s` SET `CONST_VALUE` = ? WHERE `CONST_KEY` = ?;"

class AppConst
{
public:
    static int loadConsts(sqlite3 * sqlite, const std::string &sConstTable = "CONST_TABLE");

    static const std::map<std::string, std::string>& getConsts();

    //sConstumentName: std::string("name1")
    static bool isExistConstKey(const std::string &sKey);

    static std::string getConstValue(const std::string &sKey);

    static bool updateConstValue(sqlite3 * sqlite, const std::string &sKey, const std::string &sValue);

};


#endif //APP_CONST_H
