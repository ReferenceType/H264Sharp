#if defined(__aarch64__)

#include <arm_neon.h>
#include <cstdint>
#include "Yuv2Rgb.h"
#include "Converter.h"


namespace H264Sharp
{

    inline void ConvertYUVToRGB_NEON_Body(
        const uint8_t* y_plane,
        const uint8_t* u_plane,
        const uint8_t* v_plane,
        uint8_t* rgb_buffer,
        int width,
        int begin,
        int end);

    void Yuv2Rgb::ConvertYUVToRGB_NEON( const uint8_t* y_plane, const uint8_t* u_plane, const uint8_t* v_plane, 
        uint8_t* rgb_buffer, int width,int heigth)
    {
        ConvertYUVToRGB_NEON_Body(y_plane, u_plane, v_plane, rgb_buffer, width, 0, heigth);
    }

     void Yuv2Rgb::ConvertYUVToRGB_NEON_Parallel(const uint8_t* y_plane, const uint8_t* u_plane, const uint8_t* v_plane,
        uint8_t* rgb_buffer, int width, int heigth, int numThreads)
    {
        int chunkLen = heigth / numThreads;
        ThreadPool::For(int(0), numThreads, [&](int j)
            {
                int bgn = chunkLen * j;
                int end = chunkLen * (j + 1);
                if (j == numThreads - 1)
                {
                    end = heigth;
                }

                ConvertYUVToRGB_NEON_Body(y_plane, u_plane, v_plane, rgb_buffer, width, bgn, end);

            });
    }
    
    /*
    * R = CLAMP((Y-16)*1.164 +           1.596*V)
      G = CLAMP((Y-16)*1.164 - 0.391*U - 0.813*V)
      B = CLAMP((Y-16)*1.164 + 2.018*U          )
    */
    // BT.601-7 studio range constants
    const int16x8_t const_16 = vdupq_n_s16(16);
    const int16x8_t const_128 = vdupq_n_s16(128);
  
    const int16_t y_factor = 149;      // 1.164 * 128
    const int16_t v_to_r_coeff = 102;  // 1.596 * 64
    const int16_t u_to_g_coeff = 25;   // 0.391 * 64
    const int16_t v_to_g_coeff = 52;  // 0.813 * 64
    const int16_t u_to_b_coeff = 129;  // 2.018 * 64

