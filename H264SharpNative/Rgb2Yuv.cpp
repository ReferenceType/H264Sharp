#include "Rgb2Yuv.h"
#include <stdint.h>

namespace H264Sharp
{
    const int16_t YB = 25;
    const int16_t YG = 129;
    const int16_t YR = 66;
    const int16_t Shift = 8;
    const int16_t YOffset = 16;

    const int16_t UR = 112;
    const int16_t UG = -94;
    const int16_t UB = -18;

    const int16_t VR = -38;
    const int16_t VG = -74;
    const int16_t VB = 112;
    const int16_t UVOffset = 128;

    template <int NUM_CH, bool IS_RGB>
    inline void RGBX2YUVP_ParallelBody(const unsigned char* RESTRICT bgra, unsigned char* RESTRICT dst, const int width, const int height, const int stride, const int begin) {
        
        int R_INDEX, G_INDEX, B_INDEX;
        if constexpr (IS_RGB) {
            R_INDEX = 0; G_INDEX = 1; B_INDEX = 2;
        }
        else {
            B_INDEX = 0; G_INDEX = 1;  R_INDEX = 2;
        }

        const int wi = width / 2;
        const int uvlineBegin = begin * wi;
        const int yPlaneSize = width * height;

        int vIndex = yPlaneSize + uvlineBegin;
        int uIndex = yPlaneSize + (yPlaneSize / 4) + (uvlineBegin);
        const int readBegin = 2 * stride * begin;
        int index = readBegin;
        int yIndex = width * 2 * begin;

        unsigned char* buffer = dst;
     
#pragma clang loop vectorize(assume_safety)
        for (int i = 0; i < wi; ++i)
        {
            const int16_t r = bgra[index + R_INDEX];
            const int16_t g = bgra[index + G_INDEX];
            const int16_t b = bgra[index + B_INDEX];

            index += (NUM_CH > 3) ? 4 : 3;

            const int16_t r1 = bgra[index + R_INDEX];
            const int16_t g1 = bgra[index + G_INDEX];
            const int16_t b1 = bgra[index + B_INDEX];

            index += (NUM_CH > 3) ? 4 : 3;

            buffer[yIndex++] = ((YB * b + YG * g + YR * r) >> Shift) + YOffset;
            buffer[yIndex++] = ((YB * b1 + YG * g1 + YR * r1) >> Shift) + YOffset;


            buffer[uIndex++] = ((UR * r + UG * g + UB * b) >> Shift) + UVOffset;
            buffer[vIndex++] = ((VR * r + VG * g + VB * b) >> Shift) + UVOffset;
        }

        int indexNext = (readBegin)+(stride);
        // Next Line
#pragma clang loop vectorize(assume_safety)
        for (int i = 0; i < width; ++i)
        {
            const int16_t r2 = bgra[indexNext + R_INDEX];
            const int16_t g2 = bgra[indexNext + G_INDEX];
            const int16_t b2 = bgra[indexNext + B_INDEX];

            indexNext += (NUM_CH > 3) ? 4 : 3;

            buffer[yIndex++] = ((YB * b2 + YG * g2 + YR * r2) >> Shift) + YOffset;
            
        }
    }

