#include "cxassert.h"

#include <mutex>
#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <chrono>
#include <sstream>
#include <thread>
#include <stdexcept>
#include <string>

#ifdef _WIN32
#include <windows.h>
#elif defined(__linux__)
#include <X11/Xlib.h>
  #include <X11/Xutil.h>
  #include <X11/keysym.h>
#elif defined(__APPLE__)
  #include <CoreFoundation/CoreFoundation.h>
  #include <CoreFoundation/CFUserNotification.h>
#endif

// Optional Qt dialog (define USE_QT_ASSERT and link QtWidgets)
#ifdef USE_QT_ASSERT
#include <QMessageBox>
  #include <QString>
  #include <QCoreApplication>
#endif

namespace cx {

    // ---- internal singletons ----
    static std::mutex& Mtx() { static std::mutex m; return m; }
    static bool& BuiltinDialogEnabledRef() { static bool e = true; return e; }
    static AssertCallback& UserCbRef() { static AssertCallback cb; return cb; }

    void SetAssertCallback(AssertCallback cb) {
        std::lock_guard<std::mutex> lk(Mtx());
        UserCbRef() = std::move(cb);
    }
    void ClearAssertCallback() {
        std::lock_guard<std::mutex> lk(Mtx());
        UserCbRef() = nullptr;
    }
    void SetBuiltinDialogEnabled(bool enabled) {
        std::lock_guard<std::mutex> lk(Mtx());
        BuiltinDialogEnabledRef() = enabled;
    }

    // ---- time / tid / logging ----
    static std::string NowFileStamp() {
        using namespace std::chrono;
        auto tp = system_clock::now();
        auto ms = duration_cast<milliseconds>(tp.time_since_epoch()) % 1000;
        std::time_t t = system_clock::to_time_t(tp);
        std::tm tm_{};
#if defined(_WIN32)
        localtime_s(&tm_, &t);
#else
        localtime_r(&t, &tm_);
#endif
        char buf[64];
        std::snprintf(buf, sizeof(buf), "%04d%02d%02d_%02d%02d%02d_%03d",
                      tm_.tm_year + 1900, tm_.tm_mon + 1, tm_.tm_mday,
                      tm_.tm_hour, tm_.tm_min, tm_.tm_sec, int(ms.count()));
        return buf;
    }

    std::uint64_t CurrentThreadId() {
        auto id = std::this_thread::get_id();
        std::ostringstream oss; oss << id;
        try { return static_cast<std::uint64_t>(std::stoull(oss.str())); }
        catch (...) { return static_cast<std::uint64_t>(std::hash<std::string>{}(oss.str())); }
    }

    static void AppendLog(const AssertInfo& info) {
        std::string fname = "assert_" + NowFileStamp() + ".log";
        if (FILE* f = std::fopen(fname.c_str(), "w")) {
            std::fprintf(f,
                         "Assertion failed!\n"
                         "Expr: %s\n"
                         "File: %s(%d)\n"
                         "Func: %s\n"
                         "TID: %llu\n"
                         "%s%s\n",
                         info.expr, info.file, info.line, info.func,
                         static_cast<unsigned long long>(info.tid),
                         (info.message ? "Msg: " : ""),
                         (info.message ? info.message : ""));
            std::fclose(f);
        }
    }

