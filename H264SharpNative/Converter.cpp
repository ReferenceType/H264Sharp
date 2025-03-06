#include "Converter.h"
#include "ThreadPool.h"
#include "Yuv2Rgb.h"
#include "Rgb2Yuv.h"
namespace H264Sharp {

    ConverterConfig Converter::Config;
    Converter::ConfigInitializer Converter::initializer;
 

    template<int NUM_CH, bool RGB>
    void Converter::Yuv420PtoRGB(uint8_t* dst_ptr,
        const uint8_t* y_ptr,
        const uint8_t* u_ptr,
        const uint8_t* v_ptr,
        int32_t width,
        int32_t height,
        int32_t y_span,
        int32_t uv_span,
        int32_t dst_span)
    {

        int numThreads = Converter::Config.Numthreads;
        numThreads = width * height < Converter::minSize ? 1 : numThreads;
#ifndef __arm__

        int enableSSE = Converter::Config.EnableSSE;
        int enableAvx2 = Converter::Config.EnableAvx2;

        if (enableAvx2 > 0 && width % 32 == 0)
        {
            Yuv2Rgb::ConvertYUVToRGB_AVX2<NUM_CH, RGB>(width,
                height,
                y_ptr,
                u_ptr,
                v_ptr,
                y_span,
                uv_span,
                dst_ptr,
                dst_span,
                numThreads);
        }
        else if (enableSSE > 0 && width % 16 == 0)
        {

            // SSE, may parallel, not arm
            Yuv2Rgb::yuv420_rgb24_sse<NUM_CH, RGB>(width,
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
        }
        else
        {

            Yuv2Rgb::Yuv420P2RGBDefault<NUM_CH, RGB>(dst_ptr,
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

#elif defined(__arm__)

        int enableNeon = Converter::Config.EnableNeon;

        if (enableNeon > 0 && width % 16 == 0)
        {

            Yuv2Rgb::ConvertYUVToRGB_NEON<NUM_CH, RGB>(
                y_ptr,
                u_ptr,
                v_ptr,
                y_span,
                uv_span,
                dst_ptr,
                width,
                height,
                numThreads);
        }
        else
        {
            Yuv2Rgb::Yuv420P2RGBDefault<NUM_CH, RGB>(dst_ptr,
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
        Yuv2Rgb::Yuv420P2RGBDefault<NUM_CH, RGB>(dst_ptr,
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

#pragma endregion



    template <int NUM_CH, bool IS_RGB>
    void Converter::RGBXtoYUV420Planar(const uint8_t* bgra, uint8_t* dst, int32_t  width, int32_t  height, int32_t  stride)
    {
        int numThreads = Converter::Config.Numthreads;
        numThreads = width * height < minSize ? 1 : numThreads;

#ifdef __arm__
        int enableNeon = Converter::Config.EnableNeon;
        if (enableNeon > 0 && width % 16 == 0)
            Rgb2Yuv::RGBXtoYUV420PlanarNeon<NUM_CH, IS_RGB>(bgra, dst, width, height, stride, numThreads);
        else
            Rgb2Yuv::RGBXtoYUV420Planar<NUM_CH, IS_RGB>(bgra, dst, width, height, stride, numThreads);

#else

        int enableAVX2 = Converter::Config.EnableAvx2;
        int enableSSE = Converter::Config.EnableSSE;

        if (enableAVX2 > 0 && width % 32 == 0)
            Rgb2Yuv::RGBXToI420_AVX2<NUM_CH, IS_RGB>(bgra, dst, width, height, stride, numThreads);
        else if (enableSSE > 0 && width % 16 == 0)
            Rgb2Yuv::RGBToI420_SSE<NUM_CH, IS_RGB>(bgra, dst, width, height, stride, numThreads);
        else
            Rgb2Yuv::RGBXtoYUV420Planar<NUM_CH, IS_RGB>(bgra, dst, width, height, stride, numThreads);

#endif

    }

    template<int NUM_CH, bool RGB>
    void Converter::Yuv_NV12ToRGB(const YuvNV12Native& from, uint8_t* dst_ptr, int32_t dst_span)
    {
        int numThreads = Converter::Config.Numthreads;
       

        uint8_t* y_ptr = from.Y;
        uint8_t* uv_ptr = from.UV;
        int width = from.width;
        int height = from.height;
        int y_span = from.yStride;
        int uv_span = from.uvStride;
        numThreads = width * height < minSize ? 1 : numThreads;

#ifdef __arm__
        int enableNeon = Converter::Config.EnableNeon;
        if (enableNeon > 0 && width % 16 == 0)
            Yuv2Rgb::ConvertYUVNV12ToRGB_NEON<NUM_CH, RGB>(
                y_ptr,
                uv_ptr,
                y_span,
                uv_span,
                dst_ptr,
                width,
                height,
                numThreads);
        else
            Yuv2Rgb::YuvNV122RGBDefault<NUM_CH, RGB>(dst_ptr,
                y_ptr,
                uv_ptr,
                width,
                height,
                y_span,
                uv_span,
                dst_span,
                numThreads);

#else

        int enableAVX2 = Converter::Config.EnableAvx2;

        if (enableAVX2 > 0 && width % 32 == 0)
            Yuv2Rgb::ConvertYUVNV12ToRGB_AVX2<NUM_CH, RGB>(width,
                height,
                y_ptr,
                uv_ptr,
                y_span,
                uv_span,
                dst_ptr,
                dst_span,
                numThreads);
        else if(Converter::Config.EnableSSE && width % 16 == 0)
        {
            Yuv2Rgb::yuv_nv12_rgb24_sse<NUM_CH, RGB>(width,
                height,
                y_ptr,
                uv_ptr,
                y_span,
                uv_span,
                dst_ptr,
                dst_span,
                numThreads);
        }
        else
            Yuv2Rgb::YuvNV122RGBDefault<NUM_CH, RGB>(dst_ptr,
                y_ptr,
                uv_ptr,
                width,
                height,
                y_span,
                uv_span,
                dst_span,
                numThreads);
           

#endif
    }

    template void Converter::RGBXtoYUV420Planar<4, true>(const uint8_t* bgra, uint8_t* dst, int32_t  width, int32_t  height, int32_t  stride);
    template void Converter::RGBXtoYUV420Planar<4, false>(const uint8_t* bgra, uint8_t* dst, int32_t  width, int32_t  height, int32_t  stride);
    template void Converter::RGBXtoYUV420Planar<3, true>(const uint8_t* bgra, uint8_t* dst, int32_t  width, int32_t  height, int32_t  stride);
    template void Converter::RGBXtoYUV420Planar<3, false>(const uint8_t* bgra, uint8_t* dst, int32_t  width, int32_t  height, int32_t  stride);

    //-------------------------------Downscale------------------------------------------------
    /*hints for future
    * __m256i indices = _mm256_setr_epi32(0, 6, 12, 18, 24, 30, 36, 42); // Indices to gather

    __m256i result = _mm256_i32gather_epi32((int*)rgb, indices, 1);   
    result = _mm256_shuffle_epi8(result , _mm256_setr_epi8(0, 1, 2, 4, 5, 6, 8, 9, 10, 12, 13, 14, -1, -1, -1, -1,
                                                           0, 1, 2, 4, 5, 6, 8, 9, 10, 12, 13, 14, -1, -1, -1, -1));
    result = _mm256_permutevar8x32_epi32(result, _mm256_setr_epi32(0,1,2,4,5,6,7,3));
    */
    void Converter::Downscale24(const uint8_t* RESTRICT rgbSrc, int32_t  width, int32_t  height, int32_t  stride, uint8_t* RESTRICT dst, int32_t  multiplier)
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

    void  Converter::Downscale32(const uint8_t* RESTRICT rgbaSrc, int32_t width, int32_t height, int32_t stride, uint8_t* RESTRICT dst, int32_t multiplier)
    {
        auto* src32 = reinterpret_cast<const uint32_t*>(rgbaSrc);
        auto* dst32 = reinterpret_cast<uint32_t*>(dst);

        int32_t index = 0;
        int32_t dinx = 0;

        int32_t stride32 = stride / 4;
        int32_t width32 = width / multiplier;
        int32_t height32 = height / multiplier;

        for (int32_t i = 0; i < height32; i++)
        {
            index = stride32 * multiplier * i;

#pragma clang loop vectorize(assume_safety)
            for (int32_t j = 0; j < width32; j++)
            {
                dst32[dinx++] = src32[index];
                index += multiplier;
            }
        }
    }

    template  void Converter::Yuv420PtoRGB<3, true>(uint8_t* dst_ptr,
        const uint8_t* y_ptr,
        const uint8_t* u_ptr,
        const uint8_t* v_ptr,
        int32_t   width,
        int32_t height,
        int32_t y_span,
        int32_t uv_span,
        int32_t dst_span);
    template  void Converter::Yuv420PtoRGB<4, true>(uint8_t* dst_ptr,
        const uint8_t* y_ptr,
        const uint8_t* u_ptr,
        const uint8_t* v_ptr,
        int32_t   width,
        int32_t height,
        int32_t y_span,
        int32_t uv_span,
        int32_t dst_span);
    template  void Converter::Yuv420PtoRGB<3, false>(uint8_t* dst_ptr,
        const uint8_t* y_ptr,
        const uint8_t* u_ptr,
        const uint8_t* v_ptr,
        int32_t   width,
        int32_t height,
        int32_t y_span,
        int32_t uv_span,
        int32_t dst_span);
    template  void Converter::Yuv420PtoRGB<4, false>(uint8_t* dst_ptr,
        const uint8_t* y_ptr,
        const uint8_t* u_ptr,
        const uint8_t* v_ptr,
        int32_t   width,
        int32_t height,
        int32_t y_span,
        int32_t uv_span,
        int32_t dst_span);

    //---

    template  void Converter::Yuv_NV12ToRGB<3, true>(const YuvNV12Native& from, uint8_t* dst_ptr, int32_t dst_span);
    template  void Converter::Yuv_NV12ToRGB<4, true>(const YuvNV12Native& from, uint8_t* dst_ptr, int32_t dst_span);
    template  void Converter::Yuv_NV12ToRGB<3, false>(const YuvNV12Native& from, uint8_t* dst_ptr, int32_t dst_span);
    template  void Converter::Yuv_NV12ToRGB<4, false>(const YuvNV12Native& from, uint8_t* dst_ptr, int32_t dst_span);
}