    template <int R_INDEX, int G_INDEX, int B_INDEX, int NUM_CH>
    inline void RGBX2YUVP_ParallelBody_2x2Sampling(const unsigned char* RESTRICT bgra, unsigned char* RESTRICT dst, const int width, const int height, const int stride, const int begin) {
        //begin = begin / 2;
       //end = end / 2;
        const int wi = width / 2;
        const int uvlineBegin = begin * wi;
        const int yPlaneSize = width * height;

        int vIndex = yPlaneSize + uvlineBegin;
        int uIndex = yPlaneSize + (yPlaneSize / 4) + (uvlineBegin);
        const int readBegin = 2 * stride * begin;
        int index = readBegin;
        int yIndex = width * 2 * begin;

        unsigned char* buffer = dst;

        int nlineIndex = index + stride;
        int nextYIndex = yIndex + width;

#pragma clang loop vectorize(assume_safety)
        //#pragma clang loop unroll_count(8)
        for (int i = 0; i < wi; ++i)
        {


            const int16_t r = bgra[index + R_INDEX];
            const int16_t g = bgra[index + G_INDEX];
            const int16_t b = bgra[index + B_INDEX];
            index += (NUM_CH > 3) ? 4 : 3;

            const int16_t r1 = bgra[index + R_INDEX];
            const int16_t g1 = bgra[index + G_INDEX];
            const int16_t b1 = bgra[index + B_INDEX];
            index += (NUM_CH > 3) ? 4 : 3;

            //next line
            const int16_t r2 = bgra[nlineIndex + R_INDEX];
            const int16_t g2 = bgra[nlineIndex + G_INDEX];
            const int16_t b2 = bgra[nlineIndex + B_INDEX];
            nlineIndex += (NUM_CH > 3) ? 4 : 3;

            const int16_t r3 = bgra[nlineIndex + R_INDEX];
            const int16_t g3 = bgra[nlineIndex + G_INDEX];
            const int16_t b3 = bgra[nlineIndex + B_INDEX];
            nlineIndex += (NUM_CH > 3) ? 4 : 3;
            //--


            buffer[yIndex++] = ((YB * b + YG * g + YR * r) >> Shift) + YOffset;
            buffer[yIndex++] = ((YB * b1 + YG * g1 + YR * r1) >> Shift) + YOffset;

            buffer[nextYIndex++] = ((YB * b2 + YG * g2 + YR * r2) >> Shift) + YOffset;
            buffer[nextYIndex++] = ((YB * b3 + YG * g3 + YR * r3) >> Shift) + YOffset;


            const int16_t ravg = (r + r1 + r2 + r3) >> 2;
            const int16_t gavg = (g + g1 + g2 + g3) >> 2;
            const int16_t bavg = (b + b1 + b2 + b3) >> 2;

            buffer[uIndex++] = ((UR * ravg + UG * gavg + UB * bavg) >> Shift) + UVOffset;
            buffer[vIndex++] = ((VR * ravg + VG * gavg + VB * bavg) >> Shift) + UVOffset;
        }

        index += stride;
        yIndex += width;

    }

    template <int NUM_CH, bool IS_RGB>
    void Rgb2Yuv::RGBXtoYUV420Planar(unsigned char* RESTRICT bgra, unsigned char* RESTRICT dst, int width, int height, int stride, int numThreads)
    {
        const int hi = height / 2;
        if (numThreads > 1)
        {
            ThreadPool::For(int(0), numThreads, [bgra, dst, width, height, stride, hi, numThreads](int j)
                {
                    int bgn = ((hi / numThreads) * (j));
                    int end = ((hi / numThreads) * (j + 1));
                    if (j == numThreads - 1)
                    {
                        end = hi;
                    }

                    for (int i = bgn; i < end; i++)
                    {
                        [[clang::always_inline]] RGBX2YUVP_ParallelBody<3,true>(bgra, dst, width, height, stride, i);
                    }
                });

        }
        else
        {
            for (int j = 0; j < hi; j++)
            {
                [[clang::always_inline]] RGBX2YUVP_ParallelBody<3,true>(bgra, dst, width, height, stride, j);
            }
        }

    }

template void Rgb2Yuv::RGBXtoYUV420Planar<3,false>(unsigned char* RESTRICT bgra, unsigned char* RESTRICT dst, int width, int height, int stride, int numThreads);
template void Rgb2Yuv::RGBXtoYUV420Planar<3,true>(unsigned char* RESTRICT bgra, unsigned char* RESTRICT dst, int width, int height, int stride, int numThreads);
template void Rgb2Yuv::RGBXtoYUV420Planar<4,false>(unsigned char* RESTRICT bgra, unsigned char* RESTRICT dst, int width, int height, int stride, int numThreads);
template void Rgb2Yuv::RGBXtoYUV420Planar<4,true>(unsigned char* RESTRICT bgra, unsigned char* RESTRICT dst, int width, int height, int stride, int numThreads);
}