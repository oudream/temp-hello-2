/* src/ct_recon_impl.c
 * 目标：演示 DLL 侧对 ct_recon_api.h 的完整实现（示例/Mock）
 * 语言：C（C99）
 * 平台：Windows/Linux/macOS
 * 说明：此示例不会做真实 FDK/SIRT/CGLS ，而是做：
 *   1) 严格的参数校验（几何、输入、输出缓冲区）
 *   2) 双输入模式（文件列表/内存块）统一视图
 *   3) 回调（进度/日志）与取消
 *   4) 将一个“投影均值”的简单函数写入体素（float32）作为占位结果
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <errno.h>

#if defined(_WIN32) || defined(_WIN64)
  #include <windows.h>
  static void ms_sleep(unsigned ms) { Sleep(ms); }
#else
  #include <time.h>
  static void ms_sleep(unsigned ms) {
      struct timespec ts;
      ts.tv_sec = ms / 1000;
      ts.tv_nsec = (ms % 1000) * 1000000L;
      nanosleep(&ts, NULL);
  }
#endif

#define RECON_BUILD_DLL
#include "ct_recon_api.h"

/* ------------------ 内部结构体：上下文 ------------------ */
typedef struct RECON_InternalContext {
    int gpu_index;  /* 示例中未真正使用，仅占位 */
    int reserved0;
} RECON_InternalContext;

/* ------------------ 日志辅助 ------------------ */
static void log_msg(const RECON_Request* req, enum RECON_LogLevel lv, const char* msg) {
    if (req && req->log_cb) {
        req->log_cb(lv, msg, req->user_data);
    }
}

/* ------------------ 进度回调辅助 ------------------ */
static void report_progress(const RECON_Request* req, int percent) {
    if (req && req->progress_cb) {
        if (percent < 0) percent = 0;
        if (percent > 100) percent = 100;
        req->progress_cb(percent, req->user_data);
    }
}

/* ------------------ 取消判定 ------------------ */
static int is_canceled(const RECON_Request* req) {
    if (req && req->cancel_flag) {
        /* 按定义：外部可将 *cancel_flag 设为 1 以取消 */
        return (*req->cancel_flag != 0);
    }
    return 0;
}

/* ------------------ 输入统一读取：文件模式（演示版） ------------------
 * 真实实现中：请在此按需使用高性能读取（mmap/多线程/矢量化/IO 排队等）
 * 这里仅模拟从文件读入 uint8/uint16/uint32，转为 float32 累加用
 */
static int read_single_file_to_buffer(const char* path, void** out_buf, unsigned long long* out_bytes) {
    FILE* fp = fopen(path, "rb");
    if (!fp) return RECON_ERR_IO;

    fseek(fp, 0, SEEK_END);
    long long sz = ftell(fp);
    if (sz < 0) { fclose(fp); return RECON_ERR_IO; }
    fseek(fp, 0, SEEK_SET);

    void* buf = malloc((size_t)sz);
    if (!buf) { fclose(fp); return RECON_ERR_OUT_OF_MEMORY; }

    size_t rd = fread(buf, 1, (size_t)sz, fp);
    fclose(fp);
    if (rd != (size_t)sz) { free(buf); return RECON_ERR_IO; }

    *out_buf = buf;
    *out_bytes = (unsigned long long)sz;
    return RECON_OK;
}

/* 将任意 8/16/32bit 像素缓冲转为 float32 并计算一个“均值”作为简单代表值 */
static float compute_mean_from_buffer(const void* data, unsigned long long bytes, int pixel_bits) {
    if (!data || bytes == 0) return 0.0f;
    unsigned long long count = 0;
    long double sum = 0.0L;

    if (pixel_bits == 8) {
        const uint8_t* p = (const uint8_t*)data;
        count = bytes;
        for (unsigned long long i = 0; i < count; ++i) sum += p[i];
    } else if (pixel_bits == 16) {
        const uint16_t* p = (const uint16_t*)data;
        count = bytes / 2ULL;
        for (unsigned long long i = 0; i < count; ++i) sum += p[i];
    } else if (pixel_bits == 32) {
        const uint32_t* p = (const uint32_t*)data;
        count = bytes / 4ULL;
        for (unsigned long long i = 0; i < count; ++i) sum += p[i];
    } else {
        return 0.0f;
    }

    if (count == 0) return 0.0f;
    long double mean = sum / (long double)count;
    return (float)mean;
}

/* ------------------ Mock 重建内核：按均值生成体数据 ------------------
 * 为了演示 output、进度与取消，我们将“体数据”填充为简单函数：
 *   val(x,y,z) = 基于输入投影均值的一个归一化数 * (0.2 + 0.8 * z/volZ)
 * 以此体现体素沿 z 方向有梯度，同时不同输入会影响基值。
 */
