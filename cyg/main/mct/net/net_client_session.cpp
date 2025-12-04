#include "net_client_session.h"

#include <helpers/log_helper.h>
#include <ccxx/cxstring.h>
#include <ccxx/datetime.h>

#include <helpers/opencv_helper.h>

#include "rebuild_task_manager.h"


constexpr int CACHE_SIZE = 40 * 1024 * 1024;   // 40 MB
constexpr size_t CHUNK_SIZE = 4 * 1024 * 1024; // 4  MB

// -------------------- Command 常量定义 --------------------
constexpr const char *CMD_HEARTBEAT = "Heartbeat";
constexpr const char *CMD_RELOAD_CONFIG = "ReloadConfig";
constexpr const char *CMD_REBUILD = "Rebuild";   // 带图像数据

// func 常量：首次 / 数据过程 / 尾次
constexpr const char *FUNC_BEGIN = "Begin";
constexpr const char *FUNC_DATA = "Data";
constexpr const char *FUNC_END = "End";
constexpr const char *FUNC_PROGRESS = "Progress";   // 查询当前重建进度

// 工作机制说明
// FUNC_BEGIN	    Rebuild>abc001,1731150912000,Begin,0,,,0,imageCount=64	开始任务，准备图片缓存
// FUNC_DATA	    Rebuild>abc001.000.png,1731150912000,Data,0,img1.png,9437184,0,Rows=2048,Cols=2048	上传单张图片数据
// FUNC_END	        Rebuild>abc001,1731150912000,End,0,,,0	通知服务器全部图片已上传，启动重建
// FUNC_PROGRESS	Rebuild>abc001,1731150912000,Progress,0,,,0

// 参数，规则是：小写 加 下划线
constexpr const char *EXTRA_ROWS = "rows";
constexpr const char *EXTRA_COLS = "cols";

using clock_ = std::chrono::steady_clock;


using uchar = cx::uchar;


enum class ClientMode
{
    CommandReceiveMode, DataReceiveMode
};

int reloadConfig_()
{
    return -1;
}

std::vector<std::string> getInferErrorMessages_()
{
    std::vector<std::string> errMsgs;
//    int modelIdMax = ModelIdMax();
//    for (int ii = 1; ii <= modelIdMax; ii++)
//    {
//        auto& errMsg = modelInfer->AqRess_[ii].err_msg;
//        if (errMsg != "")
//            errMsgs.push_back(errMsg);
//    }
    return errMsgs;
}


// command>sectionId,sectionTime,func,flag,dataName,dataSize,dataType
// > 表示请求
// < 表示回复
struct ClientContext
{
    // ---------- 网络 ----------
    cx::socket_t socket;
    // ---------- 当前接收到的数据 ----------
    ClientMode mode;            // 接收方式
    std::vector<char> buffer;   // 接收缓存，一般为 40 * 1024 * 1024
    size_t currentSize;         // 当前缓冲区使用的大小
    std::string commandText;        // 接收到的普通命令
    std::string cmd;      // section id
    std::string sectionId;      // section id
    cx::DateTime sectionTime;   // section 时间戳
    std::string func;           // 功能
    std::string flag;           // 标志
    std::string dataName;       // 数据名，可能是像文件名: img1.jpg  img2.png
    size_t dataSize;            // 数据大小
    int dataType;               // 数据类型码，前16位是大类型，后16位是小类型。支持：opencv的图像格式--> 0:CV_8UC1, 16:CV_8UC3
    int majorType;              // 数据主类型
    int minorType;              // 数据子类型
    std::unordered_map<std::string, std::string> extras;
    cx::DateTime lastCommunicationTime; // 最后通信时间
};

