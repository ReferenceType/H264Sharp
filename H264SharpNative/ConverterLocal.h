#ifndef CONVERTER_LOCAL
#define CONVERTER_LOCAL
#include<stdint.h>
#include "ConverterLocal.h"

#if defined(__aarch64__) || defined(__ARM_ARCH)
#define __arm__
#endif

#ifndef __arm__
#include <emmintrin.h>
#endif
#ifdef _WIN32
#include <ppl.h>
#endif

void Yuv420PtoRGB(unsigned char* dst_ptr,
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

void BGRAtoYUV420Planar(const unsigned char* bgra, unsigned char* dst, int width, int height, int stride, int threadCount);
void BGRtoYUV420Planar(unsigned char* bgr, unsigned char* dst, int width, int height, int stride, int threadCount);
void RGBAtoYUV420Planar(unsigned char* bgr, unsigned char* dst, int width, int height, int stride, int threadCount);
void RGBtoYUV420Planar(unsigned char* bgr, unsigned char* dst, int width, int height, int stride, int  threadCount);
void Downscale24(unsigned char* rgbSrc, int width, int height, int stride, unsigned char* dst, int multiplier);
void Downscale32(unsigned char* rgbSrc, int width, int height, int stride, unsigned char* dst, int multiplier);

void yuv420_rgb24_sse(uint32_t width,
    uint32_t height,
    const uint8_t* Y,
    const uint8_t* U,
    const uint8_t* V,
    uint32_t Y_stride,
    uint32_t UV_stride,
    uint8_t* RGB,
    uint32_t RGB_stride,
    int numThreads);

void Yuv420P2RGBDefault(unsigned char* dst_ptr,
    const unsigned char* y_ptr,
    const unsigned char* u_ptr,
    const unsigned char* v_ptr,
    signed   int   width,
    signed   int   height,
    signed   int   y_span,
    signed   int   uv_span,
    signed   int   dst_span,
    int numThreads);

void Yuv2RgbDefault_PB(int k, unsigned char* dst_ptr,
    const unsigned char* y_ptr,
    const unsigned char* u_ptr,
    const unsigned char* v_ptr,
    signed   int   width,
    signed   int   height,
    signed   int   y_span,
    signed   int   uv_span,
    signed   int   dst_span);

inline void Yuv2BgrSSE_ParallelBody(uint32_t y,
    uint32_t width,
    uint32_t height,
    const uint8_t* Y,
    const uint8_t* U,
    const uint8_t* V,
    uint32_t Y_stride,
    uint32_t UV_stride,
    uint8_t* RGB, 
    uint32_t RGB_stride);
extern const unsigned int yuv2rgb565_table1[];


#endif