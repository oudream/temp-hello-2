#include "opencv_helper.h"

#include <cstdio>
#include <sstream>
#include <utility>

#if defined(_WIN32)

#else
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#endif

namespace cvh
{

// ========================= CoreHelper =========================
    static CoreHelper::LogFn g_logger; // 默认为空 -> 使用默认 stderr 输出

    static void defaultLog(CoreHelper::LogLevel lv, const std::string &msg)
    {
        const char *lvStr = (
                lv == CoreHelper::LogLevel::Debug ? "DEBUG" :
                (
                        lv == CoreHelper::LogLevel::Info ? "INFO " :
                        (lv == CoreHelper::LogLevel::Warn ? "WARN " : "ERROR")));
        std::fprintf(stderr, "[CVH][%s] %s\n", lvStr, msg.c_str());
    }

    void CoreHelper::setLogger(LogFn fn)
    {
        g_logger = std::move(fn);
    }

    void CoreHelper::log(LogLevel lv, const std::string &msg)
    {
        if (g_logger) g_logger(lv, msg);
        else defaultLog(lv, msg);
    }

// ========================= UtilHelper =========================
    std::string UtilHelper::typeToString(int type)
    {
        std::ostringstream oss;
        int depth = type & CV_MAT_DEPTH_MASK;
        int chans = 1 + (type >> CV_CN_SHIFT);
        const char *dstr =
                depth == CV_8U ? "8U" :
                depth == CV_8S ? "8S" :
                depth == CV_16U ? "16U" :
                depth == CV_16S ? "16S" :
                depth == CV_32S ? "32S" :
                depth == CV_32F ? "32F" :
                depth == CV_64F ? "64F" : "User";
        oss << "CV_" << dstr << "C" << chans;
        return oss.str();
    }

    std::string UtilHelper::matInfo(const cv::Mat &img)
    {
        if (img.empty()) return "Mat[empty]";
        std::ostringstream oss;
        oss << "Mat[" << img.cols << "x" << img.rows
            << ", " << UtilHelper::typeToString(img.type()) << "]";
        return oss.str();
    }

    bool UtilHelper::compareMse(const cv::Mat &a, const cv::Mat &b, double tol, double *outMse)
    {
        if (a.empty() || b.empty())
        {
            CoreHelper::log(CoreHelper::LogLevel::Warn, "compareMse: empty input");
            return false;
        }
        if (a.size() != b.size() || a.type() != b.type())
        {
            CoreHelper::log(CoreHelper::LogLevel::Warn, "compareMse: size/type mismatch");
            return false;
        }
        cv::Mat diff;
        cv::absdiff(a, b, diff);
        diff.convertTo(diff, CV_32F);
        diff = diff.mul(diff);
        cv::Scalar s = cv::sum(diff);
        double mse = 0.0;
        int n = a.channels() * a.rows * a.cols;
        for (int i = 0; i < diff.channels(); ++i)
        {
            mse += s[i];
        }
        mse /= (double) n;
        if (outMse) *outMse = mse;
        return mse <= tol;
    }

    cv::Mat UtilHelper::cloneOrEmpty(const cv::Mat &src)
    {
        return src.empty() ? cv::Mat() : src.clone();
    }

    bool UtilHelper::isGray(const cv::Mat &img)
    {
        if (img.empty()) return false;
        if (img.channels() == 1) return true;
        if (img.channels() != 3) return false;
        // 抽样检查 B==G==R（避免逐像素全图耗时）
        const int step = std::max(1, std::min(img.cols, img.rows) / 64); // 采样步长
        for (int y = 0; y < img.rows; y += step)
        {
            const auto *row = img.ptr<cv::Vec3b>(y);
            for (int x = 0; x < img.cols; x += step)
            {
                const cv::Vec3b &v = row[x];
                if (!(v[0] == v[1] && v[1] == v[2])) return false;
            }
        }
        return true;
    }

// ========================= IOHelper =========================
    cv::Mat IOHelper::load(const std::string &path, int flags)
    {
        if (path.empty())
        {
            CoreHelper::log(CoreHelper::LogLevel::Warn, "load: empty path");
            return {};
        }
        cv::Mat img = cv::imread(path, flags);
        if (img.empty())
        {
            CoreHelper::log(CoreHelper::LogLevel::Error, "load failed: " + path);
        }
        return img;
    }

