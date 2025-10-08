#include "db_manager.h"
#include <ccxx/cxinterinfo_group_define.h>
#include <ccxx/cxinterinfo.h>


using namespace std;


CXOUTINFO_DEFINE(IIG_NORMAL, IIG_NORMAL, "DB-MANAGER")


static string f_db_filepath_format;
static map<string, sqlite3 *> f_dbs;
static msepoch_t f_dbsResetTime = 0;


/**
 * SQLITE3 运行SQL，读取一个值（此值必须是 int 类型），并返回
 * 使用： select count(*) from sqlite_master
 * @param db
 * @param sql
 * @return
 */
static int _sqlite3_query_int(sqlite3 * db, char * sql)
{
    assert(db);
    assert(sql);

    int rc;
    sqlite3_stmt *stmt;
    const char*tail;
    rc=sqlite3_prepare(db,sql,(int)strlen(sql),&stmt,&tail);
    if(rc!=SQLITE_OK){
        fnError().out("._sqlite3_query_int - %s",sqlite3_errmsg(db));
        return -1;
    }
    rc=sqlite3_step(stmt);
    int r = (rc==SQLITE_ROW) ? sqlite3_column_int(stmt, 0) : -9;
    sqlite3_finalize(stmt);
    return r;
}

/**
 * SQLITE3 运行SQL，返回影响的行数；注意（DDL，如创建表）成功执行也只会返回0
 * @param db
 * @param sql
 * @return
 */
static int _sqlite3_exec_sql(sqlite3 * db, char * sql)
{
    int rc;
    char *zErr;
    int result = sqlite3_total_changes(db);
    rc = sqlite3_exec(db, sql, NULL, NULL, &zErr);
    if(rc != SQLITE_OK) {
        if (zErr != NULL) {
            fnError().out("._sqlite3_exec_sql - %s", zErr);
            sqlite3_free(zErr);
        }
        return -1;
    }
    return sqlite3_total_changes(db) - result;
}

/**
 * 判断表是否存在，大于0为存在
 * select count(*) from sqlite_master where type='table' and name='t1';
 * @param db
 * @param table
 * @return int
 */
static int _sqlite3_table_exist(sqlite3 * db, const char * table)
{
    assert(table);
    char sSql[1024];
    sprintf (sSql, "select count(*) from sqlite_master where type='table' and name='%s'", table);
    return _sqlite3_query_int(db, sSql);
}

/**
 * 判断表列是否存在，大于0为存在
 * select count(*) from sqlite_master where type='table' and name='t1';
 * @param db
 * @param table
 * @return int
 */
static int _sqlite3_table_field_exist(sqlite3 * db, const char *table, const char *field)
{
    assert(table);
    char sSql[256];
    sprintf (sSql, "PRAGMA  table_info([%s]);", table);
    //
    int rc,count;
    //
    sqlite3_stmt *stmt;
    const char*tail;
    rc=sqlite3_prepare(db,sSql,(int)strlen(sSql),&stmt,&tail);
    if(rc!=SQLITE_OK){
        fnError().out("._sqlite3_table_field_exist - %s", sqlite3_errmsg(db));
        return -1;
    }
    rc=sqlite3_step(stmt);
    count=sqlite3_column_count(stmt);
    if (count < 2)
    {
        return -2;
    }
    int r = 0;
    while(rc==SQLITE_ROW){
        const char *foo = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 1));
        if (strncmp(foo, field, max(strlen(foo), strlen(field))) == 0)
        {
            r = 1;
            break;
        }
        rc=sqlite3_step(stmt);
    }
    sqlite3_finalize(stmt);
    return r;
}

void DBManager::setDbFilePathFormat(const string &v)
{
    f_db_filepath_format = v;
}

sqlite3 * DBManager::openSqlite(const std::string &sFilePath)
{
    fnDebug().out(".openSqlite - The open database is %s", sFilePath.c_str());
    map<string, sqlite3 *>::const_iterator it = f_dbs.find(sFilePath);
    if (it != f_dbs.end()) {
        sqlite3 *db = it->second;
        return db;
    } else {
        int rc;
        sqlite3 * db;
//        rc=sqlite3_open(sFilePath.c_str(),&db);
        rc=sqlite3_open_v2(sFilePath.c_str(),&db, SQLITE_OPEN_READONLY, nullptr);
        if(rc)
        {
            fnError().out(".open - Can't open database: %s", sqlite3_errmsg(db));
            sqlite3_close(db);
            db = NULL;
        }
        else
        {
            f_dbs[sFilePath] = db;
        }
        return db;
    }
}

void DBManager::closeSqlite(sqlite3 *db)
{
    //    sqlite3_close(db);
}

void DBManager::closeAllSqlite()
{
    map<string, sqlite3 *>::const_iterator it = f_dbs.begin();
    for (; it != f_dbs.end() ; ++it)
    {
        sqlite3_close(it->second);
    }
    f_dbs.clear();
}

void DBManager::resetOpenSqlite(const msepoch_t &dtNow)
{
    if (f_dbsResetTime == 0)
    {
        f_dbsResetTime = dtNow;
        return;
    }
    if (dtNow - f_dbsResetTime > 24 * 60 * 60 * 1000)
    {
        int iCount = f_dbs.size();
        closeAllSqlite();
        f_dbsResetTime = dtNow;
        fnDebug().out(".resetOpenSqlite - [ %d ] db is close!", iCount);
    }
}

sqlite3 * DBManager::open_sqlite(const std::string &sFilePath)
{
    int rc;
    sqlite3 * db;
    rc=sqlite3_open(sFilePath.c_str(),&db);
    if(rc)
    {
        fnError().out(".open_sqlite - Can't open database: %s", sqlite3_errmsg(db));
        sqlite3_close(db);
        db = NULL;
    }
    return db;
}

void DBManager::close_sqlite(sqlite3 *db)
{
    sqlite3_close(db);
}

std::string DBManager::getDbFilePath(const string &dbName)
{
    if (f_db_filepath_format.empty())
    {
        f_db_filepath_format = string(DB_FILEPATH_FORMAT);
    }
    char sFileName[256];
    sprintf(sFileName, f_db_filepath_format.c_str(), dbName.c_str());
    return string(sFileName);
}

int DBManager::sqlite3Exec(sqlite3 *db, std::string sSql)
{
    return sqlite3_exec(db, sSql.c_str(), nullptr, nullptr, nullptr);
}

bool DBManager::sqliteTableExist(sqlite3 *db, const string &tbName)
{
    return _sqlite3_table_exist(db, tbName.c_str()) > 0;
}

bool DBManager::sqliteTableColumnExist(sqlite3 *db, const string &tbName, const string &fdName)
{
    return _sqlite3_table_field_exist(db, tbName.c_str(), fdName.c_str()) > 0;
}

int DBManager::getLastID(const string &sFilePath, const string &tbName)
{
    if (sFilePath.empty() || tbName.empty())
    {
        return -1;
    }
    sqlite3 *sqliteDb = DBManager::openSqlite(sFilePath);
    char sSql[1024];
    sprintf (sSql, "SELECT `ID` FROM %s WHERE `ID` = (SELECT MAX(`ID`) FROM `%s`)", tbName.c_str(), tbName.c_str());
    int r = -1;
    try
    {
        r = _sqlite3_query_int(sqliteDb, sSql);
    }
    catch (...)
    {
    }
    return r;
}
