#include "tcp_server.h"



// 在相同的TCP连接中处理不同类型的数据：普通命令和文件传输。这意味着你的服务器需要能够根据接收到的命令切换其接收模式。
// 基本命令解析：初始状态下，服务器假设接收到的是普通命令。这些命令以 "\r\n" 结尾。当服务器接收到这样一个命令时，它会根据命令的内容来决定下一步操作。
// 处理文件接收命令：当服务器接收到一个特定的文件接收命令时（例如 "处理文件命令|文件名|文件大小"），它将进入文件接收模式。在这个模式下，服务器按照预定的文件大小接收数据，并将数据存储在内存中。完成后，服务器可以调用一个处理函数来处理接收到的文件。
//
// 超时和重置。如果在预定时间(10秒)内没有接收数据，服务器应主动断开连接。
enum class ClientMode { CommandReceiveMode, ImageDataReceiveMode };
std::string imagesPath; //图像保存路径

struct ClientContext
{
    int socket;
    ClientMode mode;
    std::vector<char> buffer;
    size_t currentSize;  // 当前缓冲区使用的大小
    std::string content; // 接收到的普通命令
    std::string imageName; // 要保存的图像文件名: img1.jpg  img2.png
    size_t imageSize; // 图像文件大小
    int imageNum; //图像数量
    long timestamp; //时间戳
    float score; //分数
    int rows;
    int cols;
    int matType; //opencv的图像格式--> 0:CV_8UC1, 16:CV_8UC3
    std::chrono::steady_clock::time_point lastCommunicationTime; // 最后通信时间
};



//返回bbox数据
void AnswerBboxData(ClientContext& context, const std::vector<uchar>& fileData)
{
    const size_t chunkSize = 4 * 1024 * 1024; // 4MB
    size_t totalSent = 0;
    size_t dataLength = fileData.size();

    while (totalSent < dataLength)
    {
        //size_t currentChunkSize = std::min(chunkSize, dataLength - totalSent);
        size_t remainingData = dataLength - totalSent;
        size_t currentChunkSize = (remainingData < chunkSize) ? remainingData : chunkSize;
        int n = send(context.socket, reinterpret_cast<const char*>(&fileData[totalSent]), currentChunkSize, 0);
        if (n == -1)
        {
            auto sErr = GetLastErrorString();
            logger->log(Logger::Level::ERR) << "Answer bbox data tcp error, " << sErr;
            break; // 发生错误，退出发送循环
        }
        totalSent += n;
        if (n < static_cast<int>(currentChunkSize))
        {
            // 未能发送完整的块，可能需要重试或处理
            logger->log(Logger::Level::WAR) << "Answer bbox data warning, sent " << n << " bytes out of " << currentChunkSize;
            // 可以在此处添加重试逻辑或其他处理
        }
    }

    if (totalSent == dataLength)
        logger->log(Logger::Level::INF) << "Answer bbox data ok.";
    else
        logger->log(Logger::Level::ERR) << "Answer bbox data ng, failed to send all bbox data.";
}
//返回命令响应
void AnswerCommand(ClientContext& context, const std::string& command, const std::vector<std::string>& params)
{
    std::ostringstream cmdStream;
    // 构建命令字符串
    cmdStream << command;
    for (const auto& param : params)
    {
        cmdStream << "|" << param;
    }
    auto cmdStreamShow = cmdStream.str();
    cmdStream << "\n";

    std::string fullCommand = cmdStream.str();

    // 发送命令
    int n = send(context.socket, fullCommand.c_str(), fullCommand.length(), 0);
    if (n == -1)
    {
        auto sErr = GetLastErrorString();
        logger->log(Logger::Level::ERR) << "Answer command tcp error, " << sErr << ", [" << cmdStreamShow << "]";
    }
    else
    {
        logger->log(Logger::Level::INF) << "Answer command ok, " << "[" << cmdStreamShow << "]";
    }
}


