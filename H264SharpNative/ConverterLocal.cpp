#include "pch.h"
#include <cmath>

enum
{
    FLAGS = 0x40080100
};

#define READUV(U,V) (yuv2rgb565_table1[256 + (U)] + yuv2rgb565_table1[512 + (V)])
#define READY(Y)    yuv2rgb565_table1[Y]
#define FIXUP(Y)                 \
do {                             \
    int tmp = (Y) & FLAGS;       \
    if (tmp != 0)                \
    {                            \
        tmp  -= tmp>>8;          \
        (Y)  |= tmp;             \
        tmp   = FLAGS & ~(Y>>1); \
        (Y)  += tmp>>8;          \
    }                            \
} while (0 == 1)

#define STORE(Y,DSTPTR)           \
do {                              \
    unsigned int Y2       = (Y);      \
    unsigned char  *DSTPTR2 = (DSTPTR); \
    (DSTPTR2)[0] = (Y2);          \
    (DSTPTR2)[1] = (Y2)>>22;      \
    (DSTPTR2)[2] = (Y2)>>11;      \
} while (0 == 1)
extern "C"
void Yuv420P2RGB(unsigned char* dst_ptr,
    const unsigned char* y_ptr,
    const unsigned char* u_ptr,
    const unsigned char* v_ptr,
    signed   int   width,
    signed   int   height,
    signed   int   y_span,
    signed   int   uv_span,
    signed   int   dst_span,

    signed   int   dither)
{
    height -= 1;
    while (height > 0)
    {
        height -= width << 16;
        height += 1 << 16;
#pragma clang loop vectorize(assume_safety)
        while (height < 0)
        {
            /* Do 2 column pairs */
            unsigned int uv, y0, y1;

            uv = READUV(*u_ptr++, *v_ptr++);
            y1 = uv + READY(y_ptr[y_span]);
            y0 = uv + READY(*y_ptr++);
            FIXUP(y1);
            FIXUP(y0);
            STORE(y1, &dst_ptr[dst_span]);
            STORE(y0, dst_ptr);
            dst_ptr += 3;
            y1 = uv + READY(y_ptr[y_span]);
            y0 = uv + READY(*y_ptr++);
            FIXUP(y1);
            FIXUP(y0);
            STORE(y1, &dst_ptr[dst_span]);
            STORE(y0, dst_ptr);
            dst_ptr += 3;
            height += (2 << 16);
        }
        if ((height >> 16) == 0)
        {
            /* Trailing column pair */
            unsigned int uv, y0, y1;

            uv = READUV(*u_ptr, *v_ptr);
            y1 = uv + READY(y_ptr[y_span]);
            y0 = uv + READY(*y_ptr++);
            FIXUP(y1);
            FIXUP(y0);
            STORE(y0, &dst_ptr[dst_span]);
            STORE(y1, dst_ptr);
            dst_ptr += 3;
        }
        dst_ptr += dst_span * 2 - width * 3;
        y_ptr += y_span * 2 - width;
        u_ptr += uv_span - (width >> 1);
        v_ptr += uv_span - (width >> 1);
        height = (height << 16) >> 16;
        height -= 2;
    }
    if (height == 0)
    {
        /* Trail row */
        height -= width << 16;
        height += 1 << 16;
#pragma clang loop vectorize(assume_safety)
        while (height < 0)
        {
            /* Do a row pair */
            unsigned int uv, y0, y1;

            uv = READUV(*u_ptr++, *v_ptr++);
            y1 = uv + READY(*y_ptr++);
            y0 = uv + READY(*y_ptr++);
            FIXUP(y1);
            FIXUP(y0);
            STORE(y1, dst_ptr);
            dst_ptr += 3;
            STORE(y0, dst_ptr);
            dst_ptr += 3;
            height += (2 << 16);
        }
        if ((height >> 16) == 0)
        {
            /* Trailing pix */
            unsigned int uv, y0;

            uv = READUV(*u_ptr++, *v_ptr++);
            y0 = uv + READY(*y_ptr++);
            FIXUP(y0);
            STORE(y0, dst_ptr);
            dst_ptr += 3;
        }
    }

};

