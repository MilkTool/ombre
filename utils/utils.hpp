#pragma once
#include <Windows.h>
#include <string>
#include <codecvt>
#include <memory>
#include <iostream>
#include <functional>

// TODO: replace error handling - from exceptions to Result<T>
namespace win32 {
    std::wstring current_dir();
    /// <summary>
    /// Returns handle to <paramref="process_name" /> or nullptr of fail
    /// </summary>
    HANDLE process_handle(const std::wstring &process_name);
}

namespace str {
std::string ws2s(const std::wstring &wout);
std::wstring s2ws(const std::string &str);
}

struct Log {
    static void debug(const std::wstring &msg);
    static void info(const std::wstring &msg);
    static void error(const std::wstring &msg);
};

/// <summary>
/// Execute some code on scope leave
/// </summary>
class on_scope_leave_class {
public:
    using callback = std::function<void()>;

    on_scope_leave_class(const callback &on_destroy);

    // Disallow object move or copy
    on_scope_leave_class(const on_scope_leave_class &) = delete;
    on_scope_leave_class operator=(const on_scope_leave_class &) = delete;
    on_scope_leave_class(on_scope_leave_class &&) = delete;
    on_scope_leave_class operator=(on_scope_leave_class &&) = delete;

    ~on_scope_leave_class();

private:
    callback _on_destroy;
};

#define TOKENPASTE(x, y) x ## y
#define TOKENPASTE2(x, y) TOKENPASTE(x, y)
#define UNIQUE TOKENPASTE2(Unique_, __LINE__)

#define on_scope_leave(callback) on_scope_leave_class UNIQUE(callback)