static int do_mock_recon_write_volume(const RECON_Request* req,
                                      RECON_Response* resp,
                                      float base_value)
{
    if (!resp || !resp->out_volume) return RECON_ERR_INVALID_ARG;

    const unsigned vx = req->geom.vol_dim_x;
    const unsigned vy = req->geom.vol_dim_y;
    const unsigned vz = req->geom.vol_dim_z;
    const unsigned long long voxels = (unsigned long long)vx * vy * vz;
    const unsigned long long need_bytes = voxels * sizeof(float);

    if (resp->dtype != RECON_DTYPE_F32) return RECON_ERR_UNSUPPORTED;
    if (resp->out_bytes < need_bytes) return RECON_ERR_INVALID_ARG;

    float* out = resp->out_volume;

    /* 以 z 为主的渐变 + 基值（0..1 之间） */
    for (unsigned z = 0; z < vz; ++z) {
        if (is_canceled(req)) return RECON_ERR_CANCELED;
        float zf = (vz > 1) ? ((float)z / (float)(vz - 1)) : 0.0f;
        float slice_gain = 0.2f + 0.8f * zf;  /* 0.2..1.0 */

        /* 每写完一层给一点进度（这里粗略地用 20..95） */
        int progress = 20 + (int)(75.0f * ((float)z / (float) (vz ? vz : 1)));
        report_progress(req, progress);

        for (unsigned y = 0; y < vy; ++y) {
            unsigned long long row_off = ((unsigned long long)z * vy + y) * vx;
            for (unsigned x = 0; x < vx; ++x) {
                out[row_off + x] = base_value * slice_gain;
            }
        }
        /* 模拟一点耗时（真实算法请替换为计算） */
        ms_sleep(1);
    }

    return RECON_OK;
}

/* ------------------ 根据输入视图，计算一个“基础均值” ------------------ */
static int compute_base_value_from_input(const RECON_Request* req, float* out_base) {
    if (!req || !out_base) return RECON_ERR_INVALID_ARG;

    const unsigned expected_per_proj_pixels =
        req->geom.det_u * req->geom.det_v;
    const unsigned long long expected_total_pixels =
        (unsigned long long)expected_per_proj_pixels * req->geom.proj_angles;

    /* 我们只做“粗略校验”和“取均值” */
    long double sum_means = 0.0L;
    int chunks = 0;

    if (req->input.kind == RECON_INPUT_SINGLES) {
        const RECON_InputSingles* S = &req->input.u.singles;
        if (!S || !S->singles || S->count <= 0) return RECON_ERR_INVALID_ARG;

        for (int i = 0; i < S->count; ++i) {
            if (is_canceled(req)) return RECON_ERR_CANCELED;
            const RECON_InputSingle* si = &S->singles[i];
            if (!si || !si->data || si->bytes == 0) return RECON_ERR_INVALID_ARG;

            /* 容量粗验：像素数是否能整除 */
            if ((si->pixel_bits != 8) && (si->pixel_bits != 16) && (si->pixel_bits != 32)) {
                return RECON_ERR_INVALID_ARG;
            }
            unsigned bytes_per_pixel = (unsigned)(si->pixel_bits / 8);
            if (bytes_per_pixel == 0) return RECON_ERR_INVALID_ARG;
            if ((si->bytes % bytes_per_pixel) != 0ULL) return RECON_ERR_INVALID_ARG;

            unsigned long long pixels = si->bytes / bytes_per_pixel;
            /* 非严格校验：允许多块合计与期望匹配；若确实要严格，可强校验 */
            (void)expected_total_pixels; /* 如需可使用它做更强校验 */

            float m = compute_mean_from_buffer(si->data, si->bytes, si->pixel_bits);
            sum_means += (long double)m;
            ++chunks;
        }
    } else if (req->input.kind == RECON_INPUT_FILES) {
        const RECON_InputFiles* F = &req->input.u.files;
        if (!F || !F->paths_utf8 || F->count <= 0) return RECON_ERR_INVALID_ARG;

        for (int i = 0; i < F->count; ++i) {
            if (is_canceled(req)) return RECON_ERR_CANCELED;

            void* buf = NULL;
            unsigned long long bytes = 0ULL;
            int rc = read_single_file_to_buffer(F->paths_utf8[i], &buf, &bytes);
            if (rc != RECON_OK) return rc;

            float m = compute_mean_from_buffer(buf, bytes, F->pixel_bits);
            free(buf);
            sum_means += (long double)m;
            ++chunks;
        }
    } else {
        return RECON_ERR_INVALID_ARG;
    }

    if (chunks == 0) { *out_base = 0.0f; return RECON_OK; }

    long double mean_of_means = sum_means / (long double)chunks;

    /* 简单归一化到 0..1（基于像素位宽粗略范围），仅为可视化占位 */
    /* 真实实现：你应当进行偏置/归一化/平场/几何校正等复杂处理 */
    float norm = 0.0f;
    if (req->input.kind == RECON_INPUT_FILES) {
        int bits = req->input.u.files.pixel_bits;
        long double maxv = (bits == 8) ? 255.0L : (bits == 16 ? 65535.0L : 4294967295.0L);
        norm = (float)(mean_of_means / (maxv > 0.0L ? maxv : 1.0L));
    } else {
        /* Singles：用首块的 bit 宽做大致归一 */
        const RECON_InputSingles* S = &req->input.u.singles;
        int bits = (S && S->count > 0) ? S->singles[0].pixel_bits : 16;
        long double maxv = (bits == 8) ? 255.0L : (bits == 16 ? 65535.0L : 4294967295.0L);
        norm = (float)(mean_of_means / (maxv > 0.0L ? maxv : 1.0L));
    }

    if (norm < 0.0f) norm = 0.0f;
    if (norm > 1.0f) norm = 1.0f;
    *out_base = norm;
    return RECON_OK;
}

