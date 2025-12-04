#ifndef CX_CT_X2_TCP_SERVER_H
#define CX_CT_X2_TCP_SERVER_H


#include <unordered_map>
#include <mutex>
#include <thread>
#include <atomic>
#include <vector>
#include <memory>
#include <string>
#include "i_log_sink.h"
#include "i_session_callbacks.h"
#include "Protocol.h"
#include "client_session.h"

#include <ccxx/cxsocket.h>


class TcpServer
{
public:
    TcpServer(const ServerOptions& sopt,
              const SessionOptions& copt,
              INetLogSink* log,
              ISessionCallbacks* cb);
    ~TcpServer();


    bool start();
    void stop();
    void join();


    bool isRunning() const { return running_.load(); }


    size_t clientCount() const;
    std::vector<std::string> listClientIds() const;
    bool closeClient(const std::string& clientId);

private:
    void acceptLoop();
    std::string nextClientId();

private:
    ServerOptions sopt_{};
    SessionOptions copt_{};
    INetLogSink* log_ = nullptr;
    ISessionCallbacks* cb_ = nullptr;


    cx::socket_t listenSo_ = INVALID_SOCKET;
    std::thread acceptThread_;
    std::atomic<bool> running_{false};
    std::atomic<bool> stopping_{false};

    mutable std::mutex mtx_;
    std::unordered_map<std::string, std::shared_ptr<ClientSession>> sessions_;
    uint64_t clientSeq_ = 0;
};


#endif //CX_CT_X2_TCP_SERVER_H