    bool IOHelper::save(const std::string &path, const cv::Mat &img, int quality)
    {
        if (path.empty() || img.empty())
        {
            CoreHelper::log(CoreHelper::LogLevel::Warn, "save: empty path or image");
            return false;
        }
        std::vector<int> params;
        // 根据扩展名设置编码参数（常见格式）
        if (quality > 0)
        {
            size_t dot = path.find_last_of('.');
            if (dot != std::string::npos)
            {
                std::string ext = path.substr(dot + 1);
                for (auto &c: ext) c = (char) tolower(c);
                if (ext == "jpg" || ext == "jpeg")
                {
                    params.push_back(cv::IMWRITE_JPEG_QUALITY);
                    params.push_back(std::max(1, std::min(100, quality)));
                }
                else if (ext == "webp")
                {
                    params.push_back(cv::IMWRITE_WEBP_QUALITY);
                    params.push_back(std::max(1, std::min(100, quality)));
                }
                else if (ext == "tiff" || ext == "tif")
                {
                    params.push_back(cv::IMWRITE_TIFF_COMPRESSION);
                    params.push_back(1); // LZW
                }
                else if (ext == "png")
                {
                    // PNG: 压缩级别 0~9（0=无压缩, 9=最大压缩）
                    // 我们用 quality(1-100) 映射为 0~9
                    int pngLevel = 9 - std::clamp(quality / 11, 0, 9);
                    params.push_back(cv::IMWRITE_PNG_COMPRESSION);
                    params.push_back(pngLevel);
                }
                else if (ext == "bmp")
                {
                    // BMP 不支持质量参数，但可保留为无压缩格式。
                    // 如果想在未来扩展 RLE 压缩，可设置 IMWRITE_PXM_BINARY 等标志（OpenCV 当前不支持 BMP 压缩）
                    params.clear(); // 确保没有遗留参数
                }
            }
        }
        bool ok;
        try
        {
            ok = cv::imwrite(path, img, params);
        }
        catch (const cv::Exception &e)
        {
            CoreHelper::log(CoreHelper::LogLevel::Error, std::string("imwrite exception: ") + e.what());
            ok = false;
        }
        if (!ok) CoreHelper::log(CoreHelper::LogLevel::Error, "save failed: " + path);
        return ok;
    }

    void IOHelper::show(const std::string &win, const cv::Mat &img, int delay)
    {
        if (win.empty() || img.empty())
        {
            CoreHelper::log(CoreHelper::LogLevel::Warn, "show: empty window name or image");
            return;
        }
        cv::imshow(win, img);
        cv::waitKey(delay);
    }

    cv::Mat IOHelper::toGray(const cv::Mat &img)
    {
        if (img.empty())
        {
            CoreHelper::log(CoreHelper::LogLevel::Warn, "toGray: empty image");
            return {};
        }
        if (img.channels() == 1) return img.clone();
        cv::Mat out;
        if (img.channels() == 3) cv::cvtColor(img, out, cv::COLOR_BGR2GRAY);
        else if (img.channels() == 4) cv::cvtColor(img, out, cv::COLOR_BGRA2GRAY);
        else
        {
            CoreHelper::log(CoreHelper::LogLevel::Warn, "toGray: unsupported channels");
        }
        return out;
    }

    cv::Mat IOHelper::toBgr(const cv::Mat &img)
    {
        if (img.empty()) return {};
        if (img.channels() == 3) return img.clone();
        cv::Mat out;
        if (img.channels() == 1) cv::cvtColor(img, out, cv::COLOR_GRAY2BGR);
        else if (img.channels() == 4) cv::cvtColor(img, out, cv::COLOR_BGRA2BGR);
        else CoreHelper::log(CoreHelper::LogLevel::Warn, "toBgr: unsupported channels");
        return out;
    }

