#include <immintrin.h>
#include <stdint.h>

namespace H264Sharp
{
    inline void Store(__m256i r1, __m256i g1, __m256i b1, uint8_t* dst);
     void ConvertYUVToRGB_AVX2_Body(
        const uint8_t* y_plane,
        const uint8_t* u_plane,
        const uint8_t* v_plane,
        uint8_t* rgb_buffer,
        int width,
        int begin,
        int end) {

        const int uv_width = width / 2;
        const __m256i const_128 = _mm256_set1_epi16(128);
        const __m256i const_16 = _mm256_set1_epi16(16);
        const __m256i y_factor = _mm256_set1_epi16(149); // 1.164 * 128
        const __m256i u_to_g_coeff = _mm256_set1_epi16(25);  // 0.391 * 64
        const __m256i v_to_r_coeff = _mm256_set1_epi16(102); // 1.596 * 64
        const __m256i v_to_g_coeff = _mm256_set1_epi16(52);  // 0.813 * 64
        const __m256i u_to_b_coeff = _mm256_set1_epi16(129); // 2.018 * 64

        for (int y = begin; y < end; y += 2) {
            const uint8_t* y_row1 = y_plane + y * width;
            const uint8_t* y_row2 = y_row1 + width;
            const uint8_t* u_row = u_plane + (y / 2) * uv_width;
            const uint8_t* v_row = v_plane + (y / 2) * uv_width;
            uint8_t* rgb_row1 = rgb_buffer + y * width * 3;
            uint8_t* rgb_row2 = rgb_row1 + width * 3;

            for (int x = 0; x < width; x += 32) {
                // Load 16 U and V values
                __m128i u_vals8 = _mm_loadu_si128((__m128i*)(u_row + x / 2));
                __m128i v_vals8 = _mm_loadu_si128((__m128i*)(v_row + x / 2));

                // Expand U and V to 16-bit and subtract 128
                __m256i u_vals = _mm256_sub_epi16(_mm256_cvtepu8_epi16(u_vals8), const_128);
                __m256i v_vals = _mm256_sub_epi16(_mm256_cvtepu8_epi16(v_vals8), const_128);

                // Duplicate U and V values
                __m256i u_vals_low = _mm256_unpacklo_epi16(u_vals, u_vals);
                __m256i u_vals_high = _mm256_unpackhi_epi16(u_vals, u_vals);
                __m256i v_vals_low = _mm256_unpacklo_epi16(v_vals, v_vals);
                __m256i v_vals_high = _mm256_unpackhi_epi16(v_vals, v_vals);

                // Calculate UV contributions
                __m256i u_g_low = _mm256_srai_epi16(_mm256_mullo_epi16(u_vals_low, u_to_g_coeff), 6);
                __m256i u_b_low = _mm256_srai_epi16(_mm256_mullo_epi16(u_vals_low, u_to_b_coeff), 6);
                __m256i v_g_low = _mm256_srai_epi16(_mm256_mullo_epi16(v_vals_low, v_to_g_coeff), 6);
                __m256i v_r_low = _mm256_srai_epi16(_mm256_mullo_epi16(v_vals_low, v_to_r_coeff), 6);

                __m256i u_g_high = _mm256_srai_epi16(_mm256_mullo_epi16(u_vals_high, u_to_g_coeff), 6);
                __m256i u_b_high = _mm256_srai_epi16(_mm256_mullo_epi16(u_vals_high, u_to_b_coeff), 6);
                __m256i v_g_high = _mm256_srai_epi16(_mm256_mullo_epi16(v_vals_high, v_to_g_coeff), 6);
                __m256i v_r_high = _mm256_srai_epi16(_mm256_mullo_epi16(v_vals_high, v_to_r_coeff), 6);

                // Load 32 Y values each row
                __m256i y_vals1 = _mm256_loadu_si256((__m256i*)y_row1 + x);
                __m256i y_vals2 = _mm256_loadu_si256((__m256i*)y_row2 + x);

                y_vals1 = _mm256_subs_epu8(y_vals1, const_16);
                y_vals2 = _mm256_subs_epu8(y_vals2, const_16);

                __m256i y00 = _mm256_srai_epi16(_mm256_mulhi_epu16(_mm256_unpacklo_epi8(__m256i(), y_vals1), y_factor), 7);
                __m256i y01 = _mm256_srai_epi16(_mm256_mulhi_epu16(_mm256_unpackhi_epi8(__m256i(), y_vals1), y_factor), 7);
                __m256i y10 = _mm256_srai_epi16(_mm256_mulhi_epu16(_mm256_unpacklo_epi8(__m256i(), y_vals2), y_factor), 7);
                __m256i y11 = _mm256_srai_epi16(_mm256_mulhi_epu16(_mm256_unpackhi_epi8(__m256i(), y_vals2), y_factor), 7);


                // Calculate RGB for first row
                __m256i r1 = _mm256_add_epi16(y00, v_r_low);
                __m256i g1 = _mm256_sub_epi16(_mm256_sub_epi16(y00, u_g_low), v_g_low);
                __m256i b1 = _mm256_add_epi16(y00, u_b_low);

               
                __m256i r2 = _mm256_add_epi16(y01, v_r_high);
                __m256i g2 = _mm256_sub_epi16(_mm256_sub_epi16(y01, u_g_high), v_g_high);
                __m256i b2 = _mm256_add_epi16(y01, u_b_high);

                r1 = _mm256_packus_epi16(r1, r2);
                g1 = _mm256_packus_epi16(g1, g2);
                b1 = _mm256_packus_epi16(b1, b2);

                Store(r1, g1, b1, (rgb_row1 + x * 3));


                // Calculate RGB for second row
                r1 = _mm256_add_epi16(y10, v_r_low);
                g1 = _mm256_sub_epi16(_mm256_sub_epi16(y10, u_g_low), v_g_low);
                b1 = _mm256_add_epi16(y10, u_b_low);

                r2 = _mm256_add_epi16(y11, v_r_high);
                g2 = _mm256_sub_epi16(_mm256_sub_epi16(y11, u_g_high), v_g_high);
                b2 = _mm256_add_epi16(y11, u_b_high);

                r1 = _mm256_packus_epi16(r1, r2);
                g1 = _mm256_packus_epi16(g1, g2);
                b1 = _mm256_packus_epi16(b1, b2);

                Store(r1, g1, b1, (rgb_row2 + x * 3));

                //// Pack and clamp to 8-bit values
                //__m256i rgb1 = _mm256_packus_epi16(r1, g1);
                //__m256i rgb2 = _mm256_packus_epi16(b1, _mm256_setzero_si256());
                //__m256i rgb3 = _mm256_packus_epi16(r2, g2);
                //__m256i rgb4 = _mm256_packus_epi16(b2, _mm256_setzero_si256());

                //// Store the results (in BGR order)
                //_mm256_storeu_si256((__m256i*)(rgb_row1 + x * 3), rgb1);
                //_mm256_storeu_si256((__m256i*)(rgb_row1 + x * 3 + 32), rgb2);
                //_mm256_storeu_si256((__m256i*)(rgb_row2 + x * 3), rgb3);
                //_mm256_storeu_si256((__m256i*)(rgb_row2 + x * 3 + 32), rgb4);
            }
        }
    }
    inline void Store(__m256i r1, __m256i g1, __m256i b1, uint8_t* dst)
    {
        uint8_t rd[32];
        uint8_t gd[32];
        uint8_t bd[32];


        _mm256_storeu_si256((__m256i*)rd, r1);
        _mm256_storeu_si256((__m256i*)gd, g1);
        _mm256_storeu_si256((__m256i*)bd, b1);

        int j = 0;
#pragma clang loop vectorize(assume_safety)

        for (int i = 0; i < 32; i++)
        {
            j = i * 3;
            dst[j] = rd[i];
            dst[j + 1] = gd[i];
            dst[j + 2] = bd[i];
        }

    }
}

