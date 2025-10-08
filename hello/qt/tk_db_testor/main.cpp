#include <ccxx/cxstring.h>
#include <ccxx/cxtime.h>
#include <ccxx/cxfile.h>
#include <ccxx/cxthread.h>
#include <ccxx/cxlog.h>

#include <common/db_manager.h>
#include <common/point_manager.h>
#include <common/device_manager.h>


using namespace std;


CXOUTINFO_DEFINE(IIG_NORMAL, IIG_NORMAL, "MAIN")

#define APP_VERSION "1.0.0"
#define FN_OFF "off"


#define C_ON_TIME_TABLE_PREFIX  "ONTIME_TABLE"
#define C_ON_TIME_TABLE_INSERT \
		"INSERT INTO `%s`(`PHASE_INFO`, `ONTIME_POSITIVE_ENERGEY`," \
		" `ONTIME_REACTIVE_ENERGEY`, `ONTIME_FREEZEN`) "\
		" VALUES" \
		" (?, ?, ?, ?);"

static bool running = true;

static string f_bfnMeterInsert = FN_OFF;
static string f_bfnMeterLast = FN_OFF;

typedef struct ontime_table
{
    int phase_info;
    double ontime_positive_energey;
    double ontime_reactive_energey;
    std::string ontime_freezen;
}ONTIMETable;

static void sigint_handler(int signalId)
{
    running = false;
}

static void print_usage(const char *filename)
{
    printf("Usage: %s [--db-name] [--db-path-format]\n",
           filename);
}

static void print_help(const char *filename)
{
    printf("TK Gateway DB(sqlite) Test Tool.\n");
    printf(
            "--db-name: DB Name [appMeterPlcMail|...]\n"
            "--db-path-format: DB Path Format\n"
            "--fn-meter-insert: Function Table Meter Insert\n"
            "--fn-meter-last: Function Table Meter, Select Last Time\n"
            "\n");
}

const char *filename_remove_path(const char *filename_in)
{
    const char *filename_out = (char *) filename_in;

    /* allow the device ID to be set */
    if (filename_in)
    {
        filename_out = strrchr(filename_in, '\\');
        if (!filename_out)
        {
            filename_out = strrchr(filename_in, '/');
        }
        /* go beyond the slash */
        if (filename_out)
        {
            filename_out++;
        }
        else
        {
            /* no slash in filename */
            filename_out = (char *) filename_in;
        }
    }

    return filename_out;
}

static vector<string> fn_sqlite3_select_dbname(sqlite3 * db, const char *sLike)
{
    assert(sLike);

    vector<string> r;
    int rc,i,ncols;

    sqlite3_stmt *stmt;
    const char*tail;
    char sql[2048];
    cxDebugCheck(sprintf(sql, "SELECT `tbl_name` FROM `sqlite_master` WHERE `type`='table' AND `tbl_name` LIKE '%s%%';", sLike)>0, return r);

    rc=sqlite3_prepare(db,sql,(int)strlen(sql),&stmt,&tail);
    if(rc!=SQLITE_OK){
        fprintf(stderr,"SQLerror:%sn",sqlite3_errmsg(db));
    }
    while (sqlite3_step(stmt) == SQLITE_ROW)
    {
        const char *foo = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 0));
        string sTableName = foo ? std::string(foo) : "";
        r.push_back(sTableName);
    }
    sqlite3_finalize(stmt);
    return r;
}

