#ifndef CX_CT_X2_TCP_SERVER_H
#define CX_CT_X2_TCP_SERVER_H


class TcpServer {
public:
    explicit TcpServer(uint16_t port, int idle_timeout_sec = 10)
        : port_(port), idle_to_(idle_timeout_sec), running_(false) {}

    bool start() {
        if (running_) return true;
        WSADATA wsa;
        if (WSAStartup(MAKEWORD(2,2), &wsa) != 0) {
            Logger::instance().log(Logger::Level::ERR, "WSAStartup failed.");
            return false;
        }
        srv_ = ::socket(AF_INET, SOCK_STREAM, 0);
        if (srv_ == INVALID_SOCKET) {
            Logger::instance().log(Logger::Level::ERR, "socket() failed.");
            WSACleanup();
            return false;
        }

        // 复用
        BOOL yes = 1;
        setsockopt(srv_, SOL_SOCKET, SO_REUSEADDR, (const char*)&yes, sizeof(yes));

        sockaddr_in addr{}; addr.sin_family = AF_INET; addr.sin_port = htons(port_); addr.sin_addr.s_addr = INADDR_ANY;
        if (::bind(srv_, (sockaddr*)&addr, sizeof(addr)) < 0) {
            Logger::instance().log(Logger::Level::ERR, "bind() failed.");
            ::closesocket(srv_); WSACleanup(); return false;
        }
        if (::listen(srv_, SOMAXCONN) < 0) {
            Logger::instance().log(Logger::Level::ERR, "listen() failed.");
            ::closesocket(srv_); WSACleanup(); return false;
        }

        running_ = true;
        th_ = std::thread([this]{ this->accept_loop(); });
        th_.detach();
        Logger::instance().log(Logger::Level::INF, "Server listening on port " + std::to_string(port_) + " ...");
        return true;
    }

    void stop() {
        if (!running_) return;
        running_ = false;
        ::closesocket(srv_);
        WSACleanup();
        Logger::instance().log(Logger::Level::INF, "Server stopped.");
    }

private:
    void accept_loop() {
        while (running_) {
            sockaddr_in caddr{}; int clen = sizeof(caddr);
            SOCKET cli = ::accept(srv_, (sockaddr*)&caddr, &clen);
            if (cli == INVALID_SOCKET) {
                if (running_) Logger::instance().log(Logger::Level::WAR, "accept failed or server closed.");
                break;
            }
            std::thread(ClientSession(cli, idle_to_)).detach();
        }
    }

    uint16_t port_;
    int idle_to_;
    std::atomic<bool> running_;
    SOCKET srv_{ INVALID_SOCKET };
    std::thread th_;
};

#endif //CX_CT_X2_TCP_SERVER_H