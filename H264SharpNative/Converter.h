#ifndef CONVERTER_LOCAL
#define CONVERTER_LOCAL
#include "Rgb2Yuv.h"
#include "Yuv2Rgb.h"
#include "ImageTypes.h"
namespace H264Sharp 
{
    class Converter 
    {
    public:
        const static int minSize = 640 * 480;
        
        static void Yuv420PtoRGB(unsigned char* dst_ptr,
            const unsigned char* y_ptr,
            const unsigned char* u_ptr,
            const unsigned char* v_ptr,
            signed   int   width,
            signed   int   height,
            signed   int   y_span,
            signed   int   uv_span,
            signed   int   dst_span,
            bool useSSE,
            int numThreads);

        static void Yuv420PtoRGB(YuvNative& yub, unsigned char* dst , bool useSSE, int numThreads);

        static void BGRAtoYUV420Planar(const unsigned char* bgra, unsigned char* dst, int width, int height, int stride, int threadCount);
        static void BGRtoYUV420Planar(unsigned char* bgr, unsigned char* dst, int width, int height, int stride, int threadCount);
        static void RGBAtoYUV420Planar(unsigned char* bgr, unsigned char* dst, int width, int height, int stride, int threadCount);
        static void RGBtoYUV420Planar(unsigned char* bgr, unsigned char* dst, int width, int height, int stride, int  threadCount);
        static void Downscale24(unsigned char* rgbSrc, int width, int height, int stride, unsigned char* dst, int multiplier);
        static void Downscale32(unsigned char* rgbSrc, int width, int height, int stride, unsigned char* dst, int multiplier);
    };
   
}



#endif