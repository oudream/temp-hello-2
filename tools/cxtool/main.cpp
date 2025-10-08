#include <ccxx/cxapplication.h>
#include <ccxx/cxtimer.h>


using namespace std;


CXOUTINFO_DEFINE(IIG_NORMAL, IIG_NORMAL, "MAIN")

static void mainTimerTimeout(int iInterval)
{
    fnInfo() << "mainTimerTimeout";
}

int main(int argc, const char *argv[])
{
    CxApplication::init(argc, argv);
    string sFun = CxArguments::getArg1();
    bool bExe = false;
    if (sFun == "date")
    {
        fnInfo() << CxTime::currentSystemTimeString();
        fnInfo() << CxTime::currentSepochString();
        fnInfo() << CxTime::currentMsepoch();
        fnInfo() << CxTime::currentSepoch();
    }
    else if (sFun == "timer")
    {
        CxTimerManager::startTimer(mainTimerTimeout, (1000*3));
        bExe = true;
    }

    if (bExe)
    {
        return CxApplication::exec();
    }
    else
    {
        CxApplication::exit();
        return 0;
    }
}
