#include "Yuv2Rgb.h"

#if defined(__aarch64__)

#include <arm_neon.h>
#include <cstdint>
#include "Converter.h"


namespace H264Sharp
{
    /*
    * R = CLAMP((Y-16)*1.164 +           1.596*V)
      G = CLAMP((Y-16)*1.164 - 0.391*U - 0.813*V)
      B = CLAMP((Y-16)*1.164 + 2.018*U          )
    */
    // BT.601-7 studio range constants
    const int16x8_t const_16 = vdupq_n_s16(16);
    const uint8x16_t const_16_8 = vdupq_n_u8(16);
    const int16x8_t const_128 = vdupq_n_s16(128);
  
    const auto y_factor = vdupq_n_u16(149);      // 1.164 * 64
    const auto v_to_r_coeff = vdupq_n_s16(102);  // 1.596 * 64
    const auto u_to_g_coeff = vdupq_n_s16(25);   // 0.391 * 64
    const auto v_to_g_coeff = vdupq_n_s16(52);  // 0.813 * 64
    const auto u_to_b_coeff = vdupq_n_s16(129);  // 2.018 * 64

    inline void ConvertYUVToRGB_NEON_Body(
        const uint8_t* RESTRICT y_plane,
        const uint8_t* RESTRICT u_plane,
        const uint8_t* RESTRICT v_plane,
        int y_stride,
        int uv_stride,
        uint8_t* RESTRICT rgb_buffer,
        int width,
        int begin,
        int end)
    {
        for (int y = begin; y < end; y += 2) 
        {
            const uint8_t* RESTRICT y_row1 = y_plane + y * y_stride;
            const uint8_t* RESTRICT y_row2 = y_row1 + y_stride;
            const uint8_t* RESTRICT u_row = u_plane + (y / 2) * uv_stride;
            const uint8_t* RESTRICT v_row = v_plane + (y / 2) * uv_stride;
            uint8_t* RESTRICT rgb_row1 = rgb_buffer + y * width * 3;
            uint8_t* RESTRICT rgb_row2 = rgb_row1 + width * 3;

            for (int x = 0; x < width; x += 16) {


                // Load 8 U and V values
                uint8x8_t u_vals8 = vld1_u8(u_row + (x / 2));
                uint8x8_t v_vals8 = vld1_u8(v_row + (x / 2));
                // Load 16 Y values for two rows and -16
                uint8x16_t y_vals1 = vqsubq_u8(vld1q_u8(y_row1 + x), const_16_8);
                uint8x16_t y_vals2 = vqsubq_u8(vld1q_u8(y_row2 + x), const_16_8);

                // Process U/V (widen then -128)
                int16x8_t u_vals = vsubq_s16(vreinterpretq_s16_u16(vmovl_u8(u_vals8)), const_128);
                int16x8_t v_vals = vsubq_s16(vreinterpretq_s16_u16(vmovl_u8(v_vals8)), const_128);

                // multiply UV with the scaling
                int16x8_t u_vals_ug = vshrq_n_s16(vmulq_s16(u_vals, u_to_g_coeff), 6);
                int16x8_t u_vals_ub = vshrq_n_s16(vmulq_s16(u_vals, u_to_b_coeff), 6);
                int16x8_t v_vals_vg = vshrq_n_s16(vmulq_s16(v_vals, v_to_g_coeff), 6);
                int16x8_t v_vals_vr = vshrq_n_s16(vmulq_s16(v_vals, v_to_r_coeff), 6);


                ////first half duplicate(upscale) [1,2,3,4,5,6,7,8]=> [1,1,2,2,3,3,4,4] , [5,5,6,6,7,7,8,8]
                int16x8_t u_vals_ugl = vzip1q_s16(u_vals_ug, u_vals_ug);
                int16x8_t u_vals_ubl = vzip1q_s16(u_vals_ub, u_vals_ub);
                int16x8_t v_vals_vgl = vzip1q_s16(v_vals_vg, v_vals_vg);
                int16x8_t v_vals_vrl = vzip1q_s16(v_vals_vr, v_vals_vr);

                //// second half duplicate(upscale) [1,2,3,4,5,6,7,8]=> [1,1,2,2,3,3,4,4] , [5,5,6,6,7,7,8,8]
                int16x8_t u_vals_ugh = vzip2q_s16(u_vals_ug, u_vals_ug);
                int16x8_t u_vals_ubh = vzip2q_s16(u_vals_ub, u_vals_ub);
                int16x8_t v_vals_vgh = vzip2q_s16(v_vals_vg, v_vals_vg);
                int16x8_t v_vals_vrh = vzip2q_s16(v_vals_vr, v_vals_vr);


                // Convert Y to 16-bit and scale
                uint16x8_t y_vals_16_1lu = (vmovl_u8(vget_low_u8(y_vals1)));
                uint16x8_t y_vals_16_1hu = (vmovl_u8(vget_high_u8(y_vals1)));

                uint16x8_t y_vals_16_2lu = (vmovl_u8(vget_low_u8(y_vals2)));
                uint16x8_t y_vals_16_2hu = (vmovl_u8(vget_high_u8(y_vals2)));

                int16x8_t y_vals_16_1h = vreinterpretq_s16_u16(vshrq_n_u16(vmulq_u16(y_vals_16_1hu, y_factor), 7));
                int16x8_t y_vals_16_2h = vreinterpretq_s16_u16(vshrq_n_u16(vmulq_u16(y_vals_16_2hu, y_factor), 7));

                int16x8_t y_vals_16_1l = vreinterpretq_s16_u16(vshrq_n_u16(vmulq_u16(y_vals_16_1lu, y_factor), 7));
                int16x8_t y_vals_16_2l = vreinterpretq_s16_u16(vshrq_n_u16(vmulq_u16(y_vals_16_2lu, y_factor), 7));

                int16x8_t r1l = vaddq_s16(y_vals_16_1l, v_vals_vrl);
                int16x8_t g1l = vsubq_s16(vsubq_s16(y_vals_16_1l, u_vals_ugl), v_vals_vgl);
                int16x8_t b1l = vaddq_s16(y_vals_16_1l, u_vals_ubl);

                int16x8_t r2l = vaddq_s16(y_vals_16_2l, v_vals_vrl);
                int16x8_t g2l = vsubq_s16(vsubq_s16(y_vals_16_2l, u_vals_ugl), v_vals_vgl);
                int16x8_t b2l = vaddq_s16(y_vals_16_2l, u_vals_ubl);

                int16x8_t r1h = vaddq_s16(y_vals_16_1h, v_vals_vrh);
                int16x8_t g1h = vsubq_s16(vsubq_s16(y_vals_16_1h, u_vals_ugh), v_vals_vgh);
                int16x8_t b1h = vaddq_s16(y_vals_16_1h, u_vals_ubh);

                int16x8_t r2h = vaddq_s16(y_vals_16_2h, v_vals_vrh);
                int16x8_t g2h = vsubq_s16(vsubq_s16(y_vals_16_2h, u_vals_ugh), v_vals_vgh);
                int16x8_t b2h = vaddq_s16(y_vals_16_2h, u_vals_ubh);

                // Clamp values between 0 and 255
                // Store first row (in BGR order)
                uint8x8x3_t rgb1l, rgb1h;
                rgb1l.val[0] = vqmovun_s16(b1l);
                rgb1l.val[1] = vqmovun_s16(g1l);
                rgb1l.val[2] = vqmovun_s16(r1l);

                rgb1h.val[0] = vqmovun_s16(b1h);
                rgb1h.val[1] = vqmovun_s16(g1h);
                rgb1h.val[2] = vqmovun_s16(r1h);

                vst3_u8(rgb_row1 + x * 3, rgb1l);
                vst3_u8(rgb_row1 + (x * 3) + 24, rgb1h);

                rgb1l.val[0] = vqmovun_s16(b2l);
                rgb1l.val[1] = vqmovun_s16(g2l);
                rgb1l.val[2] = vqmovun_s16(r2l);

                rgb1h.val[0] = vqmovun_s16(b2h);
                rgb1h.val[1] = vqmovun_s16(g2h);
                rgb1h.val[2] = vqmovun_s16(r2h);

                vst3_u8(rgb_row2 + x * 3, rgb1l);
                vst3_u8(rgb_row2 + (x * 3) + 24, rgb1h);
            }
        }
    }



