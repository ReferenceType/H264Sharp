#include "Rgb2Yuv.h"

namespace H264Sharp
{
    inline void BGRA2YUVP_ParallelBody(const unsigned char* bgra, unsigned char* dst, const int width, const int height, const int stride, const int begin)
    {
        const int wi = width / 2;
        const int uvlineBegin = begin * wi;
        const int yPlaneSize = width * height;

        int vIndex = yPlaneSize + uvlineBegin;
        int uIndex = yPlaneSize + (yPlaneSize / 4) + (uvlineBegin);
        const int readBegin = 2 * stride * begin;
        int index = readBegin;
        int yIndex = width * 2 * begin;

        unsigned char* buffer = dst;
        //  for (int j = begin; j < end; j++)
          //{
#pragma clang loop unroll_count(2)
#pragma clang loop vectorize(assume_safety)
#pragma clang loop vectorize_width(32) interleave_count(1)
#pragma clang loop vectorize_predicate(enable)

        for (int i = 0; i < wi; ++i)
        {
            const auto b = bgra[index++];
            const auto g = bgra[index++];
            const auto r = bgra[index++];
            index++;

            const auto b1 = bgra[index++];
            const auto g1 = bgra[index++];
            const auto r1 = bgra[index++];
            index++;

            buffer[yIndex++] = ((25 * b + 129 * g + 66 * r) >> 8) + 16;
            buffer[yIndex++] = ((25 * b1 + 129 * g1 + 66 * r1) >> 8) + 16;

            buffer[uIndex++] = ((112 * r + -94 * g + -18 * b) >> 8) + 128;
            buffer[vIndex++] = ((-38 * r + -74 * g + 112 * b) >> 8) + 128;
        }

        int indexNext = (readBegin)+(stride);
#pragma clang loop unroll_count(2)
#pragma clang loop vectorize(assume_safety)
#pragma clang loop vectorize_width(32) interleave_count(1)
        for (int i = 0; i < width; ++i)
        {
            const auto b2 = bgra[indexNext++];
            const auto g2 = bgra[indexNext++];
            const auto r2 = bgra[indexNext++];
            indexNext++;
            buffer[yIndex++] = ((25 * b2 + 129 * g2 + 66 * r2) >> 8) + 16;
        }

        // }
    }
    inline void BGR2YUVP_ParallelBody(const unsigned char* bgra, unsigned char* dst, const int width, const int height, const int stride, const int begin) {

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
        //  for (int j = begin; j < end; j++)
          //{
#pragma clang loop unroll_count(2)
#pragma clang loop vectorize(assume_safety)
#pragma clang loop vectorize_width(32) interleave_count(1)
#pragma clang loop vectorize_predicate(enable)

        for (int i = 0; i < wi; ++i)
        {
            const auto b = bgra[index++];
            const auto g = bgra[index++];
            const auto r = bgra[index++];

            const auto b1 = bgra[index++];
            const auto g1 = bgra[index++];
            const auto r1 = bgra[index++];

            buffer[yIndex++] = ((25 * b + 129 * g + 66 * r) >> 8) + 16;
            buffer[yIndex++] = ((25 * b1 + 129 * g1 + 66 * r1) >> 8) + 16;

            buffer[uIndex++] = ((112 * r + -94 * g + -18 * b) >> 8) + 128;
            buffer[vIndex++] = ((-38 * r + -74 * g + 112 * b) >> 8) + 128;
        }

        int indexNext = (readBegin)+(stride);
#pragma clang loop unroll_count(2)
#pragma clang loop vectorize(assume_safety)
#pragma clang loop vectorize_width(32) interleave_count(1)
        for (int i = 0; i < width; ++i)
        {
            const auto b2 = bgra[indexNext++];
            const auto g2 = bgra[indexNext++];
            const auto r2 = bgra[indexNext++];
            buffer[yIndex++] = ((25 * b2 + 129 * g2 + 66 * r2) >> 8) + 16;
        }
    }

