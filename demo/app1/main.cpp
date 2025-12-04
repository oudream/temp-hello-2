#include <helpers/log_helper.h>

#include <ccxx/configuration.h>
#include <ccxx/datetime.h>
#include <ccxx/cxthread.h>

#include <helpers/yaml_helper.h>

#include "app_config.h"


int main(int argc, char *argv[]) {
    LogHelper::init("demo-app1.log", "logs");

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

    AppConfig conf; // 默认值
    cfg::ConfigManager mgr(new cfg::ConfigIOYAML());

    cfg::printAllConfigs(&conf);

    std::string err;

    // 声明需要解析的 FieldRef<T> 类型集合：<LoggerConfig>
    bool ok = mgr.loadAll<AppConfig>("demo-app1.yaml", &conf, &err);
    if (!ok) {
        std::cerr << "Load failed:" << err;
        return 1;
    }

    if (!mgr.save("demo-app1.yaml", &conf, &err)) {
        std::cerr << "Save failed:" << err;
        return 2;
    }

    cfg::printAllConfigs(&conf);
}
