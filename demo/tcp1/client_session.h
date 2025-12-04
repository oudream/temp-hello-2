#ifndef CX_CT_X2_CLIENT_SESSION_H
#define CX_CT_X2_CLIENT_SESSION_H


#include <atomic>
#include <thread>
#include <vector>
#include <memory>
#include <chrono>
#include <cstdint>
#include <string>
#include "i_log_sink.h"
#include "i_session_callbacks.h"
#include "Protocol.h"
#include <ccxx/cxsocket.h>


class ClientSession : public std::enable_shared_from_this<ClientSession>
{
public:
    ClientSession(cx::socket_t sock,
                  const SessionOptions& sopt,
                  INetLogSink* log,
                  ISessionCallbacks* cb,
                  std::string clientId);
    ~ClientSession();


    void start();
    void stop();
    void join();


    bool sendLine(std::string_view line);
    bool sendBytes(const uint8_t* data, size_t n);


    uint64_t bytesRecv() const { return bytesRecv_; }
    uint64_t bytesSent() const { return bytesSent_; }
    std::chrono::steady_clock::time_point lastActive() const { return lastActive_; }
    const std::string& id() const { return clientId_; }

private:
    void runLoop();
    bool readLine(std::string& out);
    bool readExact(uint8_t* buf, size_t n);

private:
    cx::socket_t so_ = INVALID_SOCKET;
    SessionOptions sopt_{};
    INetLogSink* log_ = nullptr;
    ISessionCallbacks* cb_ = nullptr;
    std::string clientId_;


    std::thread worker_;
    std::atomic<bool> stopping_{false};


    std::vector<uint8_t> dataBuf_;


    std::atomic<uint64_t> bytesRecv_{0};
    std::atomic<uint64_t> bytesSent_{0};
    std::atomic<std::chrono::steady_clock::time_point> lastActive_;

};


#endif //CX_CT_X2_CLIENT_SESSION_H
