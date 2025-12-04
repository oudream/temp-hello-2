#ifndef CX_CT_X2_NET_CLIENT_SESSION_CUH
#define CX_CT_X2_NET_CLIENT_SESSION_CUH


#include <ccxx/cxsocket.h>

#include <thread>
#include <mutex>


class NetClientSession : public std::enable_shared_from_this<NetClientSession>
{
public:
    NetClientSession(cx::socket_t sock, uint64_t clientId);
    ~NetClientSession();


    void start();
    void stop();
    void join();


    bool sendLine(std::string_view line);
    bool sendBytes(const uint8_t* data, size_t n);


    uint64_t bytesRecv() const { return bytesRecv_; }
    uint64_t bytesSent() const { return bytesSent_; }
    std::chrono::steady_clock::time_point lastActive() const { return lastActive_; }
    uint64_t id() const { return clientId_; }


    // 当 run() 退出时触发（一定会调用一次）
    void setOnExit(std::function<void()> cb) { onExit_ = std::move(cb); } // +++

private:
    void runLoop();

private:
    cx::socket_t so_ = INVALID_SOCKET;
    uint64_t clientId_;


    std::thread worker_;
    std::atomic<bool> stopping_{false};


    std::vector<uint8_t> dataBuf_;


    std::atomic<uint64_t> bytesRecv_{0};
    std::atomic<uint64_t> bytesSent_{0};
    std::atomic<std::chrono::steady_clock::time_point> lastActive_;

    std::function<void()> onExit_;

};



#endif //CX_CT_X2_NET_CLIENT_SESSION_CUH