static int fn_sqlite3_insert_meter(sqlite3 * db, const char *sTableName, METERTable * meter)
{
    assert(sTableName);
    assert(meter);

    sqlite3_stmt *stmt;
    const char *tail;
    char sql[2048];
    cxDebugCheck(sprintf(sql, C_METER_TABLE_INSERT, sTableName)>0, return 0);

    int iTotalChange = sqlite3_total_changes(db);

    int rc = sqlite3_prepare(db, sql, (int)strlen(sql), &stmt, &tail);
    if (rc != SQLITE_OK)
    {
        fprintf(stderr, "SQLerror:%s",sqlite3_errmsg(db));
        return -1;
    }
    rc = sqlite3_reset(stmt);
    if (rc != SQLITE_OK)
    {
        fprintf(stderr, "SQLerror:%s",sqlite3_errmsg(db));
        sqlite3_finalize(stmt);
        return -2;
    }
    sqlite3_bind_double(stmt, 1 , meter->POWER_METER_CPU_OC);
    sqlite3_bind_double(stmt, 2 , meter->POWER_METER_ENERGE);
    sqlite3_bind_double(stmt, 3 , meter->POWER_METER_FORTH_HAVE_POWER_TOTAL);
    sqlite3_bind_double(stmt, 4 , meter->POWER_METER_BACK_HAVE_POWER_TOTAL);
    sqlite3_bind_double(stmt, 5 , meter->POWER_METER_PHASE_A_FORTH_HAVE_POWER);
    sqlite3_bind_double(stmt, 6 , meter->POWER_METER_PHASE_A_BACK_HAVE_POWER);
    sqlite3_bind_double(stmt, 7 , meter->POWER_METER_PHASE_B_FORTH_HAVE_POWER);
    sqlite3_bind_double(stmt, 8 , meter->POWER_METER_PHASE_B_BACK_HAVE_POWER);
    sqlite3_bind_double(stmt, 9 , meter->POWER_METER_PHASE_C_FORTH_HAVE_POWER);
    sqlite3_bind_double(stmt, 10, meter->POWER_METER_PHASE_C_BACK_HAVE_POWER);
    sqlite3_bind_double(stmt, 11, meter->POWER_METER_PHASE_A_VOLT);
    sqlite3_bind_double(stmt, 12, meter->POWER_METER_PHASE_B_VOLT);
    sqlite3_bind_double(stmt, 13, meter->POWER_METER_PHASE_C_VOLT);
    sqlite3_bind_double(stmt, 14, meter->POWER_METER_PHASE_A_ELEC);
    sqlite3_bind_double(stmt, 15, meter->POWER_METER_PHASE_B_ELEC);
    sqlite3_bind_double(stmt, 16, meter->POWER_METER_PHASE_C_ELEC);
    sqlite3_bind_double(stmt, 17, meter->POWER_METER_PHASE_A_ENERGE);
    sqlite3_bind_double(stmt, 18, meter->POWER_METER_PHASE_B_ENERGE);
    sqlite3_bind_double(stmt, 19, meter->POWER_METER_PHASE_C_ENERGE);
    sqlite3_bind_double(stmt, 20, meter->POWER_METER_POWER_FATOR_A);
    sqlite3_bind_double(stmt, 21, meter->POWER_METER_POWER_FATOR_B);
    sqlite3_bind_double(stmt, 22, meter->POWER_METER_POWER_FATOR_C);
    sqlite3_bind_double(stmt, 23, meter->POWER_METER_POWER_FATOR);
    sqlite3_bind_int64 (stmt, 24, meter->POWER_METER_TS);
    rc = sqlite3_step(stmt);
    if (rc != SQLITE_OK && rc != SQLITE_DONE)
    {
        fprintf(stderr, "SQLerror:%s",sqlite3_errmsg(db));
        sqlite3_finalize(stmt);
        return -3;
    }
    sqlite3_finalize(stmt);

    iTotalChange = sqlite3_total_changes(db) - iTotalChange;
    fnInfo() << "Insert " << sTableName << ", Result=" <<  iTotalChange;
    return iTotalChange;
}

static int fn_sqlite3_insert_on_time(sqlite3 * db, const char *sTableName, ONTIMETable* onTime)
{
    assert(sTableName);
    assert(onTime);

    sqlite3_stmt *stmt;
    const char *tail;
    char sql[2048];
    cxDebugCheck(sprintf(sql, C_ON_TIME_TABLE_INSERT, sTableName)>0, return 0);

    int iTotalChange = sqlite3_total_changes(db);

    int rc = sqlite3_prepare(db, sql, (int)strlen(sql), &stmt, &tail);
    if (rc != SQLITE_OK)
    {
        fprintf(stderr, "SQLerror:%s",sqlite3_errmsg(db));
        return -1;
    }
    rc = sqlite3_reset(stmt);
    if (rc != SQLITE_OK)
    {
        fprintf(stderr, "SQLerror:%s",sqlite3_errmsg(db));
        sqlite3_finalize(stmt);
        return -2;
    }

    sqlite3_bind_int(stmt, 1 , onTime->phase_info);
    sqlite3_bind_double(stmt, 2 , onTime->ontime_positive_energey);
    sqlite3_bind_double(stmt, 3 , onTime->ontime_reactive_energey);
    sqlite3_bind_text(stmt, 4 , onTime->ontime_freezen.c_str(), onTime->ontime_freezen.length(),NULL);

    rc = sqlite3_step(stmt);
    if (rc != SQLITE_OK && rc != SQLITE_DONE)
    {
        fprintf(stderr, "SQLerror:%s",sqlite3_errmsg(db));
        sqlite3_finalize(stmt);
        return -3;
    }
    sqlite3_finalize(stmt);

    iTotalChange = sqlite3_total_changes(db) - iTotalChange;
    fnInfo() << "Insert " << sTableName << ", Result=" <<  iTotalChange;
    return iTotalChange;
}

