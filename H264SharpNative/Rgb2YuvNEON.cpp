#if defined(__aarch64__)

#include "Rgb2Yuv.h"
#include <arm_neon.h>
#include <cstdint>

namespace H264Sharp
{
    const uint16x8_t kB_Y = vdupq_n_u16(25);
    const uint16x8_t kG_Y = vdupq_n_u16(129);
    const uint16x8_t kR_Y = vdupq_n_u16(66);

    const uint8x8_t kB_Y8 = vdup_n_u8(25);
    const uint8x8_t kG_Y8 = vdup_n_u8(129);
    const uint8x8_t kR_Y8 = vdup_n_u8(66);

    const uint8x16_t offset_Y = vdupq_n_u8(16);

    const int16x8_t kR_U = vdupq_n_s16(112 / 2);
    const int16x8_t kG_U = vdupq_n_s16(-94 / 2);
    const int16x8_t kB_U = vdupq_n_s16(-18 / 2);

    const int16x8_t kR_V = vdupq_n_s16(-38 / 2);
    const int16x8_t kG_V = vdupq_n_s16(-74 / 2);
    const int16x8_t kB_V = vdupq_n_s16(112 / 2);

    const int16x8_t offset_UV = vdupq_n_s16(128);

    const uint8x16_t dropMask = { 0xFF, 0x00, 0xFF, 0x00, 0xFF, 0x00, 0xFF, 0x00,
                        0xFF, 0x00, 0xFF, 0x00, 0xFF, 0x00, 0xFF, 0x00 };//keep drop keep drop

    // Look how simple NEON is compared to FUCKING AVX and their sadistic shuffle permutes for data allignment
    template <int R_INDEX, int G_INDEX, int B_INDEX, int NUM_CH>
    inline void RGB2YUVP_ParallelBody_SIMD(
        const unsigned char* RESTRICT src,
        unsigned char* RESTRICT dst,
        const int width,
        const int height,
        const int stride,
        const int begin,
        const int end
    ) {

        int index = 0;
        int yIndex = 0;
        int uIndex = width * height;
        int vIndex = uIndex + (uIndex >> 2);
        int strideOffset = stride - (width * NUM_CH);
        for (int row = begin; row < end; row += 2) {
            // first row includes UV
            for (int i = 0; i < width; i += 16)
            {
               /* __builtin_prefetch(&src[index + 16]);
                __builtin_prefetch(&src[uIndex + 8]);
                __builtin_prefetch(&src[vIndex + 8]);*/
                uint8x16_t r, g, b;
                if constexpr (NUM_CH == 4)
                {
                    uint8x16x4_t pixels = vld4q_u8(&src[index]);

                    r = pixels.val[R_INDEX];
                    g = pixels.val[G_INDEX];
                    b = pixels.val[B_INDEX];
                }
                else
                {
                    uint8x16x3_t pixels = vld3q_u8(&src[index]);

                    r = pixels.val[R_INDEX];
                    g = pixels.val[G_INDEX];
                    b = pixels.val[B_INDEX];
                }

                uint8x8_t r_low = (vget_low_u8(r));
                uint8x8_t r_high = (vget_high_u8(r));
                uint8x8_t g_low = (vget_low_u8(g));
                uint8x8_t g_high = (vget_high_u8(g));
                uint8x8_t b_low = (vget_low_u8(b));
                uint8x8_t b_high = (vget_high_u8(b));

                // convolution, mull and add.
                uint8x8_t y00 = vqshrn_n_u16(vmlal_u8(vmlal_u8(vmull_u8(r_low, kR_Y8), g_low, kG_Y8), b_low, kB_Y8), 8);
                uint8x8_t y01 = vqshrn_n_u16(vmlal_u8(vmlal_u8(vmull_u8(r_high, kR_Y8), g_high, kG_Y8), b_high, kB_Y8), 8);

                uint8x16_t y000 = vqaddq_u8(vcombine_u8(y00, y01), offset_Y);
                vst1q_u8(&dst[yIndex], y000);
                yIndex += 16;

                // nearest neighbour, drop every second val, cast it to 16 bit after
                int16x8_t rd = vreinterpretq_s16_u8(vandq_u8(r, dropMask));
                int16x8_t gd = vreinterpretq_s16_u8(vandq_u8(g, dropMask));
                int16x8_t bd = vreinterpretq_s16_u8(vandq_u8(b, dropMask));

                int16x8_t u = vmlaq_s16(vmlaq_s16(vmulq_s16(rd, kR_U), gd, kG_U), bd, kB_U);
                u = vaddq_s16(vshrq_n_s16(u, 7), offset_UV);

                int16x8_t v = vmlaq_s16(vmlaq_s16(vmulq_s16(bd, kB_V), gd, kG_V), rd, kR_V);
                v = vaddq_s16(vshrq_n_s16(v, 7), offset_UV);

                // Store U and V
                vst1_u8(&dst[uIndex], vqmovun_s16(v));
                vst1_u8(&dst[vIndex], vqmovun_s16(u));
                uIndex += 8;
                vIndex += 8;

                index += NUM_CH * 16;
            }

            index += strideOffset;
            //second row only Y, its important to have 2 loops like this for locality.
            for (int i = 0; i < width; i += 16) {
               // __builtin_prefetch(&src[index + 16]);

                uint8x16_t r, g, b;
                if constexpr (NUM_CH == 4)
                {
                    uint8x16x4_t pixels = vld4q_u8(&src[index]);

                    r = pixels.val[R_INDEX];
                    g = pixels.val[G_INDEX];
                    b = pixels.val[B_INDEX];
                }
                else
                {
                    uint8x16x3_t pixels = vld3q_u8(&src[index]);

                    r = pixels.val[R_INDEX];
                    g = pixels.val[G_INDEX];
                    b = pixels.val[B_INDEX];
                }

                uint8x8_t r_low = (vget_low_u8(r));
                uint8x8_t r_high = (vget_high_u8(r));
                uint8x8_t g_low = (vget_low_u8(g));
                uint8x8_t g_high = (vget_high_u8(g));
                uint8x8_t b_low = (vget_low_u8(b));
                uint8x8_t b_high = (vget_high_u8(b));

                uint8x8_t y00 = vqshrn_n_u16(vmlal_u8(vmlal_u8(vmull_u8(r_low, kR_Y8), g_low, kG_Y8), b_low, kB_Y8), 8);
                uint8x8_t y01 = vqshrn_n_u16(vmlal_u8(vmlal_u8(vmull_u8(r_high, kR_Y8), g_high, kG_Y8), b_high, kB_Y8), 8);

                uint8x16_t y000 = vqaddq_u8(vcombine_u8(y00, y01), offset_Y);
                vst1q_u8(&dst[yIndex], y000);
                yIndex += 16;

                index += NUM_CH * 16;
            }

            index += strideOffset;
        }

    }