// ---------- 新协议统一应答：CommandName=SectionId:key=value,...\n ----------
// 请求中 flag 转为 status
// command<sectionId,sectionTime,func,status,flag,dataName,dataSize,dataType,key1=value1,key2=value2
void answerCommand_(ClientContext &context, const std::string &status, const std::string &flag, const std::vector<std::pair<std::string, std::string>> &params)
{
    std::ostringstream oss;
    oss << context.cmd << "<" << context.sectionId << "," << context.sectionTime.ticksMs() << "," << context.func;
    oss << "," << status;
    oss << "," << flag;
    // ",dataName,dataSize,dataType
    oss << "," << "," << ",";
    for (const auto &kv: params)
    {
        oss << ",";
        oss << kv.first << "=" << kv.second;
    }
    oss << "\n";

    const std::string msg = oss.str();

    auto n = CxSocket::send(context.socket, msg.c_str(), msg.size(), 0);
    if (n < 0)
    {
        auto sErr = CxSocket::lastErrorMessage();
        LogHelper::error() << "Net Session Answer command tcp error, " << sErr << ", [" << msg << "]";
    }
    else
    {
        LogHelper::information() << "Net Session Answer command ok, [" << msg << "]";
    }
}


//返回bbox数据
void answerData_(ClientContext &context, const std::vector<char> &data)
{
    const size_t chunkSize = CHUNK_SIZE; // 4 * 1024 * 1024
    size_t totalSent = 0;
    size_t dataLength = data.size();

    while (totalSent < dataLength)
    {
        //size_t currentChunkSize = std::min(chunkSize, dataLength - totalSent);
        size_t remainingData = dataLength - totalSent;
        size_t currentChunkSize = (remainingData < chunkSize) ? remainingData : chunkSize;
        auto n = CxSocket::send(context.socket, reinterpret_cast<const char *>(&data[totalSent]), currentChunkSize, 0);
        if (n < 0)
        {
            auto sErr = CxSocket::lastErrorMessage();
            LogHelper::error() << "Net Session Answer data error, " << sErr << ". sectionId=" << context.sectionId;
            break; // 发生错误，退出发送循环
        }
        totalSent += n;
        if (n < static_cast<int>(currentChunkSize))
        {
            // 未能发送完整的块，可能需要重试或处理
            LogHelper::warning() << "Answer data chunk less, sent " << n << " bytes out of " << currentChunkSize;
            // 可以在此处添加重试逻辑或其他处理
        }
    }

    if (totalSent == dataLength)
        LogHelper::information() << "Net Session Answer bbox data ok.";
    else
        LogHelper::error() << "Net Session Answer bbox data ng, failed to send all bbox data.";
}

// command<sectionId,sectionTime,func,status,flag,dataName,dataSize,dataType,key1=value1,key2=value2
// Rebuild<CT-20251116-190541,1763291142990,Progress,NG,flag,dataName,dataSize,dataType,progress=0
void answerCommand_(ClientContext &context, const std::string &status, const std::string &flag, const std::string &dataName, const std::vector<char> &data, int dataType, const std::vector<std::pair<std::string, std::string>> &params)
{
    std::ostringstream oss;
    oss << context.cmd << "<" << context.sectionId << "," << context.sectionTime.ticksMs() << "," << context.func;
    oss << "," << status;
    oss << "," << flag;
    // ",dataName,dataSize,dataType
    oss << "," << dataName << "," << data.size() << "," << dataType;
    for (const auto &kv: params)
    {
        oss << ",";
        oss << kv.first << "=" << kv.second;
    }
    oss << "\n";

    const std::string msg = oss.str();

    auto n = CxSocket::send(context.socket, msg.c_str(), static_cast<int>(msg.size()), 0);
    if (n < 0)
    {
        auto sErr = CxSocket::lastErrorMessage();
        LogHelper::error() << "Net Session Answer command tcp error, " << sErr << ", [" << msg << "]";
        return;
    }

    answerData_(context, data);
}