    inline void RGBA2YUVP_ParallelBody(const unsigned char* bgra, unsigned char* dst, const int width, const int height, const int stride, const int begin) {
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
        //  for (int j = begin; j < end; j++)
          //{
#pragma clang loop unroll_count(2)
#pragma clang loop vectorize(assume_safety)
#pragma clang loop vectorize_width(32) interleave_count(1)
#pragma clang loop vectorize_predicate(enable)

        for (int i = 0; i < wi; ++i)
        {
            const auto r = bgra[index++];
            const auto g = bgra[index++];
            const auto b = bgra[index++];
            index++;

            const auto r1 = bgra[index++];
            const auto g1 = bgra[index++];
            const auto b1 = bgra[index++];
            index++;

            buffer[yIndex++] = ((25 * b + 129 * g + 66 * r) >> 8) + 16;
            buffer[yIndex++] = ((25 * b1 + 129 * g1 + 66 * r1) >> 8) + 16;

            buffer[uIndex++] = ((112 * r + -94 * g + -18 * b) >> 8) + 128;
            buffer[vIndex++] = ((-38 * r + -74 * g + 112 * b) >> 8) + 128;
        }

        int indexNext = (readBegin)+(stride);
#pragma clang loop unroll_count(2)
#pragma clang loop vectorize(assume_safety)
#pragma clang loop vectorize_width(32) interleave_count(1)
        for (int i = 0; i < width; ++i)
        {
            const auto r2 = bgra[indexNext++];
            const auto g2 = bgra[indexNext++];
            const auto b2 = bgra[indexNext++];
            indexNext++;
            buffer[yIndex++] = ((25 * b2 + 129 * g2 + 66 * r2) >> 8) + 16;
        }
    }
    inline void RGB2YUVP_ParallelBody(const unsigned char* bgra, unsigned char* dst, const int width, const int height, const int stride, const int begin)
    {

        const int wi = width / 2;
        const int uvlineBegin = begin * wi;
        const int yPlaneSize = width * height;

        int vIndex = yPlaneSize + uvlineBegin;
        int uIndex = yPlaneSize + (yPlaneSize / 4) + (uvlineBegin);
        const int readBegin = 2 * stride * begin;
        int index = readBegin;
        int yIndex = width * 2 * begin;

        unsigned char* buffer = dst;

#pragma clang loop unroll_count(2)
#pragma clang loop vectorize(assume_safety)
#pragma clang loop vectorize_width(32) interleave_count(1)
#pragma clang loop vectorize_predicate(enable)

        for (int i = 0; i < wi; ++i)
        {
            const auto r = bgra[index++];
            const auto g = bgra[index++];
            const auto b = bgra[index++];

            const auto r1 = bgra[index++];
            const auto g1 = bgra[index++];
            const auto b1 = bgra[index++];

            buffer[yIndex++] = ((25 * b + 129 * g + 66 * r) >> 8) + 16;
            buffer[yIndex++] = ((25 * b1 + 129 * g1 + 66 * r1) >> 8) + 16;

            buffer[uIndex++] = ((112 * r + -94 * g + -18 * b) >> 8) + 128;
            buffer[vIndex++] = ((-38 * r + -74 * g + 112 * b) >> 8) + 128;
        }

        int indexNext = (readBegin)+(stride);
#pragma clang loop unroll_count(2)
#pragma clang loop vectorize(assume_safety)
#pragma clang loop vectorize_width(32) interleave_count(1)
        for (int i = 0; i < width; ++i)
        {
            const auto r2 = bgra[indexNext++];
            const auto g2 = bgra[indexNext++];
            const auto b2 = bgra[indexNext++];
            buffer[yIndex++] = ((25 * b2 + 129 * g2 + 66 * r2) >> 8) + 16;
        }
    }

    void Rgb2Yuv::BGRAtoYUV420Planar(const unsigned char* bgra, unsigned char* dst, const int width, const int height, const int stride, int numThreads)
    {
        const int hi = height / 2;
        if ( numThreads > 1)
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
                        [[clang::always_inline]] BGRA2YUVP_ParallelBody(bgra, dst, width, height, stride, i);
                    }
                });
        }
        else
        {
            for (int j = 0; j < hi; j++)
            {
                [[clang::always_inline]] BGRA2YUVP_ParallelBody(bgra, dst, width, height, stride, j);
            }
        }
    }

    void Rgb2Yuv::RGBAtoYUV420Planar(unsigned char* bgra, unsigned char* dst, int width, int height, int stride, int numThreads)
    {
        const int hi = height / 2;
        if ( numThreads > 1)
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
                        [[clang::always_inline]] RGBA2YUVP_ParallelBody(bgra, dst, width, height, stride, i);
                    }
                });


        }
        else
        {
            for (int j = 0; j < hi; j++)
            {
                [[clang::always_inline]] RGBA2YUVP_ParallelBody(bgra, dst, width, height, stride, j);
            }
        }

    }

    void Rgb2Yuv::BGRtoYUV420Planar(unsigned char* bgra, unsigned char* dst, int width, int height, int stride, int numThreads)
    {
        const int hi = height / 2;
        if ( numThreads > 1)
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
                        [[clang::always_inline]] BGR2YUVP_ParallelBody(bgra, dst, width, height, stride, i);
                    }
                });

        }
        else
        {
            for (int j = 0; j < hi; j++)
            {
                [[clang::always_inline]] BGR2YUVP_ParallelBody(bgra, dst, width, height, stride, j);
            }
        }
    }

    void Rgb2Yuv::RGBtoYUV420Planar(unsigned char* bgra, unsigned char* dst, int width, int height, int stride, int numThreads)
    {
        const int hi = height / 2;
        if ( numThreads > 1)
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
                        [[clang::always_inline]] RGB2YUVP_ParallelBody(bgra, dst, width, height, stride, i);
                    }
                });

        }
        else
        {
            for (int j = 0; j < hi; j++)
            {
                [[clang::always_inline]] RGB2YUVP_ParallelBody(bgra, dst, width, height, stride, j);
            }
        }

    }
}