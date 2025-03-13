#pragma once
#include "ThreadPool.h"
#include "pch.h"

namespace H264Sharp
{

	class Rgb2Yuv
	{
		public:
			template <int NUM_CH, bool IS_RGB>
			static void RGBXtoYUV420Planar(const uint8_t* RESTRICT bgra,  uint8_t* RESTRICT dst, int32_t width, int32_t height, int32_t stride, int32_t numThreads);
#ifndef ARM

			template <int NUM_CH, bool IS_RGB>
			void static RGBXToI420_AVX2(const uint8_t* RESTRICT src,  uint8_t* RESTRICT y_plane, int32_t width, int32_t height, int32_t stride, int32_t numThreads);

			template <int NUM_CH, bool IS_RGB>
			void static RGBToI420_SSE(const uint8_t* RESTRICT src, uint8_t* RESTRICT y_plane, int32_t width, int32_t height, int32_t stride, int32_t numThreads);
#endif

#if defined(ARM64)

			template <int NUM_CH, bool IS_RGB>
			static void RGBXtoYUV420PlanarNeon(const uint8_t* RESTRICT bgr,  uint8_t* RESTRICT dst, int32_t width, int32_t height, int32_t stride, int32_t threadCount);
#endif
	};
	
}
