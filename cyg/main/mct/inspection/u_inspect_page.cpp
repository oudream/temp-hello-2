#include "u_inspect_page.h"

#include "base/app_ids.h"

#include <test/vtk/t_vti_viewer_widget.h>
#include <test/vtk/t_vti_mpr3_view_widget.h>
#include <test/vtk/t_mpr_four_view_widget.h>
#include <test/vtk/t_smart_pointer.h>


#include <QToolButton>
#include <QFile>
#include <QHBoxLayout>
#include <sstream>
#include "base/colored_svg_icon.h"   // 你已有的头文件

static QToolButton* makeTestColoredButton(QWidget* parent, const QString& label, const QString& hex)
{
    auto* b = new QToolButton(parent);
    b->setText(label);
    b->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    b->setIconSize(QSize(32, 32));
    b->setFixedWidth(48);
    b->setAutoRaise(false);             // 关掉扁平，方便看背景
    b->setCheckable(true);                  // 点一下能切换 On/Off，看选中态颜色

    // 1) 直接硬编码 SVG 路径（Qt 支持 / 斜杠写法）
    const QString path = QStringLiteral("D:/ct/cx-ct-x2/build/deploy/AppData/Icons/Buildings/ancient-gate-fill.svg");

    // 2) 读 SVG 内容
    QFile f(path);
    QByteArray svg;
    if (f.open(QIODevice::ReadOnly)) {
        svg = f.readAll();
    } else {
        qWarning() << "SVG open failed:" << path;
        return b;
    }

    // 3) 快速校验一下是否是 currentColor
    if (!svg.contains("currentColor")) {
        qWarning() << "SVG does NOT contain 'currentColor' -> may render black. path =" << path;
    }

    // 4) 用你的引擎指定颜色
    auto* eng = new ColoredSvgIconEngine(svg);

    // 默认色（无状态时 fall back）
    eng->setDefaultColor(QColor("#9E9E9E"));

    // Normal / hover(Active) 的 Off 颜色
    eng->setColor(QIcon::Normal,  QIcon::Off, QColor(hex/*"#29B6F6"*/)); // 正常
    eng->setColor(QIcon::Active,  QIcon::Off, QColor(hex/*"#42A5F5"*/)); // 悬停

    // Checked(On) 颜色
    eng->setColor(QIcon::Normal,  QIcon::On,  QColor(hex/*"#FF7043"*/)); // 选中
    eng->setColor(QIcon::Active,  QIcon::On,  QColor(hex/*"#FF5722"*/)); // 选中+悬停

    // Disabled
    eng->setColor(QIcon::Disabled, QIcon::Off, QColor(hex/*"#BDBDBD"*/));

    b->setIcon(QIcon(eng));
    return b;
}

static QToolButton* makeColorButton(QWidget* parent, const QString& label, const QString& hex)
{
    auto* b = new QToolButton(parent);
    b->setText(label);
    b->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    b->setFixedSize(64, 64);
    b->setIconSize(QSize(32, 32));
    b->setCheckable(false);
    b->setAutoRaise(false);
    b->setStyleSheet(QString("QToolButton { background: %1; border: 1px solid #666; color: white; }").arg(hex));
    return b;
}

UInspectPage::UInspectPage(QWidget *parent) : UPageBase(parent), _root(nullptr)
{
    buildUi(parent);
}

UInspectPage::~UInspectPage()
{

}

const char *UInspectPage::getName() const
{
    return AppIds::kBlock_Inspection;
}

QWidget *UInspectPage::getWidget()
{
    return _root;
}

void UInspectPage::onEnter()
{

}

void UInspectPage::onLeave()
{

}

void UInspectPage::buildUi(QWidget *parent)
{
    _root = new QWidget(parent);
    auto *v = new QVBoxLayout(_root);
    v->setContentsMargins(0, 0, 0, 0);

    // === 加一个水平布局放按钮 ===
    if (false)
    {
        auto *colorRow = new QHBoxLayout();
        colorRow->setSpacing(6);

        // 套色方案 A
        colorRow->addWidget(makeColorButton(_root, "Default",       "#B0C4D9"));
        colorRow->addWidget(makeColorButton(_root, "normal_off",    "#C0C8D2"));
        colorRow->addWidget(makeColorButton(_root, "normal_on",     "#4FC3F7"));
        colorRow->addWidget(makeColorButton(_root, "active_off",    "#71D8FF"));
        colorRow->addWidget(makeColorButton(_root, "active_on",     "#29B5FF"));
        colorRow->addWidget(makeColorButton(_root, "selected_off",  "#64B5F6"));
        colorRow->addWidget(makeColorButton(_root, "selected_on",   "#1E88E5"));
        colorRow->addWidget(makeColorButton(_root, "disabled_off",  "#6E737B"));
        colorRow->addWidget(makeColorButton(_root, "disabled_on",   "#6E737B"));

        colorRow->addStretch();
        v->addLayout(colorRow);
    }

    // === 顶部按钮行 ===
    QHBoxLayout *h = new QHBoxLayout();
    _btnTest1 = new QPushButton(tr("测试1"), _root);
    _btnTest2 = new QPushButton(tr("测试2"), _root);
    _btnTest3 = new QPushButton(tr("测试3"), _root);

    h->addWidget(_btnTest1);
    h->addWidget(_btnTest2);
    h->addWidget(_btnTest3);
    h->addStretch();
    v->addLayout(h);

    connect(_btnTest1, SIGNAL(clicked()), this, SLOT(onTest1()));
    connect(_btnTest2, SIGNAL(clicked()), this, SLOT(onTest2()));
    connect(_btnTest3, SIGNAL(clicked()), this, SLOT(onTest3()));

    // === 原来的 viewer ===
    //    auto *viewer = new TVtiViewerWidget(_root);
    auto *viewer = new TVtiMpr3ViewWidget(_root);
    //    auto *viewer = new TMprFourViewWidget(_root);
    v->addWidget(viewer);
}

