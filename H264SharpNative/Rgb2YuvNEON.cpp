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
	
    void Rgb2Yuv::BGRAtoYUV420PlanarNeon(const unsigned char* bgra, unsigned char* dst, int width, int height, int stride, int threadCount)
    {
        RGB2YUVP_ParallelBody_SIMD<0,1,2,4>(bgra, dst, width, height, stride, 0, height);

    }
    void Rgb2Yuv::BGRtoYUV420PlanarNeon(unsigned char* bgr, unsigned char* dst, int width, int height, int stride, int threadCount)
    {
        RGB2YUVP_ParallelBody_SIMD<0,1,2,3>(bgr, dst, width, height, stride, 0, height);

    }
    void Rgb2Yuv::RGBAtoYUV420PlanarNeon(unsigned char* rgba, unsigned char* dst, int width, int height, int stride, int threadCount)
    {
        RGB2YUVP_ParallelBody_SIMD<2,1,0,4>(rgba, dst, width, height, stride, 0, height);

    }
    void Rgb2Yuv::RGBtoYUV420PlanarNeon(unsigned char* rgb, unsigned char* dst, int width, int height, int stride, int threadCount)
    {
        RGB2YUVP_ParallelBody_SIMD<2,1,0,3>(rgb, dst, width, height, stride, 0, height);
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

        // SIMD constants for YUV conversion
        const uint16x8_t kR_Y = vdupq_n_u16(66);
        const uint16x8_t kG_Y = vdupq_n_u16(129);
        const uint16x8_t kB_Y = vdupq_n_u16(25);
        const uint16x8_t kR_U = vdupq_n_u16(112);  
        const int16x8_t kG_U = vdupq_n_s16(-94);   
        const int16x8_t kB_U = vdupq_n_s16(-18);   
        const int16x8_t kR_V = vdupq_n_s16(-38);  
        const int16x8_t kG_V = vdupq_n_s16(-74);   
        const uint16x8_t kB_V = vdupq_n_u16(112);  
        const uint16x8_t offset_Y = vdupq_n_u16(16);
        const uint16x8_t offset_UV = vdupq_n_u16(128);

        int index = 0;
        int yIndex = 0;
        int uIndex = width* height;
        int vIndex = uIndex + width * height/4;
        // Loop over the specified range of rows
        for (int row = begin; row < end; row+=2) {
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
                

                //Widen to 16 bits unsigned
                uint16x8_t r_low = vmovl_u8(vget_low_u8(r));
                uint16x8_t r_high = vmovl_u8(vget_high_u8(r));
                uint16x8_t g_low = vmovl_u8(vget_low_u8(g));
                uint16x8_t g_high = vmovl_u8(vget_high_u8(g));
                uint16x8_t b_low = vmovl_u8(vget_low_u8(b));
                uint16x8_t b_high = vmovl_u8(vget_high_u8(b));

                // Y Channel (Unsigned because we can overflow) 
                uint16x8_t y_low = vaddq_u16(vaddq_u16(vmulq_u16(r_low, kR_Y), vmulq_u16(g_low, kG_Y)), vmulq_u16(b_low, kB_Y));
                uint16x8_t y_high = vaddq_u16(vaddq_u16(vmulq_u16(r_high, kR_Y), vmulq_u16(g_high, kG_Y)), vmulq_u16(b_high, kB_Y));
                // div 256(shift 8) + offset
                y_low = vshrq_n_u16(y_low, 8);
                y_high = vshrq_n_u16(y_high, 8);
                y_low = vaddq_u16(y_low, offset_Y);
                y_high = vaddq_u16(y_high, offset_Y);

                // shrink combine strore
                vst1q_u8(&buffer[yIndex], vcombine_u8(vqmovn_u16(y_low), vqmovn_u16(y_high)));
                yIndex += 16;

                // we need signed here 
                int16x8_t r_low_signed = vreinterpretq_s16_u16(r_low);
                int16x8_t g_low_signed = vreinterpretq_s16_u16(g_low);
                int16x8_t b_low_signed = vreinterpretq_s16_u16(b_low);

                // Compute U channel (average over 2x2 blocks)
                int16x8_t u = vaddq_s16(vaddq_s16(vmulq_s16(r_low_signed, kR_U), vmulq_s16(g_low_signed, kG_U)), vmulq_s16(b_low_signed, kB_U));
                u = vshrq_n_s16(u, 8);
                u = vaddq_s16(u, offset_UV);

                // Compute V channel
                int16x8_t v = vaddq_s16(vaddq_s16(vmulq_s16(r_low_signed, kR_V), vmulq_s16(g_low_signed, kG_V)), vmulq_s16(b_low_signed, kB_V));
                v = vshrq_n_s16(v, 8);
                v = vaddq_s16(v, offset_UV);

                // Store U and V
                vst1_u8(&buffer[uIndex], vqmovun_s16(u));
                vst1_u8(&buffer[vIndex], vqmovun_s16(v));
                uIndex += 8;
                vIndex += 8;

                index +=  NUM_CH*16;
            }

            // add stride offset here..
            //index += row*stride - index;
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

                uint16x8_t r_low = vmovl_u8(vget_low_u8(r));
                uint16x8_t r_high = vmovl_u8(vget_high_u8(r));
                uint16x8_t g_low = vmovl_u8(vget_low_u8(g));
                uint16x8_t g_high = vmovl_u8(vget_high_u8(g));
                uint16x8_t b_low = vmovl_u8(vget_low_u8(b));
                uint16x8_t b_high = vmovl_u8(vget_high_u8(b));

                // Y Channel (Unsigned because we can overflow the signed) 
                uint16x8_t y_low = vaddq_u16(vaddq_u16(vmulq_u16(r_low, kR_Y), vmulq_u16(g_low, kG_Y)), vmulq_u16(b_low, kB_Y));
                uint16x8_t y_high = vaddq_u16(vaddq_u16(vmulq_u16(r_high, kR_Y), vmulq_u16(g_high, kG_Y)), vmulq_u16(b_high, kB_Y));
                y_low = vshrq_n_u16(y_low, 8);
                y_high = vshrq_n_u16(y_high, 8);
                y_low = vaddq_u16(y_low, offset_Y);
                y_high = vaddq_u16(y_high, offset_Y);

                vst1q_u8(&buffer[yIndex], vcombine_u8(vqmovn_u16(y_low), vqmovn_u16(y_high)));
                yIndex += 16;

                index += NUM_CH * 16;
            }
        }
    }

   

}

#endif