int LoadConfig()
{
    logger->log(Logger::Level::INF) << "Load config begin.";
    std::string productName = ""; //v53, v54, v62, v63, v64, v68
    std::string factory = "", imageName = "";


    IniReader ini;
    if (ini.LoadFromFile(InferConfigFile))
    {
        // get section
        if (false)
        {
            logger->log(Logger::Level::INF) << "Infer config section content:";
            std::map<std::string, std::string> section = ini.GetSection("Settings");
            if (section.size() > 0)
            {
                for (const auto& kv : section)
                {
                    logger->log(Logger::Level::INF) << kv.first << " = " << kv.second;
                }
            }
        }

        // get value
        auto sValue1 = ini.GetStrValue("Settings", "ProductType");
        auto sValue2 = ini.GetStrValue("Settings", "Factory");
        auto iValue3 = ini.GetIntValue("Settings", "Gpuid");
        auto iValue4 = ini.GetIntValue("Settings", "InferType");
        auto iValue5 = ini.GetIntValue("Settings", "InferParallel");

        auto lower1 = ToLowerCase(sValue1);
        auto lower2 = ToLowerCase(sValue2);
        if (lower1 == "v53") productName = lower1;
        if (lower1 == "v54") productName = lower1;
        if (lower1 == "x9610") productName = lower1;
        if (lower1 == "x9600") productName = lower1;
        if (lower1 == "x9601") productName = lower1;
        if (lower1 == "x8222") productName = lower1;
        if (lower1 == "x3755") productName = lower1;
        if (lower1 == "xAuto") productName = lower1;
        if (lower2 == "avc") factory = "AVC";
        if (lower2 == "aac") factory = "AAC";

        ProductName = productName;
        Factory = factory;
        if (iValue3 == 1) Gpuid = iValue3;
        if (iValue4 == 1) InferType = iValue4;
        if (iValue5 > 0) InferParallel = iValue5;

        WhiteDotAreaLmt1 = ini.GetIntValue("Settings", "WhiteDotAreaLmt1");
        WhiteDotVarianceLmt1 = ini.GetIntValue("Settings", "WhiteDotVarianceLmt1");
        WhiteDotAreaLmt2 = ini.GetIntValue("Settings", "WhiteDotAreaLmt2");
        WhiteDotVarianceLmt2 = ini.GetIntValue("Settings", "WhiteDotVarianceLmt2");

        ParticleAreaLmt1 = ini.GetIntValue("Settings", "ParticleAreaLmt1");
        ParticleVarianceLmt1 = ini.GetIntValue("Settings", "ParticleVarianceLmt1");
        ParticleAreaLmt2 = ini.GetIntValue("Settings", "ParticleAreaLmt2");
        ParticleVarianceLmt2 = ini.GetIntValue("Settings", "ParticleVarianceLmt2");

        MergeDistance = ini.GetIntValue("Settings", "MergeDistance");
        ParticleAreaLmtInWeld = ini.GetIntValue("Settings", "ParticleAreaLmtInWeld");

        PixelPerMm = ini.GetFloatValue("Settings", "PixelPerMm");
        ParticleDivideAspectRatio = ini.GetFloatValue("Settings", "ParticleDivideAspectRatio");
        ParticleDivideArea = ini.GetFloatValue("Settings", "ParticleDivideArea");
        ParticleDivideDensity = ini.GetFloatValue("Settings", "ParticleDivideDensity");
        ParticleDivideGray = ini.GetFloatValue("Settings", "ParticleDivideGray");

        ParticleFilterX1 = ini.GetFloatValue("Settings", "ParticleFilterX1");
        ParticleFilterY1 = ini.GetFloatValue("Settings", "ParticleFilterY1");
        ParticleFilterA1 = ini.GetFloatValue("Settings", "ParticleFilterA1");
        ParticleFilterU1 = ini.GetFloatValue("Settings", "ParticleFilterU1");

        ParticleFilterX2 = ini.GetFloatValue("Settings", "ParticleFilterX2");
        ParticleFilterY2 = ini.GetFloatValue("Settings", "ParticleFilterY2");
        ParticleFilterA2 = ini.GetFloatValue("Settings", "ParticleFilterA2");
        ParticleFilterU2 = ini.GetFloatValue("Settings", "ParticleFilterU2");

        RaisedFilterXL = ini.GetFloatValue("Settings", "RaisedFilterXL");
        RaisedFilterYL = ini.GetFloatValue("Settings", "RaisedFilterYL");
        RaisedFilterAL = ini.GetFloatValue("Settings", "RaisedFilterAL");

        RaisedFilterXS = ini.GetFloatValue("Settings", "RaisedFilterXS");
        RaisedFilterYS = ini.GetFloatValue("Settings", "RaisedFilterYS");
        RaisedFilterAS = ini.GetFloatValue("Settings", "RaisedFilterAS");

        OverflowAFilterX = ini.GetFloatValue("Settings", "OverflowAFilterX");
        OverflowAFilterY = ini.GetFloatValue("Settings", "OverflowAFilterY");
        OverflowAFilterA = ini.GetFloatValue("Settings", "OverflowAFilterA");
        OverflowAFilterU = ini.GetFloatValue("Settings", "OverflowAFilterU");

        Raised2FilterRatioA = ini.GetFloatValue("Settings", "Raised2FilterRatioA");
        Raised2FilterRatioW = ini.GetFloatValue("Settings", "Raised2FilterRatioW");
        Raised2FilterRatioH = ini.GetFloatValue("Settings", "Raised2FilterRatioH");
    }

    if (productName == "") //1是V53, 2是V54
    {
        logger->log(Logger::Level::ERR) << "Error product type, need to v53 or v54 or v68 ";
        return -1;
    }
    if (factory == "") //avc, aac
    {
        logger->log(Logger::Level::ERR) << "Error factory type, need to AAC or AVC ";
        return -1;
    }

    logger->log(Logger::Level::INF) << "product_name:" << productName
        << ", factory:" << Factory
        << ", gpu:" << Gpuid
        << ", infer_type:" << InferType
        << ", infer_parallel:" << InferParallel
        << ", whitedot_area_lmt1:" << WhiteDotAreaLmt1
        << ", whitedot_variance_lmt1:" << WhiteDotVarianceLmt1
        << ", whitedot_area_lmt2:" << WhiteDotAreaLmt2
        << ", whitedot_variance_lmt2:" << WhiteDotVarianceLmt2

        << ", particle_area_lmt1:" << ParticleAreaLmt1
        << ", particle_variance_lmt1:" << ParticleVarianceLmt1
        << ", particle_area_lmt2:" << ParticleAreaLmt2
        << ", particle_variance_lmt2:" << ParticleVarianceLmt2

        << ", merge_dist:" << MergeDistance
        << ", particle_area_lmt_in_weld:" << ParticleAreaLmtInWeld

        << ", pixel_per_mm:" << PixelPerMm

        << ", particle_divide_aspect_ratio:" << ParticleDivideAspectRatio
        << ", particle_divide_area:" << ParticleDivideArea
        << ", particle_divide_density:" << ParticleDivideDensity
        << ", particle_divide_gray:" << ParticleDivideGray

        << ", particle_filter_x1:" << ParticleFilterX1
        << ", particle_filter_y1:" << ParticleFilterY1
        << ", particle_filter_a1:" << ParticleFilterA1
        << ", particle_filter_u1:" << ParticleFilterU1

        << ", particle_filter_x2:" << ParticleFilterX2
        << ", particle_filter_y2:" << ParticleFilterY2
        << ", particle_filter_a2:" << ParticleFilterA2
        << ", particle_filter_u2:" << ParticleFilterU2

        << ", raised_filter_xl:" << RaisedFilterXL
        << ", raised_filter_yl:" << RaisedFilterYL
        << ", raised_filter_al:" << RaisedFilterAL

        << ", raised_filter_xs:" << RaisedFilterXS
        << ", raised_filter_ys:" << RaisedFilterYS
        << ", raised_filter_as:" << RaisedFilterAS

        << ", overflowA_filter_x:" << OverflowAFilterX
        << ", overflowA_filter_y:" << OverflowAFilterY
        << ", overflowA_filter_a:" << OverflowAFilterA
        << ", overflowA_filter_u:" << OverflowAFilterU

        << ", raised2_filter_ratio_a:" << Raised2FilterRatioA
        << ", raised2_filter_ratio_w:" << Raised2FilterRatioW
        << ", raised2_filter_ratio_h:" << Raised2FilterRatioH


        << ".";

    return 0;
}

