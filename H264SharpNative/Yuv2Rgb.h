#pragma once
#include "ThreadPool.h"
#include "pch.h"
#include "ImageTypes.h"
namespace H264Sharp 
{
#ifndef __arm__
#include <emmintrin.h>
#endif
#ifdef _WIN32
#include <ppl.h>
#endif

extern const unsigned int yuv2rgb565_table1[];
    class Yuv2Rgb 
    {
        public: 
            static void Yuv420P2RGBDefault(YuvNative& yuv, unsigned char* dest, int numThreads);

            static void Yuv420P2RGBDefault(unsigned char* dst_ptr,
                const unsigned char* y_ptr,
                const unsigned char* u_ptr,
                const unsigned char* v_ptr,
                signed   int   width,
                signed   int   height,
                signed   int   y_span,
                signed   int   uv_span,
                signed   int   dst_span,
                int numThreads);
        #ifndef __arm__

            static void yuv420_rgb24_sse(uint32_t width,
                uint32_t height,
                const uint8_t* Y,
                const uint8_t* U,
                const uint8_t* V,
                uint32_t Y_stride,
                uint32_t UV_stride,
                uint8_t* RGB,
                uint32_t RGB_stride,
                int numThreads);

            static void yuv420_rgb24_sse(YuvNative& yuv, unsigned char* dest, int numThreads);
           
           

            static void ConvertYUVToRGB_AVX2(
                const uint8_t* y_plane,
                const uint8_t* u_plane,
                const uint8_t* v_plane,
                uint8_t* rgb_buffer,
                int width,
                int height,
                int numThreads);
            
#elif defined(__aarch64__)

            static void ConvertYUVToRGB_NEON(const uint8_t* y_plane, const uint8_t* u_plane, const uint8_t* v_plane,
                uint8_t* rgb_buffer, int width, int height);
          
            static void ConvertYUVToRGB_NEON_Parallel(const uint8_t* y_plane, const uint8_t* u_plane, const uint8_t* v_plane,
                uint8_t* rgb_buffer, int width, int height, int numThreads);
            
    #endif

    };
};


