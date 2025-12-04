#include <QApplication>
#include <QFile>

#include <ccxx/cxstring.h>

#include <helpers/log_helper.h>

#include <base/app_ids.h>
#include <base/app_directory_helper.h>
#include <base/app_config_helper.h>
#include <base/icon_manager.h>
#include "u_ribbon_registry.h"

#include <net/net_ct_service.h>

#include "mct_main_window.h"
#include "helpers/opencv_helper.h"
#include "net/rebuild_task_manager.h"

// 加载深色主题（可选）
static void loadDarkQss(QApplication &app)
{
    QFile f("qss/dark.qss");
    if (f.open(QIODevice::ReadOnly))
    {
        app.setStyleSheet(QString::fromUtf8(f.readAll()));
    }
}

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    a.setOrganizationName("CYG");
    a.setApplicationName("CYG XRay MCT Studio");

    // 初始化APP目录
    AppDirectoryHelper::init(argv[0]);

    // 初始化日志
    LogHelper::init(AppDirectoryHelper::exeFileNameNoExt(), AppDirectoryHelper::logsPath());

    // 初始化配置
    AppConfigHelper::load(AppDirectoryHelper::configsJoin(std::string{AppIds::kAppName} + ".yaml"));

    IconManager::i().loadAll(AppIds::ToQString(AppDirectoryHelper::configsJoin(std::string{AppIds::kAppName} + ".icons.yaml")));

    // 初始化各业务功能分块
    URibbonRegistry::init();

    // 加载 QSS
    loadDarkQss(a);

    // 加载 服务 ReconstructionService
    RebuildTaskManager::init();

    // 加载 服务 CtService
    NetCtService::start();

    cvh::CoreHelper::setLogger([](cvh::CoreHelper::LogLevel lv, const std::string& msg){
        // 接入日志系统
        LogHelper::log((LogStream::LogLevel) (lv), msg);
    });

    // 加载主窗口
    MctMainWindow w;
    w.resize(1400, 900);
    w.show();
    auto rExe = a.exec();

    cvh::CoreHelper::setLogger(nullptr);

    // 停止 服务 CtService
    NetCtService::stop();

    // 卸载 服务 ReconstructionService
    RebuildTaskManager::deInit();

    // 保存配置
    AppConfigHelper::save(AppDirectoryHelper::configsJoin(std::string{AppIds::kAppName} + ".yaml"));

    return rExe;
}
