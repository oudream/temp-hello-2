#ifndef APP_LOG_H
#define APP_LOG_H

#include <string>

class AppLog
{
public:
    static int start(const std::string &path);

    static void stop();

    static void info(const char *format, ...);

    static void debug(const char *format, ...);

    static void warn(const char *format, ...);

    static void error(const char *format, ...);

};

#endif //APP_LOG_H
