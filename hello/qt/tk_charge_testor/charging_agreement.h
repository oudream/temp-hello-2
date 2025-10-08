#ifndef HELLO_IEC104_CHARGING_H
#define HELLO_IEC104_CHARGING_H

#include <iostream>
#include <ccxx/cxchannel_serial.h>

#include "adapter.h"

typedef struct tagChargingData
{
    std::string chargingLogo;
    double minPower;
    double ratedPower;
    int putGunState;
    int chargingState;
    double chargingPower;
    double voltage;
    double current;
    double electricMeterTotal;
    double electricMeterTip;
    double electricMeterPeak;
    double electricMeterFlat;
    double electricMeterValley;
}CHARGING_DATA;

class ChargingAgree
{
public:
    ChargingAgree();
    ~ChargingAgree();

    void setChargeInter(ChargeTestorInter *chargeInter);
    void OnParsing(const uchar *buff, int len, const CHARGING_DATA &chargingData);

private:
    void getMeasuringQueryPackage(const CHARGING_DATA &chargingData, std::string &out_buff);
    void getChargingApproachPackage(const CHARGING_DATA &chargingData, std::string &out_buff);

private:
    ChargeTestorInter *_chargeInter;
};


#endif //HELLO_IEC104_CHARGING_H
