#include <Windows.h>
#include <Dbghelp.h>
#include <Psapi.h>
#include <vector>
#include <utils/utils.hpp>

namespace app {
const std::wstring NAME = L"ombre";
const std::wstring VERSION = L"0.1.0";
const std::wstring DESCRIPTION = L"Hot-patch library on running process";
}

#define countof(x) (sizeof(x)/sizeof((x)[0]))

void cmd_print_usage()
{
    Log::info(app::NAME + L" " + app::VERSION + L" - " + app::DESCRIPTION);
    Log::info(L"Usage:");
    Log::info(L"  " + app::NAME + L" process.exe library.dll");
    Log::info(L"Where:");
    Log::info(L"  process.exe \t\t\t name of process to patch");
    Log::info(L"  library.dll \t\t\t name of library to reload");
}

int wmain_impl(std::vector<std::wstring> args)
{
    if (args.size() != 3) {
        Log::error(L"Invalid number of command line arguments");
        cmd_print_usage();
    }

    const std::wstring &process = args[1];
    const std::wstring &library = args[2];

    std::wstring library_path = win32::current_dir() + library;

    HANDLE process_handle = win32::process_handle(process);
    if (process_handle == INVALID_HANDLE_VALUE) {
        Log::error(L"Could not open \"" + process + L"\" process");
        return EXIT_FAILURE;
    }
    on_scope_leave([process_handle] {
        CloseHandle(process_handle);
    });

    const size_t library_path_size_in_bytes = library_path.size() * sizeof(wchar_t);
    PVOID  memory_block = VirtualAllocEx(process_handle, NULL,  library_path_size_in_bytes, MEM_COMMIT, PAGE_READWRITE);
    if (!memory_block) {
        Log::error(L"Allocation failure of " + std::to_wstring(library_path_size_in_bytes) + L" bytes in \"" + process + L"\" process memory");
        return EXIT_FAILURE;
    }
    on_scope_leave(([=] {
        VirtualFreeEx(process_handle, memory_block, library_path_size_in_bytes, MEM_RELEASE);
    }));

    SIZE_T bytes_written = 0;
    BOOL resut = WriteProcessMemory(process_handle, memory_block, library_path.c_str(), library_path_size_in_bytes, &bytes_written);
    if (!resut) {
        Log::error(L"Writing to process \"" + process + L"\" memory falied: " + std::to_wstring(GetLastError()));
        return EXIT_FAILURE;
    }
    if (bytes_written != library_path_size_in_bytes) {
        Log::error(L"Writing to process \"" + process + L"\" memory falied - wrote " + 
            std::to_wstring(bytes_written) + L" instead of " + std::to_wstring(library_path_size_in_bytes));
        return EXIT_FAILURE;
    }

    std::wstring kernel32_library = L"Kernel32";
    HMODULE kernel32_module_handle = GetModuleHandleW(kernel32_library.c_str());
    if (kernel32_module_handle == INVALID_HANDLE_VALUE) {
        Log::error(L"Could not load library \"" + kernel32_library + L"\"");
        return EXIT_FAILURE;
    }
    on_scope_leave([kernel32_module_handle] {
        CloseHandle(kernel32_module_handle);
    });

    HANDLE remote_thread_handle = CreateRemoteThread(process_handle, nullptr, 0,
                                    (LPTHREAD_START_ROUTINE)GetProcAddress(kernel32_module_handle, "LoadLibraryW"),
                                    memory_block, 0, nullptr);
    if (remote_thread_handle == INVALID_HANDLE_VALUE) {
        Log::error(L"Could not create thread in address space of \"" + process + L"\" process");
        return EXIT_FAILURE;
    }
    on_scope_leave([remote_thread_handle] {
        CloseHandle(remote_thread_handle);
    });

    WaitForSingleObject(remote_thread_handle, INFINITE);
    DWORD remote_thread_exit_code = 0;
    GetExitCodeThread(remote_thread_handle, &remote_thread_exit_code);
    Log::debug(L"Remote thread exited with code " + std::to_wstring(remote_thread_exit_code));

    return EXIT_SUCCESS;
}

int wmain(int argc, LPWSTR argv[])
{
    int exit_code = EXIT_FAILURE;
    try {
        exit_code = wmain_impl({argv, argv + argc});
    } catch (const std::exception &ex) {
        Log::error(L"Unhandled exception -> " + str::s2ws(ex.what()));
    } catch (...) {
        Log::error(L"Unhandled unrecognized exception");
    }
    
    Log::debug(L"Application exit code: " + std::to_wstring(exit_code));
    return exit_code;
}
