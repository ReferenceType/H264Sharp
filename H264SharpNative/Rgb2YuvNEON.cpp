#if defined(__aarch64__)

#include "Rgb2Yuv.h"
#include <arm_neon.h>
#include <cstdint>

namespace H264Sharp 
{
    template <int R_INDEX, int G_INDEX, int B_INDEX, int NUM_CH>
    inline void RGB2YUVP_ParallelBody_SIMD(
        const unsigned char* src,
        unsigned char* dst,
        const int width,
        const int height,
        const int stride,
        const int begin,
        const int end
    );

    template <int R_INDEX, int G_INDEX, int B_INDEX, int NUM_CH>
    inline void RGB2YUVP_ParallelBody_SIMDv2(
        const unsigned char* src,
        unsigned char* dst,
        const int width,
        const int height,
        const int stride,
        const int begin,
        const int end
    );
	
    void Rgb2Yuv::BGRAtoYUV420PlanarNeon(const unsigned char* bgra, unsigned char* dst, int width, int height, int stride, int threadCount)
    {
        if (threadCount > 1) {
            int chunkLen = height / threadCount;
            ThreadPool::For(int(0), threadCount, [&](int j)
                {
                    int bgn = chunkLen * j;
                    int end = chunkLen * (j + 1);
                    if (j == threadCount - 1)
                    {
                        end = height;
                    }

                    RGB2YUVP_ParallelBody_SIMD<2, 1, 0, 4>(bgra, dst, width, height, stride, bgn, end);


                });
        }
        else {
            RGB2YUVP_ParallelBody_SIMD<2, 1, 0, 4>(bgra, dst, width, height, stride, 0, height);

        }
    }
    void Rgb2Yuv::BGRtoYUV420PlanarNeon(unsigned char* bgr, unsigned char* dst, int width, int height, int stride, int threadCount)
    {
        if (threadCount > 1) {
            int chunkLen = height / threadCount;
            ThreadPool::For(int(0), threadCount, [&](int j)
                {
                    int bgn = chunkLen * j;
                    int end = chunkLen * (j + 1);
                    if (j == threadCount - 1)
                    {
                        end = height;
                    }

                    RGB2YUVP_ParallelBody_SIMD<2, 1, 0, 3>(bgr, dst, width, height, stride, bgn, end);



                });
        }
        else {
            RGB2YUVP_ParallelBody_SIMD<2, 1, 0, 3>(bgr, dst, width, height, stride, 0, height);
        }


    }
    void Rgb2Yuv::RGBAtoYUV420PlanarNeon(unsigned char* rgba, unsigned char* dst, int width, int height, int stride, int threadCount)
    {

        if (threadCount > 1) {
            int chunkLen = height / threadCount;
            ThreadPool::For(int(0), threadCount, [&](int j)
                {
                    int bgn = chunkLen * j;
                    int end = chunkLen * (j + 1);
                    if (j == threadCount - 1)
                    {
                        end = height;
                    }

                    RGB2YUVP_ParallelBody_SIMD<0, 1, 2, 4>(rgba, dst, width, height, stride, bgn, end);




                });
        }
        else {
            RGB2YUVP_ParallelBody_SIMD<0, 1, 2, 4>(rgba, dst, width, height, stride, 0, height);
        }

    }
    void Rgb2Yuv::RGBtoYUV420PlanarNeon(unsigned char* rgb, unsigned char* dst, int width, int height, int stride, int threadCount)
    {
        if (threadCount > 1) {
            int chunkLen = height / threadCount;
            ThreadPool::For(int(0), threadCount, [&](int j)
                {
                    int bgn = chunkLen * j;
                    int end = chunkLen * (j + 1);
                    if (j == threadCount - 1)
                    {
                        end = height;
                    }

                    RGB2YUVP_ParallelBody_SIMD<0, 1, 2, 3>(rgb, dst, width, height, stride, bgn, end);




                });
        }
        else {
            RGB2YUVP_ParallelBody_SIMD<0, 1, 2, 3>(rgb, dst, width, height, stride, 0, height);
        }
    }

