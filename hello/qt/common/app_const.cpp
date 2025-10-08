#include "app_const.h"

#include "db_manager.h"

#include <ccxx/cxstring.h>

using namespace std;

static string f_const_table_name;
static map<string, string> f_consts;

const std::map<std::string, std::string> &AppConst::getConsts()
{
    return f_consts;
}

std::string AppConst::getConstValue(const std::string &sKey)
{
    map<string, string>::const_iterator it = f_consts.find(sKey);
    if (it != f_consts.end())
        return it->second;
    else
        return std::string();
}

bool AppConst::isExistConstKey(const std::string &sKey)
{
    map<string, string>::const_iterator it = f_consts.find(sKey);
    return it != f_consts.end();
}

int AppConst::loadConsts(sqlite3 *sqlite, const std::string &sConstTable)
{
    f_const_table_name = sConstTable;
    SQL::Con db(sqlite);
    {
        string sSql;
        if (! DBManager::sqliteTableExist(sqlite, sConstTable))
        {
            return 0;
            sSql = CxString::format(CS_APP_CONST_TABLE_CREATE, sConstTable.c_str());
            if (! db.exec_sql(sSql))
            {
                printf("Create Table[%s] ERROR.", sConstTable.c_str());
                return -1;
            }
        }
        sSql = CxString::format(CS_APP_CONST_TABLE_SELECT, sConstTable.c_str());
        static map<string, string> consts;
        {
            auto a = db.bindnquery<int, string, string>(sSql);
            for (auto x:a)
            {
                string sKey = get<1>(x);
                string sValue = get<2>(x);
                consts[sKey] = sValue;
            }
        }
        f_consts = consts;
    }
    return f_consts.size();
}

bool AppConst::updateConstValue(sqlite3 * sqlite, const string &sKey, const string &sValue)
{
    SQL::Con db(sqlite);
    string sSql = CxString::format(CS_APP_CONST_TABLE_UPDATE, f_const_table_name.c_str());
    tuple<string, string> row;
    int r = db.bind1exec(sSql.c_str(), row);
    f_consts[sKey] = sValue;
    return r > 0;
}