void BGRAtoYUV420Planar(unsigned char* bgra, unsigned char* dst, int width, int height, int stride)
{
    const int frameSize = width * height;
    int yIndex = 0;
    int vIndex = frameSize;
    int uIndex = frameSize + (frameSize / 4);
    //int r, g, b, y, u, v;
    int index = 0;

    const int hi = height / 2;
    const int wi = width / 2;
    const int nextLineStride = 2 * stride - (4 * width);
    unsigned char* buffer = dst;
    for (int j = 0; j < hi; ++j)
    {
#pragma clang loop unroll_count(2)
#pragma clang loop vectorize(assume_safety)
        #pragma clang loop vectorize_width(32) interleave_count(1)
        for (int i = 0; i < wi; ++i)
        {
            int b = bgra[index];
            int g = bgra[index + 1];
            int r = bgra[index + 2];

            int b1 = bgra[index + 4];
            int g1 = bgra[index + 5];
            int r1 = bgra[index + 6];

            int nextLineIdx = index + stride;
            int b2 = bgra[nextLineIdx];
            int g2 = bgra[nextLineIdx + 1];
            int r2 = bgra[nextLineIdx + 2];

            int b3 = bgra[nextLineIdx + 4];
            int g3 = bgra[nextLineIdx + 5];
            int r3 = bgra[nextLineIdx + 6];

            int nextLineYIdx = yIndex + width;

            buffer[yIndex] = ((25 * b + 129 * g + 66 * r) >> 8) + 16;
            buffer[yIndex + 1] = ((25 * b1 + 129 * g1 + 66 * r1) >> 8) + 16;
            buffer[nextLineYIdx] = ((25 * b2 + 129 * g2 + 66 * r2) >> 8) + 16;
            buffer[nextLineYIdx + 1] = ((25 * b3 + 129 * g3 + 66 * r3) >> 8) + 16;

            buffer[uIndex++] = ((112 * r + -94 * g + -18 * b) >> 8) + 128;
            buffer[vIndex++] = ((-38 * r + -74 * g + 112 * b) >> 8) + 128;
            yIndex += 2;
            index += 8;
        }
        index += nextLineStride;
        yIndex += width;

    }

}

void RGBAtoYUV420Planar(unsigned char* bgra, unsigned char* dst, int width, int height, int stride)
{
    const int frameSize = width * height;
    int yIndex = 0;
    int vIndex = frameSize;
    int uIndex = frameSize + (frameSize / 4);
    //int r, g, b, y, u, v;
    int index = 0;

    const int hi = height / 2;
    const int wi = width / 2;
    const int nextLineStride = 2 * stride - (4 * width);
    unsigned char* buffer = dst;
    for (int j = 0; j < hi; ++j)
    {
#pragma clang loop unroll_count(2)
#pragma clang loop vectorize(assume_safety)
        #pragma clang loop vectorize_width(32) interleave_count(1)
        for (int i = 0; i < wi; ++i)
        {
            int r = bgra[index];
            int g = bgra[index + 1];
            int b = bgra[index + 2];

            int r1 = bgra[index + 4];
            int g1 = bgra[index + 5];
            int b1 = bgra[index + 6];

            int nextLineIdx = index + stride;
            int r2 = bgra[nextLineIdx];
            int g2 = bgra[nextLineIdx + 1];
            int b2 = bgra[nextLineIdx + 2];

            int r3 = bgra[nextLineIdx + 4];
            int g3 = bgra[nextLineIdx + 5];
            int b3 = bgra[nextLineIdx + 6];

            int nextLineYIdx = yIndex + width;

            buffer[yIndex] = ((25 * b + 129 * g + 66 * r) >> 8) + 16;
            buffer[yIndex + 1] = ((25 * b1 + 129 * g1 + 66 * r1) >> 8) + 16;
            buffer[nextLineYIdx] = ((25 * b2 + 129 * g2 + 66 * r2) >> 8) + 16;
            buffer[nextLineYIdx + 1] = ((25 * b3 + 129 * g3 + 66 * r3) >> 8) + 16;

            buffer[uIndex++] = ((112 * r + -94 * g + -18 * b) >> 8) + 128;
            buffer[vIndex++] = ((-38 * r + -74 * g + 112 * b) >> 8) + 128;
            yIndex += 2;
            index += 8;
        }
        index += nextLineStride;
        yIndex += width;

    }

}

