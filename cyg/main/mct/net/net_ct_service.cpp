#include "net_ct_service.h"


#include <helpers/log_helper.h>


#include "net_tcp_server.h"


static NetTcpServer _tcpServer;


void NetCtService::start(int port)
{
    if (_tcpServer.isRunning()) return;

    auto apt = NetTcpServer::ServerOptions{};
    apt.port = port;

    _tcpServer.start(apt);
}

NetTcpServer * NetCtService::tcpServer()
{
    return &_tcpServer;
}

void NetCtService::stop()
{
    // 主动停止并等待所有线程/会话真正退出
    _tcpServer.stop();
    _tcpServer.join();
}
