#if defined(__aarch64__)

#include <arm_neon.h>
#include <cstdint>
#include <cassert>
#include "Yuv2Rgb.h"

// Helper function to clamp values to the range [0, 255].
inline uint8_t clamp(int32_t value) {
    return static_cast<uint8_t>(value < 0 ? 0 : (value > 255 ? 255 : value));
};
namespace H264Sharp {



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

        // BT.601-7 studio range constants
        const int16x8_t const_16 = vdupq_n_s16(16);
        const int16x8_t const_128 = vdupq_n_s16(128);
        const int16x8_t const_0 = vdupq_n_s16(0);
        const int16x8_t const_255 = vdupq_n_s16(255);

        // Precalculated fixed-point coefficients
        // FP scaling: 1 << 7 (128) to maintain precision
        const int16_t y_factor = 149;      // 1.164 * 128
        const int16_t v_to_r_coeff = 204;  // 1.596 * 128
        const int16_t u_to_g_coeff = 50;   // 0.391 * 128
        const int16_t v_to_g_coeff = 104;  // 0.813 * 128
        const int16_t u_to_b_coeff = 258;  // 2.018 * 128

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

                // Process U/V
                int16x8_t u_vals = vsubq_s16(vreinterpretq_s16_u16(vmovl_u8(u_vals8)), const_128);
                int16x8_t v_vals = vsubq_s16(vreinterpretq_s16_u16(vmovl_u8(v_vals8)), const_128);

                // Calculate RGB for first 8 pixels
                int16x8_t r1l = vaddq_s16(y_vals_16_1l,
                    vmulq_n_s16(v_vals, v_to_r_coeff) >> 7);
                int16x8_t g1l = vsubq_s16(vsubq_s16(y_vals_16_1l,
                    vmulq_n_s16(u_vals, u_to_g_coeff) >> 7),
                    vmulq_n_s16(v_vals, v_to_g_coeff) >> 7);
                int16x8_t b1l = vaddq_s16(y_vals_16_1l,
                    vmulq_n_s16(u_vals, u_to_b_coeff) >> 7);

                // Calculate RGB for second 8 pixels
                int16x8_t r1h = vaddq_s16(y_vals_16_1h,
                    vmulq_n_s16(v_vals, v_to_r_coeff) >> 7);
                int16x8_t g1h = vsubq_s16(vsubq_s16(y_vals_16_1h,
                    vmulq_n_s16(u_vals, u_to_g_coeff) >> 7),
                    vmulq_n_s16(v_vals, v_to_g_coeff) >> 7);
                int16x8_t b1h = vaddq_s16(y_vals_16_1h,
                    vmulq_n_s16(u_vals, u_to_b_coeff) >> 7);

                // Clamp values between 0 and 255
                r1l = vmaxq_s16(vminq_s16(r1l, const_255), const_0);
                g1l = vmaxq_s16(vminq_s16(g1l, const_255), const_0);
                b1l = vmaxq_s16(vminq_s16(b1l, const_255), const_0);

                r1h = vmaxq_s16(vminq_s16(r1h, const_255), const_0);
                g1h = vmaxq_s16(vminq_s16(g1h, const_255), const_0);
                b1h = vmaxq_s16(vminq_s16(b1h, const_255), const_0);

                // Store first row (in BGR order)
                uint8x8x3_t rgb1l, rgb1h;
                rgb1l.val[0] = vmovn_u16(vreinterpretq_u16_s16(b1l));
                rgb1l.val[1] = vmovn_u16(vreinterpretq_u16_s16(g1l));
                rgb1l.val[2] = vmovn_u16(vreinterpretq_u16_s16(r1l));

                rgb1h.val[0] = vmovn_u16(vreinterpretq_u16_s16(b1h));
                rgb1h.val[1] = vmovn_u16(vreinterpretq_u16_s16(g1h));
                rgb1h.val[2] = vmovn_u16(vreinterpretq_u16_s16(r1h));

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
                int16x8_t r2l = vaddq_s16(y_vals_16_2l,
                    vmulq_n_s16(v_vals, v_to_r_coeff) >> 7);
                int16x8_t g2l = vsubq_s16(vsubq_s16(y_vals_16_2l,
                    vmulq_n_s16(u_vals, u_to_g_coeff) >> 7),
                    vmulq_n_s16(v_vals, v_to_g_coeff) >> 7);
                int16x8_t b2l = vaddq_s16(y_vals_16_2l,
                    vmulq_n_s16(u_vals, u_to_b_coeff) >> 7);

                int16x8_t r2h = vaddq_s16(y_vals_16_2h,
                    vmulq_n_s16(v_vals, v_to_r_coeff) >> 7);
                int16x8_t g2h = vsubq_s16(vsubq_s16(y_vals_16_2h,
                    vmulq_n_s16(u_vals, u_to_g_coeff) >> 7),
                    vmulq_n_s16(v_vals, v_to_g_coeff) >> 7);
                int16x8_t b2h = vaddq_s16(y_vals_16_2h,
                    vmulq_n_s16(u_vals, u_to_b_coeff) >> 7);