static void fnMeterInsert(sqlite3 *db)
{
    vector<string> sTableNames = fn_sqlite3_select_dbname(db, C_METER_TABLE_PREFIX);
    static double total1 = 28.0;
    static double total2 = 0.01;
    total1 += 28.0 * 0.01;
    total2 += 0.01 * 0.01;
    for (int i = 0; i < sTableNames.size(); ++i)
    {
        const string &sTableName = sTableNames.at(i);
        METERTable meterTable;
        meterTable.POWER_METER_CPU_OC = 30;
        meterTable.POWER_METER_ENERGE = 1;
        meterTable.POWER_METER_FORTH_HAVE_POWER_TOTAL = total1;          // 总有功
        meterTable.POWER_METER_BACK_HAVE_POWER_TOTAL = total2;           // 总无功
        meterTable.POWER_METER_PHASE_A_FORTH_HAVE_POWER = total1 / 3 - 1;        // A相有功
        meterTable.POWER_METER_PHASE_A_BACK_HAVE_POWER = total2 / 3 - 0.01;         // A相无功
        meterTable.POWER_METER_PHASE_B_FORTH_HAVE_POWER = total1 / 3;        // B相有功
        meterTable.POWER_METER_PHASE_B_BACK_HAVE_POWER = total2 / 3;         // B相无功
        meterTable.POWER_METER_PHASE_C_FORTH_HAVE_POWER = total1 / 3 + 1;        // C相有功
        meterTable.POWER_METER_PHASE_C_BACK_HAVE_POWER = total2 / 3 + 0.01;         // C相无功
        meterTable.POWER_METER_PHASE_A_VOLT = 218 + ( i % 9 * 1.01);
        meterTable.POWER_METER_PHASE_B_VOLT = 219 + ( i % 9 * 1.01);
        meterTable.POWER_METER_PHASE_C_VOLT = 220 + ( i % 9 * 1.01);
        meterTable.POWER_METER_PHASE_A_ELEC = 2.1 + ( i % 20 * 0.1);
        meterTable.POWER_METER_PHASE_B_ELEC = 2.2 + ( i % 20 * 0.1);
        meterTable.POWER_METER_PHASE_C_ELEC = 2.3 + ( i % 20 * 0.1);
        meterTable.POWER_METER_PHASE_A_ENERGE = 0.05 + ( i % 50 * 0.01);
        meterTable.POWER_METER_PHASE_B_ENERGE = 0.06 + ( i % 50 * 0.01);
        meterTable.POWER_METER_PHASE_C_ENERGE = 0.07 + ( i % 50 * 0.01);
        meterTable.POWER_METER_POWER_FATOR_A = 2.8 + ( i % 100 * 0.01);
        meterTable.POWER_METER_POWER_FATOR_B = 1 + ( i % 100 * 0.01);
        meterTable.POWER_METER_POWER_FATOR_C = 1 + ( i % 100 * 0.01);
        meterTable.POWER_METER_POWER_FATOR = 1 + ( i % 100 * 0.01);
        meterTable.POWER_METER_TS = CxTime::currentSepoch();                // 时间戳
        fn_sqlite3_insert_meter(db, sTableName.c_str(), &meterTable);
    }
    //
//    CxThread::sleep(300 * 1000);
}