void BGRtoYUV420Planar(unsigned char* bgra, unsigned char* dst, int width, int height, int stride)
{
    const int frameSize = width * height;
    int yIndex = 0;
    int vIndex = frameSize;
    int uIndex = frameSize + (frameSize / 4);
    //int r, g, b, y, u, v;
    int index = 0;

    const int hi = height / 2;
    const int wi = width / 2;
    const int nextLineSttride = 2 * stride - (3 * width);
    unsigned char* buffer = dst;
    for (int j = 0; j < hi; ++j)
    {
#pragma clang loop unroll_count(2)
#pragma clang loop vectorize(assume_safety)
        //#pragma clang loop vectorize_width(32) interleave_count(8)
        for (int i = 0; i < wi; ++i)
        {
            int b = bgra[index];
            int g = bgra[index + 1];
            int r = bgra[index + 2];

            int b1 = bgra[index + 3];
            int g1 = bgra[index + 4];
            int r1 = bgra[index + 5];

            int nextLineIdx = index + stride;
            int b2 = bgra[nextLineIdx];
            int g2 = bgra[nextLineIdx + 1];
            int r2 = bgra[nextLineIdx + 2];

            int b3 = bgra[nextLineIdx + 3];
            int g3 = bgra[nextLineIdx + 4];
            int r3 = bgra[nextLineIdx + 5];

            int nextLineYIdx = yIndex + width;

            buffer[yIndex] = ((25 * b + 129 * g + 66 * r) >> 8) + 16;
            buffer[yIndex + 1] = ((25 * b1 + 129 * g1 + 66 * r1) >> 8) + 16;
            buffer[nextLineYIdx] = ((25 * b2 + 129 * g2 + 66 * r2) >> 8) + 16;
            buffer[nextLineYIdx + 1] = ((25 * b3 + 129 * g3 + 66 * r3) >> 8) + 16;

            buffer[uIndex++] = ((112 * r + -94 * g + -18 * b) >> 8) + 128;
            buffer[vIndex++] = ((-38 * r + -74 * g + 112 * b) >> 8) + 128;
            yIndex += 2;
            index += 6;
        }
        index += nextLineSttride;
        yIndex += width;

    }

}

void RGBtoYUV420Planar(unsigned char* bgra, unsigned char* dst, int width, int height, int stride)
{
    const int frameSize = width * height;
    int yIndex = 0;
    int vIndex = frameSize;
    int uIndex = frameSize + (frameSize / 4);
    //int r, g, b, y, u, v;
    int index = 0;

    const int hi = height / 2;
    const int wi = width / 2;
    const int nextLineSttride = 2 * stride - (3 * width);
    unsigned char* buffer = dst;
    for (int j = 0; j < hi; ++j)
    {
#pragma clang loop unroll_count(2)
#pragma clang loop vectorize(assume_safety)
        //#pragma clang loop vectorize_width(32) interleave_count(8)
        for (int i = 0; i < wi; ++i)
        {
            int r = bgra[index];
            int g = bgra[index + 1];
            int b = bgra[index + 2];

            int r1 = bgra[index + 3];
            int g1 = bgra[index + 4];
            int b1 = bgra[index + 5];

            int nextLineIdx = index + stride;
            int r2 = bgra[nextLineIdx];
            int g2 = bgra[nextLineIdx + 1];
            int b2 = bgra[nextLineIdx + 2];

            int r3 = bgra[nextLineIdx + 3];
            int g3 = bgra[nextLineIdx + 4];
            int b3 = bgra[nextLineIdx + 5];

            int nextLineYIdx = yIndex + width;

            buffer[yIndex] = ((25 * b + 129 * g + 66 * r) >> 8) + 16;
            buffer[yIndex + 1] = ((25 * b1 + 129 * g1 + 66 * r1) >> 8) + 16;
            buffer[nextLineYIdx] = ((25 * b2 + 129 * g2 + 66 * r2) >> 8) + 16;
            buffer[nextLineYIdx + 1] = ((25 * b3 + 129 * g3 + 66 * r3) >> 8) + 16;

            buffer[uIndex++] = ((112 * r + -94 * g + -18 * b) >> 8) + 128;
            buffer[vIndex++] = ((-38 * r + -74 * g + 112 * b) >> 8) + 128;
            yIndex += 2;
            index += 6;
        }
        index += nextLineSttride;
        yIndex += width;

    }

}






