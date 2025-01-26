#ifndef PCH_H
#define PCH_H

#define WIN32_LEAN_AND_MEAN     

typedef unsigned char byte;

#ifdef _WIN32 // Windows-specific code

#include <windows.h>
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
#ifdef __aarch64__
#pragma message("__aarch64__ is defined")
#endif
#if defined(__aarch64__) || defined(__ARM_ARCH)
#define __arm__
#endif

#include "codec_api.h"
#include "codec_app_def.h"
#include "codec_def.h"
#include "codec_ver.h"

static bool is64Bit() {
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