#include "win_mem.h"
#include <cstdio>
#include <cassert>

static void PrintLastError(const char* msg) {
    DWORD e = GetLastError();
    LPVOID lpMsgBuf = nullptr;
    FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM |
                   FORMAT_MESSAGE_IGNORE_INSERTS,
                   NULL, e, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                   (LPSTR)&lpMsgBuf, 0, NULL);
    std::fprintf(stderr, "%s (GetLastError=%lu): %s\n", msg, (unsigned long)e,
                 lpMsgBuf ? (char*)lpMsgBuf : "(no message)");
    if (lpMsgBuf) LocalFree(lpMsgBuf);
}

bool WinMem::reserve(SIZE_T bytes) {
    SYSTEM_INFO si{};
    GetSystemInfo(&si);
    pageSize = si.dwPageSize;
    if (commitChunk % pageSize != 0) commitChunk = pageSize * 256;

    base = VirtualAlloc(nullptr, bytes, MEM_RESERVE, PAGE_READWRITE);
    if (!base) {
        PrintLastError("VirtualAlloc MEM_RESERVE failed");
        return false;
    }
    reserved = bytes;
    committed = 0;
    return true;
}

bool WinMem::ensureCommitted(SIZE_T needBytes) {
    if (needBytes <= committed) return true;
    BYTE* cur = static_cast<BYTE*>(base) + committed;
    while (committed < needBytes) {
        SIZE_T step = (commitChunk < reserved - committed) ? commitChunk : reserved - committed;
        if (step == 0) return false;
        void* r = VirtualAlloc(cur, step, MEM_COMMIT, PAGE_READWRITE);
        if (!r) {
            PrintLastError("VirtualAlloc MEM_COMMIT failed");
            return false;
        }
        *cur = *cur; // 触发提交
        cur += step;
        committed += step;
    }
    return true;
}

void WinMem::release() {
    if (base) {
        if (!VirtualFree(base, 0, MEM_RELEASE)) {
            PrintLastError("VirtualFree MEM_RELEASE failed");
        }
        base = nullptr;
        reserved = committed = 0;
    }
}
