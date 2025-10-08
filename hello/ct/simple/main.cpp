#include "awin.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    AWin w;
    w.show();
    return a.exec();
}