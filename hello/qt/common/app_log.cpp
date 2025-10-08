#include "app_log.h"

#include <ccxx/cxtime.h>
#include <ccxx/cxfile.h>
#include <ccxx/cxthread.h>

using namespace std;

// *** *** ***
// log
// *** *** ***

static string logPath;
static FILE *logFd = NULL;
static int logSize = 0;
#define ME_GOAHEAD_LOGMAXSIZE (10*1024*1024-1)

static CxMutex * fn_logLock()
{
    static CxMutex m;
    return & m;
}

static int logOpen();
static void logClose();

static void logCheck()
{
    if (logFd == NULL)
    {
        logOpen();
    }
    else if (logSize > ME_GOAHEAD_LOGMAXSIZE)
    {
        logClose();
        logOpen();
    }
}

static void defaultLogHandler(char *buf)
{
    int bufLen;

    logCheck();
    bufLen = (int) strlen(buf);
    if (logFd != NULL)
    {
        std::string dt = CxTime::currentMsepochString() + " ";
        printf("%s", dt.c_str());
        printf("%s", buf);
        printf("%s", "\n");
        fwrite(dt.c_str(), 1, dt.size(), logFd);
        fwrite(buf, 1, bufLen, logFd);
        fwrite("\n", 1, 1, logFd);
        fflush(logFd);
        logSize += bufLen + 1 + dt.size();
    }
}

static int logOpen()
{
    if (logPath.empty())
    {
        logPath = CxFileSystem::getCurrentExePath()+".log";
    }
    logFd = fopen(logPath.c_str(), "w");
    if (logFd == NULL)
    {
        return -1;
    }
    fseek(logFd, 0, SEEK_SET);
    logSize = 0;
    return 0;
}

static void logClose()
{
    if (logFd != NULL)
    {
        fclose(logFd);
        logFd = NULL;
    }
}

static void logPrint(const char *format, va_list args)
{
    CxMutexScope lock(fn_logLock());
    char buffer[4096];
    vsnprintf(buffer, sizeof(buffer), format, args);
    defaultLogHandler(buffer);
}

int AppLog::start(const string &path)
{
    logPath = path;
    return logOpen();
}

void AppLog::stop()
{
    logClose();
}

void AppLog::info(const char *format, ...)
{
    std::string fmt = std::string("INFO -> ") + std::string(format);
    va_list args;
    va_start(args, format);
    logPrint(fmt.c_str(), args);
    va_end(args);
}

void AppLog::debug(const char *format, ...)
{
    std::string fmt = std::string("DEBUG > ") + std::string(format);
    va_list args;
    va_start(args, format);
    logPrint(fmt.c_str(), args);
    va_end(args);
}

void AppLog::warn(const char *format, ...)
{
    std::string fmt = std::string("WARN -- ") + std::string(format);
    va_list args;
    va_start(args, format);
    logPrint(fmt.c_str(), args);
    va_end(args);
}

void AppLog::error(const char *format, ...)
{
    std::string fmt = std::string("ERROR - ") + std::string(format);
    va_list args;
    va_start(args, format);
    logPrint(fmt.c_str(), args);
    va_end(args);
}
