#include "client_session.h"

#include <cstdio>
#include <cstring>


using clock_ = std::chrono::steady_clock;


ClientSession::ClientSession(cx::socket_t sock,
                             const SessionOptions& sopt,
                             INetLogSink* log,
                             ISessionCallbacks* cb,
                             std::string clientId)
    : so_(sock), sopt_(sopt), log_(log), cb_(cb), clientId_(std::move(clientId))
{
    lastActive_ = clock_::now();
    dataBuf_.resize(std::max<size_t>(1 << 16, sopt_.dataBufferSize));
}


ClientSession::~ClientSession()
{
    stop();
    join();
    if (so_ != INVALID_SOCKET)
    {
        CxSocket::close(so_);
    }
}


void ClientSession::start()
{
    worker_ = std::thread([self = shared_from_this()]
    {
        self->runLoop();
    });
}


void ClientSession::stop()
{
    stopping_.store(true, std::memory_order_relaxed);
    if (so_ != INVALID_SOCKET)
    {
        CxSocket::close(so_);
    }
}


void ClientSession::join()
{
    if (worker_.joinable()) worker_.join();
}


bool ClientSession::sendLine(std::string_view line)
{
    if (so_ == INVALID_SOCKET) return false;
    std::string s(line);
    if (s.empty() || s.back() != '\n') s.push_back('\n');
    auto n = CxSocket::send(so_, s.data(), s.size());
    if (n > 0) bytesSent_ += (uint64_t)n;
    else return false;
    return true;
}


bool ClientSession::sendBytes(const uint8_t* data, size_t n)
{
    if (so_ == INVALID_SOCKET) return false;
    auto sent = CxSocket::send(so_, data, n);
    if (sent > 0)
    {
        bytesSent_ += (uint64_t)sent;
        return (size_t)sent == n;
    }
    return false;
}


bool ClientSession::readLine(std::string& out)
{
    out.assign((size_t)0, '\0');
    std::vector<char> buf(sopt_.lineMaxLen + 1);
    auto n = CxSocket::readline(so_, buf.data(), buf.size());
    if (n <= 0) return false;
    lastActive_ = clock_::now();
    bytesRecv_ += (uint64_t)n;
    out.assign(buf.data(), (size_t)n);
    return true;
}


bool ClientSession::readExact(uint8_t* buf, size_t n)
{
    size_t got = 0;
    while (got < n)
    {
        if (stopping_.load(std::memory_order_relaxed)) return false;
        ssize_t r = CxSocket::recv(so_, buf + got, n - got);
        if (r <= 0) return false;
        got += (size_t)r;
        bytesRecv_ += (uint64_t)r;
        lastActive_ = clock_::now();
    }
    return true;
}


void ClientSession::runLoop()
{
    // 模式：CommandMode / DataMode
    std::string line;
    for (;;)
    {
        if (stopping_.load(std::memory_order_relaxed)) break;


        // 读取一行命令
        if (!readLine(line)) break; // 客户端关闭或超时


        // 解析 k=v
        std::map<std::string, std::string> kv;
        if (!KeyValueParser::parseLine(line, kv)) continue; // 空行或不合法


        auto itType = kv.find("type");
        std::string type = (itType == kv.end() ? "cmd" : itType->second);


        if (type == "data")
        {
            // 必须有 data_size
            uint64_t total = 0;
            if (auto it = kv.find("data_size"); it != kv.end())
            {
                total = std::strtoull(it->second.c_str(), nullptr, 10);
            }
            if (total == 0)
            {
                if (log_) log_->logError("missing data_size");
                continue;
            }


            DataMeta meta;
            meta.sizeBytes = total;
            if (auto it = kv.find("name"); it != kv.end()) meta.name = it->second;
            if (auto it = kv.find("content_type"); it != kv.end()) meta.contentType = it->second;
            if (auto it = kv.find("extra"); it != kv.end()) meta.extra = it->second;


            if (cb_) cb_->onDataBegin(clientId_, meta);


            uint64_t remain = total;
            while (remain > 0)
            {
                size_t chunk = (size_t)std::min<uint64_t>(remain, dataBuf_.size());
                if (!readExact(dataBuf_.data(), chunk))
                {
                    if (cb_) cb_->onDataEnd(clientId_, meta, false);
                    return; // 会话结束
                }
                if (cb_) cb_->onDataChunk(clientId_, std::span<const uint8_t>(dataBuf_.data(), chunk));
                remain -= chunk;
            }
            if (cb_) cb_->onDataEnd(clientId_, meta, true);
        }
        else
        {
            if (cb_) cb_->onCommand(clientId_, kv);
        }
    }
}