#include <reconstruction/reconstruction_client.h>

static void print_result_stats(const std::vector<float>& vol,
                               unsigned vx, unsigned vy, unsigned vz) {
    double sum = 0.0;
    double mn = 1e300, mx = -1e300;
    for (size_t i = 0; i < vol.size(); ++i) {
        double v = vol[i];
        sum += v;
        if (v < mn) mn = v;
        if (v > mx) mx = v;
    }
    double mean = vol.empty() ? 0.0 : (sum / (double)vol.size());
    std::cout << "[结果统计] 体素数=" << vol.size()
              << " min=" << mn << " max=" << mx << " mean=" << mean << "\n";
}
void printProgressBar(double progress, double elapsed, double estimated);
std::vector<std::string> generate_even_sequence(const std::string& prefix,
                                                int start,
                                                int end,
                                                int step,
                                                int padding,
                                                const std::string& suffix)
{
    std::vector<std::string> filenames;
    std::ostringstream oss;

    for (int i = start; i <= end; i += step) {
        oss.str("");
        oss << prefix
            << std::setw(padding) << std::setfill('0') << i
            << suffix;
        filenames.push_back(oss.str());
    }

    return filenames;
}


void UInspectPage::onTest1()
{
    // 1) 创建客户端 + 初始化
    CtReconClient cli;
    if (!cli.init(-1)) {
        std::cerr << "ct_recon_init 失败\n";
        return;
    }

    // 2) 设置回调
    CtReconClient::Callbacks cbs;
    cbs.onProgress = [](int p) {
        std::printf("进度: %3d%%\r", p);
        std::fflush(stdout);
    };
    cbs.onLog = [](RECON_LogLevel lv, const char* msg) {
        std::printf("[LOG %d] %s\n", (int)lv, msg);
    };
    cli.setCallbacks(cbs);

    // 3) 构造参数（示例值）

    const unsigned volx = 400, voly = 400, volz = 200;
    const unsigned detU = 588, detV = 461, projAngles = 720;

    RECON_Geometry geom = CtReconClient::makeGeometry(
            volx, voly, volz,
            projAngles, detU, detV,
            /*pixelSize*/ 0.0495f*5, /*voxelSize*/ 0.1f,
            /*radiusSrc*/ 46.735f, /*radiusDet*/ 184.602f,
            /*sid*/ 68.755f, /*sdd*/ 206.745f);


    RECON_AlgoOptions algo = CtReconClient::makeAlgo(RECON_ALGO_FDK, 1 /*flags*/ /*0x3 *//*保存切片+前处理(示例)*/);//RECON_ALGO_FDK
    RECON_IterParams iters = CtReconClient::makeIters(50, 0.0f, 1.0f);

    // 4) 示例一：从“文件列表”重建（把真实投影路径放进来）
    std::vector<std::string> fileList = generate_even_sequence(R"(D:\ct\data-volumes\CLimage\tif10\)", 0, 719, 1, 4, ".bmp");//2513
    // 示例：fileList.push_back("projs/p000.raw"); ... // 自行替换

    std::vector<float> volA;
    if (!fileList.empty()) {
        std::cout << "\n=== 从文件列表重建 ===\n";
        int rc = cli.reconstructFromFiles(algo, geom, iters, fileList, /*pixelBits*/16, volA);
        std::cout << "\n返回码 rc=" << rc << "\n";
//        if (rc == RECON_OK)  watchvol(volA.data(), volx, voly, volz);
        //  if (rc == RECON_OK) print_result_stats(volA, volx, voly, volz);
    }
    else {
        std::cout << "\n(跳过文件列表示例：fileList 为空)\n";
    }

//    // 5) 示例二：从“内存块集合”重建（模拟 16bit 原始投影）
//    std::cout << "\n=== 从内存块重建（模拟数据） ===\n";
//    // 创建一块“模拟投影数据”：projAngles * detV * detU * sizeof(uint16_t)
//    const unsigned long long pixelsPerProj = (unsigned long long)detU * detV;
//    const unsigned long long totalPixels = pixelsPerProj * projAngles;
//    const unsigned long long bytes = totalPixels * sizeof(uint16_t);
//
//    std::vector<uint16_t> fakeProj;
//    try { fakeProj.resize((size_t)totalPixels); }
//    catch (...) { std::cerr << "内存不足\n"; return 2; }
//
//    // 填充一些伪数据（渐变）
//    for (unsigned long long i = 0; i < totalPixels; ++i) {
//        fakeProj[i] = (uint16_t)(i % 4096); // 0..4095 循环
//    }
//
//    CtReconClient::MemoryChunk chunk;
//    chunk.data = (const void*)fakeProj.data();
//    chunk.bytes = bytes;
//    chunk.pixelBits = 16;
//
//    std::vector<CtReconClient::MemoryChunk> chunks;
//    chunks.push_back(chunk);
//
//    std::vector<float> volB;
//    int rc2 = cli.reconstructFromMemory(algo, geom, iters, chunks, volB);
//    std::cout << "返回码 rc=" << rc2 << "\n";
//    if (rc2 == RECON_OK) print_result_stats(volB, volx, voly, volz);
//
//    // 6) （可选）演示取消：在另一个线程把 cli.requestCancel() 设为 1。
//    //    这里用单线程不演示多线程取消。

}

void UInspectPage::onTest2()
{
}

void UInspectPage::onTest3()
{
}