    // ---- built-in dialogs ----
    // Priority: Qt > Win32 > X11 > macOS > stderr
    static AssertAction DefaultDialog(const AssertInfo& info) {
        if (!BuiltinDialogEnabledRef()) return AssertAction::Abort;

#ifdef USE_QT_ASSERT
        // Assumes a Q(Core)Application exists on this thread.
        QString text = QStringLiteral("Assertion failed\nExpr: %1\nFile: %2:%3\nFunc: %4\nTID: %5%6%7")
            .arg(QString::fromUtf8(info.expr))
            .arg(QString::fromUtf8(info.file)).arg(info.line)
            .arg(QString::fromUtf8(info.func))
            .arg(static_cast<qulonglong>(info.tid))
            .arg(info.message ? "\nMsg: " : "")
            .arg(info.message ? QString::fromUtf8(info.message) : QString{});
        auto ret = QMessageBox::critical(
            nullptr, QStringLiteral("Assertion Failed"), text,
            QMessageBox::Abort | QMessageBox::Ignore | QMessageBox::Close,
            QMessageBox::Abort);
        if (ret == QMessageBox::Ignore) return AssertAction::Ignore;
        if (ret == QMessageBox::Close)  return AssertAction::Exit;
        return AssertAction::Abort;

#elif defined(_WIN32)
        std::string text = std::string("Assertion failed\n")
                           + "Expr: " + info.expr
                           + "\nFile: " + info.file + ":" + std::to_string(info.line)
                           + "\nFunc: " + info.func
                           + (info.message ? ("\n\nMsg: " + std::string(info.message)) : "")
                           + "\n\n[Abort] terminate | [Retry] exit | [Ignore] continue";
        int r = MessageBoxA(nullptr, text.c_str(), "Assertion Failed",
                            MB_ICONERROR | MB_ABORTRETRYIGNORE | MB_DEFBUTTON1 | MB_TOPMOST);
        if (r == IDIGNORE) return AssertAction::Ignore;
        if (r == IDRETRY)  return AssertAction::Exit;
        return AssertAction::Abort;

#elif defined(__linux__)
        const char* disp = std::getenv("DISPLAY");
        if (disp && *disp) {
            Display* dpy = XOpenDisplay(nullptr);
            if (dpy) {
                int screen = DefaultScreen(dpy);
                Window root = RootWindow(dpy, screen);
                unsigned int w = 640, h = 280;
                Window win = XCreateSimpleWindow(dpy, root, 200, 200, w, h, 1,
                                                 BlackPixel(dpy, screen), WhitePixel(dpy, screen));
                XStoreName(dpy, win, "Assertion Failed");
                XSelectInput(dpy, win, ExposureMask | KeyPressMask | ButtonPressMask | StructureNotifyMask);
                Atom wmDelete = XInternAtom(dpy, "WM_DELETE_WINDOW", False);
                XSetWMProtocols(dpy, win, &wmDelete, 1);
                XMapRaised(dpy, win);
                GC gc = XCreateGC(dpy, win, 0, nullptr);
                XSetForeground(dpy, gc, BlackPixel(dpy, screen));

                auto drawLine = [&](int y, const std::string& s) {
                    XDrawString(dpy, win, gc, 16, y, s.c_str(), (int)s.size());
                };

                AssertAction result = AssertAction::Abort;
                bool done = false;
                while (!done) {
                    XEvent ev; XNextEvent(dpy, &ev);
                    if (ev.type == Expose) {
                        int y = 40;
                        drawLine(y,      "Assertion failed");
                        drawLine(y+=24,  std::string("Expr: ") + info.expr);
                        drawLine(y+=20,  std::string("File: ") + info.file + ":" + std::to_string(info.line));
                        drawLine(y+=20,  std::string("Func: ") + info.func);
                        if (info.message) drawLine(y+=20, std::string("Msg: ") + info.message);
                        drawLine(y+=36,  "Press [I]=Ignore, [E]=Exit, [A/Esc]=Abort. Closing window = Abort.");
                    } else if (ev.type == ClientMessage) {
                        if ((Atom)ev.xclient.data.l[0] == wmDelete) { result = AssertAction::Abort; done = true; }
                    } else if (ev.type == KeyPress) {
                        KeySym sym = XLookupKeysym(&ev.xkey, 0);
                        if (sym == XK_i || sym == XK_I)      { result = AssertAction::Ignore; done = true; }
                        else if (sym == XK_e || sym == XK_E) { result = AssertAction::Exit;   done = true; }
                        else if (sym == XK_a || sym == XK_A || sym == XK_Escape) { result = AssertAction::Abort; done = true; }
                    } else if (ev.type == ButtonPress) {
                        result = AssertAction::Abort; done = true;
                    }
                }
                XFreeGC(dpy, gc);
                XDestroyWindow(dpy, win);
                XCloseDisplay(dpy);
                return result;
            }
        }
        std::fprintf(stderr, "Assertion failed: %s (%s:%d)\n", info.expr, info.file, info.line);
        return AssertAction::Abort;

    #elif defined(__APPLE__)
        CFStringRef header   = CFStringCreateWithCString(nullptr, "Assertion Failed", kCFStringEncodingUTF8);
        std::string msg      = std::string("Expr: ") + info.expr +
                               "\nFile: " + info.file + ":" + std::to_string(info.line) +
                               "\nFunc: " + info.func +
                               (info.message ? ("\n\nMsg: " + std::string(info.message)) : "") +
                               "\n\n[Abort] terminate | [Exit] exit | [Ignore] continue";
        CFStringRef body     = CFStringCreateWithCString(nullptr, msg.c_str(), kCFStringEncodingUTF8);
        CFStringRef btnDef   = CFStringCreateWithCString(nullptr, "Abort",  kCFStringEncodingUTF8);
        CFStringRef btnAlt   = CFStringCreateWithCString(nullptr, "Exit",   kCFStringEncodingUTF8);
        CFStringRef btnOther = CFStringCreateWithCString(nullptr, "Ignore", kCFStringEncodingUTF8);

        CFOptionFlags response = 0;
        CFUserNotificationDisplayAlert(
            0, kCFUserNotificationStopAlertLevel,
            nullptr, nullptr, nullptr,
            header, body, btnDef, btnAlt, btnOther, &response);

        if (header)   CFRelease(header);
        if (body)     CFRelease(body);
        if (btnDef)   CFRelease(btnDef);
        if (btnAlt)   CFRelease(btnAlt);
        if (btnOther) CFRelease(btnOther);

        if (response == kCFUserNotificationOtherResponse)     return AssertAction::Ignore;
        if (response == kCFUserNotificationAlternateResponse) return AssertAction::Exit;
        return AssertAction::Abort;

    #else
        std::fprintf(stderr, "Assertion failed: %s\n", info.expr);
        return AssertAction::Abort;
#endif
    }

    // ---- main entry ----
    void AssertFail(const char* expr, const char* file, int line,
                    const char* func, const char* message) {
        // Reentrancy guard: if assertion triggers within handling, abort immediately.
        thread_local bool inAssert = false;
        if (inAssert) {
            std::fprintf(stderr, "[assert reentered] %s (%s:%d)\n", expr, file, line);
            std::abort();
        }
        inAssert = true;

        AssertInfo info { expr, file, line, func, message, CurrentThreadId() };

        // 1) Always append a log file
        AppendLog(info);

        // 2) User callback or built-in dialog
        AssertAction act;
        {
            std::lock_guard<std::mutex> lk(Mtx());
            if (UserCbRef()) act = UserCbRef()(info);
            else             act = DefaultDialog(info);
        }

        // 3) Execute action
        switch (act) {
            case AssertAction::Ignore:
                inAssert = false;
                return; // continue execution
            case AssertAction::Exit:
                std::exit(EXIT_FAILURE);
            case AssertAction::Throw:
                inAssert = false;
                throw std::runtime_error("Assertion failed");
            case AssertAction::Abort:
            default:
                std::abort();
        }
    }

} // namespace cx