    void Rgb2Yuv::BGRAtoYUV420PlanarNeon(const unsigned char* RESTRICT bgra, unsigned char* RESTRICT dst, int width, int height, int stride, int numThreads)
    {
        if (numThreads > 1) {
            int chunkLen = height / numThreads;
            if (chunkLen % 2 != 0) {
                chunkLen -= 1;
            }

            ThreadPool::For(int(0), numThreads, [&](int j)
                {
                    int bgn = chunkLen * j;
                    int end = bgn + chunkLen;

                    if (j == numThreads - 1) {
                        end = height;
                    }

                    if ((end - bgn) % 2 != 0) {
                        bgn -= 1;
                    }

                    RGB2YUVP_ParallelBody_SIMD<2, 1, 0, 4>(bgra, dst, width, height, stride, bgn, end);


                });
        }
        else {
            RGB2YUVP_ParallelBody_SIMD<2, 1, 0, 4>(bgra, dst, width, height, stride, 0, height);

        }
    }
    void Rgb2Yuv::BGRtoYUV420PlanarNeon(unsigned char* RESTRICT bgr, unsigned char* RESTRICT dst, int width, int height, int stride, int numThreads)
    {
        if (numThreads > 1) {
            int chunkLen = height / numThreads;
            if (chunkLen % 2 != 0) {
                chunkLen -= 1;
            }

            ThreadPool::For(int(0), numThreads, [&](int j)
                {
                    int bgn = chunkLen * j;
                    int end = bgn + chunkLen;

                    if (j == numThreads - 1) {
                        end = height;
                    }

                    if ((end - bgn) % 2 != 0) {
                        bgn -= 1;
                    }

                    RGB2YUVP_ParallelBody_SIMD<2, 1, 0, 3>(bgr, dst, width, height, stride, bgn, end);



                });
        }
        else {
            RGB2YUVP_ParallelBody_SIMD<2, 1, 0, 3>(bgr, dst, width, height, stride, 0, height);
        }


    }
    void Rgb2Yuv::RGBAtoYUV420PlanarNeon(unsigned char* RESTRICT rgba, unsigned char* RESTRICT dst, int width, int height, int stride, int numThreads)
    {

        if (numThreads > 1) {
            int chunkLen = height / numThreads;
            if (chunkLen % 2 != 0) {
                chunkLen -= 1;
            }

            ThreadPool::For(int(0), numThreads, [&](int j)
                {
                    int bgn = chunkLen * j;
                    int end = bgn + chunkLen;

                    if (j == numThreads - 1) {
                        end = height;
                    }

                    if ((end - bgn) % 2 != 0) {
                        bgn -= 1;
                    }

                    RGB2YUVP_ParallelBody_SIMD<0, 1, 2, 4>(rgba, dst, width, height, stride, bgn, end);




                });
        }
        else {
            RGB2YUVP_ParallelBody_SIMD<0, 1, 2, 4>(rgba, dst, width, height, stride, 0, height);
        }

    }
    void Rgb2Yuv::RGBtoYUV420PlanarNeon(unsigned char* RESTRICT rgb, unsigned char* RESTRICT dst, int width, int height, int stride, int numThreads)
    {
        if (numThreads > 1) {
            int chunkLen = height / numThreads;
            if (chunkLen % 2 != 0) {
                chunkLen -= 1;
            }

            ThreadPool::For(int(0), numThreads, [&](int j)
                {
                    int bgn = chunkLen * j;
                    int end = bgn + chunkLen;

                    if (j == numThreads - 1) {
                        end = height;
                    }

                    if ((end - bgn) % 2 != 0) {
                        bgn -= 1;
                    }

                    RGB2YUVP_ParallelBody_SIMD<0, 1, 2, 3>(rgb, dst, width, height, stride, bgn, end);




                });
        }
        else {
            RGB2YUVP_ParallelBody_SIMD<0, 1, 2, 3>(rgb, dst, width, height, stride, 0, height);
        }
    }
}

#endif
