#ifndef PCH_H
#define PCH_H


#if defined(__arm__) || defined(__aarch64__) || defined(_M_ARM) || defined(_M_ARM64) || defined(__ARM_ARCH)
#define ARM
#endif

#if defined(__aarch64__) || defined(_M_ARM64)
#define ARM64
#endif

#include "Logger.h"


#include "codec_api.h"
#include "codec_app_def.h"
#include "codec_def.h"
#include "codec_ver.h"

#include <stdlib.h>
#include <stdio.h>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <cstring>

#include <memory>
#include <new>

#ifdef _WIN32 // Windows-specific code
#define _WIN32_WINNT 0x0602  // Windows 8+
#include <windows.h>
#include <malloc.h>  // for _aligned_malloc and _aligned_free
#include <synchapi.h>

#define DLL_LOAD_FUNCTION LoadLibraryW
#define DLL_GET_FUNCTION GetProcAddress
#define DLL_CLOSE_FUNCTION FreeLibrary
#define DLL_EXTENSION L".dll"
#define DLL_ERROR_CODE GetLastError()

#else // Linux-specific

#include <dlfcn.h>
#define DLL_LOAD_FUNCTION dlopen
#define DLL_GET_FUNCTION dlsym
#define DLL_CLOSE_FUNCTION dlclose
#define DLL_EXTENSION ".so"
#define DLL_ERROR_CODE dlerror()
#if defined(__x86_64__) || defined(_M_X64) || defined(__i386__) || defined(_M_IX86)
#include <cpuid.h> 
#endif
#endif



#ifndef RESTRICT_H
#define RESTRICT_H

#if defined(_MSC_VER) && !defined(__clang__)
#define RESTRICT __restrict  
#elif defined(__clang__) || defined(__GNUC__)
#define RESTRICT __restrict__  
#else
#define RESTRICT
#endif

#endif 


static Logger logger;


constexpr size_t alignment = 64;

inline void* AllignAlloc(size_t capacity) {
    // Ensure capacity is a multiple of alignment
    if (capacity % alignment != 0) {
        capacity += alignment - (capacity % alignment);
    }

#ifdef _WIN32
    void* ptr = _aligned_malloc(capacity, alignment);
#else
    void* ptr = nullptr;
    if (posix_memalign(&ptr, alignment, capacity) != 0) {
        ptr = nullptr;
    }
#endif

    return ptr;
}

inline void FreeAllignAlloc(void* p) {
    if (p) {
#ifdef _WIN32
        _aligned_free(p);  
#else
        free(p);
#endif
    }
}



inline bool hasAVX512() {
#if defined(__x86_64__) || defined(_M_X64) || defined(__i386__) || defined(_M_IX86)

	int cpuInfo[4] = { 0,0,0,0 };

#ifdef _MSC_VER
	__cpuidex(cpuInfo, 7, 0);
#else
	__cpuid_count(7, 0, cpuInfo[0], cpuInfo[1], cpuInfo[2], cpuInfo[3]);
#endif

	return (cpuInfo[1] & (1 << 16)) != 0;  // Check EBX bit 16 for AVX-512F
#else
	return false;
#endif
}

inline bool hasAVX2() {
#if defined(__x86_64__) || defined(_M_X64) || defined(__i386__) || defined(_M_IX86)

	int cpuInfo[4] = { 0,0,0,0 };

#ifdef _MSC_VER
	__cpuidex(cpuInfo, 7, 0);
#else
	__cpuid_count(7, 0, cpuInfo[0], cpuInfo[1], cpuInfo[2], cpuInfo[3]);
#endif

	return (cpuInfo[1] & (1 << 5)) != 0;  // AVX2 is bit 5 of EBX (EAX=7, ECX=0)

#else
	return false;
#endif
}

inline bool hasSSE41() {
#if defined(__x86_64__) || defined(_M_X64) || defined(__i386__) || defined(_M_IX86)

    int cpuInfo[4] = { 0, 0, 0, 0 };

#ifdef _MSC_VER
    __cpuidex(cpuInfo, 1, 0);  // Use leaf 1 to check for SSE4.2
#else
    __cpuid_count(1, 0, cpuInfo[0], cpuInfo[1], cpuInfo[2], cpuInfo[3]);
#endif

    // Check bit 20 in ECX for SSE4.2 support
    return (cpuInfo[2] & (1 << 19)) != 0;
#else
    return false;
#endif
}


#if defined(ARM) 
#include <sys/auxv.h>   // For getauxval() on Linux/Android
#include <fstream>      // For /proc/cpuinfo
#include <string>
#if defined(__APPLE__)  
#include <sys/sysctl.h>
#endif
#endif

inline bool hasNEON() {
#if defined(__x86_64__) || defined(_M_X64) || defined(__i386__) || defined(_M_IX86)
    return false;  
#elif defined(__aarch64__)  
#if defined(__ANDROID__)  
    return true;
#elif defined(__APPLE__) 
    return true;
#else  
    return true;
#endif
#elif defined(__arm__)  
#if defined(__ANDROID__) 
#if defined(__ARM_NEON) || defined(__ARM_NEON__)
    return true;
#else
    return false;
#endif
#else  
#if defined(__ARM_NEON) || defined(__ARM_NEON__)
    return true;
#else
    // Fallback to reading /proc/cpuinfo
    std::ifstream cpuinfo("/proc/cpuinfo");
    std::string line;
    while (std::getline(cpuinfo, line)) {
        if (line.find("neon") != std::string::npos) {
            return true;
        }
    }
    return false;
#endif
#endif
#else
    return false;  // Unknown platform
#endif
}

inline bool is64Bit() {
	const int* pInt = nullptr;
    return sizeof(pInt) == 8 ? true : false;
	
}
#endif //PCH_H