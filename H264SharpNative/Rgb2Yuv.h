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

			static void BGRAtoYUV420PlanarNeon(const unsigned char* bgra, unsigned char* dst, int width, int height, int stride, int threadCount);
			static void BGRtoYUV420PlanarNeon(unsigned char* bgr, unsigned char* dst, int width, int height, int stride, int threadCount);
			static void RGBAtoYUV420PlanarNeon(unsigned char* bgr, unsigned char* dst, int width, int height, int stride, int threadCount);
			static void RGBtoYUV420PlanarNeon(unsigned char* bgr, unsigned char* dst, int width, int height, int stride, int  threadCount);
	};
	
}
