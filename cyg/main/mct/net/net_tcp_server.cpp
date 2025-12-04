#include "net_tcp_server.h"

#include <helpers/log_helper.h>


NetTcpServer::NetTcpServer()
{

}


NetTcpServer::~NetTcpServer()
{
    stop();
    join();
    if (listenSo_ != INVALID_SOCKET) CxSocket::close(listenSo_);
}


bool NetTcpServer::start(const ServerOptions &opt)
{
    if (running_) return true;


    opt_ = opt;


    // 创建监听 socket（支持 IPv4/IPv6）
    listenSo_ = CxSocket::create(AF_INET, SOCK_STREAM, 0);
    if (listenSo_ == INVALID_SOCKET)
    {
        LogHelper::error() << ("create listen socket failed");
        return false;
    }


    CxIpAddressExtend addr(DEFAULT_FAMILY, opt_.bindIp.c_str(), std::to_string(opt_.port).c_str());
    // 绑定并监听
    if (CxSocket::bindTo(listenSo_, addr.get()))
    {
        LogHelper::error() << ("bind failed");
        CxSocket::close(listenSo_);
        listenSo_ = INVALID_SOCKET;
        return false;
    }
    if (CxSocket::listenTo(listenSo_, 5))
    {
        LogHelper::error() << ("listen failed");
        CxSocket::close(listenSo_);
        listenSo_ = INVALID_SOCKET;
        return false;
    }


    running_ = true;
    stopping_ = false;


    acceptThread_ = std::thread([this]
                                { acceptLoop(); });
    LogHelper::information() << ("NetTcpServer started");
    return true;
}

void NetTcpServer::stop()
{
    if (!running_) return;
    stopping_ = true;
    if (listenSo_ != INVALID_SOCKET)
    {
        CxSocket::close(listenSo_);
    }


    // 关闭所有会话
    std::vector<std::shared_ptr<NetClientSession>> v;
    {
        std::lock_guard<std::mutex> lk(mtx_);
        for (auto &kv: sessions_) v.push_back(kv.second);
        sessions_.clear();
    }
    for (auto &s: v)
    {
        s->stop();
//        s->join();
    }
}


void NetTcpServer::join()
{
    if (acceptThread_.joinable()) acceptThread_.join();
    std::vector<std::shared_ptr<NetClientSession>> v;
    {
        std::lock_guard<std::mutex> lk(mtx_);
        for (auto &kv: sessions_) v.push_back(kv.second);
    }
    for (auto &s: v)
    {
        s->join();
    }
    running_ = false;
}


size_t NetTcpServer::clientCount() const
{
    std::lock_guard<std::mutex> lk(mtx_);
    return sessions_.size();
}


std::vector<uint64_t> NetTcpServer::listClientIds() const
{
    std::vector<uint64_t> ids;
    std::lock_guard<std::mutex> lk(mtx_);
    ids.reserve(sessions_.size());
    for (auto &kv: sessions_) ids.push_back(kv.first);
    return ids;
}


bool NetTcpServer::closeClient(uint64_t clientId)
{
    std::shared_ptr<NetClientSession> s;
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


uint64_t NetTcpServer::nextClientId()
{
    return ++clientSeq_;
}


void NetTcpServer::acceptLoop()
{
    while (!stopping_)
    {
        struct sockaddr_storage ra{};
        cx::socket_t so = CxSocket::acceptFrom(listenSo_, &ra);
        if (so == INVALID_SOCKET)
        {
            if (stopping_) break;
            continue;
        }


        // 套接字选项
        if (opt_.recvTimeoutMs > 0) CxSocket::receiveTimeout(so, opt_.recvTimeoutMs);
        if (opt_.sendTimeoutMs > 0) CxSocket::sendTimeout(so, opt_.sendTimeoutMs);
        if (opt_.tcpNoDelay) CxSocket::nodelay(so);
        if (opt_.keepAlive) CxSocket::keepALive(so, true);


        // 限流：最多 maxClients
        {
            std::lock_guard<std::mutex> lk(mtx_);
            if (sessions_.size() >= static_cast<size_t>(opt_.maxClients))
            {
                LogHelper::warning() << ("too many clients, refuse new connection");
                CxSocket::close(so);
                continue;
            }
        }

        auto id = nextClientId();
        auto session = std::make_shared<NetClientSession>(so, id);

        // 注册 on-exit：会话退出即从 sessions_ 移除
        // 注意：避免捕获 shared_ptr 造成循环引用，用 key + this
        session->setOnExit([this, id]
                           {
                               std::lock_guard<std::mutex> lk(this->mtx_);
                               this->eraseSessionUnsafe(id);
                           });

        {
            std::lock_guard<std::mutex> lk(mtx_);
            sessions_.emplace(id, session);
            LogHelper::debug() << ("Session added. total=" + std::to_string(sessions_.size()));
        }

        session->start(); // 内部启动线程
    }
}

void NetTcpServer::stopAllSessions()
{
    std::vector<std::shared_ptr<NetClientSession>> snapshot;
    {
        std::lock_guard<std::mutex> lk(mtx_);
        for (auto &kv: sessions_) snapshot.push_back(kv.second);
    }
    for (auto &s: snapshot) if (s) s->stop();
}

void NetTcpServer::eraseSessionUnsafe(uint64_t key)
{
    auto it = sessions_.find(key);
    if (it != sessions_.end())
    {
        sessions_.erase(it);
        LogHelper::debug() << ("Session erased. remain=" + std::to_string(sessions_.size()));
    }
}