    cv::Mat IOHelper::toRgb(const cv::Mat &img)
    {
        if (img.empty()) return {};
        cv::Mat out;
        if (img.channels() == 1) cv::cvtColor(img, out, cv::COLOR_GRAY2RGB);
        else if (img.channels() == 3) cv::cvtColor(img, out, cv::COLOR_BGR2RGB);
        else if (img.channels() == 4) cv::cvtColor(img, out, cv::COLOR_BGRA2RGB);
        else CoreHelper::log(CoreHelper::LogLevel::Warn, "toRgb: unsupported channels");
        return out;
    }

// ========================= BasicHelper =========================
    cv::Mat BasicHelper::resize(const cv::Mat &img, int w, int h, int interp)
    {
        if (img.empty() || w <= 0 || h <= 0)
        {
            CoreHelper::log(CoreHelper::LogLevel::Warn, "resize: bad args or empty image");
            return {};
        }
        cv::Mat out;
        cv::resize(img, out, cv::Size(w, h), 0, 0, interp);
        return out;
    }

    cv::Mat BasicHelper::crop(const cv::Mat &img, const cv::Rect &roi)
    {
        if (img.empty())
        {
            CoreHelper::log(CoreHelper::LogLevel::Warn, "crop: empty image");
            return {};
        }
        cv::Rect r = roi & cv::Rect(0, 0, img.cols, img.rows);
        if (r.width <= 0 || r.height <= 0)
        {
            CoreHelper::log(CoreHelper::LogLevel::Warn, "crop: roi out of range");
            return {};
        }
        return img(r).clone();
    }

    cv::Mat BasicHelper::pad(const cv::Mat &img, int border, const cv::Scalar &color)
    {
        if (img.empty() || border < 0)
        {
            CoreHelper::log(CoreHelper::LogLevel::Warn, "pad: empty image or bad border");
            return {};
        }
        cv::Mat out;
        cv::copyMakeBorder(img, out, border, border, border, border, cv::BORDER_CONSTANT, color);
        return out;
    }

    cv::Mat BasicHelper::rotate(const cv::Mat &img, double angleDeg, cv::Point2f center, int interp, const cv::Scalar &border)
    {
        if (img.empty())
        {
            CoreHelper::log(CoreHelper::LogLevel::Warn, "rotate: empty image");
            return {};
        }
        if (center.x < 0 || center.y < 0) center = cv::Point2f(static_cast<float>(img.cols) * 0.5f, static_cast<float>(img.rows) * 0.5f);
        cv::Mat M = cv::getRotationMatrix2D(center, angleDeg, 1.0);
        cv::Mat out;
        cv::warpAffine(img, out, M, img.size(), interp, cv::BORDER_CONSTANT, border);
        return out;
    }

    cv::Mat BasicHelper::flip(const cv::Mat &img, int mode)
    {
        if (img.empty())
        {
            CoreHelper::log(CoreHelper::LogLevel::Warn, "flip: empty image");
            return {};
        }
        cv::Mat out;
        cv::flip(img, out, mode);
        return out;
    }

    cv::Mat BasicHelper::blend(const cv::Mat &a, const cv::Mat &b, double alpha)
    {
        if (a.empty() || b.empty())
        {
            CoreHelper::log(CoreHelper::LogLevel::Warn, "blend: empty inputs");
            return {};
        }
        if (a.size() != b.size() || a.type() != b.type())
        {
            CoreHelper::log(CoreHelper::LogLevel::Warn, "blend: size/type mismatch");
            return {};
        }
        alpha = std::max(0.0, std::min(1.0, alpha));
        cv::Mat out;
        cv::addWeighted(a, alpha, b, 1.0 - alpha, 0.0, out);
        return out;
    }

// ========================= EnhanceHelper =========================
    cv::Mat EnhanceHelper::brightnessContrast(const cv::Mat &img, double alpha, int beta)
    {
        if (img.empty()) return {};
        cv::Mat out;
        img.convertTo(out, img.type(), alpha, beta);
        return out;
    }

