#include <helpers/log_helper.h>
#include <ccxx/configuration.h>
#include <ccxx/datetime.h>
#include <ccxx/cxthread.h>

int main(int argc, char *argv[]) {
    LogHelper::init("demo-app1.log");

    std::cout << "start" << std::endl;
    LogHelper::debug() << "start";

    LogHelper::debug() << cx::DateTime::currentDateTimeString();

    for (int i = 0; i < 100; ++i)
    {
        CxThread::sleep(10);
        LogHelper::debug() << cx::DateTime::currentDateTimeString();
    }

    LogHelper::debug() << "end";
    std::cout << "end" << std::endl;
}
