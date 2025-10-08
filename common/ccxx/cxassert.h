#ifndef CX_CT_X2_CXASSERT_H
#define CX_CT_X2_CXASSERT_H

#include <functional>
#include <cstdint>

namespace cx {

    enum class AssertAction {
        Ignore = 0,  // continue execution
        Exit,        // exit(EXIT_FAILURE)
        Throw,       // throw std::runtime_error
        Abort        // std::abort()
    };

    struct AssertInfo {
        const char* expr;     // expression text
        const char* file;     // source file
        int         line;     // line number
        const char* func;     // function name (__func__)
        const char* message;  // optional message (nullable)
        std::uint64_t tid;    // current thread id (hashed if not numeric)
    };

    using AssertCallback = std::function<AssertAction(const AssertInfo&)>;

// API
    void SetAssertCallback(AssertCallback cb);
    void ClearAssertCallback();
    void SetBuiltinDialogEnabled(bool enabled);
    std::uint64_t CurrentThreadId();
    void AssertFail(const char* expr, const char* file, int line,
                    const char* func, const char* message = nullptr);

}

// Macros
#define CX_ASSERT(expr) \
    do { if (!(expr)) ::cx::AssertFail(#expr, __FILE__, __LINE__, __func__, nullptr); } while (0)

#define CX_ASSERT_MSG(expr, msg_literal) \
    do { if (!(expr)) ::cx::AssertFail(#expr, __FILE__, __LINE__, __func__, (msg_literal)); } while (0)

#define CX_ASSERT_STRING(expr, msg_literal) \
    do { if (!(expr)) ::cx::AssertFail(#expr, __FILE__, __LINE__, __func__, (msg_literal.c_str())); } while (0)

// VERIFY returns the boolean value back to caller, but triggers assertion on false.
#define CX_VERIFY(expr) \
    ([&](){ auto __ok = static_cast<bool>(expr); if (!__ok) ::cx::AssertFail(#expr, __FILE__, __LINE__, __func__, nullptr); return __ok; }())

#define CX_UNREACHABLE(msg_literal) \
    (::cx::AssertFail("UNREACHABLE", __FILE__, __LINE__, __func__, (msg_literal)))

#define CX_UNREACHABLE_STRING(msg_literal) \
    (::cx::AssertFail("UNREACHABLE", __FILE__, __LINE__, __func__, (msg_literal.c_str())))

#ifndef CX_NO_OVERRIDE_C_ASSERT
#ifdef assert
#undef assert
#endif
#ifndef NDEBUG
#define assert(cond) CX_ASSERT(cond)
#else
#define assert(cond) ((void)0)
#endif
#endif

#endif //CX_CT_X2_CXASSERT_H
