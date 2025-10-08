#ifndef CX_CT_X2_LOG_HELPER_H
#define CX_CT_X2_LOG_HELPER_H


#include <spdlog/spdlog.h> // 核心日誌函式庫
#include <string>
#include <memory>
#include <sstream> // 用於組合流式訊息
#include <utility> // 用於 std::move


/**
 * @brief 輔助類別，用於實現流式日誌記錄 (Fluent Interface)。
 * 這是一個臨時物件，在其生命週期結束時（通常是在 ';' 處），
 * 其解構函式會將組合好的日誌訊息提交給 LogHelper。
 */
class LogStream
{
public:
    // 定義日誌層級，與 C# 版本對應
    enum class LogLevel
    {
        Verbose,
        Debug,
        Information,
        Warning,
        Error,
        Fatal
    };

    /**
     * @brief 建構函式
     * @param level 日誌層級
     * @param tag 初始標籤 (可選)
     */
    explicit LogStream(LogLevel level, std::string tag = "");

    /**
     * @brief 解構函式。日誌的實際寫入在此發生。
     */
    ~LogStream();

    /**
     * @brief 設定或覆蓋日誌標籤的鏈式方法。
     * @param new_tag 要設定的新標籤。
     * @return 返回自身引用，以支援繼續鏈式呼叫。
     */
    LogStream &tag(std::string new_tag)
    {
        _tag = std::move(new_tag);
        return *this;
    }

    /**
     * @brief 重載 << 運算子，以接收各種資料類型。
     * @tparam T 資料類型
     * @param msg 要記錄的訊息片段
     * @return 返回自身引用，以支援連續的 << 操作。
     */
    template<typename T>
    LogStream &operator<<(const T &msg)
    {
        _ss << msg;
        return *this;
    }

private:
    LogLevel _level;
    std::string _tag;
    std::stringstream _ss;
};


/**
 * @brief 全域日誌記錄器類別，使用單例模式。
 */
class LogHelper
{
public:
    // 禁止複製和賦值，確保單例的唯一性
    LogHelper(const LogHelper &) = delete;

    LogHelper &operator=(const LogHelper &) = delete;

    // --- 核心靜態方法 ---
    static LogHelper &getInstance();

    static void init(const std::string &logFileName, const std::string &logDirectory = "logs");

    static void shutdown();

    static void setLogLevel(LogStream::LogLevel level);

    // 將自訂 LogLevel 轉換為 spdlog 的層級
    static spdlog::level::level_enum toSpdlogLevel(LogStream::LogLevel level);

    // --- 傳統日誌記錄方法 (向後相容) ---
    static void log(LogStream::LogLevel logLevel, const std::string &message, const std::string &tag = "");

    static void verbose(const std::string &message, const std::string &tag = "");

    static void debug(const std::string &message, const std::string &tag = "");

    static void information(const std::string &message, const std::string &tag = "");

    static void warning(const std::string &message, const std::string &tag = "");

    static void error(const std::string &message, const std::string &tag = "");

    static void fatal(const std::string &message, const std::string &tag = "");

    // --- 優化後的流式 API 方法 ---
    // LogHelper.Debug().Tag("tag1") << "log1";
    static LogStream verbose();

    static LogStream debug();

    static LogStream information();

    static LogStream warning();

    static LogStream error();

    static LogStream fatal();

private:
    // 將 LogStream 設為友元，使其可以呼叫私有的 Log 方法
    friend class LogStream;

    // 私有建構/解構函式，防止外部實例化
    LogHelper() = default;

    ~LogHelper() = default;

    // spdlog 的 logger 實例
    std::shared_ptr<spdlog::logger> _logger;

};

#endif //CX_CT_X2_LOG_HELPER_H