// 解析接收到的命令, 已去除了末尾的'\n'
// command>sectionId,sectionTime,func,flag,dataName,dataSize,dataType,key1=value1,key2=value2
// 重建需要接收图像数据，而且会有多张图像，一张图像一次命令，都要保存先来。有首次、数据过程、尾次三个func，同一个命令。
// 首次与尾次都没有数据，首次会告知有多少图像。完成toMatType，如果extras没有rows、cols就从dataSize与MatType算出来，因为是正方形
void processCommandMode_(ClientContext &context)
{
    std::string content = CxString::trim(context.commandText);
    // if (!content.empty() && (content.back() == '\r')) content.pop_back();
    // 去尾部换行
    // while (!content.empty() && (content.back() == '\n' || content.back() == '\r')) content.pop_back();
    if (content.empty()) return;

    // 形如：DetectFile>sid001,1722483905340,Detect,0,img1.png,9437184,0|CV_8UC1,k1=v1,k2=v2
    size_t gt = content.find('>');
    if (gt == std::string::npos)
    {
        LogHelper::error() << "Net Session Bad command: " << content;
        return;
    }

    std::string cmd = content.substr(0, gt);
    std::string args = content.substr(gt + 1);

    // 固定字段（最多7个）
    size_t d1 = args.find(',');
    size_t d2 = args.find(',', d1 + 1);
    size_t d3 = args.find(',', d2 + 1);
    size_t d4 = args.find(',', d3 + 1);
    size_t d5 = args.find(',', d4 + 1);
    size_t d6 = args.find(',', d5 + 1);
    size_t d7 = args.find(',', d6 + 1);

    if (d6 == std::string::npos)
    {
        LogHelper::error() << "Net Session Field count mismatch: " << args;
        return;
    }

    context.cmd = cmd;
    context.sectionId = args.substr(0, d1);
    std::string sectionTime = args.substr(d1 + 1, d2 - d1 - 1);
    context.func = args.substr(d2 + 1, d3 - d2 - 1);
    context.flag = args.substr(d3 + 1, d4 - d3 - 1);
    context.dataName = args.substr(d4 + 1, d5 - d4 - 1);
    context.dataSize = std::stoull(args.substr(d5 + 1, d6 - d5 - 1));

    // ---- dataType ----
    std::string dataTypeStr;
    std::string rest; // 后续 key=value 字符串
    if (d7 == std::string::npos)
    {
        dataTypeStr = args.substr(d6 + 1);
    }
    else
    {
        dataTypeStr = args.substr(d6 + 1, d7 - d6 - 1);
        rest = args.substr(d7 + 1); // 剩余部分
    }

    // dataType: 支持十进制或0x十六进制
    auto dataType = static_cast<int>(std::stoul(dataTypeStr));
    // 拆高低 16 位（方便日志调试）
    int majorType = (dataType >> 16) & 0xFFFF;
    int minorType = dataType & 0xFFFF;
    context.dataType = dataType;
    context.majorType = majorType;
    context.minorType = minorType;

    // sectionTime
    long long ms = std::stoll(sectionTime);
    context.sectionTime = cx::DateTime(ms);

    // ------------------- 扩展参数解析 key=value -------------------
    std::unordered_map<std::string, std::string> extras;
    context.extras.clear();
    size_t pos = 0;
    while (pos < rest.size())
    {
        size_t comma = rest.find(',', pos);
        std::string item = (comma == std::string::npos) ?
                           rest.substr(pos) : rest.substr(pos, comma - pos);
        pos = (comma == std::string::npos) ? rest.size() : comma + 1;
        if (item.empty()) continue;
        size_t eq = item.find('=');
        if (eq != std::string::npos)
        {
            std::string key = item.substr(0, eq);
            std::string val = item.substr(eq + 1);
            context.extras[key] = val;
        }
        else
        {
            context.extras[item] = {};
        }
    }

    context.lastCommunicationTime = cx::DateTime::now();

    LogHelper::debug()
            << "Cmd=" << cmd
            << " sectionId=" << context.sectionId
            << " sectionTime=" << context.sectionTime.toString()
            << " dataName=" << context.dataName
            << " dataSize=" << context.dataSize
            << " dataType=0x" << std::hex << context.dataType << std::dec
            << " (Major=" << majorType << ", Minor=" << minorType << ")"
            << " Extras=" << extras.size();


    // ========== Heartbeat ==========
    if (cmd == CMD_HEARTBEAT)
    {
        answerCommand_(context, cx::STR_OK, std::string{}, {
                {"version", "v20251027a"},
        });
        return;
    }

    if (cmd == CMD_RELOAD_CONFIG)
    {
        int ret = reloadConfig_();
        answerCommand_(context, (ret == 0 ? cx::STR_OK : cx::STR_NG), std::string{}, {});
        return;
    }

    if (cmd == CMD_REBUILD)
    {
        // Begin 阶段
        if (context.func == FUNC_BEGIN)
        {
            // 告知后续有多少张图片
            int imageCount = 0;
            if (context.extras.count("imageCount"))
            {
                imageCount = std::stoi(context.extras["imageCount"]);
            }
            context.extras["imageRemain"] = std::to_string(imageCount);
            LogHelper::information() << "Net Session [Rebuild-Begin] Expect " << imageCount << " images.";

            auto pixBits = 16;//CV_ELEM_SIZE(context.minorType);

            // 接收重建任务 1 - 创建任务
            bool bStatus = RebuildTaskManager::beginTask(context.sectionId, context.sectionTime, imageCount, pixBits, context.extras);
            std::string sStatus = bStatus ? cx::STR_OK : cx::STR_NG;

            answerCommand_(context, sStatus, std::string{}, {{"expectImages", std::to_string(imageCount)}});

            return;
        }

        // Data 阶段：每个命令带一张图片
        if (context.func == FUNC_DATA)
        {
            if (context.dataSize > 0)
            {
                // 接收重建任务 2 - 往任务中压入 图像数据
                context.mode = ClientMode::DataReceiveMode;

                LogHelper::verbose() << "Net Session 1 mod=" << static_cast<int>(context.mode);

                // 立即应答 OK（带回 sectionId）
                answerCommand_(context, cx::STR_OK, std::string{}, {
                        {"dataName", context.dataName}
                });

                LogHelper::information() << "Net Session [Rebuild-Data] Ready to receive " << context.dataSize << " bytes.";
            }
            else
            {
                LogHelper::warning() << "[Rebuild-Data] No dataSize.";
            }
            return;
        }

        // End 阶段：汇总结束
        if (context.func == FUNC_END)
        {
            LogHelper::information() << "Net Session [Rebuild-End] All images received.";
            answerCommand_(context, cx::STR_OK, std::string{}, {{"status", "complete"}});

            // 接收重建任务 3 - 开始执行任务
            RebuildTaskManager::endTaskAndReconstruct(context.sectionId);

            return;
        }

        // Progress 阶段：客户端查询当前重建进度
        if (context.func == FUNC_PROGRESS)
        {
            RebuildTaskState st{};
            bool ok = RebuildTaskManager::getTaskState(context.sectionId, st);
            int percent = ok ? st.percent : -1;
            std::string dir = ok ? RebuildTaskManager::getTaskVolumeDir(st.taskDir) : std::string{};
            answerCommand_(context,
                           (ok ? cx::STR_OK : cx::STR_NG),
                           std::string{},
                           {
                                   {"progress", std::to_string(std::max(0, percent))},
                                   {"dir", dir}
                           });
            LogHelper::information() << "Net Session [Rebuild-Progress] id=" << context.sectionId
                                     << " progress=" << percent;
            return;
        }
    }

    if (context.dataSize > 0)
    {
        // 进入收数据模式
        context.mode = ClientMode::DataReceiveMode;

        LogHelper::verbose() << "Net Session 2 mod=" << static_cast<int>(context.mode);

        // 立即应答 OK（带回 sectionId）
        answerCommand_(context, cx::STR_OK, std::string{}, {
                {"dataName", context.dataName}
        });
    }

    // 其它命令……（同样：先 cmd，再 sectionId，再 kv）

}


