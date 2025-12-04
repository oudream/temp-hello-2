/* ct_recon_api.h — 纯 C 接口，稳定 ABI，支持进度/日志回调与取消
 * 说明：与现有 CLDimensions3D/inputdata 对应的参数做了归一化与分层。
 * 作者：CYG-XRAY 项目接口重构草案
 */

#ifndef CX_CT_X2_RECON_API_H
#define CX_CT_X2_RECON_API_H

#ifdef __cplusplus
extern "C" {
#endif

/* ---- 导出宏（Windows / Linux / macOS 通用） ---- */
#if defined(_WIN32) || defined(_WIN64)
  #if defined(RECON_BUILD_DLL)
    #define RECON_API __declspec(dllexport)
  #else
    #define RECON_API __declspec(dllimport)
  #endif
#else
  #define RECON_API __attribute__((visibility("default")))
#endif

/* ---- 基本类型与错误码 ---- */
typedef void* RECON_Handle;       /* 上下文句柄，ct_recon_init 成功后获得 */
typedef void* RECON_UserData;     /* 用户自定义指针，回调时透传 */

enum RECON_Error {
    RECON_OK                     = 0,   /* 成功 */
    RECON_ERR_INVALID_ARG        = 1,   /* 参数无效 */
    RECON_ERR_NOT_INITIALIZED    = 2,   /* 未初始化 */
    RECON_ERR_OUT_OF_MEMORY      = 3,   /* 内存不足 */
    RECON_ERR_INTERNAL           = 4,   /* 内部错误 */
    RECON_ERR_UNSUPPORTED        = 5,   /* 不支持的功能/算法 */
    RECON_ERR_CANCELED           = 6,   /* 用户取消 */
    RECON_ERR_IO                 = 7,   /* 读写失败（为后续扩展预留） */
    RECON_ERR_DEVICE             = 8    /* GPU/加速设备相关错误（预留） */
};

/* ---- 日志级别 ---- */
enum RECON_LogLevel {
    RECON_LOG_TRACE = 0,
    RECON_LOG_DEBUG = 1,
    RECON_LOG_INFO  = 2,
    RECON_LOG_WARN  = 3,
    RECON_LOG_ERROR = 4
};

/* ---- 算法类型（可扩展）---- */
enum RECON_Algorithm {
    RECON_ALGO_FDK  = 0,  /* 经典 FDK */
    RECON_ALGO_CGLS = 1,  /* 共轭梯度最小二乘 */
    RECON_ALGO_SIRT = 2   /* 代数迭代 SIRT */
};

/* ---- 回调签名 ---- */
typedef void (*RECON_ProgressCallback)(int percent /*0..100*/, RECON_UserData ud);
typedef void (*RECON_LogCallback)(enum RECON_LogLevel level, const char* message, RECON_UserData ud);

/* ---- 初始化选项 ---- */
typedef struct RECON_InitOptions {
    int gpu_index;         /* GPU 序号，<0 表示 CPU 或默认策略 */
    int reserved0;         /* 预留 */
    int reserved1;         /* 预留 */
} RECON_InitOptions;

/* ---- 几何与标定（对齐 CLDimensions3D 的字段） ---- */
typedef struct RECON_Geometry {
    /* 体素尺寸（重建体三维大小，以体素数计） */
    unsigned int vol_dim_x;     /* 重建体素 X */
    unsigned int vol_dim_y;     /* 重建体素 Y */
    unsigned int vol_dim_z;     /* 重建体素 Z */

    /* 探测器/投影几何 */
    unsigned int proj_angles;   /* 采图张数 */
    unsigned int det_u;         /* 探测器列（宽）像素数 */
    unsigned int det_v;         /* 探测器行（高）像素数 */

    /* 物理尺寸（单位：mm，或与投影像素配套的真实物理单位） */
    float   pixel_size;         /* 单个探测器像素尺寸，如 0.0495*2 */
    float   voxel_size;         /* 重建体素物理尺寸 */
    float   radius_source;      /* 光源轨迹半径（类似 circleRs） */
    float   radius_detector;    /* 探测器轨迹半径（类似 circleRd） */
    float   sid;                /* 源到物体距离（Source-to-Isocenter Distance） */
    float   sdd;                /* 源到探测器距离（Source-to-Detector Distance） */

    /* 校正：偏移 + 倾斜（单位：像素/度） */
    int     offset_u;           /* 探测器列方向偏移（像素） */
    int     offset_v;           /* 探测器行方向偏移（像素） */
    float   tilt_u_deg;         /* 绕 U 轴倾斜角（度） */
    float   tilt_v_deg;         /* 绕 V 轴倾斜角（度） */
    float   tilt_inplane_deg;   /* 探测器平面内旋转角（度） */

    int     type;               /* 机型/几何类型：1=高级平面CT；0=倾斜圆锥束 等 */
    int     reserved[8];        /* 预留0~7 */
} RECON_Geometry;

/* ---- 迭代参数 ---- */
typedef struct RECON_IterParams {
    int   iter_count;           /* 迭代次数（如 100） */
    float max_constraint;       /* 迭代上限/约束 */
    float min_constraint;       /* 迭代下限/约束 */
} RECON_IterParams;

/* ---- 输入数据（两种模式任选其一）---- */
typedef struct RECON_InputSingle {
    /* 连续内存：proj_angles * det_v * det_u 张整型投影（如 uint16） */
    const void* data;           /* 指向连续投影数据起始地址 */
    unsigned long long bytes;   /* 数据总字节数 */
    int pixel_bits;             /* 像素位宽：8/16/32（支持 16 为主） */
} RECON_InputSingle;

/* ---------- 多个 RECON_InputSingle 作为一组 ---------- */
typedef struct RECON_InputSingles {
    const RECON_InputSingle* singles;   /* 单元数组首地址 */
    int count;                  /* 单元数量 */
} RECON_InputSingles;

/* ---------- 文件路径列表（UTF-8） ---------- */
typedef struct RECON_InputFiles {
    const char* const* paths_utf8; /* 字符串指针数组，UTF-8 编码 */
    int count;                     /* 文件数量，顺序即投影序 */
    int pixel_bits;                /* 8/16/32，供快速预分配/读取 */
} RECON_InputFiles;

/* ---- 输入视图 ---- */
enum RECON_InputKind {
    RECON_INPUT_FILES   = 0,    /* 文件路径列表 */
    RECON_INPUT_SINGLES = 1     /* 一组 RECON_InputSingle */
};

typedef struct RECON_InputView {
    enum RECON_InputKind kind;
    union {
        RECON_InputFiles   files;
        RECON_InputSingles singles;
    } u;
} RECON_InputView;

/* ---------- 算法选项（封装 algorithm/flags） ---------- */
typedef struct RECON_AlgoOptions {
    enum RECON_Algorithm algorithm;     /* FDK / CGLS / SIRT … */
    unsigned int flags;                 /* 扩展位：bit0=保存切片；bit1=启用前处理；… */
    /* 预留扩展 */
    int   reserved0;
} RECON_AlgoOptions;

/* ---------- 重建结果（封装 out_volume/out_bytes） ---------- */
enum RECON_DataType { RECON_DTYPE_F32 = 0 /* 目前固定 float32，后续可扩 U16/F16 等 */ };

typedef struct RECON_Response {
    float* out_volume;                  /* 调用方提供的外部缓冲 */
    unsigned long long out_bytes;       /* 外部缓冲大小（字节） */
    enum RECON_DataType dtype;          /* 目前固定 F32 */
    unsigned int     alignment;         /* 对齐字节（可选，0 表示不强制） */
    unsigned int     reserved_flags;    /* 扩展位：bit0=行对齐到 alignment；… */
} RECON_Response;

/* ---- 重建请求 ---- */
typedef struct RECON_Request {
    RECON_AlgoOptions  algo;            /* 算法选项 */
    RECON_Geometry     geom;            /* 几何参数 */
    RECON_IterParams   iter;            /* 迭代参数（FDK 可忽略） */
    RECON_InputView    input;           /* 输入图像数据（单块或多块） */

    /* 回调与取消 */
    RECON_ProgressCallback progress_cb; /* 进度回调，可为 NULL */
    RECON_LogCallback      log_cb;      /* 日志回调，可为 NULL */
    RECON_UserData         user_data;   /* 透传句柄 */
    volatile int*       cancel_flag;    /* 外部可置 1 取消，可为 NULL */
} RECON_Request;

/* ---- 仅 3 个对外函数 ---- */

/* 初始化：创建上下文 */
RECON_API int ct_recon_init(const RECON_InitOptions* opt, RECON_Handle* out_handle);

/* 反初始化：释放上下文 */
RECON_API int ct_recon_deinit(RECON_Handle handle);

/* 重建：阻塞式执行，支持回调、取消、外部/内部输出缓冲区 */
RECON_API int ct_recon_reconstruct(RECON_Handle handle, const RECON_Request* req, RECON_Response* resp);

#ifdef __cplusplus
}
#endif


#endif //CX_CT_X2_RECON_API_H