                // Clamp second row values
                r2l = vmaxq_s16(vminq_s16(r2l, const_255), const_0);
                g2l = vmaxq_s16(vminq_s16(g2l, const_255), const_0);
                b2l = vmaxq_s16(vminq_s16(b2l, const_255), const_0);

                r2h = vmaxq_s16(vminq_s16(r2h, const_255), const_0);
                g2h = vmaxq_s16(vminq_s16(g2h, const_255), const_0);
                b2h = vmaxq_s16(vminq_s16(b2h, const_255), const_0);

                // Store second row (in BGR order)
                uint8x8x3_t rgb2l, rgb2h;
                rgb2l.val[0] = vmovn_u16(vreinterpretq_u16_s16(b2l));
                rgb2l.val[1] = vmovn_u16(vreinterpretq_u16_s16(g2l));
                rgb2l.val[2] = vmovn_u16(vreinterpretq_u16_s16(r2l));

                rgb2h.val[0] = vmovn_u16(vreinterpretq_u16_s16(b2h));
                rgb2h.val[1] = vmovn_u16(vreinterpretq_u16_s16(g2h));
                rgb2h.val[2] = vmovn_u16(vreinterpretq_u16_s16(r2h));

                vst3_u8(rgb_row2 + x * 3, rgb2l);
                vst3_u8(rgb_row2 + (x + 8) * 3, rgb2h);
            }
        }
    }
    


    // Convert a single YUV I420 frame to RGB.
   