    void Yuv2Rgb::ConvertYUVToRGB_NEON(const uint8_t* RESTRICT y_plane, const uint8_t* RESTRICT u_plane, const uint8_t* RESTRICT v_plane, uint32_t Y_stride,
        uint32_t UV_stride,
        uint8_t* RESTRICT rgb_buffer, int width, int heigth)
    {
        ConvertYUVToRGB_NEON_Body(y_plane, u_plane, v_plane, Y_stride, UV_stride, rgb_buffer, width, 0, heigth);
    }

    void Yuv2Rgb::ConvertYUVToRGB_NEON_Parallel(const uint8_t* RESTRICT y_plane, const uint8_t* RESTRICT u_plane, const uint8_t* RESTRICT v_plane, uint32_t Y_stride,
        uint32_t UV_stride,
        uint8_t* RESTRICT rgb_buffer, int width, int heigth, int numThreads)
    {
        int chunkLen = heigth / numThreads;
        if (chunkLen % 2 != 0) {
            chunkLen -= 1;
        }

        ThreadPool::For(int(0), numThreads, [&](int j)
            {
                int bgn = chunkLen * j;
                int end = bgn + chunkLen;

                if (j == numThreads - 1) {
                    end = heigth;
                }

                if ((end - bgn) % 2 != 0) {
                    bgn -= 1;
                }

                ConvertYUVToRGB_NEON_Body(y_plane, u_plane, v_plane, Y_stride, UV_stride, rgb_buffer, width, bgn, end);

            });
    }
} 
#endif // #if defined(__aarch64__)