    inline void ConvertYUVToRGB_NEON_Body(
        const uint8_t* y_plane,
        const uint8_t* u_plane,
        const uint8_t* v_plane,
        uint8_t* rgb_buffer,
        int width,
        int begin,
        int end)
    {
        const int uv_width = width / 2;
        for (int y = begin; y < end; y += 2) {
            const uint8_t* y_row1 = y_plane + y * width;
            const uint8_t* y_row2 = y_row1 + width;
            const uint8_t* u_row = u_plane + (y / 2) * uv_width;
            const uint8_t* v_row = v_plane + (y / 2) * uv_width;
            uint8_t* rgb_row1 = rgb_buffer + y * width * 3;
            uint8_t* rgb_row2 = rgb_row1 + width * 3;

            for (int x = 0; x < width; x += 16) {

                // Load 8 U and V values
                uint8x8_t u_vals8 = vld1_u8(u_row + (x / 2));
                uint8x8_t v_vals8 = vld1_u8(v_row + (x / 2));

                // Process U/V (widen then -128)
                int16x8_t u_vals = vsubq_s16(vreinterpretq_s16_u16(vmovl_u8(u_vals8)), const_128);
                int16x8_t v_vals = vsubq_s16(vreinterpretq_s16_u16(vmovl_u8(v_vals8)), const_128);

                // duplicate [1,2,3,4,5,6,7,8]=> [1,1,2,2,3,3,4,4] , [5,5,6,6,7,7,8,8]
                int16x8_t u_valsl = vzip1q_s16(u_vals, u_vals);
                int16x8_t u_valsh = vzip2q_s16(u_vals, u_vals);

                int16x8_t v_valsl = vzip1q_s16(v_vals, v_vals);
                int16x8_t v_valsh = vzip2q_s16(v_vals, v_vals);

                // multiply UV with the scaling
                int16x8_t u_vals_ugl = vshrq_n_s16(vmulq_n_s16(u_valsl, u_to_g_coeff),6);
                int16x8_t u_vals_ubl = vshrq_n_s16(vmulq_n_s16(u_valsl, u_to_b_coeff),6);
                int16x8_t v_vals_vgl = vshrq_n_s16(vmulq_n_s16(v_valsl, v_to_g_coeff), 6);
                int16x8_t v_vals_vrl = vshrq_n_s16(vmulq_n_s16(v_valsl, v_to_r_coeff), 6);

                int16x8_t u_vals_ugh = vshrq_n_s16(vmulq_n_s16(u_valsh, u_to_g_coeff), 6);
                int16x8_t u_vals_ubh = vshrq_n_s16(vmulq_n_s16(u_valsh, u_to_b_coeff), 6);
                int16x8_t v_vals_vgh = vshrq_n_s16(vmulq_n_s16(v_valsh, v_to_g_coeff), 6);
                int16x8_t v_vals_vrh = vshrq_n_s16(vmulq_n_s16(v_valsh, v_to_r_coeff), 6);

                // Load 16 Y values for two rows
                uint8x16_t y_vals1 = vld1q_u8(y_row1 + x);
                uint8x16_t y_vals2 = vld1q_u8(y_row2 + x);


                // Convert Y to 16-bit and adjust range
                int16x8_t y_vals_16_1l = vsubq_s16(vreinterpretq_s16_u16(vmovl_u8(vget_low_u8(y_vals1))), const_16);
                int16x8_t y_vals_16_1h = vsubq_s16(vreinterpretq_s16_u16(vmovl_u8(vget_high_u8(y_vals1))), const_16);
                //second row
                int16x8_t y_vals_16_2l = vsubq_s16(vreinterpretq_s16_u16(vmovl_u8(vget_low_u8(y_vals2))), const_16);
                int16x8_t y_vals_16_2h = vsubq_s16(vreinterpretq_s16_u16(vmovl_u8(vget_high_u8(y_vals2))), const_16);

                // Scale Y (-16 and multiply by 1.164)
                y_vals_16_1l = vshrq_n_s16(vmulq_n_s16(y_vals_16_1l, y_factor), 7);
                y_vals_16_1h = vshrq_n_s16(vmulq_n_s16(y_vals_16_1h, y_factor), 7);
                y_vals_16_2l = vshrq_n_s16(vmulq_n_s16(y_vals_16_2l, y_factor), 7);
                y_vals_16_2h = vshrq_n_s16(vmulq_n_s16(y_vals_16_2h, y_factor), 7);

                // Calculate RGB for first 8 pixels
                int16x8_t r1l = vaddq_s16(y_vals_16_1l, v_vals_vrl);
                int16x8_t g1l = vsubq_s16(vsubq_s16(y_vals_16_1l, u_vals_ugl), v_vals_vgl);
                int16x8_t b1l = vaddq_s16(y_vals_16_1l, u_vals_ubl);
               
                int16x8_t r1h = vaddq_s16(y_vals_16_1h, v_vals_vrh);
                int16x8_t g1h = vsubq_s16(vsubq_s16(y_vals_16_1h, u_vals_ugh), v_vals_vgh);
                int16x8_t b1h = vaddq_s16(y_vals_16_1h, u_vals_ubh);

                // Calculate RGB for second row
                int16x8_t r2l = vaddq_s16(y_vals_16_2l, v_vals_vrl);
                int16x8_t g2l = vsubq_s16(vsubq_s16(y_vals_16_2l, u_vals_ugl), v_vals_vgl);
                int16x8_t b2l = vaddq_s16(y_vals_16_2l, u_vals_ubl);

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




} 
#endif // #if defined(__aarch64__)

