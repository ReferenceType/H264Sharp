#ifndef CONVERTER_LOCAL
#define CONVERTER_LOCAL
#include "pch.h"

#include "Rgb2Yuv.h"
#include "Yuv2Rgb.h"
#include "ImageTypes.h"
namespace H264Sharp
{
    struct ConverterConfig
    {
        int Numthreads = 1;
        int EnableSSE = 1;
        int EnableNeon = 1;
        int EnableAvx2 = 1;
        int EnableAvx512 = 0;
        int EnableCustomThreadPool = 0;
        int EnableDebugPrints = 0;
        int ForceNaive = 0;
    };

    class Converter
    {
    public:
        const static int minSize = 640 * 480;

        static ConverterConfig Config;

        template<int NUM_CH, bool RGB>
        static void Yuv420PtoRGB(uint8_t* dst_ptr,
            const uint8_t* y_ptr,
            const uint8_t* u_ptr,
            const uint8_t* v_ptr,
            int32_t   width,
            int32_t height,
            int32_t y_span,
            int32_t uv_span,
            int32_t dst_span);

        template <int NUM_CH, bool IS_RGB>
        static void RGBXtoYUV420Planar(const uint8_t* bgra, uint8_t* dst, int32_t  width, int32_t  height, int32_t  stride);

        static void Yuv_NV12ToYV12(const YuvNV12Native& from, YuvNative& to, uint8_t* buffer);
        static void Yuv_NV12ToYV12(const YuvNV12Native& from, YuvNative& to);

        template<int NUM_CH, bool RGB>
        static void Yuv_NV12ToRGB(const YuvNV12Native& from, uint8_t* dst, int32_t dst_span);

        static void Downscale24(const uint8_t* RESTRICT rgbSrc, int32_t  width, int32_t  height, int32_t  stride, uint8_t* RESTRICT dst, int32_t  multiplier);
        static void Downscale32(const uint8_t* RESTRICT rgbSrc, int32_t  width, int32_t  height, int32_t  stride, uint8_t* RESTRICT dst, int32_t  multiplier);

        static void SetConfig(ConverterConfig& config)
        {
            if (!hasNEON())
            {
                config.EnableNeon = 0;
            }
            if (!hasSSE41())
            {
                config.EnableSSE = 0;
            }
            if (!hasAVX2()) 
            {
                config.EnableAvx2 = 0;
            }
            if (!hasAVX512())
            {
                config.EnableAvx512 = 0;
            }
            if (config.Numthreads> std::thread::hardware_concurrency())
            {
                config.Numthreads = std::thread::hardware_concurrency();
            }

            if (config.EnableDebugPrints > 0) 
            {
                std::cout << (hasSSE41() ? "SSE4 is supported!" : "SSE4 is NOT supported!") << std::endl;
                std::cout << (hasAVX2() ? "AVX2 is supported!" : "AVX2 is NOT supported!") << std::endl;
                std::cout << (hasAVX512() ? "AVX-512 is supported!" : "AVX-512 is NOT supported!") << std::endl;
                std::cout << (hasNEON() ? "NEON is supported!" : "NEON is NOT supported!") << std::endl;

            }

            ThreadPool::Expand(config.Numthreads);
#ifdef _WIN32

            ThreadPool::SetCustomPool(config.EnableCustomThreadPool, config.Numthreads);
#endif

            Config = config;

        }

    private:
        struct ConfigInitializer {
            bool isArmArchitecture() {
#if defined(__arm__) || defined(__aarch64__) || defined(_M_ARM) || defined(_M_ARM64)
                return true;
#else
                return false;
#endif
            }
            ConfigInitializer() 
            {
                Config.EnableSSE = hasSSE41() ? 1 : 0;
                Config.EnableAvx2 = hasAVX2() ? 1 : 0;
                Config.EnableAvx512 = hasAVX512() ? 1 : 0;
                Config.EnableNeon = hasNEON() ? 1 : 0;
                if (Config.Numthreads > std::thread::hardware_concurrency())
                {
                    Config.Numthreads = std::thread::hardware_concurrency();
                }

                if (isArmArchitecture())
                    Config.Numthreads = 0;
              
                ThreadPool::Expand(Config.Numthreads);
            }
        };

        static ConfigInitializer initializer;
    };



}



#endif