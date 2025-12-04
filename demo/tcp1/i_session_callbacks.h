#ifndef CX_CT_X2_I_SESSION_CALLBACKS_H
#define CX_CT_X2_I_SESSION_CALLBACKS_H


#include <string>
#include <map>
#include <span>
#include <cstdint>


struct DataMeta {
    std::string name; // 可选：文件名/资源名
    uint64_t sizeBytes = 0; // 必填：数据大小
    std::string contentType; // 可选：如 image/tiff
    std::string extra; // 可选：附加信息
};


struct ISessionCallbacks {
    virtual ~ISessionCallbacks() = default;


    // 命令型：k=v 解析后的键值集合
    virtual void onCommand(const std::string& clientId,
    const std::map<std::string,std::string>& kvs) = 0;


    // 数据型：按 Begin/Chunk/End 生命周期回调
    virtual void onDataBegin(const std::string& clientId, const DataMeta& meta) = 0;
    virtual void onDataChunk(const std::string& clientId, std::span<const uint8_t> chunk) = 0;
    virtual void onDataEnd (const std::string& clientId, const DataMeta& meta, bool ok) = 0;

};


#endif //CX_CT_X2_I_SESSION_CALLBACKS_H