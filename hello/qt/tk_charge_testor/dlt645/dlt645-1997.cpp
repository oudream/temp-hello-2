#include <string.h>
#include "dlt645-1997.h"
#include "dlt645-private.h"
#include "dlt645-1997-private.h"
#include "dlt645.h"


CDlt645_1997::CDlt645_1997(ChargeTestorInter *charge_testor_inter) : IDlt645(charge_testor_inter)
{

}

CDlt645_1997::~CDlt645_1997()
{

}

void CDlt645_1997::DoParsingMessageImpl(const uint8_t *message, int len)
{
    if (recv_check((uint8_t*)message, len) < 0)
    {
        return;
    }
    uint8_t code = message[DL645_CONTROL_POS];
    if (code == C_1997_CODE_RD)
    {
        read_data_reply((uint8_t *)message, len);
    }
}

int CDlt645_1997::recv_check(uint8_t *msg, int len)
{
    if (IDlt645::common_check(msg, len) < 0)
    {
        return -1;
    }

    return 0;
}

void CDlt645_1997::read_data_reply(uint8_t *msg, int len)
{
    uint32_t data;
    int dLen = 0;
    uint32_t code;
    uint8_t addr[6];
    int index = 0;
    uint8_t sendBuf[DL645_RESP_LEN];

    memcpy(addr, msg + 1,6);
    code = (msg[DL645_DATA_POS] - 0x33) + ((msg[DL645_DATA_POS + 1] -0x33) << 8);
    dLen = generate_data(code, data);
    data = dec_to_bcd(data);

    memcpy(sendBuf+1, addr, 6);
    index += DL645_CONTROL_POS;
    sendBuf[index++] = 0x81;
    sendBuf[index++] = dLen + 2;
    sendBuf[index++] = (code & 0xff) + 0x33;
    sendBuf[index++] = ((code >> 8) & 0xff) + 0x33;
    for (int i = 0; i < dLen; ++i)
    {
        sendBuf[index++] = (data & 0xff) + 0x33;
        data = data >> 8;
    }
    index += dLen;
    send_msg(sendBuf, index);
}
/**
 *  Name:
 *      generate_data
 *  Brief:
 *      根据 code 得到需要发送的数据和数据的字节数
 *  Input:
 *      @param in_code 需要获取数据的类型
 *  Output:
 *      @param out_data 保存实际的数据
 *      @return size 返回数据的字节数
**/
int CDlt645_1997::generate_data(uint32_t in_code, uint32_t &out_data)
{
    int size = 0;
    double data;
    switch (in_code)
    {
    case DIC_B611:
        IDlt645::_pChargeTestorInter->GetData(ChargeTestorInter::kDlt645AVoltage, data);
        out_data = data;
        size = 2;
        break;
    case DIC_B612:
        IDlt645::_pChargeTestorInter->GetData(ChargeTestorInter::kDlt645BVoltage, data);
        out_data = data;
        size = 2;
        break;
    case DIC_B613:
        IDlt645::_pChargeTestorInter->GetData(ChargeTestorInter::kDlt645CVoltage, data);
        out_data = data;
        size = 2;
        break;
    case DIC_B621:
        IDlt645::_pChargeTestorInter->GetData(ChargeTestorInter::kDlt645ACurrent, data);
        out_data = data * 100;
        size = 2;
        break;
    case DIC_B622:
        IDlt645::_pChargeTestorInter->GetData(ChargeTestorInter::kDlt645BCurrent, data);
        out_data = data * 100;
        size = 2;
        break;
    case DIC_B623:
        IDlt645::_pChargeTestorInter->GetData(ChargeTestorInter::kDlt645CCurrent, data);
        out_data = data * 100;
        size = 2;
        break;
    default:
        break;
    }

    return size;
}





