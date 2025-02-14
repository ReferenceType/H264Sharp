#pragma once
#include "ThreadPool.h"
#include "pch.h"
#include "ImageTypes.h"
namespace H264Sharp 
{

#ifndef __arm__
#include <emmintrin.h>
#include <immintrin.h>
#endif
#ifdef _WIN32
#include <ppl.h>
#endif

extern const unsigned int yuv2rgb565_table1[];
    class Yuv2Rgb 
    {
        public: 

            template<int NUM_CH, bool RGB>
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

            template<int NUM_CH, bool RGB>
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

            template<int NUM_CH, bool RGB>
            static void ConvertYUVToRGB_AVX2(uint32_t width,
                uint32_t height,
                const uint8_t* Y,
                const uint8_t* U,
                const uint8_t* V,
                uint32_t Y_stride,
                uint32_t UV_stride,
                uint8_t* RGB,
                uint32_t RGB_stride,
                int numThreads);
            
#elif defined(__aarch64__)

            static void ConvertYUVToRGB_NEON(const uint8_t* RESTRICT y_plane, const uint8_t* RESTRICT u_plane, const uint8_t* RESTRICT v_plane, uint32_t Y_stride,
                uint32_t UV_stride,
                uint8_t* RESTRICT rgb_buffer, int width, int height);
          
            static void ConvertYUVToRGB_NEON_Parallel(const uint8_t* RESTRICT y_plane, const uint8_t* RESTRICT u_plane, const uint8_t* RESTRICT v_plane, uint32_t Y_stride,
                uint32_t UV_stride,
                uint8_t* RESTRICT rgb_buffer, int width, int height, int numThreads);
            
    #endif

    };
};


