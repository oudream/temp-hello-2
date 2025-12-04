#ifndef CX_CT_X2_I_LOG_SINK_H
#define CX_CT_X2_I_LOG_SINK_H


#include <string>
#include <string_view>


struct INetLogSink {
    virtual ~INetLogSink() = default;
    virtual void logInfo(std::string_view msg) = 0;
    virtual void logWarn(std::string_view msg) = 0;
    virtual void logError(std::string_view msg) = 0;
    virtual void logDebug(std::string_view msg) = 0;
};


#endif //CX_CT_X2_I_LOG_SINK_H