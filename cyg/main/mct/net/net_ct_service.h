#ifndef CX_CT_X2_NET_CT_SERVICE_H
#define CX_CT_X2_NET_CT_SERVICE_H


#include "net_tcp_server.h"



class NetCtService
{
public:

    // 设置服务器地址信息
    static void start(int port = DEFAULT_CT_SERVICE_PORT);

    static void stop();

    static NetTcpServer *tcpServer();

};


#endif //CX_CT_X2_NET_CT_SERVICE_H