    cv::Mat EnhanceHelper::gamma(const cv::Mat &img, double gammaVal)
    {
        if (img.empty() || gammaVal <= 0)
        {
            CoreHelper::log(CoreHelper::LogLevel::Warn, "gamma: empty image or bad gamma");
            return {};
        }

        double invGamma = 1.0 / gammaVal; // 預先計算 1.0 / gamma

        if (img.depth() == CV_8U)
        {
            // 8U: 使用 LUT (原邏輯，非常高效)
            cv::Mat lut(1, 256, CV_8U);
            uchar *p = lut.data;
            for (int i = 0; i < 256; ++i)
            {
                p[i] = cv::saturate_cast<uchar>(std::pow(i / 255.0, invGamma) * 255.0);
            }
            cv::Mat out;
            cv::LUT(img, lut, out);
            return out;
        }
        else if (img.depth() == CV_32F || img.depth() == CV_64F)
        {
            // 浮點類型 (32F 或 64F): 假設範圍是 0.0 - 1.0，直接應用 pow
            cv::Mat out;
            cv::pow(img, invGamma, out); // 直接對 0.0-1.0 的值計算
            return out;
        }
        else
        {
            // 其他類型 (例如 16U, 16S, 32S):
            // 轉換為浮點數，根據其理論最大值進行歸一化
            double maxVal = 1.0;
            if (img.depth() == CV_16U) maxVal = 65535.0;
            else if (img.depth() == CV_16S) maxVal = 32767.0;
            else if (img.depth() == CV_32S) maxVal = 2147483647.0;
            // (您也可以在此處添加一個 minMaxLoc 來獲取實際範圍，但歸一化更通用)

            cv::Mat f, out;
            img.convertTo(f, CV_32F, 1.0 / maxVal); // 歸一化到 0-1
            cv::pow(f, invGamma, f);
            f.convertTo(out, img.type(), maxVal); // 轉換回原始類型和範圍
            return out;
        }
    }

    cv::Mat EnhanceHelper::blur(const cv::Mat &img, int ksize)
    {
        if (img.empty() || ksize <= 1 || (ksize % 2) == 0)
        {
            CoreHelper::log(CoreHelper::LogLevel::Warn, "blur: empty image or bad ksize (odd>1)");
            return {};
        }
        cv::Mat out;
        cv::blur(img, out, cv::Size(ksize, ksize));
        return out;
    }

    cv::Mat EnhanceHelper::gaussian(const cv::Mat &img, int ksize, double sigma)
    {
        if (img.empty() || ksize <= 1 || (ksize % 2) == 0)
        {
            CoreHelper::log(CoreHelper::LogLevel::Warn, "gaussian: empty image or bad ksize (odd>1)");
            return {};
        }
        cv::Mat out;
        cv::GaussianBlur(img, out, cv::Size(ksize, ksize), sigma);
        return out;
    }

    cv::Mat EnhanceHelper::sharpen(const cv::Mat &img)
    {
        if (img.empty()) return {};
        cv::Mat kernel = (
                cv::Mat_<float>(3, 3) <<
                                      0, -1, 0,
                        -1, 5, -1,
                        0, -1, 0);
        cv::Mat out;
        cv::filter2D(img, out, img.depth(), kernel);
        return out;
    }

    cv::Mat EnhanceHelper::equalizeGray(const cv::Mat &img)
    {
        if (img.empty()) return {};
        if (img.channels() == 1)
        {
            cv::Mat out;
            cv::equalizeHist(img, out);
            return out;
        }
        else if (img.channels() == 3)
        {
            cv::Mat ycrcb;
            cv::cvtColor(img, ycrcb, cv::COLOR_BGR2YCrCb);
            std::vector<cv::Mat> ch;
            cv::split(ycrcb, ch);
            cv::equalizeHist(ch[0], ch[0]); // 仅均衡亮度
            cv::merge(ch, ycrcb);
            cv::Mat out;
            cv::cvtColor(ycrcb, out, cv::COLOR_YCrCb2BGR);
            return out;
        }
        else
        {
            CoreHelper::log(CoreHelper::LogLevel::Warn, "equalizeGray: unsupported channels");
            return {};
        }
    }

// ========================= ThreshHelper =========================
    cv::Mat ThreshHelper::threshold(const cv::Mat &img, double t, double maxv, int type)
    {
        cv::Mat gray = IOHelper::toGray(img);
        if (gray.empty()) return {};
        cv::Mat out;
        cv::threshold(gray, out, t, maxv, type);
        return out;
    }

    cv::Mat ThreshHelper::adaptive(const cv::Mat &img, int blockSize, double C, int method, int type)
    {
        cv::Mat gray = IOHelper::toGray(img);
        if (gray.empty()) return {};
        if (blockSize <= 1 || (blockSize % 2) == 0)
        {
            CoreHelper::log(CoreHelper::LogLevel::Warn, "adaptive: bad blockSize (odd>1)");
            return {};
        }
        cv::Mat out;
        cv::adaptiveThreshold(gray, out, 255, method, type, blockSize, C);
        return out;
    }