//    void Yuv2Rgb::ConvertYUVToRGB_NEON(
//        const uint8_t* y_plane,
//        const uint8_t* u_plane,
//        const uint8_t* v_plane,
//        uint8_t* rgb_buffer,
//        int width,
//        int height)
//    {
//        const int uv_width = width / 2;
//        const int uv_height = height / 2;
//
//        // YUV to RGB conversion constants
//        const int16x8_t const_128 = vdupq_n_s16(128);
//        const int16x8_t const_0 = vdupq_n_s16(0);
//        const int16x8_t const_255 = vdupq_n_s16(255);
//
//        // BT.601 conversion matrix coefficients
//        // R = Y + 1.402 * (V - 128)
//        // G = Y - 0.344136 * (U - 128) - 0.714136 * (V - 128)
//        // B = Y + 1.772 * (U - 128)
//        const int16x8_t y_shift = vdupq_n_s16(0);  // No shift for Y
//        const int16_t v_to_r_coeff = 359;     // 1.402 * 256
//        const int16_t u_to_g_coeff = 88;      // 0.344136 * 256
//        const int16_t v_to_g_coeff = 183;     // 0.714136 * 256
//        const int16_t u_to_b_coeff = 454;     // 1.772 * 256
//
//        for (int y = 0; y < height; y += 2) {  // Process 2 rows at a time for UV sampling
//            const uint8_t* y_row1 = y_plane + y * width;
//            const uint8_t* y_row2 = y_row1 + width;
//            const uint8_t* u_row = u_plane + (y / 2) * uv_width;
//            const uint8_t* v_row = v_plane + (y / 2) * uv_width;
//            uint8_t* rgb_row1 = rgb_buffer + y * width * 3;
//            uint8_t* rgb_row2 = rgb_row1 + width * 3;
//
//            for (int x = 0; x < width; x += 16) {
//                // Load 16 Y values for two rows
//                uint8x16_t y_vals1 = vld1q_u8(y_row1 + x);
//                uint8x16_t y_vals2 = vld1q_u8(y_row2 + x);
//
//                // Load 8 U and V values (one U,V pair per 2x2 Y block)
//                uint8x8_t u_vals8 = vld1_u8(u_row + x / 2);
//                uint8x8_t v_vals8 = vld1_u8(v_row + x / 2);
//
//                // Convert U,V to 16-bit and subtract 128
//                int16x8_t u_vals = vsubq_s16(vreinterpretq_s16_u16(vmovl_u8(u_vals8)), const_128);
//                int16x8_t v_vals = vsubq_s16(vreinterpretq_s16_u16(vmovl_u8(v_vals8)), const_128);
//
//                // Process first row, first 8 pixels
//                int16x8_t y_vals_16_1l = vreinterpretq_s16_u16(vmovl_u8(vget_low_u8(y_vals1)));
//
//                // Calculate RGB
//                int16x8_t r1l = vaddq_s16(y_vals_16_1l,
//                    vmulq_n_s16(v_vals, v_to_r_coeff) >> 8);
//
//                int16x8_t g1l = vsubq_s16(vsubq_s16(y_vals_16_1l,
//                    vmulq_n_s16(u_vals, u_to_g_coeff) >> 8),
//                    vmulq_n_s16(v_vals, v_to_g_coeff) >> 8);
//
//                int16x8_t b1l = vaddq_s16(y_vals_16_1l,
//                    vmulq_n_s16(u_vals, u_to_b_coeff) >> 8);
//
//                // Process first row, second 8 pixels
//                int16x8_t y_vals_16_1h = vreinterpretq_s16_u16(vmovl_u8(vget_high_u8(y_vals1)));
//
//                int16x8_t r1h = vaddq_s16(y_vals_16_1h,
//                    vmulq_n_s16(v_vals, v_to_r_coeff) >> 8);
//
//                int16x8_t g1h = vsubq_s16(vsubq_s16(y_vals_16_1h,
//                    vmulq_n_s16(u_vals, u_to_g_coeff) >> 8),
//                    vmulq_n_s16(v_vals, v_to_g_coeff) >> 8);
//
//                int16x8_t b1h = vaddq_s16(y_vals_16_1h,
//                    vmulq_n_s16(u_vals, u_to_b_coeff) >> 8);
//
//                // Clamp values between 0 and 255
//                r1l = vmaxq_s16(vminq_s16(r1l, const_255), const_0);
//                g1l = vmaxq_s16(vminq_s16(g1l, const_255), const_0);
//                b1l = vmaxq_s16(vminq_s16(b1l, const_255), const_0);
//
//                r1h = vmaxq_s16(vminq_s16(r1h, const_255), const_0);
//                g1h = vmaxq_s16(vminq_s16(g1h, const_255), const_0);
//                b1h = vmaxq_s16(vminq_s16(b1h, const_255), const_0);
//
//                // Store first row
//                uint8x8x3_t rgb1l, rgb1h;
//                rgb1l.val[0] = vmovn_u16(vreinterpretq_u16_s16(b1l));
//                rgb1l.val[1] = vmovn_u16(vreinterpretq_u16_s16(g1l));
//                rgb1l.val[2] = vmovn_u16(vreinterpretq_u16_s16(r1l));
//
//                rgb1h.val[0] = vmovn_u16(vreinterpretq_u16_s16(b1h));
//                rgb1h.val[1] = vmovn_u16(vreinterpretq_u16_s16(g1h));
//                rgb1h.val[2] = vmovn_u16(vreinterpretq_u16_s16(r1h));
//
//                vst3_u8(rgb_row1 + x * 3, rgb1l);
//                vst3_u8(rgb_row1 + (x + 8) * 3, rgb1h);
//
//                // Process second row (same U,V values due to YUV420 format)
//                int16x8_t y_vals_16_2l = vreinterpretq_s16_u16(vmovl_u8(vget_low_u8(y_vals2)));
//                int16x8_t y_vals_16_2h = vreinterpretq_s16_u16(vmovl_u8(vget_high_u8(y_vals2)));
//
//                // Calculate and store second row (similar to first row)
//                int16x8_t r2l = vaddq_s16(y_vals_16_2l,
//                    vmulq_n_s16(v_vals, v_to_r_coeff) >> 8);
//                int16x8_t g2l = vsubq_s16(vsubq_s16(y_vals_16_2l,
//                    vmulq_n_s16(u_vals, u_to_g_coeff) >> 8),
//                    vmulq_n_s16(v_vals, v_to_g_coeff) >> 8);
//                int16x8_t b2l = vaddq_s16(y_vals_16_2l,
//                    vmulq_n_s16(u_vals, u_to_b_coeff) >> 8);
//
//                int16x8_t r2h = vaddq_s16(y_vals_16_2h,
//                    vmulq_n_s16(v_vals, v_to_r_coeff) >> 8);
//                int16x8_t g2h = vsubq_s16(vsubq_s16(y_vals_16_2h,
//                    vmulq_n_s16(u_vals, u_to_g_coeff) >> 8),
//                    vmulq_n_s16(v_vals, v_to_g_coeff) >> 8);
//                int16x8_t b2h = vaddq_s16(y_vals_16_2h,
//                    vmulq_n_s16(u_vals, u_to_b_coeff) >> 8);
//
//                // Clamp second row values
//                r2l = vmaxq_s16(vminq_s16(r2l, const_255), const_0);
//                g2l = vmaxq_s16(vminq_s16(g2l, const_255), const_0);
//                b2l = vmaxq_s16(vminq_s16(b2l, const_255), const_0);
//
//                r2h = vmaxq_s16(vminq_s16(r2h, const_255), const_0);
//                g2h = vmaxq_s16(vminq_s16(g2h, const_255), const_0);
//                b2h = vmaxq_s16(vminq_s16(b2h, const_255), const_0);
//
//                // Store second row
//                uint8x8x3_t rgb2l, rgb2h;
//                rgb2l.val[0] = vmovn_u16(vreinterpretq_u16_s16(b2l));
//                rgb2l.val[1] = vmovn_u16(vreinterpretq_u16_s16(g2l));
//                rgb2l.val[2] = vmovn_u16(vreinterpretq_u16_s16(r2l));
//
//                rgb2h.val[0] = vmovn_u16(vreinterpretq_u16_s16(b2h));
//                rgb2h.val[1] = vmovn_u16(vreinterpretq_u16_s16(g2h));
//                rgb2h.val[2] = vmovn_u16(vreinterpretq_u16_s16(r2h));
//
//                vst3_u8(rgb_row2 + x * 3, rgb2l);
//                vst3_u8(rgb_row2 + (x + 8) * 3, rgb2h);
//            }
//        }
//    }
}
#endif // #if defined(__aarch64__)