//解析接收到的命令, 已去除了末尾的'\n'
void ProcessCommand(ClientContext& context)
{
    auto& productName = ProductName;
    auto &content = context.content;
    if (content.find("DetectFile") == 0 || content.find("DealFile") == 0)
    {
        //收到的命令格式为：$"DetectFile|{imageName}|{imageSize}|{imageNum}|{timestamp}|0.5|{height}|{width}|{matTypeInt}\n";
        //收到的命令格式为： "DetectFile|imageName|9437184|2|1722483905340|0.5|3072|3072|0\n"
        size_t delim1 = content.find('|');
        size_t delim2 = content.find('|', delim1 + 1);
        size_t delim3 = content.find('|', delim2 + 1);
        size_t delim4 = content.find('|', delim3 + 1);
        size_t delim5 = content.find('|', delim4 + 1);
        size_t delim6 = content.find('|', delim5 + 1);
        size_t delim7 = content.find('|', delim6 + 1);
        size_t delim8 = content.find('|', delim7 + 1);
        //size_t delim9 = content.find('|', delim8 + 1);

        // 提取图像文件名
        context.imageName = content.substr(delim1 + 1, delim2 - delim1 - 1);
        // 提取图像文件大小
        context.imageSize = std::stoul(content.substr(delim2 + 1, delim3 - delim2 - 1));
        // 提取图像数量
        context.imageNum = std::stoi(content.substr(delim3 + 1, delim4 - delim3 - 1));
        // 提取时间戳
        context.timestamp = std::stoll(content.substr(delim4 + 1, delim5 - delim4 - 1));
        // 提取分数
        context.score = std::stod(content.substr(delim5 + 1, delim6 - delim5 - 1));
        // 提取rows
        context.rows = std::stoi(content.substr(delim6 + 1, delim7 - delim6 - 1));
        // 提取cols
        context.cols = std::stoi(content.substr(delim7 + 1, delim8 - delim7 - 1));
        // 提取matTypeInt
        context.matType = std::stoi(content.substr(delim8 + 1));
        // 设置模式为文件接收模式
        context.mode = ClientMode::ImageDataReceiveMode;

        std::vector<std::string> params = { "OK", context.imageName, GetNowTimeStr(), productName };
        AnswerCommand(context, "DetectFile", params);
        //DetectFile|OK|图像名|2024-10-27 20:49:23|v53\n
    }
    else if (content.find("Heartbeat") == 0)
    {
        std::string modelNames = "";
        int modelIdMax = ModelIdMax();
        for (int modelId = 1; modelId <= modelIdMax; modelId++)
        {
            auto ln = (LN)modelId;
            auto modelName = Ln2Str(ln);
            auto exportTime = modelInfer->GetModelExportTime(modelId);
            auto startT = modelInfer->AqRess_[modelId].StartTime;
            auto endT = modelInfer->AqRess_[modelId].ModelState;

            modelNames += modelName + "," + exportTime + "," + startT + "," + endT + "" + ";";
        }
        auto modelNames_ = modelNames.substr(0, modelNames.length() - 1);

        std::vector<std::string> params = { version_, modelNames_, GetNowTimeStr(), productName };
        AnswerCommand(context, "Heartbeat", params);
        //"Heartbeat|1.241028.1|NozzleCrack,2024-10-27 20:17:39;WhiteDot,2024-10-27 20:49:23;WeldDot,2024-10-27 20:50:39|1730108539770|v53\n"
    }
    else if (content.find("UpdateConfig") == 0)
    {
        // 命令码|功能码|参数1|参数2
        // 参数为：k1=v1 形式
        // "UpdateConfig|none\n"
        // "UpdateConfig|reboot|delay=3000\n"
        // "UpdateConfig|readfile|k1=v1|k2=v2\n"
        // "UpdateConfig|writefile|k1=v1|k2=v2\n"

        std::string bk = "NG", a2 = "";
        bool bIsReboot = false;
        int rebootDelayMs = 0;
        size_t delim1 = content.find('|');
        if (delim1 < 9999)
        {
            size_t delim2 = content.find('|', delim1 + 1);
            if (delim2 < 9999)
            {
                a2 = content.substr(delim1 + 1, delim2 - delim1 - 1);
                if (a2 == "reboot")
                {
                    auto a3 = content.substr(delim2 + 1);
                    size_t delim3 = a3.find('=');
                    rebootDelayMs = std::stoi(a3.substr(delim3 + 1));
                    bk = "OK";
                    bIsReboot = true;
                }
            }
            else
            {
                a2 = content.substr(delim1 + 1, delim2 - delim1 - 1);
                if (a2 == "none")
                {
                    auto ret = LoadConfig();
                    if (ret == 0) bk = "OK";
                }
            }
        }

        std::vector<std::string> params = { a2, bk, GetNowTimeStr(), productName };
        AnswerCommand(context, "UpdateConfig", params);

        if (bIsReboot)
        {
            logger->log(Logger::Level::INF) << "Exit exe.";
            Sleep(rebootDelayMs);
            exit(0);
        }

        //"UpdateConfig|none|OK|2025-03-26 25:19:00|v53\n"
    }
    else
    {
        // 处理其他类型的命令
        // ...
    }
}
//处理接收到的图像文件
void ProcessImage(ClientContext& context)
{
    if (context.content.find("DetectFile") == 0)
    {
        // 写入数据
        if (context.imageSize > 0 && context.imageSize * context.imageNum < 40 * 1024 * 1024)
        {
            auto bboxData = InferImage(context.buffer.data(), context.imageSize, context.rows, context.cols, context.matType);
            auto errMsgs = GetInferErrMsgs();
            if (bboxData.size() > 0)
            {
                //DetectFile|OK|CVCHFC1005000014L0+5B08B09SP13,CVCHFC1105100014L0+5B08B10SP13|63|2025-04-30 11:28:43|0.5000|v53
                //DetectFile|ERROR|CVCHFC1005000014L0+5B08B09SP13,CVCHFC1105100014L0+5B08B10SP13|0|2025-04-30 11:28:43|0.5000|v53|错误信息1|错误信息2|...|错误信息n
                std::vector<std::string> params = { "OK", context.imageName, ToString(bboxData.size()), GetNowTimeStr(), ToString(context.score), ProductName };

                bool bIsErr = errMsgs.size() > 0;
                if (bIsErr)
                {
                    params[0] = "ERROR";
                    params[2] = "0";
                    for (auto &errMsg : errMsgs)
                        params.push_back(errMsg);
                }

                AnswerCommand(context, "DetectFile", params);
                Sleep(1);
                if (bIsErr == false)
                {
                    AnswerBboxData(context, bboxData);
                    logger->log(Logger::Level::INF) << "Answer ok, imageName:" << context.imageName << ", imageSize:" << context.imageSize
                        << ", imageNum:" << context.imageNum
                        << ", bboxSize:" << bboxData.size()
                        << ", timestamp:" << context.timestamp << ", now:" << GetNowTimeStr();
                }
            }
        }
    }
#pragma region 其它的TCP测试命令
    else if (context.content.find("DealFile") == 0)
    {
        if (context.imageSize > 0 && context.imageSize * context.imageNum < 40 * 1024 * 1024)
        {
            /*auto visImgData = modelInfer->InferImage2VisImg(1, context.buffer.data(), context.imageSize, context.score, context.imageName, "");
            if (visImgData.size() > 0)
            {
                std::vector<std::string> params = { context.imageName, ToString(visImgData.size()), GetNowTimeStr(), ToString(context.score) };
                AnswerCommand(context, "DealFile", params);
                Sleep(0);
                AnswerBboxData(context, visImgData);
                logger->log(Logger::Level::INF) << "SendFileData Finish, org.file.size: " << context.imageSize << ", detectionFileSize: " << visImgData.size()
                    << ", timestamp: " << context.timestamp << ", now: " << GetNowTimeMs();
            }*/
        }
    }
    else if (context.content.find("SaveFile") == 0)
    {
        /*if (context.imageSize > 0 && context.imageSize * context.imageNum < 40 * 1024 * 1024)
        {
            auto saveFileName = CombinePath(imagesPath, context.imageName);
            auto visImgData = modelInfer->InferImage2VisImg(1, context.buffer.data(), context.imageSize, context.score, "", saveFileName);
            auto bok = visImgData.size() > 0;
            auto level = bok ? Logger::Level::INF : Logger::Level::ERR;
            auto ss = bok ? "Save vis_img ok" : "Save vis_img ng";
            logger->log(level) << ss << ", timestamp: " << context.timestamp << ", now: " << GetNowTimeMs();
        }*/
    }
    else if (context.content.find("TestFile") == 0)
    {
        // 写入数据
        //if (context.imageSize > 0 && context.imageSize * context.imageNum < 40 * 1024 * 1024)
        //{
        //    std::string filePath = CombinePath(imagesPath, context.imageName);
        //    // 打开文件以写入，以二进制模式
        //    std::ofstream file(filePath, std::ios::out | std::ios::binary);
        //    if (!file)
        //    {
        //        // 文件打开失败
        //        logger->log(Logger::Level::ERR) << "processFileChunk Fail, Open [" << filePath << "]  Error.";
        //        return;
        //    }
        //    // 写入数据
        //    file.write(context.buffer.data(), context.imageSize);
        //    if (file.good())
        //    {
        //        logger->log(Logger::Level::ERR) << "TestFile [" << filePath << "]  FileSize: " << context.imageSize
        //            << ", timestamp: " << context.timestamp << ", now: " << GetNowTimeStr();
        //    }
        //}
    }
#pragma endregion

}
//重置上下文，准备接收新的命令
void ResetClientContext(ClientContext& context)
{
    context.mode = ClientMode::CommandReceiveMode;
    context.imageName.clear();
    context.imageSize = 0;
    context.imageNum = 0;
    // 不需要处理将未处理的数据移动到 buffer 的开头
    context.currentSize = 0;
}


