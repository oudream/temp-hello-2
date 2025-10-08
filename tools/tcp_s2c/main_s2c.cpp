#include <ccxx/cxapplication.h>
#include <ccxx/cxarguments.h>
#include <ccxx/cxinterinfo.h>
#include <ccxx/cxinterinfo_group_define.h>

#include <ccxx/cxfile.h>
#include <ccxx/cxtimer.h>

#include "uv_tcp_server.h"
#include "uv_tcp_client.h"


using namespace std;


#define MODULE_NAME "MAIN"
#define APP_VERSION_TEXT "1.0.0"

CXOUTINFO_DEFINE(IIG_NORMAL, IIG_NORMAL, "MAIN-S2C")

/**
 # main

./tcp_s2c --listen-port 7000 --remote-ip 192.168.91.232 --remote-port 10086

 */
/**
 *
 * @param filename
 */
static void printUsage(const std::string &sArg0)
{
    printf("Usage: %s [--listen-port] [--remote-ip] [--remote-port] [--app-dir]\n", sArg0.c_str());
    printf("\n e.g. :\n");
    printf("%s --listen-port 8234 --col-duration 30 --app-dir /opt/tcp_s2c\n", sArg0.c_str());
}

static void printHelp()
{
    printf("Tcp Server 2 Tcp Client Tools.\n");
    printf("--listen-port: Listening TCP IP Port\n"
           "--remote-ip: TCP | UDP Connect To\n"
           "--remote-port: TCP | UDP Connect To\n"
           "\n");
    printf("--app-dir: Log File Dir, Collecting Data Save To\n"
           "\n");
}



class S2CApp: public CxJoinableThread
{
protected:
    void run() override
    {
        CxApplication::exec();
    }

};

static void mainAppStop(void)
{
    UVTcpServer::stop();
    fnInfo().out("UVTcpServer stop...");
    CxThread::sleep(100);
}

static void mainTimeout(int iInterval)
{
    fnInfo().out("main timeout...");
}

int main(int argc, const char *argv[])
{
    CxApplication::init(argc, argv);
    CxInterinfoOut::disableLevel(CxInterinfo::LevelIO);
    CxInterinfoOut::disableLevel(CxInterinfo::LevelCore);

    string sHelp = CxArguments::getArgValue("help");
    string sVersion = CxArguments::getArgValue("version");
    string sAppDir = CxArguments::getArgValue("app-dir");

    // help
    if (!sHelp.empty())
    {
        printUsage(CxArguments::getArg0());
        printHelp();
        return 0;
    }

    // version
    if (!sVersion.empty())
    {
        printf("%s %s\n", CxArguments::getArg0().c_str(), APP_VERSION_TEXT);
        printf("Copyright (C) 2022 by SS.\n");
        return 0;
    }

    string sListenPort = CxArguments::getArgValue("listen-port");
    string sRemoteIp = CxArguments::getArgValue("remote-ip");
    string sRemotePort = CxArguments::getArgValue("remote-port");
    if (!sRemoteIp.empty() && ! CxString::isValidIp(sRemoteIp))
    {
        fnError().out("remote-ip is invalid.");
        printUsage(CxArguments::getArg0());
        printHelp();
        CxApplication::exit();
        return -1;
    }
    int iListenPort = CxString::toInt32(sListenPort);
    int iRemotePort = CxString::toInt32(sRemotePort);
    if (! CxString::isValidPort(iListenPort) || ! CxString::isValidPort(iRemotePort))
    {
        fnError().out("listen port | remote-port is invalid.");
        printUsage(CxArguments::getArg0());
        printHelp();
        CxApplication::exit();
        return -1;
    }
    fnInfo().out(MODULE_NAME".arguments.listen-port: %d", iListenPort);
    fnInfo().out(MODULE_NAME".arguments.remote-ip: %s", sRemoteIp.c_str());
    fnInfo().out(MODULE_NAME".arguments.remote-port: %d", iRemotePort);

    if (UVTcpClient::start(sRemoteIp, iRemotePort) == NULL)
    {
        fnError().out(MODULE_NAME" Tcp Client Connect Fail! errno=%d", errno);
        CxApplication::exit();
        return -1;
    }

    S2CApp s2CApp;
    s2CApp.start();
    if (! s2CApp.isRunning())
    {
        fnError().out(MODULE_NAME" S2CApp Run Error! errno=%d", errno);
        return -1;
    }

    CxTimerManager::startTimer(mainTimeout, 1000);

    //
    CxApplication::registStopFunction(mainAppStop);
    return UVTcpServer::start(iListenPort);
}
