#ifndef CX_CT_X2_UTILS_CVQT_H
#define CX_CT_X2_UTILS_CVQT_H

#include <QImage>
#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>

// 将 OpenCV 的 cv::Mat（BGR/GRAY）转换为 Qt 的 QImage（RGB/Indexed）
inline QImage cvMatToQImage(const cv::Mat &mat)
{
    if (mat.empty()) return QImage();

    switch (mat.type()) {
        case CV_8UC1: {
            QImage img(mat.cols, mat.rows, QImage::Format_Grayscale8);
            if (mat.isContinuous())
                memcpy(img.bits(), mat.data, size_t(mat.rows * mat.step));
            else {
                for (int y = 0; y < mat.rows; ++y)
                    memcpy(img.scanLine(y), mat.ptr(y), size_t(mat.cols));
            }
            return img;
        }
        case CV_8UC3: {
            // OpenCV: BGR -> Qt: RGB
            QImage img(mat.data, mat.cols, mat.rows, int(mat.step), QImage::Format_RGB888);
            return img.rgbSwapped().copy();
        }
        case CV_8UC4: {
            QImage img(mat.data, mat.cols, mat.rows, int(mat.step), QImage::Format_ARGB32);
            return img.copy();
        }
        default: {
            // 其他类型统一转换到 8 位 BGR 再处理
            cv::Mat tmp;
            mat.convertTo(tmp, CV_8U, 255.0/(cv::norm(mat, cv::NORM_INF)+1e-9));
            if (tmp.channels()==1) return cvMatToQImage(tmp);
            if (tmp.channels()==3) return cvMatToQImage(tmp);
            if (tmp.channels()==4) return cvMatToQImage(tmp);
            return QImage();
        }
    }
}

// 将 QImage（RGB/GRAY）转换为 cv::Mat（BGR/GRAY）
inline cv::Mat qImageToCvMat(const QImage &img)
{
    if (img.isNull()) return {};

    switch (img.format()) {
        case QImage::Format_Grayscale8: {
            cv::Mat m(img.height(), img.width(), CV_8UC1);
            for (int y = 0; y < img.height(); ++y)
                memcpy(m.ptr(y), img.constScanLine(y), size_t(img.width()));
            return m;
        }
        case QImage::Format_RGB888: {
            cv::Mat m(img.height(), img.width(), CV_8UC3);
            for (int y = 0; y < img.height(); ++y)
                memcpy(m.ptr(y), img.constScanLine(y), size_t(img.bytesPerLine()));
            cv::cvtColor(m, m, cv::COLOR_RGB2BGR);
            return m;
        }
        case QImage::Format_ARGB32:
        case QImage::Format_ARGB32_Premultiplied: {
            cv::Mat m(img.height(), img.width(), CV_8UC4);
            for (int y = 0; y < img.height(); ++y)
                memcpy(m.ptr(y), img.constScanLine(y), size_t(img.bytesPerLine()));
            cv::cvtColor(m, m, cv::COLOR_BGRA2BGR); // 去掉 alpha
            return m;
        }
        default: {
            QImage conv = img.convertToFormat(QImage::Format_RGB888);
            return qImageToCvMat(conv);
        }
    }
}


#endif //CX_CT_X2_UTILS_CVQT_H
