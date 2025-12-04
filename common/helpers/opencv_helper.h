#ifndef CX_CT_X2_OPENCV_HELPER_H
#define CX_CT_X2_OPENCV_HELPER_H


// 常用 OpenCV 静态类帮助库

#include <opencv2/opencv.hpp>
#include <string>
#include <vector>
#include <functional>

namespace cvh {

// ========================= 核心：日志与配置 =========================
    class CoreHelper {
    public:
        // 日志级别
        enum class LogLevel { Trace, Debug, Info, Warn, Error };

        // 日志回调：lv=级别, msg=消息文本
        using LogFn = std::function<void(LogLevel, const std::string&)>;

        // 设置日志回调；未设置则使用默认 stderr 打印
        static void setLogger(LogFn fn);

        // 发送日志；内部用于统一输出
        static void log(LogLevel lv, const std::string& msg);

    private:
        CoreHelper() = delete;
    };

// ========================= 工具函数 =========================
    class UtilHelper {
    public:
        UtilHelper() = delete;

        // OpenCV Mat 类型转字符串，如 "CV_8UC3"
        static std::string typeToString(int type);

        // 返回 Mat 信息（尺寸、通道、深度），用于调试日志
        static std::string matInfo(const cv::Mat& img);

        // 近似比较：尺寸/类型一致时计算 MSE；返回是否 <= 阈值；可带出实际 MSE
        static bool compareMse(const cv::Mat& a, const cv::Mat& b, double tol, double* outMse = nullptr);

        // 安全克隆：空 Mat 返回空，否则返回克隆
        static cv::Mat cloneOrEmpty(const cv::Mat& src);

        // 是否灰度图：单通道直接为真；3 通道时做抽样一致性判断（避免整图逐像素耗时）
        static bool isGray(const cv::Mat& img);
    };

// ========================= I/O 与显示 =========================
    class IOHelper {
    public:
        IOHelper() = delete;

        // 读取图像；flags 默认 cv::IMREAD_COLOR；失败返回空 Mat
        static cv::Mat load(const std::string& path, int flags = cv::IMREAD_COLOR);

        // 保存图像到文件；quality（JPEG 质量 1-100，PNG 等忽略）；自动创建目录；成功返回 true
        static bool save(const std::string& path, const cv::Mat& img, int quality = 95);

        // 显示图像；delay 毫秒（0=一直等待）；用于调试场景
        static void show(const std::string& win, const cv::Mat& img, int delay = 0);

        // 转灰度（支持 3/4 通道），失败返回空
        static cv::Mat toGray(const cv::Mat& img);

        // 转 BGR（三通道），失败返回空
        static cv::Mat toBgr(const cv::Mat& img);

        // 转 RGB（三通道），失败返回空
        static cv::Mat toRgb(const cv::Mat& img);
    };

// ========================= 基础几何操作 =========================
    class BasicHelper {
    public:
        BasicHelper() = delete;

        // 等比例或非等比例缩放到固定宽高；w/h 必须 >0
        static cv::Mat resize(const cv::Mat& img, int w, int h, int interp = cv::INTER_LINEAR);

        // 裁剪 ROI；ROI 必须在图像范围内
        static cv::Mat crop(const cv::Mat& img, const cv::Rect& roi);

        // 外扩边框；border >= 0
        static cv::Mat pad(const cv::Mat& img, int border, const cv::Scalar& color = cv::Scalar(0,0,0));

        // 旋转 angle（度）；center 默认图像中心；支持边界填充与插值
        static cv::Mat rotate(const cv::Mat& img, double angleDeg, cv::Point2f center = cv::Point2f(-1,-1),
                              int interp = cv::INTER_LINEAR, const cv::Scalar& border = cv::Scalar(0,0,0));

        // 翻转：mode=0(x轴) 1(y轴) -1(同时)
        static cv::Mat flip(const cv::Mat& img, int mode = 1);

        // 融合：要求尺寸/类型一致，alpha∈[0,1]；返回 a*alpha + b*(1-alpha)
        static cv::Mat blend(const cv::Mat& a, const cv::Mat& b, double alpha);
    };

// ========================= 增强与滤波 =========================
    class EnhanceHelper {
    public:
        EnhanceHelper() = delete;

        // 亮度对比度：dst = alpha*img + beta
        static cv::Mat brightnessContrast(const cv::Mat& img, double alpha, int beta);

        // Gamma 校正：gamma>0；内部 LUT 实现
        static cv::Mat gamma(const cv::Mat& img, double gammaVal);

        // 均值模糊：ksize 奇数且>1
        static cv::Mat blur(const cv::Mat& img, int ksize);

        // 高斯模糊：ksize 奇数且>1；sigma=0 时由 OpenCV 估计
        static cv::Mat gaussian(const cv::Mat& img, int ksize, double sigma = 0);

        // 简单锐化：拉普拉斯/卷积核实现
        static cv::Mat sharpen(const cv::Mat& img);

        // 灰度直方图均衡化（3 通道自动转 YCrCb，仅均衡 Y）
        static cv::Mat equalizeGray(const cv::Mat& img);
    };

// ========================= 阈值 / 边缘 / 形态 =========================
    class ThreshHelper {
    public:
        ThreshHelper() = delete;

