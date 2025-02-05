#pragma once
#include "ThreadPool.h"
#include "pch.h"
#ifndef __arm__
#include <emmintrin.h>
#include <immintrin.h>
#endif
namespace H264Sharp
{

	class Rgb2Yuv
	{
		public:
			static void BGRAtoYUV420Planar(const unsigned char* RESTRICT bgra, unsigned char* RESTRICT dst, int width, int height, int stride, int threadCount);
			static void BGRtoYUV420Planar(unsigned char* RESTRICT bgr, unsigned char* RESTRICT dst, int width, int height, int stride, int threadCount);
			static void RGBAtoYUV420Planar(unsigned char* RESTRICT bgr, unsigned char* RESTRICT dst, int width, int height, int stride, int threadCount);
			static void RGBtoYUV420Planar(unsigned char* RESTRICT bgr, unsigned char* RESTRICT dst, int width, int height, int stride, int  threadCount);
#ifndef __arm__
			static void RGBToI420_AVX2(const uint8_t* RESTRICT src, uint8_t* RESTRICT y_plane, int width, int height, int stride, int  threadCount);
			static void RGBAToI420_AVX2(const uint8_t* RESTRICT src, uint8_t* RESTRICT y_plane, int width, int height, int stride, int  threadCount);
			static void BGRToI420_AVX2(const uint8_t* RESTRICT src, uint8_t* RESTRICT y_plane, int width, int height, int stride, int  threadCount);
			static void BGRAToI420_AVX2(const uint8_t* RESTRICT src, uint8_t* RESTRICT y_plane, int width, int height, int stride, int  threadCount);
#endif

#if defined(__aarch64__)

			static void BGRAtoYUV420PlanarNeon(const unsigned char* RESTRICT bgra, unsigned char* RESTRICT dst, int width, int height, int stride, int threadCount);
			static void BGRtoYUV420PlanarNeon(unsigned char* RESTRICT bgr, unsigned char* RESTRICT dst, int width, int height, int stride, int threadCount);
			static void RGBAtoYUV420PlanarNeon(unsigned char* RESTRICT bgr, unsigned char* RESTRICT dst, int width, int height, int stride, int threadCount);
			static void RGBtoYUV420PlanarNeon(unsigned char* RESTRICT bgr, unsigned char* RESTRICT dst, int width, int height, int stride, int  threadCount);
#endif
	};
	
}
