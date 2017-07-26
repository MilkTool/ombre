#include <Windows.h>
#include <Dbghelp.h>
#include <iostream>

extern "C" void __declspec(dllexport) print_hello()
{
    std::wcout << L"Hello form dll" << std::endl;
}

extern "C" int __declspec(dllexport) get_random_number()
{
    return 1000;
}

// hook function for GetProcAddress
FARPROC WINAPI GetProcAddress_hooked(_In_ HMODULE hModule, _In_ LPCSTR  lpProcName)
{
    if (std::string(lpProcName) == "print_hello")
        return (FARPROC)print_hello;
    else if (std::string(lpProcName) == "get_random_number")
        return (FARPROC)get_random_number;
    else
        return GetProcAddress(hModule, lpProcName);
}

//void api_hook()
//{
//    ULONG ulSize = 0;
//    HMODULE hModule = GetModuleHandleW(nullptr);
//    PROC pNewFunction = (PROC)GetProcAddress_hooked;
//    PROC pActualFunction = GetProcAddress(GetModuleHandleW(L"Kernel32.dll"), "GetProcAddress");
//    PIMAGE_IMPORT_DESCRIPTOR pImportDesc = (PIMAGE_IMPORT_DESCRIPTOR)ImageDirectoryEntryToData(hModule, TRUE, IMAGE_DIRECTORY_ENTRY_IMPORT, &ulSize);
//    if (!pImportDesc)
//        return;
//    for (; pImportDesc->Name; pImportDesc++) {
//        // get the module name
//        PSTR pszModName = (PSTR)((PBYTE)hModule + pImportDesc->Name);
//        if (!pszModName)
//            continue;
//
//        // check if the module is kernel32.dll
//        if (lstrcmpiA(pszModName, "Kernel32.dll") != 0)
//            continue;
//        // get the module
//        PIMAGE_THUNK_DATA pThunk = (PIMAGE_THUNK_DATA)((PBYTE)hModule + pImportDesc->FirstThunk);
//
//        for (; pThunk->u1.Function; pThunk++) {
//            PROC* ppfn = (PROC*)&pThunk->u1.Function;
//            if (*ppfn == pActualFunction)
//                continue;
//
//            WriteProcessMemory(GetCurrentProcess(), ppfn, &pActualFunction, sizeof(pNewFunction), NULL);
//            DWORD dwTest = GetLastError();
//
//            if (!WriteProcessMemory(GetCurrentProcess(), ppfn, &pActualFunction, sizeof(pNewFunction), NULL))
//                continue;
//            DWORD dwOldProtect = 0;
//            if (!VirtualProtect(ppfn, sizeof(pNewFunction), PAGE_WRITECOPY, &dwOldProtect))
//                continue;
//            // perform the write ....
//            WriteProcessMemory(GetCurrentProcess(), ppfn, &pNewFunction, sizeof(pNewFunction), NULL);
//            VirtualProtect(ppfn, sizeof(pNewFunction), dwOldProtect, &dwOldProtect);
//        }
//    }
//}
//
//BOOL APIENTRY DllMain(HMODULE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved)
//{
//    if (ul_reason_for_call == DLL_PROCESS_ATTACH)
//        api_hook();
//
//    return TRUE;
//}
