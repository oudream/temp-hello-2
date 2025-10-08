#include "test_channel_win.h"

#include <QApplication>
#include <ccxx/cxapplication_qt.h>

//#undef main
int main(int argc, const char *argv[])
{
    QApplication a(argc, (char **)argv);
    CxApplicationQt::init(argc, argv);

    TestChannelWin w;
    w.show();

    return CxApplicationQt::exec();
}