// 计算 rows/cols（优先 extras，没有则按正方形推回）
// 求每像素总字节数	CV_ELEM_SIZE(matType) 或 mat.elemSize()
// 求每通道字节数	CV_ELEM_SIZE1(matType) 或 mat.elemSize1()
// 求通道数	CV_MAT_CN(matType) 或 mat.channels()
static inline bool inferRowsColsSquare_(size_t dataSize, int matType, int &rows, int &cols)
{
    // CV_MAT_DEPTH
    // CV_MAT_CN
    const uint64_t bpp = static_cast<uint64_t>(CV_ELEM_SIZE(matType));  // bytes per pixel
    if (bpp == 0 || dataSize < bpp)
    {
        LogHelper::error() << "Net Session inferRowsColsSquare_: invalid bpp or dataSize";
        return false;
    }

    long double pixels = static_cast<long double>(dataSize) / bpp;
    int side = static_cast<int>(std::llround(std::sqrt(pixels)));

    if (static_cast<uint64_t>(side) * static_cast<uint64_t>(side) * bpp != dataSize)
    {
        LogHelper::warning() << "dataSize is not a perfect square for inferred image, "
                             << "side=" << side << ", dataSize=" << dataSize << ", bpp=" << bpp;
    }

    rows = cols = std::max(1, side);
    return true;
}

