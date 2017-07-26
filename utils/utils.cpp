#include <Windows.h>
#include <TlHelp32.h>
//#include <Psapi.h>
#include <string>
#include <codecvt>
#include <iostream>
#include <functional>
#include <vector>
#include <utils/utils.hpp>

//namespace win32::impl {
//    std::vector<DWORD> list_processes()
//    {
//        // TODO: please, refactor this :(
//        const DWORD size = 1024;
//        DWORD processes_raw[size]{};
//        DWORD size_in_bytes = 0;
//        EnumProcesses(processes_raw, size, &size_in_bytes);
//        return {processes_raw, processes_raw + (size_in_bytes / sizeof(DWORD))};
//    }
//}

namespace win32 {
    std::wstring current_dir()
    {
        DWORD buffer_size = GetCurrentDirectoryW(0, nullptr);
        if (!buffer_size)
            throw std::runtime_error("Could not retrive current directory -> " + std::to_string(GetLastError()));
        std::unique_ptr<wchar_t[]> buffer(new wchar_t[buffer_size] {});
        DWORD copied_characters = GetCurrentDirectoryW(buffer_size, buffer.get());
        if (!copied_characters)
            throw std::runtime_error("Could not retrive current directory -> " + std::to_string(GetLastError()));

        return std::wstring(reinterpret_cast<wchar_t *>(buffer.get()), copied_characters);
    }

    HANDLE process_handle(const std::wstring &process_name)
    {
        PROCESSENTRY32W entry{};
        entry.dwSize = sizeof(PROCESSENTRY32W);
        HANDLE snapshot_handle = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
        on_scope_leave([snapshot_handle] {
            CloseHandle(snapshot_handle);
        });

        HANDLE process_handle = INVALID_HANDLE_VALUE;
        if (Process32FirstW(snapshot_handle, &entry) == TRUE)
            while (Process32NextW(snapshot_handle, &entry) == TRUE)
                if (_wcsicmp(entry.szExeFile, process_name.c_str()) == 0)
                    process_handle = OpenProcess(PROCESS_ALL_ACCESS, FALSE, entry.th32ProcessID);

        return process_handle;
    }

}

namespace str {
    std::string ws2s(const std::wstring &wout)
    {
        typedef std::codecvt_utf8<wchar_t> convert_type;
        std::wstring_convert<convert_type, wchar_t> converter;
        return converter.to_bytes(wout);
    }

    std::wstring s2ws(const std::string &str)
    {
        std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
        return converter.from_bytes(str);
    }
}

void Log::debug(const std::wstring &msg)
{
#ifdef _DEBUG
    std::wcout << msg << std::endl;
#endif
}

void Log::info(const std::wstring &msg)
{
    std::wcout << msg << std::endl;
}

void Log::error(const std::wstring &msg)
{
    std::wcout << msg << std::endl;
}

on_scope_leave_class::on_scope_leave_class(const callback &on_destroy)
{
    _on_destroy = on_destroy;
}

on_scope_leave_class::~on_scope_leave_class()
{
    _on_destroy();
}
