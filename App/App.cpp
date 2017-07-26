#include <Windows.h>
#include <iostream>
#include <thread>
#include <chrono>
#include <utils/utils.hpp>

typedef void (WINAPI *print_hello_t)();
typedef int (WINAPI *get_random_number_t)();

int wmain(int argc, LPWSTR argv[])
{
    HMODULE lib_handle = LoadLibraryW(L"Module.dll");
    on_scope_leave([lib_handle] {
        FreeLibrary(lib_handle);
    });

    while (true) {
        print_hello_t print_hello = reinterpret_cast<print_hello_t>(GetProcAddress(lib_handle, "print_hello"));
        get_random_number_t get_random_number =
            reinterpret_cast<get_random_number_t>(GetProcAddress(lib_handle, "get_random_number"));

        if (print_hello)
            print_hello();

        if (get_random_number) {
            UINT random_number = get_random_number();
            std::wcout << L"Random number is: " << random_number << std::endl;
            std::this_thread::sleep_for(std::chrono::milliseconds(random_number));
        }
    }

    return EXIT_SUCCESS;
}
