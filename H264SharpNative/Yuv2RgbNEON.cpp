#if defined(__aarch64__)

#include <arm_neon.h>
#include <cstdint>
#include "Yuv2Rgb.h"
#include "Converter.h"


namespace H264Sharp
{

    // BT.601-7 studio range constants
    const int16x8_t const_16 = vdupq_n_s16(16);
    const int16x8_t const_128 = vdupq_n_s16(128);
    const int16x8_t const_0 = vdupq_n_s16(0);
    const int16x8_t const_255 = vdupq_n_s16(255);
    // divide them by 2 here
    // Precalculated fixed-point coefficients
    // FP scaling: 1 << 7 (128) to maintain precision
    const int16_t y_factor = 149;      // 1.164 * 128
    const int16_t v_to_r_coeff = 204;  // 1.596 * 128
    const int16_t u_to_g_coeff = 50;   // 0.391 * 128
    const int16_t v_to_g_coeff = 104;  // 0.813 * 128
    const int16_t u_to_b_coeff = 258;  // 2.018 * 128
   

    void Yuv2Rgb::ConvertYUVToRGB_NEON(
        const uint8_t* y_plane,
        const uint8_t* u_plane,
        const uint8_t* v_plane,
        uint8_t* rgb_buffer,
        int width,
        int height)
    {
            const int uv_width = width / 2;
            const int uv_height = height / 2;
            for (int y = 0; y < height; y += 2) {
                const uint8_t* y_row1 = y_plane + y * width;
                const uint8_t* y_row2 = y_row1 + width;
                const uint8_t* u_row = u_plane + (y / 2) * uv_width;
                const uint8_t* v_row = v_plane + (y / 2) * uv_width;
                uint8_t* rgb_row1 = rgb_buffer + y * width * 3;
                uint8_t* rgb_row2 = rgb_row1 + width * 3;

                for (int x = 0; x < width; x += 16) {
                    // Load 16 Y values for two rows
                    uint8x16_t y_vals1 = vld1q_u8(y_row1 + x);
                    uint8x16_t y_vals2 = vld1q_u8(y_row2 + x);

                    // Load 8 U and V values
                    uint8x8_t u_vals8 = vld1_u8(u_row + x / 2);
                    uint8x8_t v_vals8 = vld1_u8(v_row + x / 2);

                    // Convert Y to 16-bit and adjust range
                    int16x8_t y_vals_16_1l = vsubq_s16(
                        vreinterpretq_s16_u16(vmovl_u8(vget_low_u8(y_vals1))),
                        const_16);
                    int16x8_t y_vals_16_1h = vsubq_s16(
                        vreinterpretq_s16_u16(vmovl_u8(vget_high_u8(y_vals1))),
                        const_16);

                    // Scale Y (multiply by 1.164)
                    y_vals_16_1l = vmulq_n_s16(y_vals_16_1l, y_factor) >> 7;
                    y_vals_16_1h = vmulq_n_s16(y_vals_16_1h, y_factor) >> 7;

                    // Process U/V (widen then -128)
                    int16x8_t u_vals = vsubq_s16(vreinterpretq_s16_u16(vmovl_u8(u_vals8)), const_128);
                    int16x8_t v_vals = vsubq_s16(vreinterpretq_s16_u16(vmovl_u8(v_vals8)), const_128);

                    // multiply UV with the scaling
                    int16x8_t u_vals_ug = vmulq_n_s16(u_vals, u_to_g_coeff) >> 7;
                    int16x8_t u_vals_ub = vmulq_n_s16(u_vals, u_to_b_coeff) >> 7;
                    int16x8_t v_vals_vg = vmulq_n_s16(v_vals, v_to_g_coeff) >> 7;
                    int16x8_t v_vals_vr = vmulq_n_s16(v_vals, v_to_r_coeff) >> 7;

                    // Calculate RGB for first 8 pixels
                    int16x8_t r1l = vaddq_s16(y_vals_16_1l, v_vals_vr);
                    int16x8_t g1l = vsubq_s16(vsubq_s16(y_vals_16_1l, u_vals_ug), v_vals_vg);
                    int16x8_t b1l = vaddq_s16(y_vals_16_1l, u_vals_ub);

                    // Calculate RGB for second 8 pixels
                    int16x8_t r1h = vaddq_s16(y_vals_16_1h, v_vals_vr);
                    int16x8_t g1h = vsubq_s16(vsubq_s16(y_vals_16_1h, u_vals_ug), v_vals_vg);
                    int16x8_t b1h = vaddq_s16(y_vals_16_1h, u_vals_ub);


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
                    vst3_u8(rgb_row1 + (x + 8) * 3, rgb1h);

                    // Process second row
                    int16x8_t y_vals_16_2l = vsubq_s16(
                        vreinterpretq_s16_u16(vmovl_u8(vget_low_u8(y_vals2))),
                        const_16);
                    int16x8_t y_vals_16_2h = vsubq_s16(
                        vreinterpretq_s16_u16(vmovl_u8(vget_high_u8(y_vals2))),
                        const_16);

                    y_vals_16_2l = vmulq_n_s16(y_vals_16_2l, y_factor) >> 7;
                    y_vals_16_2h = vmulq_n_s16(y_vals_16_2h, y_factor) >> 7;

                    // Calculate RGB for second row
                    // Calculate RGB for first 8 pixels
                    int16x8_t r2l = vaddq_s16(y_vals_16_2l, v_vals_vr);
                    int16x8_t g2l = vsubq_s16(vsubq_s16(y_vals_16_2l, u_vals_ug), v_vals_vg);
                    int16x8_t b2l = vaddq_s16(y_vals_16_2l, u_vals_ub);

                    // Calculate RGB for second 8 pixels
                    int16x8_t r2h = vaddq_s16(y_vals_16_2h, v_vals_vr);
                    int16x8_t g2h = vsubq_s16(vsubq_s16(y_vals_16_2h, u_vals_ug), v_vals_vg);
                    int16x8_t b2h = vaddq_s16(y_vals_16_2h, u_vals_ub);

                    // Clamp second row values
                    // Store second row (in BGR order)
                    uint8x8x3_t rgb2l, rgb2h;
                    rgb2l.val[0] = vqmovun_s16(b2l);
                    rgb2l.val[1] = vqmovun_s16(g2l);
                    rgb2l.val[2] = vqmovun_s16(r2l);

                    rgb2h.val[0] = vqmovun_s16(b2h);
                    rgb2h.val[1] = vqmovun_s16(g2h);
                    rgb2h.val[2] = vqmovun_s16(r2h);

                    vst3_u8(rgb_row2 + x * 3, rgb2l);
                    vst3_u8(rgb_row2 + (x + 8) * 3, rgb2h);
                }
            }
        }
    

