#include "pch.h"

#ifdef _WIN32

#include <Windows.h>

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}

#else // Linux

#include <iostream>

extern "C" void __attribute__((constructor)) dll_load(void);
extern "C" void __attribute__((destructor)) dll_unload(void);

void dll_load()
{
   // std::cout << "Library loaded.\n";
    // Perform initialization tasks here
}

void dll_unload()
{
    //std::cout << "Library unloaded.\n";
    // Perform cleanup tasks here
}

#endif // _WIN32
