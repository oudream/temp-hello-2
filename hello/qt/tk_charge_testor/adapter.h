#ifndef HELLO_IEC104_ADAPTER_H
#define HELLO_IEC104_ADAPTER_H

class ChargeTestorInter
{
public:
    typedef enum __protocolType
    {
        kProDlt645,
    }ProtocolType;

    typedef enum __dlt645DataType
    {
        // 电压
        kDlt645AVoltage,
        kDlt645BVoltage,
        kDlt645CVoltage,
        // 电流
        kDlt645ACurrent,
        kDlt645BCurrent,
        kDlt645CCurrent,
        // 有功功率
        kDlt645AActivePower,
        kDlt645BActivePower,
        kDlt645CActivePower,
        kDlt645TActivePower,
        // 无功功率
        kDlt645AWattlessPower,
        kDlt645BWattlessPower,
        kDlt645CWattlessPower,
        kDlt645TWattlessPower,
        // 功率因数
        kDlt645APowerFactor,
        kDlt645BPowerFactor,
        kDlt645CPowerFactor,
        kDlt645TPowerFactor,
        // 电量
        kDlt645AElectricEnergy,
        kDlt645BElectricEnergy,
        kDlt645CElectricEnergy,
        kDlt645TElectricEnergy,
    }Dlt645DataType;
public:
    virtual int DoSendData(const char* pData, int iLength) = 0;
    virtual void DoShowWindowImpl(const char* pStr) = 0;
    virtual int AddrCheck(const char* addr, ProtocolType pro_type) = 0;
    virtual int GetData(Dlt645DataType data_type, double &out_data) = 0;

};

#endif //HELLO_IEC104_ADAPTER_H
