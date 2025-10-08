#ifndef HELLO_IEC104_DLT645_2007_H
#define HELLO_IEC104_DLT645_2007_H

#include "dlt645.h"

#define DATA_ITEM_MAX_LEN  10

typedef struct __data_item
{
    int cont;                               // 一共有多少个数据
    int size;                               // 一个数据需要的字节数
    int data[DATA_ITEM_MAX_LEN];         // 实际数据的存放
}DataItem;

class CDlt645_2007 : public IDlt645
{
public:
    CDlt645_2007(ChargeTestorInter* charge_testor_inter);
    ~CDlt645_2007();
public:
    void DoParsingMessageImpl(const uint8_t* message, int len);

private:
    int recv_check(uint8_t *msg, int len);
    void read_data_reply(uint8_t *msg, int len);
    int generate_data(uint32_t in_code, DataItem &out_data);
};

#endif //HELLO_IEC104_DLT645_2007_H