    cv::Mat ThreshHelper::canny(const cv::Mat &img, double low, double high, int aperture, bool L2grad)
    {
        cv::Mat gray = IOHelper::toGray(img);
        if (gray.empty()) return {};
        cv::Mat out;
        cv::Canny(gray, out, low, high, aperture, L2grad);
        return out;
    }

    cv::Mat ThreshHelper::morph(const cv::Mat &img, int op, int ksize, int iterations)
    {
        if (img.empty() || ksize <= 0 || iterations <= 0)
        {
            CoreHelper::log(CoreHelper::LogLevel::Warn, "morph: bad args or empty image");
            return {};
        }
        cv::Mat element = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(ksize, ksize));
        cv::Mat out;
        cv::morphologyEx(img, out, op, element, cv::Point(-1, -1), iterations);
        return out;
    }

// ========================= ContourHelper / DrawHelper =========================
    std::vector<std::vector<cv::Point>> ContourHelper::find(const cv::Mat &binary, int mode, int method)
    {
        if (binary.empty())
        {
            CoreHelper::log(CoreHelper::LogLevel::Warn, "contour.find: empty image");
            return {};
        }
        cv::Mat b = binary;
        if (binary.channels() != 1)
        {
            CoreHelper::log(CoreHelper::LogLevel::Warn, "contour.find: expect 1-channel binary");
            b = IOHelper::toGray(binary);
            if (b.empty()) return {};
            cv::threshold(b, b, 0, 255, cv::THRESH_BINARY | cv::THRESH_OTSU);
        }
        std::vector<std::vector<cv::Point>> contours;
        cv::findContours(b, contours, mode, method);
        return contours;
    }

    void ContourHelper::draw(cv::Mat &img, const std::vector<std::vector<cv::Point>> &contours, bool filled,
                             const cv::Scalar &color, int thickness)
    {
        if (img.empty()) return;
        if (filled)
        {
            cv::drawContours(img, contours, -1, color, cv::FILLED);
        }
        else
        {
            cv::drawContours(img, contours, -1, color, thickness);
        }
    }

    void DrawHelper::line(cv::Mat &img, cv::Point p1, cv::Point p2, const cv::Scalar &color, int thickness, int lineType)
    {
        if (img.empty()) return;
        cv::line(img, p1, p2, color, thickness, lineType);
    }

    void DrawHelper::rect(cv::Mat &img, const cv::Rect &rc, const cv::Scalar &color, int thickness, int lineType)
    {
        if (img.empty()) return;
        cv::rectangle(img, rc, color, thickness, lineType);
    }

    void DrawHelper::circle(cv::Mat &img, cv::Point c, int r, const cv::Scalar &color, int thickness, int lineType)
    {
        if (img.empty() || r <= 0) return;
        cv::circle(img, c, r, color, thickness, lineType);
    }

    void DrawHelper::polygon(cv::Mat &img, const std::vector<cv::Point> &pts, const cv::Scalar &color, bool filled, int thickness, int lineType)
    {
        if (img.empty() || pts.size() < 2) return;
        if (filled)
        {
            const std::vector<std::vector<cv::Point>> polys{pts};
            cv::fillPoly(img, polys, color, lineType);
        }
        else
        {
            for (size_t i = 0; i < pts.size(); ++i)
            {
                cv::line(img, pts[i], pts[(i + 1) % pts.size()], color, thickness, lineType);
            }
        }
    }

    void DrawHelper::text(cv::Mat &img, const std::string &txt, cv::Point pos, double scale, const cv::Scalar &color, int thickness)
    {
        if (img.empty() || txt.empty()) return;
        cv::putText(img, txt, pos, cv::FONT_HERSHEY_SIMPLEX, scale, color, thickness, cv::LINE_AA);
    }