        // 普通阈值化（自动转灰）
        static cv::Mat threshold(const cv::Mat& img, double t, double maxv, int type);

        // 自适应阈值（自动转灰）
        static cv::Mat adaptive(const cv::Mat& img, int blockSize, double C,
                                int method = cv::ADAPTIVE_THRESH_MEAN_C, int type = cv::THRESH_BINARY);

        // Canny 边缘（自动转灰）
        static cv::Mat canny(const cv::Mat& img, double low, double high, int aperture = 3, bool L2grad = false);

        // 形态学：op=开闭/膨胀/腐蚀等；ksize>0；iterations>=1
        static cv::Mat morph(const cv::Mat& img, int op, int ksize, int iterations = 1);
    };

// ========================= 轮廓与绘制 =========================
    class ContourHelper {
    public:
        ContourHelper() = delete;

        // 查找轮廓：输入为二值图（非零为前景）
        static std::vector<std::vector<cv::Point>> find(const cv::Mat& binary,
                                                        int mode = cv::RETR_EXTERNAL,
                                                        int method = cv::CHAIN_APPROX_SIMPLE);

        // 绘制轮廓：filled=true 填充；否则按 thickness 绘制
        static void draw(cv::Mat& img, const std::vector<std::vector<cv::Point>>& contours, bool filled = false,
                         const cv::Scalar& color = cv::Scalar(0,255,0), int thickness = 1);
    };

    class DrawHelper {
    public:
        DrawHelper() = delete;

        // 直线
        static void line(cv::Mat& img, cv::Point p1, cv::Point p2, const cv::Scalar& color, int thickness = 1, int lineType = cv::LINE_AA);

        // 矩形
        static void rect(cv::Mat& img, const cv::Rect& rc, const cv::Scalar& color, int thickness = 1, int lineType = cv::LINE_AA);

        // 圆
        static void circle(cv::Mat& img, cv::Point c, int r, const cv::Scalar& color, int thickness = 1, int lineType = cv::LINE_AA);

        // 多边形（filled=true 填充）
        static void polygon(cv::Mat& img, const std::vector<cv::Point>& pts, const cv::Scalar& color, bool filled = false, int thickness = 1, int lineType = cv::LINE_AA);

        // 文本绘制
        static void text(cv::Mat& img, const std::string& txt, cv::Point pos, double scale = 1.0, const cv::Scalar& color = cv::Scalar(0,255,0), int thickness = 1);
    };

// ========================= 测量与拟合 =========================
    class MeasureHelper {
    public:
        MeasureHelper() = delete;

        // 面积
        static double area(const std::vector<cv::Point>& c);

        // 周长（closed=true 视为闭合）
        static double length(const std::vector<cv::Point>& c, bool closed);

        // 图像矩
        static cv::Moments moments(const std::vector<cv::Point>& c);

        // 质心（由矩计算，空或零面积返回 (-1,-1)）
        static cv::Point2d centroid(const std::vector<cv::Point>& c);

        // 直线拟合（返回 Vec4f: [vx, vy, x0, y0]）
        static cv::Vec4f fitLine(const std::vector<cv::Point>& pts, int distType = cv::DIST_L2, double param = 0, double reps = 0.01, double aeps = 0.01);

        // 椭圆拟合（点数>=5）
        static cv::RotatedRect fitEllipse(const std::vector<cv::Point>& pts);

        // 外接矩形
        static cv::Rect boundingBox(const std::vector<cv::Point>& c);
    };

// ========================= 颜色与通道 =========================
    class ColorHelper {
    public:
        ColorHelper() = delete;

        // 颜色空间转换，如 cv::COLOR_BGR2HSV
        static cv::Mat convert(const cv::Mat& img, int code);

        // 通道拆分
        static std::vector<cv::Mat> splitChannels(const cv::Mat& img);

        // 通道合并
        static cv::Mat mergeChannels(const std::vector<cv::Mat>& ch);

        // HSV 区间掩码：输入自动转 HSV
        static cv::Mat hsvRange(const cv::Mat& img, const cv::Scalar& low, const cv::Scalar& high);
    };

// ========================= 特征与匹配 =========================
    class FeatureHelper {
    public:
        FeatureHelper() = delete;

        // ORB 特征检测与描述；nfeatures 缺省 500
        static void orb(const cv::Mat& img, std::vector<cv::KeyPoint>& kps, cv::Mat& desc, int nfeatures = 500);

        // 暴力匹配（汉明距离 + 交叉校验）
        static std::vector<cv::DMatch> matchBf(const cv::Mat& d1, const cv::Mat& d2, int normType = cv::NORM_HAMMING, bool crossCheck = true);

        // 绘制匹配结果图
        static cv::Mat drawMatchesImage(const cv::Mat& img1, const std::vector<cv::KeyPoint>& kp1,
                                        const cv::Mat& img2, const std::vector<cv::KeyPoint>& kp2,
                                        const std::vector<cv::DMatch>& matches);
    };

} // namespace cvh


#endif //CX_CT_X2_OPENCV_HELPER_H
