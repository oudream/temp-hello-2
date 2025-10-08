#ifndef HELLO_IEC104_POINT_CSV_H
#define HELLO_IEC104_POINT_CSV_H


#include <string>
#include "point_manager.h"


class PointCsv
{
public:
    static void start();

    static std::string toCsv(const std::vector<YxPoint> &yxPoints, const std::vector<YcPoint> &ycPoints);

};


#endif //HELLO_IEC104_POINT_CSV_H