void HandleClient(int clientSocket)
{
    using std::chrono::steady_clock;
    using std::chrono::duration_cast;
    using std::chrono::seconds;

    ClientContext context;
    context.socket = clientSocket;
    context.mode = ClientMode::CommandReceiveMode;
    context.buffer.resize(40 * 1024 * 1024); // 分配40MB的空间
    context.currentSize = 0;
    context.lastCommunicationTime = steady_clock::now(); // 初始化最后通信时间


    fd_set readfds;
    struct timeval tv;
    int retval;

    while (true)
    {
        FD_ZERO(&readfds);
        FD_SET(clientSocket, &readfds);

        tv.tv_sec = 5;  // 设置超时时间为3秒
        tv.tv_usec = 0;

        retval = select(clientSocket + 1, &readfds, NULL, NULL, &tv);

        if (retval == -1)
        {
            logger->log(Logger::Level::ERR) << "select() error";  // 记录错误
            break; // 发生错误
        }
        else if (retval == 0)
        {
            // 检查自上次通信以来是否已经超过10秒
            if (duration_cast<seconds>(steady_clock::now() - context.lastCommunicationTime).count() >= 10)
            {
                logger->log(Logger::Level::ERR) << "No communication for 10 seconds. Closing connection.";
                break;
            }
            continue;
        }

        size_t availableSpace = 40 * 1024 * 1024 - context.currentSize;
        int n = recv(clientSocket, &context.buffer[context.currentSize], availableSpace, 0);
        if (n <= 0)
        {
            if (n == 0)
                logger->log(Logger::Level::INF) << "Client disconnected.";
            else
                logger->log(Logger::Level::ERR) << "recv() error";  // 记录错误
            break; // 客户端断开连接或发生错误
        }

        logger->log(Logger::Level::INF) << "==============>TCP收到数据长度: " << n;

        context.currentSize += n; // 更新已使用的缓冲区大小
        context.lastCommunicationTime = steady_clock::now(); // 更新最后通信时间

        // 根据当前模式处理数据
        if (context.mode == ClientMode::CommandReceiveMode)
        {
            std::string aasd(context.buffer.data(), n);
            if (!aasd.empty() && aasd.back() == '\n')
                aasd.pop_back();
            logger->log(Logger::Level::INF) << "==============>TCP收到: " << aasd;

            // 处理命令模式下的数据
            auto endPos = std::find(context.buffer.begin(), context.buffer.begin() + context.currentSize, '\n');
            if (endPos != context.buffer.begin() + context.currentSize)
            {
                // 处理找到的命令
                context.content.assign(context.buffer.begin(), endPos); // 不包括 '\n'
                ProcessCommand(context);

                // 需要处理将未处理的数据移动到 buffer 的开头, 计算已处理的数据长度
                size_t processedLength = std::distance(context.buffer.begin(), endPos) + 1;
                if (context.currentSize - processedLength > 0)
                {
                    // 使用 memcpy 将剩余的数据移动到 buffer 的开头
                    std::memcpy(context.buffer.data(), context.buffer.data() + processedLength, context.currentSize - processedLength);
                    // 更新 currentSize
                    context.currentSize -= processedLength;
                }
                else
                {
                    context.currentSize = 0;
                }
            }
        }
        else if (context.mode == ClientMode::ImageDataReceiveMode)
        {
            // 处理文件接收模式下的数据
            if (context.currentSize >= context.imageSize * context.imageNum)
            {
                ProcessImage(context);
                ResetClientContext(context);
            }
        }
    }

    closesocket(clientSocket);
    logger->log(Logger::Level::INF) << "Connection closed.\n";
}

