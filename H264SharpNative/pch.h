// pch.h: This is a precompiled header file.
// Files listed below are compiled only once, improving build performance for future builds.
// This also affects IntelliSense performance, including code completion and many code browsing features.
// However, files listed here are ALL re-compiled if any one of them is updated between builds.
// Do not add files here that you will be updating frequently as this negates the performance advantage.

#ifndef PCH_H
#define PCH_H

#define WIN32_LEAN_AND_MEAN     

typedef unsigned char byte;

#ifdef _WIN32 // Windows-specific code

#include <windows.h>
#define DLL_LOAD_FUNCTION LoadLibrary
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


#include "codec_api.h"
#include "codec_app_def.h"
#include "codec_def.h"
#include "codec_ver.h"
//#include "ImageTypes.h"
//#include "ConverterLocal.h"
//#include "EncodedFrame.h"
//#include "TranscoderFactory.h"



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