#include "Yuv2Rgb.h"

#if defined(__arm__)

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
    const int8x8_t alpha = vdup_n_u8(255);
    const int16x8_t const_16 = vdupq_n_s16(16);
    const uint8x16_t const_16_8 = vdupq_n_u8(16);
    const int16x8_t const_128 = vdupq_n_s16(128);
  
    const auto y_factor = vdupq_n_u16(149);      // 1.164 * 64
    const auto v_to_r_coeff = vdupq_n_s16(102);  // 1.596 * 64
    const auto u_to_g_coeff = vdupq_n_s16(25);   // 0.391 * 64
    const auto v_to_g_coeff = vdupq_n_s16(52);  // 0.813 * 64
    const auto u_to_b_coeff = vdupq_n_s16(129);  // 2.018 * 64

    inline void  Convert(uint8x16_t y_vals1, uint8x16_t y_vals2, int16x8_t u_valsl, int16x8_t u_valsh, int16x8_t v_valsl, int16x8_t v_valsh,
        uint8x16_t& r1l, uint8x16_t& g1l, uint8x16_t& b1l, uint8x16_t& r1h, uint8x16_t& g1h, uint8x16_t& b1h);
        
    template<int NUM_CH, bool RGB>
    inline void ConvertYUVToRGB_NEON_Body(
        const uint8_t* RESTRICT y_plane,
        const uint8_t* RESTRICT u_plane,
        const uint8_t* RESTRICT v_plane,
        int32_t y_stride,
        int32_t uv_stride,
        uint8_t* RESTRICT rgb_buffer,
        int32_t width,
        int32_t begin,
        int32_t end)
    {
        int ridx, gidx, bidx;
        if constexpr (RGB)
        {
            ridx = 0; gidx = 1; bidx = 2;
        }
        else
        {
            ridx = 2; gidx = 1; bidx = 0;
        }

        for (int y = begin; y < end; y += 2) 
        {
            const uint8_t* RESTRICT y_row1 = y_plane + y * y_stride;
            const uint8_t* RESTRICT y_row2 = y_row1 + y_stride;
            const uint8_t* RESTRICT u_row = u_plane + (y / 2) * uv_stride;
            const uint8_t* RESTRICT v_row = v_plane + (y / 2) * uv_stride;
            uint8_t* RESTRICT rgb_row1 = rgb_buffer + y * width * NUM_CH;
            uint8_t* RESTRICT rgb_row2 = rgb_row1 + width * NUM_CH;

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

                // duplicate [1,2,3,4,5,6,7,8]=> [1,1,2,2,3,3,4,4] , [5,5,6,6,7,7,8,8]
                int16x8_t u_valsl = vzip1q_s16(u_vals, u_vals);
                int16x8_t u_valsh = vzip2q_s16(u_vals, u_vals);

                int16x8_t v_valsl = vzip1q_s16(v_vals, v_vals);
                int16x8_t v_valsh = vzip2q_s16(v_vals, v_vals);

                uint8x16_t r1l, g1l, b1l, r1h, g1h, b1h;
                Convert(y_vals1, y_vals2, u_valsl, u_valsh, v_valsl, v_valsh,
                    r1l, g1l, b1l, r1h, g1h, b1h);

                
                if constexpr (NUM_CH < 4) {
                    uint8x16x3_t rgb1, rgb2;
                    rgb1.val[ridx] = b1l;
                    rgb1.val[gidx] = g1l;
                    rgb1.val[bidx] = r1l;
                    vst3q_u8(rgb_row1 + x * 3, rgb1);

                    rgb2.val[ridx] = b1h;
                    rgb2.val[gidx] = g1h;
                    rgb2.val[bidx] = r1h;
                    vst3q_u8(rgb_row2 + x * 3, rgb2);
                }
                else {
                    const int8x16_t alpha = vdupq_n_u8(255);

                    uint8x16x4_t rgb1, rgb2;
                    rgb1.val[ridx] = b1l;
                    rgb1.val[gidx] = g1l;
                    rgb1.val[bidx] = r1l;
                    rgb1.val[3] = alpha;
                    vst4q_u8(rgb_row1 + x * 4, rgb1);

                    rgb2.val[ridx] = b1h;
                    rgb2.val[gidx] = g1h;
                    rgb2.val[bidx] = r1h;
                    rgb2.val[3] = alpha;
                    vst4q_u8(rgb_row2 + x * 4, rgb2);
                }

            }
        }
    }

    template<int NUM_CH, bool RGB>
    inline void ConvertYUVNV12toRGB_NEON_Body(
        const uint8_t* RESTRICT y_plane,
        const uint8_t* RESTRICT uv_plane,
        int32_t y_stride,
        int32_t uv_stride,
        uint8_t* RESTRICT rgb_buffer,
        int32_t width,
        int32_t begin,
        int32_t end)
    {
        int ridx, gidx, bidx;
        if constexpr (RGB)
        {
            ridx = 0; gidx = 1; bidx = 2;
        }
        else
        {
            ridx = 2; gidx = 1; bidx = 0;
        }

        for (int y = begin; y < end; y += 2)
        {
            const uint8_t* RESTRICT y_row1 = y_plane + y * y_stride;
            const uint8_t* RESTRICT y_row2 = y_row1 + y_stride;
            const uint8_t* RESTRICT uv_row = uv_plane + (y / 2) * uv_stride;
            uint8_t* RESTRICT rgb_row1 = rgb_buffer + y * width * NUM_CH;
            uint8_t* RESTRICT rgb_row2 = rgb_row1 + width * NUM_CH;

            for (int x = 0; x < width; x += 16) 
            {

                uint8x8x2_t uv = vld2_u8(uv_row + x);
                uint8x8_t u_vals8 = uv.val[0];
                uint8x8_t v_vals8 = uv.val[1];

                // Load 16 Y values for two rows and -16
                uint8x16_t y_vals1 = vqsubq_u8(vld1q_u8(y_row1 + x), const_16_8);
                uint8x16_t y_vals2 = vqsubq_u8(vld1q_u8(y_row2 + x), const_16_8);

                // Process U/V (widen then -128)
                int16x8_t u_vals = vsubq_s16(vreinterpretq_s16_u16(vmovl_u8(u_vals8)), const_128);
                int16x8_t v_vals = vsubq_s16(vreinterpretq_s16_u16(vmovl_u8(v_vals8)), const_128);

                // duplicate [1,2,3,4,5,6,7,8]=> [1,1,2,2,3,3,4,4] , [5,5,6,6,7,7,8,8]
                int16x8_t u_valsl = vzip1q_s16(u_vals, u_vals);
                int16x8_t u_valsh = vzip2q_s16(u_vals, u_vals);

                int16x8_t v_valsl = vzip1q_s16(v_vals, v_vals);
                int16x8_t v_valsh = vzip2q_s16(v_vals, v_vals);

                uint8x16_t r1l, g1l, b1l, r1h, g1h, b1h;
                Convert(y_vals1, y_vals2, u_valsl, u_valsh, v_valsl, v_valsh,
                    r1l, g1l, b1l, r1h, g1h, b1h);

                
                if constexpr (NUM_CH < 4) {
                    uint8x16x3_t rgb1, rgb2;
                    rgb1.val[ridx] = b1l;
                    rgb1.val[gidx] = g1l;
                    rgb1.val[bidx] = r1l;
                    vst3q_u8(rgb_row1 + x * 3, rgb1);

                    rgb2.val[ridx] = b1h;
                    rgb2.val[gidx] = g1h;
                    rgb2.val[bidx] = r1h;
                    vst3q_u8(rgb_row2 + x * 3, rgb2);
                }
                else {
                    const int8x16_t alpha = vdupq_n_u8(255);

                    uint8x16x4_t rgb1, rgb2;
                    rgb1.val[ridx] = b1l;
                    rgb1.val[gidx] = g1l;
                    rgb1.val[bidx] = r1l;
                    rgb1.val[3] = alpha;
                    vst4q_u8(rgb_row1 + x * 4, rgb1);

                    rgb2.val[ridx] = b1h;
                    rgb2.val[gidx] = g1h;
                    rgb2.val[bidx] = r1h;
                    rgb2.val[3] = alpha;
                    vst4q_u8(rgb_row2 + x * 4, rgb2);
                }

            }
        }
    }

    inline void Convert(uint8x16_t y_vals1, uint8x16_t y_vals2, int16x8_t u_valsl, int16x8_t u_valsh, int16x8_t v_valsl, int16x8_t v_valsh,
        uint8x16_t& r1l, uint8x16_t& g1l, uint8x16_t& b1l, uint8x16_t& r1h, uint8x16_t& g1h, uint8x16_t& b1h)
    {
        // multiply UV with the scaling
        int16x8_t u_vals_ugl = vshrq_n_s16(vmulq_s16(u_valsl, u_to_g_coeff), 6);
        int16x8_t u_vals_ubl = vshrq_n_s16(vmulq_s16(u_valsl, u_to_b_coeff), 6);
        int16x8_t v_vals_vgl = vshrq_n_s16(vmulq_s16(v_valsl, v_to_g_coeff), 6);
        int16x8_t v_vals_vrl = vshrq_n_s16(vmulq_s16(v_valsl, v_to_r_coeff), 6);

        int16x8_t u_vals_ugh = vshrq_n_s16(vmulq_s16(u_valsh, u_to_g_coeff), 6);
        int16x8_t u_vals_ubh = vshrq_n_s16(vmulq_s16(u_valsh, u_to_b_coeff), 6);
        int16x8_t v_vals_vgh = vshrq_n_s16(vmulq_s16(v_valsh, v_to_g_coeff), 6);
        int16x8_t v_vals_vrh = vshrq_n_s16(vmulq_s16(v_valsh, v_to_r_coeff), 6);


        // Convert Y to 16-bit and scale
        uint16x8_t y_vals_16_1lu = (vmovl_u8(vget_low_u8(y_vals1)));
        uint16x8_t y_vals_16_1hu = (vmovl_u8(vget_high_u8(y_vals1)));

        uint16x8_t y_vals_16_2lu = (vmovl_u8(vget_low_u8(y_vals2)));
        uint16x8_t y_vals_16_2hu = (vmovl_u8(vget_high_u8(y_vals2)));

        int16x8_t y_vals_16_1h = vreinterpretq_s16_u16(vshrq_n_u16(vmulq_u16(y_vals_16_1hu, y_factor), 7));
        int16x8_t y_vals_16_2h = vreinterpretq_s16_u16(vshrq_n_u16(vmulq_u16(y_vals_16_2hu, y_factor), 7));

        int16x8_t y_vals_16_1l = vreinterpretq_s16_u16(vshrq_n_u16(vmulq_u16(y_vals_16_1lu, y_factor), 7));
        int16x8_t y_vals_16_2l = vreinterpretq_s16_u16(vshrq_n_u16(vmulq_u16(y_vals_16_2lu, y_factor), 7));

        int16x8_t r1l_ = vaddq_s16(y_vals_16_1l, v_vals_vrl);
        int16x8_t g1l_ = vsubq_s16(vsubq_s16(y_vals_16_1l, u_vals_ugl), v_vals_vgl);
        int16x8_t b1l_ = vaddq_s16(y_vals_16_1l, u_vals_ubl);

        int16x8_t r1h_ = vaddq_s16(y_vals_16_1h, v_vals_vrh);
        int16x8_t g1h_ = vsubq_s16(vsubq_s16(y_vals_16_1h, u_vals_ugh), v_vals_vgh);
        int16x8_t b1h_ = vaddq_s16(y_vals_16_1h, u_vals_ubh);

        b1l = vcombine_u8(vqmovun_s16(b1l_), vqmovun_s16(b1h_));
        g1l = vcombine_u8(vqmovun_s16(g1l_), vqmovun_s16(g1h_));
        r1l = vcombine_u8(vqmovun_s16(r1l_), vqmovun_s16(r1h_));


        int16x8_t r2l_ = vaddq_s16(y_vals_16_2l, v_vals_vrl);
        int16x8_t g2l_ = vsubq_s16(vsubq_s16(y_vals_16_2l, u_vals_ugl), v_vals_vgl);
        int16x8_t b2l_ = vaddq_s16(y_vals_16_2l, u_vals_ubl);

        int16x8_t r2h_ = vaddq_s16(y_vals_16_2h, v_vals_vrh);
        int16x8_t g2h_ = vsubq_s16(vsubq_s16(y_vals_16_2h, u_vals_ugh), v_vals_vgh);
        int16x8_t b2h_ = vaddq_s16(y_vals_16_2h, u_vals_ubh);

       
        b1h = vcombine_u8(vqmovun_s16(b2l_), vqmovun_s16(b2h_));
        g1h = vcombine_u8(vqmovun_s16(g2l_), vqmovun_s16(g2h_));
        r1h = vcombine_u8(vqmovun_s16(r2l_), vqmovun_s16(r2h_));



    }

    template<int NUM_CH, bool RGB>
    inline void Yuv2Rgb::ConvertYUVToRGB_NEON(const uint8_t* RESTRICT y_plane,
        const uint8_t* RESTRICT u_plane,
        const uint8_t* RESTRICT v_plane,
        int32_t Y_stride,
        int32_t UV_stride,
        uint8_t* RESTRICT rgb_buffer,
        int32_t width,
        int32_t heigth,
        int32_t numThreads)
    {
		//std::cout << "ConvertYUVToRGB_NEON - " << NUM_CH << "ISRGB "<<RGB << std::endl;
        if(numThreads <2)
            ConvertYUVToRGB_NEON_Body<NUM_CH,RGB >(y_plane, u_plane, v_plane, Y_stride, UV_stride, rgb_buffer, width, 0, heigth);
        else
        {
            if (Rgb2Yuv::useLoadBalancer > 0)
            {
                ThreadPool::For2(int(0), height, [&](int begin, int end)
                    {
                        ConvertYUVToRGB_NEON_Body<NUM_CH, RGB >(y_plane, u_plane, v_plane, Y_stride, UV_stride, rgb_buffer, width, begin, end);
                    });
            }
            else
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

                        ConvertYUVToRGB_NEON_Body<NUM_CH, RGB >(y_plane, u_plane, v_plane, Y_stride, UV_stride, rgb_buffer, width, bgn, end);

                    });
            }
        }
    }

    template<int NUM_CH, bool RGB>
    inline void Yuv2Rgb::ConvertYUVNV12ToRGB_NEON(const uint8_t* RESTRICT y_plane,
        const uint8_t* RESTRICT uv_plane,
        int32_t Y_stride,
        int32_t UV_stride,
        uint8_t* RESTRICT rgb_buffer,
        int32_t width,
        int32_t heigth,
        int32_t numThreads)
    {
        if (numThreads < 2)
            ConvertYUVNV12toRGB_NEON_Body<NUM_CH, RGB >(y_plane, uv_plane, Y_stride, UV_stride, rgb_buffer, width, 0, heigth);
        else
        {
            if (Rgb2Yuv::useLoadBalancer > 0)
            {
                ThreadPool::For2(int(0), height, [&](int begin, int end)
                    {
                        ConvertYUVNV12toRGB_NEON_Body<NUM_CH, RGB >(y_plane, uv_plane, Y_stride, UV_stride, rgb_buffer, width, begin, end);
                    });
            }
            else
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

                        ConvertYUVNV12toRGB_NEON_Body<NUM_CH, RGB >(y_plane, uv_plane, Y_stride, UV_stride, rgb_buffer, width, bgn, end);

                    });
            }
            }
            
    }
    //explicit inst
    template void Yuv2Rgb::ConvertYUVToRGB_NEON<3, true>(const uint8_t* RESTRICT y_plane,
        const uint8_t* RESTRICT u_plane,
        const uint8_t* RESTRICT v_plane,
        int32_t y_stride,
        int32_t uv_stride,
        uint8_t* RESTRICT rgb_buffer,
        int32_t width,
        int32_t begin,
        int32_t end);

    template void Yuv2Rgb::ConvertYUVToRGB_NEON< 4, true>(const uint8_t* RESTRICT y_plane,
        const uint8_t* RESTRICT u_plane,
        const uint8_t* RESTRICT v_plane,
        int32_t y_stride,
        int32_t uv_stride,
        uint8_t* RESTRICT rgb_buffer,
        int32_t width,
        int32_t begin,
        int32_t end);

    template void Yuv2Rgb::ConvertYUVToRGB_NEON<3, false>(const uint8_t* RESTRICT y_plane,
        const uint8_t* RESTRICT u_plane,
        const uint8_t* RESTRICT v_plane,
        int32_t y_stride,
        int32_t uv_stride,
        uint8_t* RESTRICT rgb_buffer,
        int32_t width,
        int32_t begin,
        int32_t end);

    template void Yuv2Rgb::ConvertYUVToRGB_NEON<4, false>(const uint8_t* RESTRICT y_plane,
        const uint8_t* RESTRICT u_plane,
        const uint8_t* RESTRICT v_plane,
        int32_t y_stride,
        int32_t uv_stride,
        uint8_t* RESTRICT rgb_buffer,
        int32_t width,
        int32_t begin,
        int32_t end);

    //--

    template void Yuv2Rgb::ConvertYUVNV12ToRGB_NEON<3, true>(const uint8_t* RESTRICT y_plane,
        const uint8_t* RESTRICT uv_plane,
        int32_t y_stride,
        int32_t uv_stride,
        uint8_t* RESTRICT rgb_buffer,
        int32_t width,
        int32_t begin,
        int32_t end);

    template void Yuv2Rgb::ConvertYUVNV12ToRGB_NEON< 4, true>(const uint8_t* RESTRICT y_plane,
        const uint8_t* RESTRICT uv_plane,
        int32_t y_stride,
        int32_t uv_stride,
        uint8_t* RESTRICT rgb_buffer,
        int32_t width,
        int32_t begin,
        int32_t end);

    template void Yuv2Rgb::ConvertYUVNV12ToRGB_NEON<3, false>(const uint8_t* RESTRICT y_plane,
        const uint8_t* RESTRICT uv_plane,
        int32_t y_stride,
        int32_t uv_stride,
        uint8_t* RESTRICT rgb_buffer,
        int32_t width,
        int32_t begin,
        int32_t end);

    template void Yuv2Rgb::ConvertYUVNV12ToRGB_NEON<4, false>(const uint8_t* RESTRICT y_plane,
        const uint8_t* RESTRICT uv_plane,
        int32_t y_stride,
        int32_t uv_stride,
        uint8_t* RESTRICT rgb_buffer,
        int32_t width,
        int32_t begin,
        int32_t end);
} 
#endif 

