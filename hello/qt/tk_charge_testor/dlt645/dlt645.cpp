#include <stdio.h>
#include <string.h>
#include "dlt645.h"
#include "dlt645-private.h"

IDlt645::IDlt645(ChargeTestorInter *charge_testor_inter)
{
    _pChargeTestorInter = charge_testor_inter;
}

IDlt645::~IDlt645()
{

}

int IDlt645::common_check(uint8_t *msg, int len)
{
    //数据包长度校验
    if (len < 7)
    {
        return -1;
    }
    //数据帧标志校验
    if (msg[0] != DL645_START_CODE ||
        msg[DL645_ADDR_LEN + 1] != DL645_START_CODE ||
        msg[len - 1] != DL645_STOP_CODE)
    {
        IDlt645::_pChargeTestorInter->DoShowWindowImpl("check code error!\n");
        return -1;
    }
    //CRC校验
    uint8_t _crc = crc(msg, len - 2);
    if (_crc != msg[len - 2])
    {
        IDlt645::_pChargeTestorInter->DoShowWindowImpl("check crc error!\n");
        return -1;
    }
    //控制码主从校验
    if ((msg[DL645_CONTROL_POS] & C_TD_MASK) == (C_TD_SLAVE << C_TD_POS))
    {
        IDlt645::_pChargeTestorInter->DoShowWindowImpl("check control direction error!\n");
        return -1;
    }
    //请求地址校验
    uint8_t addr[6];
    numToHexCs((char *)msg + 1, (char *)addr, 6);
    if (IDlt645::_pChargeTestorInter->AddrCheck((char *)addr, ChargeTestorInter::kProDlt645) < 0)
    {
        char buff[255] = {0};
        sprintf(buff, "No electricity meter with address %s!\n", (char *)addr);
        IDlt645::_pChargeTestorInter->DoShowWindowImpl(buff);
        return -1;
    }
    return 0;
}

int IDlt645::crc(uint8_t *msg, int len)
{
    uint8_t crc = 0;
    while (len--)
    {
        crc += *msg++;
    }
    return crc;
}

int IDlt645::send_msg(uint8_t *msg, int len)
{
    msg[0] = DL645_START_CODE;
    msg[DL645_ADDR_LEN + 1] = DL645_START_CODE;
    msg[len - 1] = DL645_STOP_CODE;
    msg[len - 2] = crc(msg, len - 2);
    return IDlt645::_pChargeTestorInter->DoSendData((char *)msg, len);
}

/**
 * Name:    dec2bcd
 * Brief:   十进制转BCD码（目前支持32位数字大小）
 * Input:
 *  @val:   十进制值
 * Output:  BCD码值
 */
uint32_t IDlt645::dec_to_bcd(uint32_t val)
{
    uint32_t data = 0;

    if (val < 100)
    {
        uint8_t byte0 = val % 10;
        uint8_t byte1 = val / 10;
        data = (byte1 << 4) + byte0;
    }
    else if (val < 10000)
    {
        uint8_t byte0 = val % 10;
        uint8_t byte1 = (val / 10) % 10;
        uint8_t byte2 = (val / 100) % 10;
        uint8_t byte3 = (val / 1000) % 10;
        data = (byte3 << 12) +
               (byte2 << 8) +
               (byte1 << 4) + byte0;
    }
    else if (val < 1000000)
    {
        uint8_t byte0 = val % 10;
        uint8_t byte1 = (val / 10) % 10;
        uint8_t byte2 = (val / 100) % 10;
        uint8_t byte3 = (val / 1000) % 10;
        uint8_t byte4 = (val / 10000) % 10;
        uint8_t byte5 = (val / 100000) % 10;
        data = (byte5 << 20) +
               (byte4 << 16) +
               (byte3 << 12) +
               (byte2 << 8) +
               (byte1 << 4) + byte0;
    }
    else if (val < 100000000)
    {
        uint8_t byte0 = val % 10;
        uint8_t byte1 = (val / 10) % 10;
        uint8_t byte2 = (val / 100) % 10;
        uint8_t byte3 = (val / 1000) % 10;
        uint8_t byte4 = (val / 10000) % 10;
        uint8_t byte5 = (val / 100000) % 10;
        uint8_t byte6 = (val / 1000000) % 10;
        uint8_t byte7 = (val / 10000000) % 10;
        data = (byte7 << 28) +
               (byte6 << 24) +
               (byte5 << 20) +
               (byte4 << 16) +
               (byte3 << 12) +
               (byte2 << 8) +
               (byte1 << 4) + byte0;
    }
    return data;
}

void IDlt645::numToHexCs(const char *in_src, char *out_dec, int len)
{
    for(int i = 0; i < len; i++)
    {
        char tmp = (in_src[i] >> 4) & 0x0F;
        if (0 <= tmp && tmp <= 9)
        {
            out_dec[i*2] = tmp + '0';
        }
        else if (10 <= tmp && tmp <= 16)
        {
            out_dec[i*2] = tmp - 10 + 'A';
        }

        tmp = in_src[i] & 0x0F;
        if (0 <= tmp && tmp <= 9)
        {
            out_dec[i*2+1] = tmp + '0';
        }
        else if (10 <= tmp && tmp <= 16)
        {
            out_dec[i*2+1] = tmp - 10 + 'A';
        }
    }
}