static void fnOnTimeInsert(sqlite3 *db)
{
    //ONTIMETable
    vector<string> sTableNames = fn_sqlite3_select_dbname(db, C_ON_TIME_TABLE_PREFIX);

    for (int i = 0; i < sTableNames.size(); ++i)
    {
        const string &sTableName = sTableNames.at(i);
        ONTIMETable onTimeTable;
        onTimeTable.phase_info = 4;
        onTimeTable.ontime_positive_energey = 0.0500000007450581;
        onTimeTable.ontime_reactive_energey = 0;
        onTimeTable.ontime_freezen = "22/07/12/19/00";
        fn_sqlite3_insert_on_time(db, sTableName.c_str(), &onTimeTable);
    }
    //
//    CxThread::sleep(300 * 1000);
}

static void fnMeterLast(sqlite3 *db)
{
    vector<string> ss;
    vector<string> sTableNames = fn_sqlite3_select_dbname(db, C_METER_TABLE_PREFIX);
    for (int i = 0; i < sTableNames.size(); ++i)
    {
        const string &sTableName = sTableNames.at(i);
        METERTable meterTable;
        if (DeviceManager::getLastMETERTable(db, sTableName, &meterTable))
        {
            string s = CxString::format("%s , %lld , %s", sTableName.c_str(), meterTable.POWER_METER_TS, CxTime::toStringSepoch(sepoch_t(meterTable.POWER_METER_TS), '-', ':', '-').c_str());
            ss.push_back(s);
        }
    }
    CxFile::save("meter-last.csv", ss);
    fnInfo().out("fn-meter-last - save to meter-last.cvs - %d", ss.size());
}

int main(int argc, const char *argv[])
{
    int argi = 0;
    const char *filename = NULL;
    //
    filename = filename_remove_path(argv[0]);
    string dataName;
    string dataPath;
    for (argi = 1; argi < argc; argi++)
    {
        if (strcmp(argv[argi], "--help") == 0 || strcmp(argv[argi], "-h") == 0)
        {
            print_usage(filename);
            print_help(filename);
            return 0;
        }
        if (strcmp(argv[argi], "--version") == 0)
        {
            printf("%s %s\n", filename, APP_VERSION);
            printf("Copyright (C) 2021 by TK.\n");
            return 0;
        }
        if (strcmp(argv[argi], "--db-name") == 0)
        {
            if (++argi < argc)
            {
                dataName = argv[argi];
            }
        }
        if (strcmp(argv[argi], "--db-path-format") == 0)
        {
            if (++argi < argc)
            {
                dataPath = argv[argi];
            }
        }
        if (strcmp(argv[argi], "--fn-meter-insert") == 0)
        {
            f_bfnMeterInsert = "";
            if (++argi < argc)
            {
                f_bfnMeterInsert = string(argv[argi]);
            }
        }
        if (strcmp(argv[argi], "--fn-meter-last") == 0)
        {
            f_bfnMeterLast = "";
            if (++argi < argc)
            {
                f_bfnMeterLast = string(argv[argi]);
            }
        }
    }
    if (dataName.empty())
    {
        dataName = string("appMeterPlcMail");
    }
    if (dataPath.empty())
    {
        dataPath = string(DB_FILEPATH_FORMAT);
    }

    /* Add Ctrl-C handler */
    signal(SIGINT, sigint_handler);

    CxInterinfo::startInterInfo(NULL, NULL, false);
    CxLogManager::startLog();
    fnInfo().out("MAIN - The program version v%s", APP_VERSION);

    DBManager::setDbFilePathFormat(dataPath);

    sqlite3 *db = DBManager::open_sqlite(DBManager::getDbFilePath(dataName));
    // load config (of CCS) to var

    while (running)
    {
        if (f_bfnMeterInsert != FN_OFF)
        {
            fnMeterInsert(db);
            fnOnTimeInsert(db);
        }
        CxThread::sleep(300 * 1000);
    }

    if (f_bfnMeterLast != FN_OFF)
    {
        fnMeterLast(db);
    }

    DBManager::close_sqlite(db);
    CxInterinfo::stopInterInfo();
    CxLogManager::stopLog();
    return 0;
}

/*
 SELECT * FROM TABLE WHERE ID = (SELECT MAX(ID) FROM TABLE);
 */
