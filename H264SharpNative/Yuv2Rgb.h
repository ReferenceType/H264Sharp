#pragma once
#include "ThreadPool.h"
#include "pch.h"
#include "ImageTypes.h"
namespace H264Sharp 
{

#ifndef __arm__
#include "AVX2Common.h"

#endif
#ifdef _WIN32
#include <ppl.h>
#endif

extern const unsigned int yuv2rgb565_table1[];
    class Yuv2Rgb 
    {
        public: 

            static int useLoadBalancer;

            template<int NUM_CH, bool RGB>
            static void Yuv420P2RGBDefault(uint8_t* RESTRICT dst_ptr,
                const uint8_t* RESTRICT y_ptr,
                const uint8_t* RESTRICT u_ptr,
                const uint8_t* RESTRICT v_ptr,
                int32_t width,
                int32_t height,
                int32_t y_span,
                int32_t uv_span,
                int32_t dst_span,
                int32_t numThreads);

            template<int NUM_CH, bool RGB>
            static void YuvNV122RGBDefault(uint8_t* RESTRICT dst_ptr,
                const uint8_t* RESTRICT y_ptr,
                const uint8_t* RESTRICT uv_ptr,
                int32_t width,
                int32_t height,
                int32_t y_span,
                int32_t uv_span,
                int32_t dst_span,
                int32_t numThreads);
          
#ifndef __arm__

            template<int NUM_CH, bool RGB>
            static void yuv420_rgb24_sse(int32_t width,
                int32_t height,
                const uint8_t* RESTRICT Y,
                const uint8_t* RESTRICT U,
                const uint8_t* RESTRICT V,
                int32_t Y_stride,
                int32_t UV_stride,
                uint8_t* RESTRICT Rgb,
                int32_t RGB_stride,
                int32_t numThreads);

            template<int NUM_CH, bool RGB>
            static void Yuv2Rgb::yuv_nv12_rgb24_sse(int32_t width, int32_t height, const uint8_t* RESTRICT Y, const uint8_t* RESTRICT UV,
                int32_t Y_stride, int32_t UV_stride, uint8_t* RESTRICT Rgb, int32_t RGB_stride, int32_t numThreads);

            template<int NUM_CH, bool RGB>
            static void ConvertYUVToRGB_AVX2(int32_t width,
                int32_t height,
                const uint8_t* RESTRICT Y,
                const uint8_t* RESTRICT U,
                const uint8_t* RESTRICT V,
                int32_t Y_stride,
                int32_t UV_stride,
                uint8_t* RESTRICT Rgb,
                int32_t RGB_stride,
                int32_t numThreads);

            template<int NUM_CH, bool RGB>
            static void ConvertYUVNV12ToRGB_AVX2(
                int32_t width,
                int32_t height,
                const uint8_t* RESTRICT Y,
                const uint8_t* RESTRICT UV,
                int32_t Y_stride,
                int32_t UV_stride,
                uint8_t* RESTRICT Rgb,
                int32_t RGB_stride,
                int32_t numThreads);
            
#elif defined(__arm__)

            template<int NUM_CH, bool RGB>
            static void ConvertYUVToRGB_NEON(const uint8_t* RESTRICT y_plane,
                const uint8_t* RESTRICT u_plane,
                const uint8_t* RESTRICT v_plane, 
                int32_t Y_stride,
                int32_t UV_stride,
                uint8_t* RESTRICT rgb_buffer,
                int32_t width,
                int32_t height,
                int32_t numThreads);

            template<int NUM_CH, bool RGB>
            static void ConvertYUVNV12ToRGB_NEON(const uint8_t* RESTRICT y_plane,
                const uint8_t* RESTRICT uv_plane,
                int32_t Y_stride,
                int32_t UV_stride,
                uint8_t* RESTRICT rgb_buffer,
                int32_t width,
                int32_t heigth,
                int32_t numThreads);
    #endif

    };

   
};


