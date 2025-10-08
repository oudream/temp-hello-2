#ifndef DB_MANAGER_H
#define DB_MANAGER_H

#include <ccxx/cxglobal.h>

#include <ccxx/cxsqlite3template.hpp>

#define DB_FILEPATH_FORMAT "/userdata/gwdb/%s.db"
#define DB_NAME_GW_SYS "gw_sys"
#define DB_NAME_GW_IOT "gw_iot"

class DBManager
{
public:
    static void setDbFilePathFormat(const std::string &v);

    /**
     * open with ( r )
     * @param sFilePath
     * @return
     */
    static sqlite3 * openSqlite(const std::string &sFilePath);

    static void closeSqlite(sqlite3 *db);

    static void closeAllSqlite();

    static void resetOpenSqlite(const msepoch_t &dtNow);

    /**
     * open with ( r w )
     * @param sFilePath
     * @return
     */
    static sqlite3 * open_sqlite(const std::string &sFilePath);

    static void close_sqlite(sqlite3 *db);

    /**
     * get db file path by DB_FILEPATH_FORMAT
     * @param dbName
     * @return
     */
    static std::string getDbFilePath(const std::string &dbName);

    static bool sqliteTableExist(sqlite3 *db, const std::string &tbName);
    
    static bool sqliteTableColumnExist(sqlite3 *db, const std::string &tbName, const std::string &fdName);

    static int sqlite3Exec(sqlite3 *db, std::string sSql);

    static int getLastID(const std::string &sFilePath, const std::string &tbName);

};

#endif //DB_MANAGER_H
