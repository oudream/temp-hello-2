#include "tcp_server.h"

#include <cstdio>
#include <cstring>


TcpServer::TcpServer(const ServerOptions& sopt,
                     const SessionOptions& copt,
                     ILogSink* log,
                     ISessionCallbacks* cb)
        : sopt_(sopt), copt_(copt), log_(log), cb_(cb) {}


TcpServer::~TcpServer(){ stop(); join(); if(listenSo_!=INVALID_SOCKET) CxSocket::close(listenSo_); }


bool TcpServer::start(){
    if(running_.load()) return true;


// 创建监听 socket（支持 IPv4/IPv6）
    CxIpAddressExtend addr(DEFAULT_FAMILY, sopt_.bindIp.c_str(), std::to_string(sopt_.port).c_str());
    listenSo_ = CxSocket::create(addr, SOCK_STREAM, 0);
    if(listenSo_==INVALID_SOCKET){ if(log_) log_->logError("create listen socket failed"); return false; }


// 绑定并监听
    if(CxSocket::bindto(listenSo_, addr.get())){
        if(log_) log_->logError("bind failed");
        CxSocket::close(listenSo_); listenSo_ = INVALID_SOCKET; return false;
    }
    if(CxSocket::listento(listenSo_, sopt_.listenBacklog)){
        if(log_) log_->logError("listen failed");
        CxSocket::close(listenSo_); listenSo_ = INVALID_SOCKET; return false;
    }


    running_.store(true);
    stopping_.store(false);


    acceptThread_ = std::thread([this]{ acceptLoop(); });
    if(log_) log_->logInfo("TcpServer started");
    return true;
}

void TcpServer::stop()
{
    if (!running_.load()) return;
    stopping_.store(true);
    if (listenSo_ != INVALID_SOCKET)
    {
        CxSocket::close(listenSo_);
    }


    // 关闭所有会话
    std::vector<std::shared_ptr<ClientSession>> v;
    {
        std::lock_guard<std::mutex> lk(mtx_);
        for (auto& kv : sessions_) v.push_back(kv.second);
        sessions_.clear();
    }
    for (auto& s : v) { s->stop(); }
}


void TcpServer::join()
{
    if (acceptThread_.joinable()) acceptThread_.join();
    std::vector<std::shared_ptr<ClientSession>> v;
    {
        std::lock_guard<std::mutex> lk(mtx_);
        for (auto& kv : sessions_) v.push_back(kv.second);
    }
    for (auto& s : v) { s->join(); }
    running_.store(false);
}


size_t TcpServer::clientCount() const
{
    std::lock_guard<std::mutex> lk(mtx_);
    return sessions_.size();
}


std::vector<std::string> TcpServer::listClientIds() const
{
    std::vector<std::string> ids;
    std::lock_guard<std::mutex> lk(mtx_);
    ids.reserve(sessions_.size());
    for (auto& kv : sessions_) ids.push_back(kv.first);
    return ids;
}


bool TcpServer::closeClient(const std::string& clientId)
{
    std::shared_ptr<ClientSession> s;
    {
        std::lock_guard<std::mutex> lk(mtx_);
        auto it = sessions_.find(clientId);
        if (it == sessions_.end()) return false;
        s = it->second;
        sessions_.erase(it);
    }
    if (s)
    {
        s->stop();
        s->join();
    }
    return true;
}


std::string TcpServer::nextClientId()
{
    char buf[64];
    std::snprintf(buf, sizeof(buf), "C%llu", (unsigned long long)++clientSeq_);
    return std::string(buf);
}


void TcpServer::acceptLoop()
{
    while (!stopping_.load())
    {
        struct sockaddr_storage ra{};
        cx::socket_t so = CxSocket::acceptfrom(listenSo_, &ra);
        if (so == INVALID_SOCKET)
        {
            if (stopping_.load()) break;
            continue;
        }


        // 套接字选项
        if (sopt_.recvTimeoutMs > 0) CxSocket::receiveTimeout(so, sopt_.recvTimeoutMs);
        if (sopt_.sendTimeoutMs > 0) CxSocket::sendTimeout(so, sopt_.sendTimeoutMs);
        if (sopt_.tcpNoDelay) CxSocket::nodelay(so);
        if (sopt_.keepAlive) CxSocket::keepalive(so, true);


        // 限流：最多 maxClients
        {
            std::lock_guard<std::mutex> lk(mtx_);
            if (sessions_.size() >= static_cast<size_t>(sopt_.maxClients))
            {
                if (log_) log_->logWarn("too many clients, refuse new connection");
                CxSocket::close(so);
                continue;
            }
        }


        auto id = nextClientId();
        auto session = std::make_shared<ClientSession>(so, copt_, log_, cb_, id);
        {
            std::lock_guard<std::mutex> lk(mtx_);
            sessions_.emplace(id, session);
        }
        if (log_) log_->logInfo("client accepted: " + id);
        session->start();
    }
}
