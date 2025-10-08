#include <QApplication>
#include <QFile>
#include "mct_main_window.h"

#include <ccxx/cxstring.h>

// 加载深色主题（可选）
static void loadDarkQss(QApplication &app) {
    QFile f("qss/dark.qss");
    if (f.open(QIODevice::ReadOnly)) {
        app.setStyleSheet(QString::fromUtf8(f.readAll()));
    }
}

#include <gtest/gtest.h>

#include <ccxx/cxstring.h>

int main(int argc, char *argv[]) {
    QApplication a(argc, argv);

    a.setOrganizationName("CYG");
    a.setApplicationName("CYG XRay MCT Studio");

    std::string sHex = CxString::toHexString(1);
    std::cout << sHex << std::endl;

    loadDarkQss(a);

    MctMainWindow w;
    w.resize(1400, 900);
    w.show();
    return a.exec();
}
