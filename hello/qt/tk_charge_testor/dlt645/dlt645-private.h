#ifndef _DLT645_PRIVATE_H
#define _DLT645_PRIVATE_H

#include "dlt645.h"

#ifdef DLT645_DEBUG
#define DLT645_LOG kprintf
#else
#define DLT645_LOG(...)
#endif

#define DL645_START_CODE 0x68
#define DL645_STOP_CODE 0x16

#define DL645_PREMBLE_ENABLE 0 //前导码使能标记
#define DL645_PREMBLE 0xFE     //前导码

#define DL645_ADDR_LEN 6    //设备地址长度
#define DL645_CONTROL_POS 8 //控制码位置
#define DL645_LEN_POS 9     //长度位置
#define DL645_DATA_POS 10   //数据位置
#define DL645_WR_LEN 50     //写入数据命令的长度
#define DL645_RESP_LEN 60   //读取数据命令的长度

#define C_TD_MASK 0x80      //主从标志位
#define C_TD_POS 7          //主从标志位比特位
#define C_TD_MASTER 0       //主站发出的命令帧
#define C_TD_SLAVE 1        //从站发出的应答帧

#define C_ACK_MASK 0x40     //从站是否正确应答标志位
#define C_ACK_POS 6         //从站应答标志位比特位
#define C_ACK_OK 0          //从站应答正确
#define C_ACK_ERR 1         //从站应答错误

#define C_FU_MASK 0x20      //是否有后续帧标志位
#define C_FU_POS 5          //后续帧标志位比特位
#define C_FU_NONE 0         //无后续帧
#define C_FU_EXT 1          //有后续帧

#define C_CODE_MASK 0x1F    //功能码标志位


#endif
