#include <QApplication>
#include <QFile>
#include "main_window.h"

static void loadStyleSheet(QApplication &app) {
    QFile f("qss/dark.qss");
    if (f.open(QIODevice::ReadOnly)) {
        app.setStyleSheet(QString::fromUtf8(f.readAll()));
    }
}

int main(int argc, char *argv[]) {
    QApplication a(argc, argv);
    a.setApplicationName("CTFrameQt");
    a.setOrganizationName("DemoOrg");

    loadStyleSheet(a);

    MainWindow w;
    w.resize(1400, 900);
    w.show();
    return a.exec();
}
