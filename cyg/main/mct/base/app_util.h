#ifndef MCT_UTIL_H
#define MCT_UTIL_H

#include <QString>

namespace AppUtils
{
    // 四视图布局
    enum class FourViewLayout
    {
        Grid2x2,
        OneAndThree
    };

    namespace Prefs
    {
        static const char *kLastPage = "ui/last_page";
        static const char *kLayoutMode = "ui/layout_mode";
        static const char *kPanelVis = "ui/panel/%1/visible"; // 格式化
        static const char *kLogLevel = "ui/log/level";
    }

}


#endif // MCT_UTIL_H
