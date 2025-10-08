#include <string.h>
#include "dlt645-2007.h"
#include "dlt645-private.h"
#include "dlt645-2007-private.h"


CDlt645_2007::CDlt645_2007(ChargeTestorInter *charge_testor_inter) : IDlt645(charge_testor_inter)
{

}

CDlt645_2007::~CDlt645_2007()
{

}

void CDlt645_2007::DoParsingMessageImpl(const uint8_t *message, int len)
{
    if (recv_check((uint8_t*)message, len) < 0)
    {
        return;
    }
    uint8_t code = message[DL645_CONTROL_POS];
    if (code == C_2007_CODE_RD)
    {
        read_data_reply((uint8_t *)message, len);
    }
}

int CDlt645_2007::recv_check(uint8_t *msg, int len)
{
    if (IDlt645::common_check(msg, len) < 0)
    {
        return -1;
    }

    return 0;
}

void CDlt645_2007::read_data_reply(uint8_t *msg, int len)
{
    DataItem item;
    uint32_t code;
    uint8_t addr[6];
    int index = 0;
    uint8_t sendBuf[DL645_RESP_LEN];

    memcpy(addr, msg + 1,6);
    code = (msg[DL645_DATA_POS] - 0x33) + ((msg[DL645_DATA_POS + 1] -0x33) << 8) + ((msg[DL645_DATA_POS + 2] -0x33) << 16) + ((msg[DL645_DATA_POS + 3] -0x33) << 24);
    if (generate_data(code, item) < 0)
    {
        return ;
    }

    memcpy(sendBuf + 1, addr, 6);
    index += DL645_CONTROL_POS;
    sendBuf[index++] = 0x91;
    sendBuf[index++] = item.cont * item.size + 4;
    sendBuf[index++] = msg[DL645_DATA_POS];
    sendBuf[index++] = msg[DL645_DATA_POS+1];
    sendBuf[index++] = msg[DL645_DATA_POS+2];
    sendBuf[index++] = msg[DL645_DATA_POS+3];
    for (int i = 0; i < item.cont; ++i)
    {
        item.data[i] = dec_to_bcd(item.data[i]);
        for (int j = 0; j < item.size; ++j)
        {
            sendBuf[index++] = (item.data[i] & 0xff) + 0x33;
            item.data[i] = item.data[i] >> 8;
        }
    }
    index += item.cont * item.size;
    send_msg(sendBuf, index);
}

int CDlt645_2007::generate_data(uint32_t in_code, DataItem &out_data)
{
    int res = 0;
    double data;
    switch (in_code)
    {
    case DIC_2010100:
        IDlt645::_pChargeTestorInter->GetData(ChargeTestorInter::kDlt645AVoltage, data);
        out_data.data[0] = data * 10;
        out_data.cont = 1;
        res = out_data.size = 2;
        break;
    case DIC_2010200:
        IDlt645::_pChargeTestorInter->GetData(ChargeTestorInter::kDlt645BVoltage, data);
        out_data.data[0] = data * 10;
        out_data.cont = 1;
        res = out_data.size = 2;
        break;
    case DIC_2010300:
        IDlt645::_pChargeTestorInter->GetData(ChargeTestorInter::kDlt645CVoltage, data);
        out_data.data[0] = data * 10;
        out_data.cont = 1;
        res = out_data.size = 2;
        break;
    case DIC_2020100:
        IDlt645::_pChargeTestorInter->GetData(ChargeTestorInter::kDlt645ACurrent, data);
        out_data.data[0] = data * 1000;
        out_data.cont = 1;
        res = out_data.size = 3;
        break;
    case DIC_2020200:
        IDlt645::_pChargeTestorInter->GetData(ChargeTestorInter::kDlt645BCurrent, data);
        out_data.data[0] = data * 1000;
        out_data.cont = 1;
        res = out_data.size = 3;
        break;
    case DIC_2020300:
        IDlt645::_pChargeTestorInter->GetData(ChargeTestorInter::kDlt645CCurrent, data);
        out_data.data[0] = data * 1000;
        out_data.cont = 1;
        res = out_data.size = 3;
        break;

    default:
        res = -1;
        break;
    }

    return res;
}