// ========================= MeasureHelper =========================
    double MeasureHelper::area(const std::vector<cv::Point> &c)
    {
        if (c.empty()) return 0.0;
        return std::fabs(cv::contourArea(c));
    }

    double MeasureHelper::length(const std::vector<cv::Point> &c, bool closed)
    {
        if (c.empty()) return 0.0;
        return cv::arcLength(c, closed);
    }

    cv::Moments MeasureHelper::moments(const std::vector<cv::Point> &c)
    {
        return cv::moments(c);
    }

    cv::Point2d MeasureHelper::centroid(const std::vector<cv::Point> &c)
    {
        cv::Moments m = cv::moments(c);
        double a = m.m00;
        if (std::abs(a) < 1e-12) return {-1, -1};
        return {m.m10 / a, m.m01 / a};
    }

    cv::Vec4f MeasureHelper::fitLine(const std::vector<cv::Point> &pts, int distType, double param, double reps, double aeps)
    {
        cv::Vec4f line(0, 0, 0, 0);
        if (pts.size() < 2) return line;
        cv::fitLine(pts, line, distType, param, reps, aeps);
        return line;
    }

    cv::RotatedRect MeasureHelper::fitEllipse(const std::vector<cv::Point> &pts)
    {
        if (pts.size() < 5) return {};
        return cv::fitEllipse(pts);
    }

    cv::Rect MeasureHelper::boundingBox(const std::vector<cv::Point> &c)
    {
        if (c.empty()) return {};
        return cv::boundingRect(c);
    }

// ========================= ColorHelper =========================
    cv::Mat ColorHelper::convert(const cv::Mat &img, int code)
    {
        if (img.empty()) return {};
        cv::Mat out;
        cv::cvtColor(img, out, code);
        return out;
    }

    std::vector<cv::Mat> ColorHelper::splitChannels(const cv::Mat &img)
    {
        std::vector<cv::Mat> ch;
        if (img.empty()) return ch;
        cv::split(img, ch);
        return ch;
    }

    cv::Mat ColorHelper::mergeChannels(const std::vector<cv::Mat> &ch)
    {
        if (ch.empty()) return {};
        cv::Mat out;
        cv::merge(ch, out);
        return out;
    }

    cv::Mat ColorHelper::hsvRange(const cv::Mat &img, const cv::Scalar& low, const cv::Scalar& high)
    {
        if (img.empty()) return {};
        cv::Mat hsv;
        cv::cvtColor(img, hsv, cv::COLOR_BGR2HSV);
        cv::Mat mask;
        cv::inRange(hsv, low, high, mask);
        return mask;
    }

// ========================= FeatureHelper =========================
    void FeatureHelper::orb(const cv::Mat &img, std::vector<cv::KeyPoint> &kps, cv::Mat &desc, int nfeatures)
    {
        kps.clear();
        desc.release();
        if (img.empty())
        {
            CoreHelper::log(CoreHelper::LogLevel::Warn, "feature.orb: empty image");
            return;
        }
        cv::Ptr<cv::Feature2D> o = cv::ORB::create(nfeatures);
        cv::Mat gray = IOHelper::toGray(img);
        if (gray.empty()) gray = img;
        o->detectAndCompute(gray, cv::noArray(), kps, desc);
    }

    std::vector<cv::DMatch> FeatureHelper::matchBf(const cv::Mat &d1, const cv::Mat &d2, int normType, bool crossCheck)
    {
        std::vector<cv::DMatch> matches;
        if (d1.empty() || d2.empty())
        {
            CoreHelper::log(CoreHelper::LogLevel::Warn, "feature.matchBf: empty descriptors");
            return matches;
        }
        try
        {
            cv::BFMatcher matcher(normType, crossCheck);
            matcher.match(d1, d2, matches);
        }
        catch (const cv::Exception &e)
        {
            CoreHelper::log(CoreHelper::LogLevel::Error, std::string("BFMatcher exception: ") + e.what());
            matches.clear();
        }
        return matches;
    }

    cv::Mat FeatureHelper::drawMatchesImage(const cv::Mat &img1, const std::vector<cv::KeyPoint> &kp1,
                                            const cv::Mat &img2, const std::vector<cv::KeyPoint> &kp2,
                                            const std::vector<cv::DMatch> &matches)
    {
        if (img1.empty() || img2.empty()) return {};
        cv::Mat out;
        cv::drawMatches(img1, kp1, img2, kp2, matches, out);
        return out;
    }

} // namespace cvh
