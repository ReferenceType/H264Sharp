#include "Converter.h"
#include "ThreadPool.h"
#include "Yuv2Rgb.h"
#include "Rgb2Yuv.h"
namespace H264Sharp {

   
    int Converter::EnableSSE = 1;
    int Converter::EnableNEON = 1;
    int Converter::NumThreads = 4;

    void Converter::Yuv420PtoRGB(unsigned char* dst_ptr,
        const unsigned char* y_ptr,
        const unsigned char* u_ptr,
        const unsigned char* v_ptr,
        signed   int   width,
        signed   int   height,
        signed   int   y_span,
        signed   int   uv_span,
        signed   int   dst_span)
    {
    
        int numThreads = Converter::NumThreads;
        
        if (Converter::EnableSSE>0 && width % 32 == 0)
        {
          
    #ifndef __arm__
            // SSE, may parallel, not arm
            Yuv2Rgb::yuv420_rgb24_sse(width,
                height,
                y_ptr,
                u_ptr,
                v_ptr,
                y_span,
                uv_span,
                dst_ptr,
                dst_span,
                numThreads);
            return;
    #elif defined(__aarch64__)
            if (Converter::EnableNEON > 0) {
                if(numThreads>1)
                    Yuv2Rgb::ConvertYUVToRGB_NEON_Parallel(
                        y_ptr,
                        u_ptr,
                        v_ptr,
                        dst_ptr,
                        width,
                        height, numThreads);
                else
                    Yuv2Rgb::ConvertYUVToRGB_NEONv2(
                        y_ptr,
                        u_ptr,
                        v_ptr,
                        dst_ptr,
                        width,
                        height);
            }
                
            else 
            {
                Yuv2Rgb::Yuv420P2RGBDefault(dst_ptr,
                    y_ptr,
                    u_ptr,
                    v_ptr,
                    width,
                    height,
                    y_span,
                    uv_span,
                    dst_span,
                    numThreads);
            }
                
    #else
            Yuv2Rgb::Yuv420P2RGBDefault(dst_ptr,
                y_ptr,
                u_ptr,
                v_ptr,
                width,
                height,
                y_span,
                uv_span,
                dst_span,
                numThreads);
    #endif

            }
        else
        {
           
            Yuv2Rgb::Yuv420P2RGBDefault(dst_ptr,
                y_ptr,
                u_ptr,
                v_ptr,
                width,
                height,
                y_span,
                uv_span,
                dst_span,
                numThreads);
        }


    }
   
    void Converter::Yuv420PtoRGB(YuvNative& yuv, unsigned char* destination)
    {
        int numThreads = Converter::NumThreads;
        if (Converter::EnableSSE>0 && yuv.width % 32 == 0)
        {
           
#ifndef __arm__
            // SSE, may parallel, not arm
            Yuv2Rgb::yuv420_rgb24_sse(yuv, destination,
                numThreads);
#else
            Yuv2Rgb::Yuv420P2RGBDefault(yuv, destination,
                numThreads);
#endif
        }
        else
        {
            Yuv2Rgb::Yuv420P2RGBDefault(yuv,destination,
                numThreads);
        }
    }
    ;


    #pragma endregion



    void Converter::BGRAtoYUV420Planar(const unsigned char* bgra, unsigned char* dst, const int width, const int height, const int stride)
    {
        int numThreads = Converter::NumThreads;

#if defined(__aarch64__)
        Rgb2Yuv::BGRAtoYUV420PlanarNeon(bgra, dst, width, height, stride, numThreads);

#else
        if (width * height > minSize)
        {
            Rgb2Yuv::BGRAtoYUV420Planar(bgra, dst, width, height, stride, numThreads);
        }
        else
        {
            Rgb2Yuv::BGRAtoYUV420Planar(bgra, dst, width, height, stride, 1);
        }
#endif
    }

    void Converter::RGBAtoYUV420Planar(unsigned char* bgra, unsigned char* dst, int width, int height, int stride)
    {
        int numThreads = Converter::NumThreads;

#if defined(__aarch64__)
        Rgb2Yuv::RGBAtoYUV420PlanarNeon(bgra, dst, width, height, stride, numThreads);

#else
        if (width * height > minSize)
        {
            Rgb2Yuv::RGBAtoYUV420Planar(bgra, dst, width, height, stride, numThreads);
        }
        else
        {
            Rgb2Yuv::RGBAtoYUV420Planar(bgra, dst, width, height, stride, 1);
        }
#endif


    }

    void Converter::BGRtoYUV420Planar(unsigned char* bgra, unsigned char* dst, int width, int height, int stride)
    {
        int numThreads = Converter::NumThreads;

#if defined(__aarch64__)
        Rgb2Yuv::BGRtoYUV420PlanarNeon(bgra, dst, width, height, stride, numThreads);

#else
        if (width * height > minSize)
        {
            Rgb2Yuv::BGRtoYUV420Planar(bgra, dst, width, height, stride, numThreads);
        }
        else
        {
            Rgb2Yuv::BGRtoYUV420Planar(bgra, dst, width, height, stride, 1);
        }
#endif

    }

    void Converter::RGBtoYUV420Planar(unsigned char* bgra, unsigned char* dst, int width, int height, int stride)
    {
        int numThreads = Converter::NumThreads;

#if defined(__aarch64__)
        Rgb2Yuv::RGBtoYUV420PlanarNeon(bgra, dst, width, height, stride, numThreads);

#else
        if (width * height > minSize)
        {
            Rgb2Yuv::RGBtoYUV420Planar(bgra, dst, width, height, stride, numThreads);
        }
        else
        {
            Rgb2Yuv::RGBtoYUV420Planar(bgra, dst, width, height, stride, 1);
        }
#endif

    }

    //-------------------------------Downscale------------------------------------------------

    void Converter::Downscale24(unsigned char* rgbSrc, int width, int height, int stride, unsigned char* dst, int multiplier)
    {
        int index = 0;
        int dinx = 0;
        for (int i = 0; i < height / multiplier; i++)
        {
    #pragma clang loop vectorize(assume_safety)
            for (int j = 0; j < width / multiplier; j++)
            {

                dst[dinx++] = rgbSrc[index];
                dst[dinx++] = rgbSrc[index + 1];
                dst[dinx++] = rgbSrc[index + 2];

                index += 3 * multiplier;
            }
            index = stride * multiplier * (i + 1);
        }
    }

    void Converter::Downscale32(unsigned char* rgbaSrc, int width, int height, int stride, unsigned char* dst, int multiplier)
    {
        int index = 0;
        int dinx = 0;
        for (int i = 0; i < height / multiplier; i++)
        {
    #pragma clang loop vectorize(assume_safety)

            for (int j = 0; j < width / multiplier; j++)
            {
                dst[dinx++] = rgbaSrc[index];
                dst[dinx++] = rgbaSrc[index + 1];
                dst[dinx++] = rgbaSrc[index + 2];

                index += 4 * multiplier;
            }
            index = stride * multiplier * (i + 1);
        }
    }
}


