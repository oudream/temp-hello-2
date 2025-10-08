#include <QApplication>
#include "mainwindow.h"

#include <opencv2/imgcodecs.hpp>


int main(int argc, char *argv[])
{
//    cv::Mat mat1;
//    mat1.release();
//    mat1.release();
//    mat1 = cv::imread("D:\\tmp\\a.png", cv::IMREAD_COLOR);

    QApplication a(argc, argv);
    MainWindow w;
    w.show();
    return a.exec();
}