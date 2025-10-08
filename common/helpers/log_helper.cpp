#include "log_helper.h"
#include <spdlog/sinks/daily_file_sink.h>
#include <spdlog/async.h>
#include <iostream>

using LogLevel = LogStream::LogLevel;

// --- LogStream implementation ---

LogStream::LogStream(LogLevel level, std::string tag)
        : _level(level), _tag(std::move(tag))
{}

LogStream::~LogStream()
{
    std::string message = _ss.str();
    // 只有在訊息不為空時才執行記錄，避免產生空日誌
    if (!message.empty())
    {
        LogHelper::log(_level, message, _tag);
    }
}


// --- LogHelper implementation ---

spdlog::level::level_enum LogHelper::toSpdlogLevel(LogLevel level)
{
    switch (level)
    {
        case LogLevel::Verbose:
            return spdlog::level::trace;
        case LogLevel::Debug:
            return spdlog::level::debug;
        case LogLevel::Information:
            return spdlog::level::info;
        case LogLevel::Warning:
            return spdlog::level::warn;
        case LogLevel::Error:
            return spdlog::level::err;
        case LogLevel::Fatal:
            return spdlog::level::critical;
        default:
            return spdlog::level::info;
    }
}

LogHelper &LogHelper::getInstance()
{
    static LogHelper instance;
    return instance;
}

void LogHelper::init(const std::string &logFileName, const std::string &logDirectory)
{
    auto &instance = getInstance();
    if (instance._logger)
    {
        return; // 防止重複初始化
    }
    try
    {
        spdlog::init_thread_pool(8192, 1); // 初始化非同步日誌的執行緒池
        std::string logPath = logDirectory + "/" + logFileName + ".txt";

        // 建立每日滾動的日誌檔案接收器（sink），執行緒安全，保留7天日誌
        auto daily_sink = std::make_shared<spdlog::sinks::daily_file_sink_mt>(logPath, 0, 0, false, 7);

        // 建立非同步 logger
        instance._logger = std::make_shared<spdlog::async_logger>(
                logFileName, daily_sink, spdlog::thread_pool(), spdlog::async_overflow_policy::block
        );

        // 設定日誌格式：[時間] [logger名] [層級] [標籤] 訊息
        instance._logger->set_pattern("[%Y-%m-%d %H:%M:%S.%f] [%n] [%l] %v");
        instance._logger->set_level(spdlog::level::trace); // 預設記錄所有層級

        spdlog::register_logger(instance._logger);
    }
    catch (const spdlog::spdlog_ex &ex)
    {
        std::cerr << "Log initialization failed: " << ex.what() << std::endl;
    }
}

void LogHelper::shutdown()
{
    spdlog::shutdown(); // 關閉 spdlog，確保所有緩衝的日誌都寫入檔案
}

void LogHelper::setLogLevel(LogLevel level)
{
    auto &instance = getInstance();
    if (instance._logger)
    {
        instance._logger->set_level(toSpdlogLevel(level));
    }
}

// 核心的日誌寫入方法
void LogHelper::log(LogLevel logLevel, const std::string &message, const std::string &tag)
{
    auto &instance = getInstance();
    if (instance._logger)
    {
        // 如果有標籤，將其格式化到訊息前面
        std::string formatted_message = tag.empty() ? message : "[" + tag + "] " + message;
        instance._logger->log(toSpdlogLevel(logLevel), formatted_message);
    }
}

// 傳統方法的實現
void LogHelper::verbose(const std::string &message, const std::string &tag)
{ log(LogLevel::Verbose, message, tag); }

void LogHelper::debug(const std::string &message, const std::string &tag)
{ log(LogLevel::Debug, message, tag); }

void LogHelper::information(const std::string &message, const std::string &tag)
{ log(LogLevel::Information, message, tag); }

void LogHelper::warning(const std::string &message, const std::string &tag)
{ log(LogLevel::Warning, message, tag); }

void LogHelper::error(const std::string &message, const std::string &tag)
{ log(LogLevel::Error, message, tag); }

void LogHelper::fatal(const std::string &message, const std::string &tag)
{ log(LogLevel::Fatal, message, tag); }

// 流式 API 方法的實現 (回傳 LogStream 物件)
LogStream LogHelper::verbose()
{ return LogStream{LogLevel::Verbose}; }

LogStream LogHelper::debug()
{ return LogStream{LogLevel::Debug}; }

LogStream LogHelper::information()
{ return LogStream{LogLevel::Information}; }

LogStream LogHelper::warning()
{ return LogStream{LogLevel::Warning}; }

LogStream LogHelper::error()
{ return LogStream{LogLevel::Error}; }

LogStream LogHelper::fatal()
{ return LogStream{LogLevel::Fatal}; }