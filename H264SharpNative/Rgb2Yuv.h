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

			template <int NUM_CH, bool IS_RGB>
			static void RGBXtoYUV420Planar(unsigned char* RESTRICT bgra, unsigned char* RESTRICT dst, int width, int height, int stride, int numThreads);			
#ifndef __arm__

			template <int NUM_CH, bool IS_RGB>
			void static RGBXToI420_AVX2(const uint8_t* RESTRICT src, uint8_t* RESTRICT y_plane, int width, int height, int stride, int numThreads);
#endif

#if defined(__aarch64__)

			template <int NUM_CH, bool IS_RGB>
			static void RGBXtoYUV420PlanarNeon(unsigned char* RESTRICT bgr, unsigned char* RESTRICT dst, int width, int height, int stride, int  threadCount);
#endif
	};
	
}
