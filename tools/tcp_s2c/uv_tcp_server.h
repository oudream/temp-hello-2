#ifndef UV_TCP_SERVER_H
#define UV_TCP_SERVER_H

#include <uv.h>
#include <string>


#ifdef _WIN32
#define LX_PTHREAD_MUTEX_INITIALIZER   {0}
#else
#define LX_PTHREAD_MUTEX_INITIALIZER   PTHREAD_MUTEX_INITIALIZER
#endif


class UVTcpServer
{
public:
    static int start(int iListenPort);

    static int pushData(const unsigned char *pData, int iLength);

    static void stop();

};


#endif //UV_TCP_SERVER_H
