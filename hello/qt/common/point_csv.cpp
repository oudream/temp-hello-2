#include "point_csv.h"

#include <ccxx/cxarguments.h>
#include <ccxx/cxfile.h>
#include <ccxx/cxinterinfo.h>
#include <ccxx/cxinterinfo_group_define.h>

#include <yaml-cpp/yaml.h>


using namespace std;


CXOUTINFO_DEFINE(IIG_NORMAL, IIG_NORMAL, "POINT-CSV")


static map<string, vector<string>> modelCsvModelAttrs;


std::string fn_getCsvString(const std::vector<YxPoint> &yxPoints, const std::vector<YcPoint> &ycPoints, const string &sDeviceId, const string &sModel)
{
    map<string, vector<string>>::const_iterator it = modelCsvModelAttrs.find(sModel);
    if (it == modelCsvModelAttrs.end())
    {
        return string();
    }
    const vector<string> &csvModelAttrs = it->second;
    stringstream ss;
    ss.precision(3);
    ss.setf(std::ios::fixed);
    ss << sDeviceId << ",";
    for (int i = 0; i < csvModelAttrs.size(); ++i)
    {
        const string &attr = csvModelAttrs.at(i);
        bool bExist = false;
        for (int j = 0; j < ycPoints.size(); ++j)
        {
            const YcPoint &yc = ycPoints.at(j);

            if (yc.DEVICE_ATTR == attr && yc.TABLE_NAME.rfind(sDeviceId) != string::npos && yc.MODEL == sModel)
            {
                if (yc.DEVICE_ATTR == C_ONTIME_TABLE_ATTR_PHASE_INFO)
                {
                    if (yc.VALUE == 0)
                    {
                        ss << "ABC" << ",";
                        bExist = true;
                        break;
                    }
                    else if(yc.VALUE == 1)
                    {
                        ss << "A" << ",";
                        bExist = true;
                        break;
                    }
                    else if (yc.VALUE == 2)
                    {
                        ss << "B" << ",";
                        bExist = true;
                        break;
                    }
                    else if (yc.VALUE == 3)
                    {
                        ss << "C" << ",";
                        bExist = true;
                        break;
                    }
                    else
                    {
                        ss << "RST" << ",";
                        bExist = true;
                        break;
                    }
                }
                ss << yc.VALUE << ",";
                bExist = true;
                break;
            }
        }
        if (!bExist)
        {
            ss << ",";
        }
    }
    string r = ss.str();
    if (r.size() > 0)
    {
        r[r.size()-1] = '\r';
        r.push_back('\n');
    }
    return r;
}

void PointCsv::start()
{
    string sAppConfig = CxArguments::getArg0FileName() + ".yaml";
    if (CxFileSystem::isExist(sAppConfig))
    {
        fnInfo() << "Load " << sAppConfig;
        YAML::Node config = YAML::LoadFile(sAppConfig);
        try
        {
            YAML::Node csv = config["csv"];
            if (!csv)
            {
                fnInfo() << "But Can Not Find Node 'adc'.";
                return;
            }
            YAML::Node models = csv["models"];
            if (models)
            {
                fnInfo() << "models:";
                for (YAML::const_iterator it = models.begin(); it != models.end(); ++it)
                {
                    YAML::Node modelItem = *it;
                    string sName = modelItem["name"].as<string>();
                    string sModel = modelItem["model"].as<string>();
                    modelCsvModelAttrs[sName] = CxString::split(sModel, ',');
                    fnInfo() << "name: " << sName << ", model" << sModel;
                }
            }
        }
        catch (...)
        {
            fnError() << "LoadFile fail." << sAppConfig;
            return;
        }
    }
}

string PointCsv::toCsv(const std::vector<YxPoint> &yxPoints, const std::vector<YcPoint> &ycPoints)
{
    std::string r;

    std::map<std::string, std::string> deviceModels;

    for (int i = 0; i < ycPoints.size(); i++)
    {
        const YcPoint &currentYC = ycPoints.at(i);
        std::string currentDeviceID = PointManager::getCharLastString(currentYC.TABLE_NAME, '_');

        fnInfo().out(".YCToString - currentDeviceID: [%s]", currentDeviceID.c_str());

        if (!currentDeviceID.empty() && ! CxContainer::contain(deviceModels, currentDeviceID))
        {
            deviceModels[currentDeviceID] = currentYC.MODEL;
        }
    }

    for (std::map<std::string, std::string>::iterator it = deviceModels.begin(); it != deviceModels.end(); it++)
    {
        string sCsv = fn_getCsvString(yxPoints, ycPoints, it->first, it->second);
        if (! sCsv.empty())
        {
            r += sCsv;
        }
    }
    fnInfo().out(".YCToString - nextDeviceID: [%s]", r.c_str());
    return r;
}

