#ifndef CX_CT_X2_WIN_MEM_H
#define CX_CT_X2_WIN_MEM_H


#include <windows.h>
#include <cstddef>
#include <cstdint>

class WinMem {
public:
    void*  base = nullptr;
    SIZE_T reserved = 0;
    SIZE_T committed = 0;
    SIZE_T pageSize = 4096;
    SIZE_T commitChunk = 1ull * 1024ull * 1024ull; // 默认 1MB

    bool reserve(SIZE_T bytes);
    bool ensureCommitted(SIZE_T needBytes);
    void release();
};


#endif //CX_CT_X2_WIN_MEM_H