int SrvTcp()
{
    unsigned short version = MAKEWORD(2, 2);
    WSADATA status;
    status.wVersion = 0;
    WSAStartup(version, &status);

    int serverSocket, clientSocket;
    struct sockaddr_in serverAddr, clientAddr;
    socklen_t clientAddrSize = sizeof(clientAddr);

    // 创建服务器套接字
    serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket < 0)
    {
        logger->log(Logger::Level::ERR) << "Error creating socket.";
        exit(EXIT_FAILURE);
    }


    // 设置服务器地址信息
    int port = Gpuid == 0 ? 7352 : 7354;

    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(port);  // 设置端口
    serverAddr.sin_addr.s_addr = INADDR_ANY; //监听来自任何IP地址的连接。服务端和客户端都在同一台机器上时IP=127.0.0.1

    //const char* ip_str = "127.0.0.1"; // 点分十进制表示的IP地址
    //if (inet_pton(AF_INET, ip_str, &serverAddr.sin_addr) <= 0)
    //{
    //    logger->log(Logger::Level::ERR) << "Invalid IP address: " << ip_str;
    //    return 1; // 返回错误代码
    //}


    // 绑定套接字
    if (bind(serverSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0)
    {
        auto sErr = GetLastErrorString();
        logger->log(Logger::Level::ERR) << "Error binding socket: " + sErr;
        exit(EXIT_FAILURE);
    }


    // 监听连接请求
    if (listen(serverSocket, 5) < 0)
    {
        logger->log(Logger::Level::ERR) << "Error listening for connections.";
        exit(EXIT_FAILURE);
    }

    logger->log(Logger::Level::INF) << "Server listening on port " << port << " ...\n";
    //std::cout << "Server listening on port 12345..." << std::endl;

    while (true)  // 不断接收新客户端
    {
        clientSocket = accept(serverSocket, (struct sockaddr*)&clientAddr, &clientAddrSize);
        if (clientSocket < 0)
        {
            logger->log(Logger::Level::ERR) << "Error accepting client connection.";
            continue; // 处理下一个连接
        }
        logger->log(Logger::Level::INF) << "Client connected, IP:" << clientAddr.sin_addr.s_addr << ", Port:" << clientAddr.sin_port;

        // 为每个客户端创建一个新线程
        // 设置接收缓存大小为 40MB
        //int recv_buf_size = 40 * 1024 * 1024; // 40MB
        //if (setsockopt(clientSocket, SOL_SOCKET, SO_RCVBUF, (char*)&recv_buf_size, sizeof(recv_buf_size)) < 0)
        //{
        //    logger->log(Logger::Level::ERR) << "Failed to set receive buffer size.";
        //    closesocket(clientSocket);
        //    continue;
        //}

        // 获取设置后的接收缓存大小
        /*int actual_recv_buf_size = 0;
        socklen_t optlen = sizeof(actual_recv_buf_size);
        if (getsockopt(clientSocket, SOL_SOCKET, SO_RCVBUF, (char*)&actual_recv_buf_size, &optlen) < 0)
        {
            logger->log(Logger::Level::ERR) << "Failed to get receive buffer size.";
            closesocket(clientSocket);
            continue;
        }*/

        std::thread clientThread(HandleClient, clientSocket);
        clientThread.detach(); // 让线程独立执行, 然后马上执行下一句
    }

    // 关闭服务器套接字（理论上永远不会到达这里）
    closesocket(serverSocket);

    return 0;
}


