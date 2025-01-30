#pragma once
#include "ThreadPool.h"
namespace H264Sharp
{
	class Rgb2Yuv
	{
		public:
			static void BGRAtoYUV420Planar(const unsigned char* bgra, unsigned char* dst, int width, int height, int stride, int threadCount);
			static void BGRtoYUV420Planar(unsigned char* bgr, unsigned char* dst, int width, int height, int stride, int threadCount);
			static void RGBAtoYUV420Planar(unsigned char* bgr, unsigned char* dst, int width, int height, int stride, int threadCount);
			static void RGBtoYUV420Planar(unsigned char* bgr, unsigned char* dst, int width, int height, int stride, int  threadCount);

			static void RGBToI420_AVX2(const uint8_t* src, uint8_t* y_plane, int width, int height, int stride, int  threadCount);
			static void RGBAToI420_AVX2(const uint8_t* src, uint8_t* y_plane, int width, int height, int stride, int  threadCount);
			static void BGRToI420_AVX2(const uint8_t* src, uint8_t* y_plane, int width, int height, int stride, int  threadCount);
			static void BGRAToI420_AVX2(const uint8_t* src, uint8_t* y_plane, int width, int height, int stride, int  threadCount);

#if defined(__aarch64__)

			static void BGRAtoYUV420PlanarNeon(const unsigned char* bgra, unsigned char* dst, int width, int height, int stride, int threadCount);
			static void BGRtoYUV420PlanarNeon(unsigned char* bgr, unsigned char* dst, int width, int height, int stride, int threadCount);
			static void RGBAtoYUV420PlanarNeon(unsigned char* bgr, unsigned char* dst, int width, int height, int stride, int threadCount);
			static void RGBtoYUV420PlanarNeon(unsigned char* bgr, unsigned char* dst, int width, int height, int stride, int  threadCount);
#endif
	};
	
}