    void PB(int begin,
        int end,
        const uint8_t* y_plane,
        const uint8_t* u_plane,
        const uint8_t* v_plane,
        uint8_t* rgb_buffer,
        int width,
        int height);

    void Yuv2Rgb::ConvertYUVToRGB_NEON_Parallel(
        const uint8_t* y_plane,
        const uint8_t* u_plane,
        const uint8_t* v_plane,
        uint8_t* rgb_buffer,
        int width,
        int height,
        int numThreads)
    {
        ThreadPool::For(0, numThreads, [&](int j)
            {
                int hi = height;
                int bgn = ((hi / numThreads) * (j));
                int end = ((hi / numThreads) * (j + 1));
                if (j == numThreads - 1)
                {
                    end = hi;
                }

                PB(bgn, end, y_plane,
                    u_plane,
                    v_plane,
                    rgb_buffer,
                    width,
                    height);
            });
    };

    void PB(int begin,
        int end,
        const uint8_t* y_plane,
        const uint8_t* u_plane,
        const uint8_t* v_plane,
        uint8_t* rgb_buffer,
        int width,
        int height)
    {
        const int uv_width = width / 2;
        const int uv_height = height / 2;

        for (int y = begin; y < end; y += 2) {
            const uint8_t* y_row1 = y_plane + y * width;
            const uint8_t* y_row2 = y_row1 + width;
            const uint8_t* u_row = u_plane + (y / 2) * uv_width;
            const uint8_t* v_row = v_plane + (y / 2) * uv_width;
            uint8_t* rgb_row1 = rgb_buffer + y * width * 3;
            uint8_t* rgb_row2 = rgb_row1 + width * 3;

            for (int x = 0; x < width; x += 16) {
                // Load 16 Y values for two rows
                uint8x16_t y_vals1 = vld1q_u8(y_row1 + x);
                uint8x16_t y_vals2 = vld1q_u8(y_row2 + x);

                // Load 8 U and V values
                uint8x8_t u_vals8 = vld1_u8(u_row + x / 2);
                uint8x8_t v_vals8 = vld1_u8(v_row + x / 2);

                // Convert Y to 16-bit and adjust range
                int16x8_t y_vals_16_1l = vsubq_s16(
                    vreinterpretq_s16_u16(vmovl_u8(vget_low_u8(y_vals1))),
                    const_16);
                int16x8_t y_vals_16_1h = vsubq_s16(
                    vreinterpretq_s16_u16(vmovl_u8(vget_high_u8(y_vals1))),
                    const_16);

                // Scale Y (multiply by 1.164)
                y_vals_16_1l = vmulq_n_s16(y_vals_16_1l, y_factor) >> 7;
                y_vals_16_1h = vmulq_n_s16(y_vals_16_1h, y_factor) >> 7;

                // Process U/V (widen then -128)
                int16x8_t u_vals = vsubq_s16(vreinterpretq_s16_u16(vmovl_u8(u_vals8)), const_128);
                int16x8_t v_vals = vsubq_s16(vreinterpretq_s16_u16(vmovl_u8(v_vals8)), const_128);

                // multiply UV with the scaling
                int16x8_t u_vals_ug = vmulq_n_s16(u_vals, u_to_g_coeff) >> 7;
                int16x8_t u_vals_ub = vmulq_n_s16(u_vals, u_to_b_coeff) >> 7;
                int16x8_t v_vals_vg = vmulq_n_s16(v_vals, v_to_g_coeff) >> 7;
                int16x8_t v_vals_vr = vmulq_n_s16(v_vals, v_to_r_coeff) >> 7;

                // Calculate RGB for first 8 pixels
                int16x8_t r1l = vaddq_s16(y_vals_16_1l, v_vals_vr);
                int16x8_t g1l = vsubq_s16(vsubq_s16(y_vals_16_1l, u_vals_ug), v_vals_vg);
                int16x8_t b1l = vaddq_s16(y_vals_16_1l, u_vals_ub);

                // Calculate RGB for second 8 pixels
                int16x8_t r1h = vaddq_s16(y_vals_16_1h, v_vals_vr);
                int16x8_t g1h = vsubq_s16(vsubq_s16(y_vals_16_1h, u_vals_ug), v_vals_vg);
                int16x8_t b1h = vaddq_s16(y_vals_16_1h, u_vals_ub);


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
                vst3_u8(rgb_row1 + (x + 8) * 3, rgb1h);

                // Process second row
                int16x8_t y_vals_16_2l = vsubq_s16(
                    vreinterpretq_s16_u16(vmovl_u8(vget_low_u8(y_vals2))),
                    const_16);
                int16x8_t y_vals_16_2h = vsubq_s16(
                    vreinterpretq_s16_u16(vmovl_u8(vget_high_u8(y_vals2))),
                    const_16);

                y_vals_16_2l = vmulq_n_s16(y_vals_16_2l, y_factor) >> 7;
                y_vals_16_2h = vmulq_n_s16(y_vals_16_2h, y_factor) >> 7;

                // Calculate RGB for second row
                // Calculate RGB for first 8 pixels
                int16x8_t r2l = vaddq_s16(y_vals_16_2l, v_vals_vr);
                int16x8_t g2l = vsubq_s16(vsubq_s16(y_vals_16_2l, u_vals_ug), v_vals_vg);
                int16x8_t b2l = vaddq_s16(y_vals_16_2l, u_vals_ub);

                // Calculate RGB for second 8 pixels
                int16x8_t r2h = vaddq_s16(y_vals_16_2h, v_vals_vr);
                int16x8_t g2h = vsubq_s16(vsubq_s16(y_vals_16_2h, u_vals_ug), v_vals_vg);
                int16x8_t b2h = vaddq_s16(y_vals_16_2h, u_vals_ub);

                // Clamp second row values
                // Store second row (in BGR order)
                uint8x8x3_t rgb2l, rgb2h;
                rgb2l.val[0] = vqmovun_s16(b2l);
                rgb2l.val[1] = vqmovun_s16(g2l);
                rgb2l.val[2] = vqmovun_s16(r2l);

                rgb2h.val[0] = vqmovun_s16(b2h);
                rgb2h.val[1] = vqmovun_s16(g2h);
                rgb2h.val[2] = vqmovun_s16(r2h);

                vst3_u8(rgb_row2 + x * 3, rgb2l);
                vst3_u8(rgb_row2 + (x + 8) * 3, rgb2h);
            }
        }
    }



} 
#endif // #if defined(__aarch64__)

