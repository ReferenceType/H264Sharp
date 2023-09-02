// pch.h: This is a precompiled header file.
// Files listed below are compiled only once, improving build performance for future builds.
// This also affects IntelliSense performance, including code completion and many code browsing features.
// However, files listed here are ALL re-compiled if any one of them is updated between builds.
// Do not add files here that you will be updating frequently as this negates the performance advantage.

#ifndef PCH_H
#define PCH_H
public enum class FrameType { Invalid, IDR, I, P, Skip, IPMixed };

// add headers that you want to pre-compile here
#include "Windows.h"
#include "codec_api.h"
#include "codec_app_def.h"
#include "codec_def.h"
#include "codec_ver.h"
#include "ImageTypes.h"
#include "ConverterLocal.h"

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