//处理接收到的图像文件
void processDataMode_(ClientContext &context)
{
    if (context.dataSize <= 0 || context.dataSize > CACHE_SIZE) return;

//    auto replyNG = [&](const std::string &command, const std::string &reason)
//    {
//        answerCommand_(context, command, cx::STR_NG, {
//                {"reason", reason},
//                {"time",   cx::DateTime::currentDateTimeString()}
//        });
//    };

    // 写入数据
    if (context.cmd == CMD_REBUILD)
    {
        // 提取 rows, cols
        int rows = 0, cols = 0;
        if (!context.extras.empty())
        {
            if (context.extras.count(EXTRA_ROWS)) rows = std::stoi(context.extras[EXTRA_ROWS]);
            if (context.extras.count(EXTRA_COLS)) cols = std::stoi(context.extras[EXTRA_COLS]);
        }

        int matType = context.minorType;

        // 如果没有提供 rows/cols，就根据 dataSize 自动推导（假设正方形）
        if (rows == 0 || cols == 0)
        {
            if (!inferRowsColsSquare_(context.dataSize, matType, rows, cols))
            {
                return;
            }
        }

        auto bSave = RebuildTaskManager::appendImageBuffer(
                context.sectionId, context.buffer.data(),
                context.dataSize, rows, cols, matType
        );

        answerCommand_(context, (bSave ? cx::STR_OK : cx::STR_NG), std::string{}, {});
    }

}


//重置上下文，准备接收新的命令
void resetClientContext_(ClientContext &context)
{
    context.mode = ClientMode::CommandReceiveMode;
    LogHelper::verbose() << "Net Session 3 mod=" << static_cast<int>(context.mode);
    context.dataName.clear();
    context.dataSize = 0;
    // 不需要处理将未处理的数据移动到 buffer 的开头
    context.currentSize = 0;
}


NetClientSession::NetClientSession(cx::socket_t sock, uint64_t clientId)
        : so_(sock), clientId_(clientId)
{
    lastActive_ = clock_::now();
    // 40MB
    dataBuf_.resize(std::max<size_t>(1 << 16, CACHE_SIZE));
}


NetClientSession::~NetClientSession()
{
    stop();
    if (worker_.joinable())
    {
        if (std::this_thread::get_id() == worker_.get_id())
        {
            worker_.detach();
        }
        else
        {
            worker_.join();
        }
    }
    if (so_ != INVALID_SOCKET)
    {
        CxSocket::close(so_);
    }
}


void NetClientSession::start()
{
// [] 里的初始化捕获（如 self = shared_from_this()、weak = shared_from_this()）是在调用 std::thread 构造函数的当前线程里完成的，也就是 start() 所在的线程。
//
//    worker_ = std::thread([self = shared_from_this()]
//                          {
//                              self->runLoop();
//                          });

//    std::shared_ptr<NetClientSession> share = shared_from_this();
//    worker_ = std::thread([share]
//                          {
//                              share->runLoop();
//                          });

    std::weak_ptr<NetClientSession> weak = shared_from_this();
    worker_ = std::thread([weak]
                          {
                              if (auto self = weak.lock()) self->runLoop();
                          });
}


void NetClientSession::stop()
{
    stopping_ = true;
    if (so_ != INVALID_SOCKET)
    {
        CxSocket::close(so_);
        so_ = INVALID_SOCKET;
    }
}


void NetClientSession::join()
{
    if (worker_.joinable())
    {
        worker_.join();
    }
}


bool NetClientSession::sendLine(std::string_view line)
{
    if (so_ == INVALID_SOCKET) return false;
    std::string s(line);
    if (s.empty() || s.back() != '\n') s.push_back('\n');
    auto n = CxSocket::send(so_, s.data(), s.size());
    if (n > 0) bytesSent_ += (uint64_t) n;
    else return false;
    return true;
}


bool NetClientSession::sendBytes(const uint8_t *data, size_t n)
{
    if (so_ == INVALID_SOCKET) return false;
    auto sent = CxSocket::send(so_, data, n);
    if (sent > 0)
    {
        bytesSent_ += (uint64_t) sent;
        return (size_t) sent == n;
    }
    return false;
}

