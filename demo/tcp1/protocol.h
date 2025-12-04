#ifndef CX_CT_X2_PROTOCOL_H
#define CX_CT_X2_PROTOCOL_H


#include <string>
#include <string_view>
#include <map>
#include <cstdint>


struct ServerOptions {
    std::string bindIp = "0.0.0.0";
    uint16_t port = 9000;
    int maxClients = 64;
    bool tcpNoDelay = false;
    bool keepAlive = false;
    int recvTimeoutMs = 30'000; // 30s
    int sendTimeoutMs = 30'000; // 30s
};


struct SessionOptions {
    int lineMaxLen = 16 * 1024; // 命令行最大长度
    size_t dataBufferSize = 40ull * 1024 * 1024; // 分块缓存：40MB
    int idleCloseSeconds= 300; // 无通信超时
};


// 解析一行: 支持以 空格/逗号/分号/& 分隔的 k=v 对
struct KeyValueParser {
    static bool parseLine(std::string_view line,
    std::map<std::string,std::string>& out) {
        out.clear();
        auto trim = [](std::string_view s){
            while(!s.empty() && (s.front()==' '||s.front()=='\t' || s.front()=='\r')) s.remove_prefix(1);
            while(!s.empty() && (s.back ()==' '||s.back ()=='\t' || s.back ()=='\r' || s.back()=='\n')) s.remove_suffix(1);
            return s;
        };
        std::string token;
        token.reserve(line.size());
        auto flushToken = [&](std::string_view t){
            t = trim(t);
            if(t.empty()) return;
            auto eq = t.find('=');
            if(eq==std::string_view::npos) return;
            std::string k(t.substr(0,eq));
            std::string v(t.substr(eq+1));
            // 去掉可能的引号
            auto unquote=[&](std::string &s){
                if(s.size()>=2 && ((s.front()=='"' && s.back()=='"')||(s.front()=='\'' && s.back()=='\''))) {
                    s = s.substr(1, s.size()-2);
                }
            }; unquote(k); unquote(v);
            out[k]=v;
        };
        for(char c: line){
            if(c==' '||c=='\t'||c==','||c==';'||c=='&'){
                flushToken(token); token.clear();
            } else token.push_back(c);
        }
        flushToken(token);
        return !out.empty();
    }
};


#endif //CX_CT_X2_PROTOCOL_H