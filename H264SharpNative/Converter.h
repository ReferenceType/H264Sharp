#ifndef CONVERTER_LOCAL
#define CONVERTER_LOCAL
#include "Rgb2Yuv.h"
#include "Yuv2Rgb.h"
#include "ImageTypes.h"
namespace H264Sharp 
{
    struct ConverterConfig {
        int NumthreadsRgb2Yuv = 1;
        int NumthreadsYuv2Rgb = 1;
        int EnableSSE = 1;
        int EnableNeon = 1;
        int EnableAvx2 = 1;
        int EnableAvx512 = 1;
    };
    class Converter 
    {
    public:
        const static int minSize = 640 * 480;
        static int EnableSSE __attribute__((visibility("default")));
        static int EnableNEON __attribute__((visibility("default")));
        static int NumThreads __attribute__((visibility("default")));
        static void Yuv420PtoRGB(unsigned char* dst_ptr,
            const unsigned char* y_ptr,
            const unsigned char* u_ptr,
            const unsigned char* v_ptr,
            signed   int   width,
            signed   int   height,
            signed   int   y_span,
            signed   int   uv_span,
            signed   int   dst_span);


        static void BGRAtoYUV420Planar(const unsigned char* bgra, unsigned char* dst, int width, int height, int stride);
        static void BGRtoYUV420Planar(unsigned char* bgr, unsigned char* dst, int width, int height, int stride);
        static void RGBAtoYUV420Planar(unsigned char* bgr, unsigned char* dst, int width, int height, int stride);
        static void RGBtoYUV420Planar(unsigned char* bgr, unsigned char* dst, int width, int height, int stride);
        static void Downscale24(unsigned char* rgbSrc, int width, int height, int stride, unsigned char* dst, int multiplier);
        static void Downscale32(unsigned char* rgbSrc, int width, int height, int stride, unsigned char* dst, int multiplier);

        static void SetConfig(ConverterConfig& config) 
        {
            Converter::EnableSSE = config.EnableSSE;
            Converter::EnableNEON = config.EnableNeon;
            Converter::NumThreads = config.NumthreadsRgb2Yuv;
            std::cout << "SET";
        }
    };
    
}



#endif