void NetClientSession::runLoop()
{
    cx::socket_t clientSocket = so_;

    using std::chrono::steady_clock;
    using std::chrono::duration_cast;
    using std::chrono::seconds;

    ClientContext context{};
    context.socket = clientSocket;
    context.mode = ClientMode::CommandReceiveMode;
    LogHelper::verbose() << "Net Session 4 mod=" << static_cast<int>(context.mode);
    context.buffer.resize(CACHE_SIZE); // 分配40MB的空间
    context.currentSize = 0;
    context.lastCommunicationTime = cx::DateTime::now(); // 初始化最后通信时间

    while (!stopping_)
    {
        // socket select
        int status = CxSocket::wait(clientSocket, 5000, &status);

        if (status == -1)
        {
            LogHelper::error() << "Net Session select() error";  // 记录错误
            break; // 发生错误
        }
        else if (status == 0)
        {
            auto now = cx::DateTime::now();
            auto idleSec = now - context.lastCommunicationTime;
            if (idleSec >= 30 * 1000)
            {
                LogHelper::error() << "Net Session No communication for 30 seconds. Closing connection.";
                break;
            }
            else if (idleSec >= 10 * 1000)
            {
                // 自动复位：回到命令接收模式并清空缓存；避免粘残包导致协议乱序
                LogHelper::warning() << "Idle " << idleSec
                                     << "s, auto reset to CommandReceiveMode (clear buffer).";
                context.mode = ClientMode::CommandReceiveMode;
                LogHelper::verbose() << "Net Session 5 mod=" << static_cast<int>(context.mode);
                context.currentSize = 0;
                context.commandText.clear();
                // 如需更“彻底”的复位，可按需清理以下字段（可选）：
                // context.dataName.clear(); context.dataSize = 0; context.imageNum = 0; // 等
                context.lastCommunicationTime = now; // 防止反复刷日志
            }
            continue;
        }

        int availableSpace = static_cast<int>(CACHE_SIZE - context.currentSize);
        if (availableSpace <= 0)
        {
            LogHelper::error() << "Net Session availableSpace error availableSpace=" << availableSpace;  // 记录错误
            break;
        }
        char *recvBuffer = context.buffer.data();
        auto n = CxSocket::recv(clientSocket, recvBuffer + context.currentSize, availableSpace, 0);
        if (n <= 0)
        {
            if (n == 0)
                LogHelper::information() << "Net Session Client disconnected.";
            else
                LogHelper::error() << "Net Session recv() error n=" << n;  // 记录错误
            break; // 客户端断开连接或发生错误
        }

        LogHelper::information() << "Net Session ==============>TCP收到数据长度: " << n;

        context.currentSize += n; // 更新已使用的缓冲区大小
        context.lastCommunicationTime = cx::DateTime::now(); // 更新最后通信时间

        // 根据当前模式处理数据
        if (context.mode == ClientMode::CommandReceiveMode)
        {
            if (n > 4096)
            {
                LogHelper::error() << "Net Session 命令模式下，TCP收到无效数据长度: " << n;
                continue;
            }
            char *buffer = context.buffer.data();
            std::string sPrint(buffer, n);
            if (!sPrint.empty() && sPrint.back() == '\n')
                sPrint.pop_back();
            LogHelper::information() << "Net Session ==============>TCP收到: " << sPrint;

            // 处理命令模式下的数据
            auto endPos = std::find(buffer, buffer + context.currentSize, '\n');
            if (endPos != buffer + context.currentSize)
            {
                // 处理找到的命令
                context.commandText.assign(buffer, endPos); // 不包括 '\n'
                processCommandMode_(context);

                // 如果是处理命令：是一条一条处理
                // 如果是处理数据：是从0开始接收
                context.currentSize = 0;
            }
        }
        else
        {
            // 处理文件接收模式下的数据
            if (context.currentSize >= context.dataSize)
            {
                processDataMode_(context);
                resetClientContext_(context);
            }
        }
    }

    stop();

    // 最后：通知上层（一定执行，确保从 sessions_ 移除）  // +++
    if (onExit_)
    {
        try
        { onExit_(); }
        catch (...)
        { /* 防御 */ }
    }

    LogHelper::information() << "Net Session Connection closed.\n";
}
