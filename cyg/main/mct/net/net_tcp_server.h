#ifndef CX_CT_X2_NET_TCP_SERVER_H
#define CX_CT_X2_NET_TCP_SERVER_H


#include "net_client_session.h"


constexpr  int DEFAULT_CT_SERVICE_PORT = 7362;


class NetTcpServer
{
public:
    struct ServerOptions {
        std::string bindIp = "0.0.0.0";
        uint16_t port = DEFAULT_CT_SERVICE_PORT;
        int maxClients = 64;
        bool tcpNoDelay = false;
        bool keepAlive = false;
        int recvTimeoutMs = 30'000; // 30s
        int sendTimeoutMs = 30'000; // 30s
    };

    NetTcpServer();
    ~NetTcpServer();


    bool start(const ServerOptions& opt);
    void stop();
    void join();


    bool isRunning() const { return running_; }


    size_t clientCount() const;
    std::vector<uint64_t> listClientIds() const;
    bool closeClient(uint64_t clientId);

    void stopAllSessions();

private:
    void acceptLoop();
    uint64_t nextClientId();
    void eraseSessionUnsafe(uint64_t key);

private:
    ServerOptions opt_{};

    cx::socket_t listenSo_ = INVALID_SOCKET;
    std::thread acceptThread_;
    std::atomic<bool> running_{false};
    std::atomic<bool> stopping_{false};

    mutable std::mutex mtx_;
    std::unordered_map<uint64_t, std::shared_ptr<NetClientSession>> sessions_;
    uint64_t clientSeq_ = 0;

};



#endif //CX_CT_X2_NET_TCP_SERVER_H
