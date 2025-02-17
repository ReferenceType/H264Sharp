#ifndef PCH_H
#define PCH_H

#if defined(__aarch64__) || defined(__ARM_ARCH)
#define __arm__
#endif

#include "codec_api.h"
#include "codec_app_def.h"
#include "codec_def.h"
#include "codec_ver.h"

#include <stdlib.h>
#include <stdio.h>
#include <cstddef>
#include <cstdlib>

#ifdef _WIN32 // Windows-specific code

#include <windows.h>
#include <malloc.h>  // for _aligned_malloc and _aligned_free

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
#endif



#ifndef RESTRICT_H
#define RESTRICT_H

#if defined(_MSC_VER) && !defined(__clang__)
#define RESTRICT __restrict  // MSVC
#elif defined(__clang__) || defined(__GNUC__)
#define RESTRICT __restrict__  // GCC, Clang (including Clang-Cl)
#else
#define RESTRICT  // Unknown
#endif

#endif 




constexpr size_t alignment = 64;

inline void* AllignAlloc(size_t capacity) {
    // Ensure capacity is a multiple of alignment
    if (capacity % alignment != 0) {
        capacity += alignment - (capacity % alignment);
    }

#ifdef _WIN32
    // Windows-specific aligned allocation
    void* ptr = _aligned_malloc(capacity, alignment);
#else
    // POSIX-compliant aligned allocation
    void* ptr = nullptr;
    if (posix_memalign(&ptr, alignment, capacity) != 0) {
        ptr = nullptr;  // Allocation failed
    }
#endif

    return ptr;
}

inline void FreeAllignAlloc(void* p) {
    if (p) {
#ifdef _WIN32
        _aligned_free(p);  
#else
        free(p);  // POSIX-compliant
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


#if defined(__aarch64__) || defined(__arm__)  // ARM-specific headers
#include <sys/auxv.h>   // For getauxval on Linux
#include <fstream>      // For /proc/cpuinfo
#include <string>
#if defined(__APPLE__)  // macOS-specific headers
#include <sys/sysctl.h>
#endif
#endif

#if defined(__aarch64__) || defined(__arm__)  // ARM-specific headers
#include <sys/auxv.h>   // For getauxval() on Linux/Android
#include <fstream>      // For /proc/cpuinfo
#include <string>
#if defined(__APPLE__)  // macOS-specific headers
#include <sys/sysctl.h>
#endif
#endif

inline bool hasNEON() {
#if defined(__x86_64__) || defined(_M_X64) || defined(__i386__) || defined(_M_IX86)
    return false;  // NEON is only for ARM, return false on x86

#elif defined(__aarch64__)  // ARM 64-bit

#if defined(__ANDROID__)  // Android (AArch64)
    return (getauxval(AT_HWCAP) & (1 << 1)) != 0;  // NEON is HWCAP bit 1

#elif defined(__APPLE__)  // macOS (M1/M2/M3 chips)
    int neon_supported = 0;
    size_t size = sizeof(neon_supported);
    sysctlbyname("hw.optional.neon", &neon_supported, &size, nullptr, 0);
    return neon_supported;

#else  // Linux (AArch64)
    return (getauxval(AT_HWCAP) & (1 << 1)) != 0;  // NEON is HWCAP bit 1
#endif

#elif defined(__arm__)  // ARM 32-bit

#if defined(__ANDROID__)  // Android (ARMv7)
    return (getauxval(AT_HWCAP) & (1 << 12)) != 0;  // NEON is HWCAP bit 12 on ARM32

#else  // Linux (ARMv7)
    std::ifstream cpuinfo("/proc/cpuinfo");
    std::string line;
    while (std::getline(cpuinfo, line)) {
        if (line.find("neon") != std::string::npos) {
            return true;
        }
    }
    return false;
#endif

#else
    return false;  // Unknown platform, assume no NEON
#endif
}

inline bool is64Bit() {
	const int* pInt = nullptr;
	if (sizeof(pInt) == 8)
	{
		return true;
	}
	else if (sizeof(pInt) == 4)
	{
		return false;
	}
}
#endif //PCH_H