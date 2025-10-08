#ifndef HELLO_IEC104_DLT645_1997_H
#define HELLO_IEC104_DLT645_1997_H

#include "dlt645.h"

class CDlt645_1997 : public IDlt645
{
public:
    CDlt645_1997(ChargeTestorInter* charge_testor_inter);
    ~CDlt645_1997();

public:
    void DoParsingMessageImpl(const uint8_t* message, int len);

private:
    int recv_check(uint8_t *msg, int len);
    void read_data_reply(uint8_t *msg, int len);
    int generate_data(uint32_t in_code, uint32_t &out_data);
};

#endif //HELLO_IEC104_DLT645_1997_H
