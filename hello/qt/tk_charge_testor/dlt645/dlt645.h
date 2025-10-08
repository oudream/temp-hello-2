#ifndef HELLO_IEC104_DLT645_H
#define HELLO_IEC104_DLT645_H

#include <stdint.h>
#include "./../adapter.h"


class IDlt645
{
public:
    IDlt645(ChargeTestorInter* charge_testor_inter);
    virtual ~IDlt645();

public:
    virtual void DoParsingMessageImpl(const uint8_t* message, int len) = 0;

protected:
    int common_check(uint8_t *msg, int len);
    int crc(uint8_t *msg, int len);
    int send_msg(uint8_t *msg, int len);
    uint32_t dec_to_bcd(uint32_t val);
    void numToHexCs(const char *in_src, char *out_dec, int len);

protected:
    ChargeTestorInter *_pChargeTestorInter;
};





#endif //HELLO_IEC104_DLT645_H