/* ------------------ API 实现 ------------------ */

RECON_API int ct_recon_init(const RECON_InitOptions* opt, RECON_Handle* out_handle) {
    if (!out_handle) return RECON_ERR_INVALID_ARG;
    *out_handle = NULL;

    RECON_InternalContext* ctx = (RECON_InternalContext*)calloc(1, sizeof(RECON_InternalContext));
    if (!ctx) return RECON_ERR_OUT_OF_MEMORY;

    if (opt) {
        ctx->gpu_index = opt->gpu_index;
    } else {
        ctx->gpu_index = -1; /* CPU 或默认策略 */
    }

    *out_handle = (RECON_Handle)ctx;
    return RECON_OK;
}

RECON_API int ct_recon_deinit(RECON_Handle handle) {
    if (!handle) return RECON_ERR_INVALID_ARG;
    RECON_InternalContext* ctx = (RECON_InternalContext*)handle;
    free(ctx);
    return RECON_OK;
}

RECON_API int ct_recon_reconstruct(RECON_Handle handle, const RECON_Request* req, RECON_Response* resp) {
    if (!handle) return RECON_ERR_NOT_INITIALIZED;
    if (!req || !resp) return RECON_ERR_INVALID_ARG;

    /* ---- 基本几何与输出缓冲区校验 ---- */
    if (req->geom.vol_dim_x == 0 || req->geom.vol_dim_y == 0 || req->geom.vol_dim_z == 0) {
        return RECON_ERR_INVALID_ARG;
    }
    if (req->geom.det_u == 0 || req->geom.det_v == 0 || req->geom.proj_angles == 0) {
        return RECON_ERR_INVALID_ARG;
    }
    if (!resp->out_volume || resp->dtype != RECON_DTYPE_F32) {
        return RECON_ERR_INVALID_ARG;
    }

    log_msg(req, RECON_LOG_INFO, "ct_recon_reconstruct: 开始重建（示例实现）");

    /* ---- 进度：0% ---- */
    report_progress(req, 0);

    /* ---- （示例）根据算法选项打印信息 ---- */
    {
        char buf[256];
        snprintf(buf, sizeof(buf),
                 "算法=%d flags=0x%08X, GPU索引=%d",
                 (int)req->algo.algorithm, req->algo.flags,
                 ((RECON_InternalContext*)handle)->gpu_index);
        log_msg(req, RECON_LOG_DEBUG, buf);
    }

    /* ---- （示例）迭代参数打印 ---- */
    {
        char buf[256];
        snprintf(buf, sizeof(buf),
                 "iter: count=%d, min=%.3f, max=%.3f",
                 req->iter.iter_count, req->iter.min_constraint, req->iter.max_constraint);
        log_msg(req, RECON_LOG_DEBUG, buf);
    }

    if (is_canceled(req)) return RECON_ERR_CANCELED;

    /* ---- 步骤1：读取或解析输入，得出一个“基值” ---- */
    float base_val = 0.0f;
    int rc = compute_base_value_from_input(req, &base_val);
    if (rc != RECON_OK) {
        log_msg(req, RECON_LOG_ERROR, "输入解析失败（示例）");
        return rc;
    }
    report_progress(req, 10);

    /* ---- 步骤2：模拟预处理（如平场、校正、滤波），这里只做等待 ---- */
    if (req->algo.flags & 0x2) { /* bit1=启用前处理（示例） */
        log_msg(req, RECON_LOG_INFO, "执行前处理（示例）...");
        for (int i = 0; i < 10; ++i) {
            if (is_canceled(req)) return RECON_ERR_CANCELED;
            ms_sleep(10);
            report_progress(req, 10 + i); /* 10..20% */
        }
    } else {
        report_progress(req, 20);
    }

    /* ---- 步骤3：执行“重建”（示例写体数据） ---- */
    rc = do_mock_recon_write_volume(req, resp, base_val);
    if (rc != RECON_OK) return rc;

    /* ---- 步骤4：可选保存切片（bit0），此处仅演示耗时 ---- */
    if (req->algo.flags & 0x1) {
        log_msg(req, RECON_LOG_INFO, "保存切片（示例耗时）...");
        for (int i = 0; i < 5; ++i) {
            if (is_canceled(req)) return RECON_ERR_CANCELED;
            ms_sleep(20);
            report_progress(req, 95 + i); /* 95..99% */
        }
    }

    report_progress(req, 100);
    log_msg(req, RECON_LOG_INFO, "ct_recon_reconstruct: 重建完成（示例实现）");
    return RECON_OK;
}