    template <int R_INDEX, int G_INDEX, int B_INDEX, int NUM_CH>
    inline void RGB2YUVP_ParallelBody_SIMD(
        const unsigned char* src,
        unsigned char* dst,
        const int width,
        const int height,
        const int stride,
        const int begin,
        const int end
    ) {

        unsigned char* buffer = dst;
 

        const uint16x8_t kB_Y = vdupq_n_u16(25);
        const uint16x8_t kG_Y = vdupq_n_u16(129);
        const uint16x8_t kR_Y = vdupq_n_u16(66);

        const uint8x8_t kB_Y8 = vdup_n_u8(25);
        const uint8x8_t kG_Y8 = vdup_n_u8(129);
        const uint8x8_t kR_Y8 = vdup_n_u8(66);

        const uint8x16_t offset_Y = vdupq_n_u8(16);


        const int16x8_t kR_U = vdupq_n_s16(112);
        const int16x8_t kG_U = vdupq_n_s16(-94);
        const int16x8_t kB_U = vdupq_n_s16(-18);

        const int16x8_t kR_V = vdupq_n_s16(-38);
        const int16x8_t kG_V = vdupq_n_s16(-74);
        const int16x8_t kB_V = vdupq_n_s16(112);

        const int16x8_t offset_UV = vdupq_n_s16(128);

        int index = 0;
        int yIndex = 0;
        int uIndex = width * height;
        int vIndex = uIndex + (uIndex >> 2);
        int strideOffset = stride - (width * NUM_CH);
        // Loop over the specified range of rows
        for (int row = begin; row < end; row += 2) {
            // first row includes UV
            for (int i = 0; i < width; i += 16) {
                // Load 16 pixels (48 bytes) from src
                uint8x16_t r, g, b;
                if constexpr (NUM_CH == 4) {
                    uint8x16x4_t pixels = vld4q_u8(&src[index]);

                    r = pixels.val[R_INDEX];
                    g = pixels.val[G_INDEX];
                    b = pixels.val[B_INDEX];
                }
                else {
                    uint8x16x3_t pixels = vld3q_u8(&src[index]);

                    r = pixels.val[R_INDEX];
                    g = pixels.val[G_INDEX];
                    b = pixels.val[B_INDEX];
                }


                ////Widen to 16 bits unsigned
                //uint16x8_t r_low = vmovl_u8(vget_low_u8(r));
                //uint16x8_t r_high = vmovl_u8(vget_high_u8(r));
                //uint16x8_t g_low = vmovl_u8(vget_low_u8(g));
                //uint16x8_t g_high = vmovl_u8(vget_high_u8(g));
                //uint16x8_t b_low = vmovl_u8(vget_low_u8(b));
                //uint16x8_t b_high = vmovl_u8(vget_high_u8(b));

                //// Y Channel (Unsigned because we can overflow) 
                //uint16x8_t y_low = vaddq_u16(vaddq_u16(vmulq_u16(r_low, kR_Y), vmulq_u16(g_low, kG_Y)), vmulq_u16(b_low, kB_Y));
                //uint16x8_t y_high = vaddq_u16(vaddq_u16(vmulq_u16(r_high, kR_Y), vmulq_u16(g_high, kG_Y)), vmulq_u16(b_high, kB_Y));

                //// div 256(shift 8) + offset
                //y_low = vaddq_u16(vshrq_n_u16(y_low, 8), offset_Y);
                //y_high = vaddq_u16(vshrq_n_u16(y_high, 8), offset_Y);

                //// shrink combine strore
                //vst1q_u8(&buffer[yIndex], vcombine_u8(vqmovn_u16(y_low), vqmovn_u16(y_high)));
                //yIndex += 16;

                uint8x8_t r_low = (vget_low_u8(r));
                uint8x8_t r_high = (vget_high_u8(r));
                uint8x8_t g_low = (vget_low_u8(g));
                uint8x8_t g_high = (vget_high_u8(g));
                uint8x8_t b_low = (vget_low_u8(b));
                uint8x8_t b_high = (vget_high_u8(b));

                uint8x8_t y00 = vqshrn_n_u16(vmlal_u8(vmlal_u8(vmull_u8(r_low, (kR_Y8)), g_low, (kG_Y8)), b_low, (kB_Y8)), 8);
                uint8x8_t y01 = vqshrn_n_u16(vmlal_u8(vmlal_u8(vmull_u8(r_high, (kR_Y8)), g_high, (kG_Y8)), b_high, (kB_Y8)), 8);
               
                uint8x16_t y000 = vqaddq_u8(vcombine_u8(y00, y01), offset_Y);
                vst1q_u8(&buffer[yIndex], y000); yIndex += 16;


                // we need signed here 
                int16x8_t r_avg_signed = vreinterpretq_s16_u16(vshrq_n_s16(vpaddlq_u8(r),1));
                int16x8_t g_avg_signed = vreinterpretq_s16_u16(vshrq_n_s16(vpaddlq_u8(g), 1));
                int16x8_t b_avg_signed = vreinterpretq_s16_u16(vshrq_n_s16(vpaddlq_u8(b), 1));

                // Compute U channel (average over 2x2 blocks)
                int16x8_t u = vaddq_s16(vaddq_s16(vmulq_s16(r_avg_signed, kR_U), vmulq_s16(g_avg_signed, kG_U)), vmulq_s16(b_avg_signed, kB_U));
                u = vaddq_s16(vshrq_n_s16(u, 8), offset_UV);

                // Compute V channel
                int16x8_t v = vaddq_s16(vaddq_s16(vmulq_s16(b_avg_signed, kB_V), vmulq_s16(g_avg_signed, kG_V)), vmulq_s16(r_avg_signed, kR_V));
                v = vaddq_s16(vshrq_n_s16(v, 8), offset_UV);

                // Store U and V
                vst1_u8(&buffer[uIndex], vqmovun_s16(v));
                vst1_u8(&buffer[vIndex], vqmovun_s16(u));
                uIndex += 8;
                vIndex += 8;

                index += NUM_CH * 16;
            }

            // add stride offset here..
            index += strideOffset;
            //second row only Y
            for (int i = 0; i < width; i += 16) {
                // Load 16 pixels (48 bytes) from src
                uint8x16_t r, g, b;
                if constexpr (NUM_CH == 4) {
                    uint8x16x4_t pixels = vld4q_u8(&src[index]);

                    r = pixels.val[R_INDEX];
                    g = pixels.val[G_INDEX];
                    b = pixels.val[B_INDEX];
                }
                else {
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

                uint8x8_t y00 = vqshrn_n_u16(vmlal_u8(vmlal_u8(vmull_u8(r_low, (kR_Y8)), g_low, (kG_Y8)), b_low, (kB_Y8)), 8);
                uint8x8_t y01 = vqshrn_n_u16(vmlal_u8(vmlal_u8(vmull_u8(r_high, (kR_Y8)), g_high, (kG_Y8)), b_high, (kB_Y8)), 8);

                uint8x16_t y000 = vqaddq_u8(vcombine_u8(y00, y01), offset_Y);
                vst1q_u8(&buffer[yIndex], y000); yIndex += 16;

                index += NUM_CH * 16;
            }

            index += strideOffset;


        }
    }

   
#define fRY  0.21260
#define fGY  0.71520
#define fBY  0.07220
#define fRU -0.11412
#define fGU -0.38392
#define fBU  0.49804
#define fRV  0.49804
#define fGV -0.45237
#define fBV -0.04567
#define vRY  0.18275
#define vGY  0.61477
#define vBY  0.06200
#define vRU -0.10072
#define vGU -0.33882
#define vBU  0.43931
#define vRV  0.43867
#define vGV -0.40048
#define vBV -0.04038
    template <int R_INDEX, int G_INDEX, int B_INDEX, int NUM_CH>
    inline void RGB2YUVP_ParallelBody_SIMDv2(
        const unsigned char* src,
        unsigned char* dst,
        const int width,
        const int height,
        const int stride,
        const int begin,
        const int end
    ) {

        unsigned char* buffer = dst;
        /*
               * buffer[yIndex++] = ((25 * b + 129 * g + 66 * r) >> 8) + 16;
           buffer[yIndex++] = ((25 * b1 + 129 * g1 + 66 * r1) >> 8) + 16;

           buffer[uIndex++] = ((112 * r + -94 * g + -18 * b) >> 8) + 128;
           buffer[vIndex++] = ((-38 * r + -74 * g + 112 * b) >> 8) + 128;
               */
               // SIMD constants for YUV conversion

        const auto kB_Y = (25);
        const auto kG_Y = (129);
        const auto kR_Y = (66);
        const auto offset_Y = vdupq_n_u16(16);


        const auto kR_U = (112);
        const auto kG_U = (-94);
        const auto kB_U = (-18);

        const auto kR_V = (-38);
        const auto kG_V = (-74);
        const auto kB_V = (112);

        const auto offset_UV = vdupq_n_s16(128);

       /* const int kB_Y = (int)(vBY * 256);
        const int kG_Y = (int)(vGY * 256);
        const int kR_Y = (int)(vRY * 256);
        const auto offset_Y = vdupq_n_u16(16);


        const int kR_U = (int)(vRU * 256);
        const int kG_U = (int)(vGU * 256);
        const int kB_U = (int)(vBU * 256);

        const int kR_V = (int)(vRV * 256);
        const int kG_V = (int)(vGV * 256);
        const int kB_V = (int)(vBV * 256);

        const int16x8_t offset_UV = vdupq_n_s16(128);*/

        int index = 0;
        int nextLineindex = stride;

        int yIndex = 0;
        int yIndexn = yIndex + width;

        int uIndex = width * height;
        int vIndex = uIndex + (uIndex >> 2);
        // Loop over the specified range of rows
        for (int row = begin; row < end; row += 2) {
            // first row includes UV

            for (int i = 0; i < width; i += 16) {
                // Load 16 pixels (48 bytes) from src
                uint8x16_t r, g, b, rn, gn, bn;
                if constexpr (NUM_CH == 4) {
                    uint8x16x4_t pixels = vld4q_u8(&src[index]);
                    uint8x16x4_t pixelsn = vld4q_u8(&src[nextLineindex]);

                    r = pixels.val[R_INDEX];
                    g = pixels.val[G_INDEX];
                    b = pixels.val[B_INDEX];

                    rn = pixelsn.val[R_INDEX];
                    gn = pixelsn.val[G_INDEX];
                    bn = pixelsn.val[B_INDEX];
                }
                else {
                    uint8x16x3_t pixels = vld3q_u8(&src[index]);
                    uint8x16x3_t pixelsn = vld3q_u8(&src[nextLineindex]);

                    r = pixels.val[R_INDEX];
                    g = pixels.val[G_INDEX];
                    b = pixels.val[B_INDEX];

                    rn = pixelsn.val[R_INDEX];
                    gn = pixelsn.val[G_INDEX];
                    bn = pixelsn.val[B_INDEX];
                }


                //Widen to 16 bits unsigned
                uint8x8_t r_low = (vget_low_u8(r));
                uint8x8_t r_high = (vget_high_u8(r));
                uint8x8_t g_low = (vget_low_u8(g));
                uint8x8_t g_high = (vget_high_u8(g));
                uint8x8_t b_low = (vget_low_u8(b));
                uint8x8_t b_high = (vget_high_u8(b));

                uint8x8_t r_lown = (vget_low_u8(rn));
                uint8x8_t r_highn = (vget_high_u8(rn));
                uint8x8_t g_lown = (vget_low_u8(gn));
                uint8x8_t g_highn = (vget_high_u8(gn));
                uint8x8_t b_lown = (vget_low_u8(bn));
                uint8x8_t b_highn = (vget_high_u8(bn));

                uint8x8_t y00 = vqshrn_n_u16(vmlal_u8(vmlal_u8(vmull_u8(r_low, vdup_n_u8(kR_Y)), g_low, vdup_n_u8(kG_Y)), b_low, vdup_n_u8(kB_Y)), 8);
                uint8x8_t y01 = vqshrn_n_u16(vmlal_u8(vmlal_u8(vmull_u8(r_high, vdup_n_u8(kR_Y)), g_high, vdup_n_u8(kG_Y)), b_high, vdup_n_u8(kB_Y)), 8);
                uint8x8_t y10 = vqshrn_n_u16(vmlal_u8(vmlal_u8(vmull_u8(r_lown, vdup_n_u8(kR_Y)), g_lown, vdup_n_u8(kG_Y)), b_lown, vdup_n_u8(kB_Y)), 8);
                uint8x8_t y11 = vqshrn_n_u16(vmlal_u8(vmlal_u8(vmull_u8(r_highn, vdup_n_u8(kR_Y)), g_highn, vdup_n_u8(kG_Y)), b_highn, vdup_n_u8(kB_Y)), 8);

             /*   uint8x16_t y000 = vcombine_u8(y00, y01);
                uint8x16_t y100 = vcombine_u8(y10, y11);*/

                uint8x16_t y000 = vqaddq_u8(vcombine_u8(y00, y01), vdupq_n_u8(16));
                uint8x16_t y100 = vqaddq_u8(vcombine_u8(y10, y11), vdupq_n_u8(16));

                vst1q_u8(&buffer[yIndex], y000); yIndex += 16;
                vst1q_u8(&buffer[yIndexn], y100); yIndexn += 16;


                int16x8_t r000 = vpadalq_u8(vpaddlq_u8(r), rn);
                int16x8_t g000 = vpadalq_u8(vpaddlq_u8(g), gn);
                int16x8_t b000 = vpadalq_u8(vpaddlq_u8(b), bn);

                uint8x8_t u00 = vrshrn_n_s16(vmlaq_s16(vmlaq_s16(vmulq_s16(r000, vdupq_n_s16(kR_U >> 2)), g000, vdupq_n_s16(kG_U >> 2)), b000, vdupq_n_s16(kB_U >> 2)), 8);
                uint8x8_t v00 = vrshrn_n_s16(vmlaq_s16(vmlaq_s16(vmulq_s16(r000, vdupq_n_s16(kR_V >> 2)), g000, vdupq_n_s16(kG_V >> 2)), b000, vdupq_n_s16(kB_V >> 2)), 8);

                u00 = vadd_u8(u00, vdup_n_u8(128));
                v00 = vadd_u8(v00, vdup_n_u8(128));

                vst1_u8(&buffer[uIndex], v00);
                vst1_u8(&buffer[vIndex], u00);
                uIndex += 8;
                vIndex += 8;

                index += NUM_CH * 16;
                nextLineindex += NUM_CH * 16;
            }
            yIndex += width;
            yIndexn += width;
            index += stride;
            nextLineindex += stride;
        }
    }



}

#endif
