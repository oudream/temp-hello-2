#include "charging_agreement.h"

#include <ccxx/cxcrc.h>


ChargingAgree::ChargingAgree()
{

}

ChargingAgree::~ChargingAgree()
{

}

void ChargingAgree::OnParsing(const uchar *buff, int len, const CHARGING_DATA &chargingData)
{
    std::string sendBuff;

    if (buff[0] != 0x68)
    {
        return;
    }

    if (CxCrc::calcsum8(&buff[1], buff[4]+4) != buff[len-2])
    {
        return;
    }

    switch (buff[3])
    {
        case 0x0D:
            getMeasuringQueryPackage(chargingData, sendBuff);
            break;
        case 0x0B:
            getChargingApproachPackage(chargingData, sendBuff);
            break;
    }

    _chargeInter->DoSendData(sendBuff.c_str(), sendBuff.length());
}

void ChargingAgree::setChargeInter(ChargeTestorInter *chargeInter)
{
    _chargeInter = chargeInter;
}

void ChargingAgree::getMeasuringQueryPackage(const CHARGING_DATA &chargingData, std::string &out_buff)
{
    int tmp;

    out_buff = {0x68, 0x01, 0x00};
    out_buff += 0x0E;
    out_buff += 0x23;
    out_buff += {0x00};
    out_buff += 0x01;

    tmp = chargingData.voltage * 100;
    out_buff += (tmp >> 24) & 0xff;
    out_buff += (tmp >> 16) & 0xff;
    out_buff += (tmp >> 8) & 0xff;
    out_buff += tmp & 0xff;

    tmp = chargingData.current * 100;
    out_buff += (tmp >> 24) & 0xff;
    out_buff += (tmp >> 16) & 0xff;
    out_buff += (tmp >> 8) & 0xff;
    out_buff += tmp & 0xff;

    tmp = chargingData.chargingPower * 100;
    out_buff += (tmp >> 24) & 0xff;
    out_buff += (tmp >> 16) & 0xff;
    out_buff += (tmp >> 8) & 0xff;
    out_buff += tmp & 0xff;

    tmp = chargingData.electricMeterTotal * 100;
    out_buff += (tmp >> 24) & 0xff;
    out_buff += (tmp >> 16) & 0xff;
    out_buff += (tmp >> 8) & 0xff;
    out_buff += tmp & 0xff;

    tmp = chargingData.electricMeterTip * 100;
    out_buff += (tmp >> 24) & 0xff;
    out_buff += (tmp >> 16) & 0xff;
    out_buff += (tmp >> 8) & 0xff;
    out_buff += tmp & 0xff;

    tmp = chargingData.electricMeterPeak * 100;
    out_buff += (tmp >> 24) & 0xff;
    out_buff += (tmp >> 16) & 0xff;
    out_buff += (tmp >> 8) & 0xff;
    out_buff += tmp & 0xff;

    tmp = chargingData.electricMeterFlat * 100;
    out_buff += (tmp >> 24) & 0xff;
    out_buff += (tmp >> 16) & 0xff;
    out_buff += (tmp >> 8) & 0xff;
    out_buff += tmp & 0xff;

    tmp = chargingData.electricMeterValley * 100;
    out_buff += (tmp >> 24) & 0xff;
    out_buff += (tmp >> 16) & 0xff;
    out_buff += (tmp >> 8) & 0xff;
    out_buff += tmp & 0xff;

    char * p_tmp = const_cast<char *>(out_buff.data());
    out_buff += CxCrc::calcsum8((uchar *)(&p_tmp[1]), p_tmp[4]+4);
    out_buff += 0x16;
}

void ChargingAgree::getChargingApproachPackage(const CHARGING_DATA &chargingData, std::string &out_buff)
{
    out_buff = {0x68, 0x03, 0x00, 0x0C, 0x02, 0x03, 0x03, 0x17, 0x16};
}

