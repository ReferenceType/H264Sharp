#ifndef __arm__
#include <immintrin.h>
#include <stdint.h>
#include "Yuv2Rgb.h"

namespace H264Sharp
{
	void ConvertYUVToRGB_AVX2_Body(
		const uint8_t* RESTRICT y_plane,
		const uint8_t* RESTRICT u_plane,
		const uint8_t* RESTRICT v_plane,
		uint8_t* RESTRICT rgb_buffer,
		int width,
		int stride,
		int begin,
		int end);

	void Yuv2Rgb::ConvertYUVToRGB_AVX2(
		const uint8_t* RESTRICT y_plane,
		const uint8_t* RESTRICT u_plane,
		const uint8_t* RESTRICT v_plane,
		uint8_t* RESTRICT rgb_buffer,
		int width,
		int stride,
		int height,
		int numThreads)
	{
		if (numThreads > 1)
		{

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

					ConvertYUVToRGB_AVX2_Body(y_plane, u_plane, v_plane, rgb_buffer, width,stride, bgn, end);

				});
		}
		else
		{
			ConvertYUVToRGB_AVX2_Body(y_plane, u_plane, v_plane, rgb_buffer, width,stride, 0, height);
		}
	}

	inline void Store(__m256i r1, __m256i g1, __m256i b1, uint8_t* dst);

	inline void Upscale(__m256i u_vals, __m256i& low, __m256i& high);

	const __m256i const_16 = _mm256_set1_epi16(16);
	const __m256i const_128 = _mm256_set1_epi16(128);

	const __m256i y_factor_vec = _mm256_set1_epi16(149);      // 1.164 * 128
	const __m256i v_to_r_coeff_vec = _mm256_set1_epi16(102);  // 1.596 * 64
	const __m256i u_to_g_coeff_vec = _mm256_set1_epi16(25);   // 0.391 * 64
	const __m256i v_to_g_coeff_vec = _mm256_set1_epi16(52);   // 0.813 * 64
	const __m256i u_to_b_coeff_vec = _mm256_set1_epi16(129);  // 2.018 * 64

	void ConvertYUVToRGB_AVX2_Body(
		const uint8_t* RESTRICT y_plane,
		const uint8_t* RESTRICT u_plane,
		const uint8_t* RESTRICT v_plane,
		uint8_t* RESTRICT rgb_buffer,
		int width,
		int stride,
		int begin,
		int end) {

		for (int y = begin; y < end; y += 2) {
			const uint8_t* y_row1 = y_plane + y * stride;
			const uint8_t* y_row2 = y_row1 + stride;
			const uint8_t* u_row = u_plane + (y / 2) * (stride / 2);
			const uint8_t* v_row = v_plane + (y / 2) * (stride / 2);
			uint8_t* rgb_row1 = rgb_buffer + y * width * 3;
			uint8_t* rgb_row2 = rgb_row1 + width * 3;

			for (int x = 0; x < width; x += 32) {
				// Load 16 U and V values (subsampled)
				__m128i u_vals8 = _mm_loadu_si128((__m128i*)(u_row + (x / 2)));
				__m128i v_vals8 = _mm_loadu_si128((__m128i*)(v_row + (x / 2)));


				/*__m128i u_vals8 = _mm_set1_epi8(125);
				__m128i v_vals8 = _mm_set1_epi8(125);*/

				//// Widen U and V to 16-bit and subtract 128
				//__m256i u_vals = _mm256_sub_epi16(_mm256_cvtepu8_epi16(u_vals8), const_128);
				//__m256i v_vals = _mm256_sub_epi16(_mm256_cvtepu8_epi16(v_vals8), const_128);

				//// Duplicate U and V values to match Y resolution
				//__m256i u_valsl = _mm256_unpacklo_epi16(u_vals, u_vals);
				//__m256i u_valsh = _mm256_unpackhi_epi16(u_vals, u_vals);
				//__m256i v_valsl = _mm256_unpacklo_epi16(v_vals, v_vals);
				//__m256i v_valsh = _mm256_unpackhi_epi16(v_vals, v_vals);

				__m256i u_vals = _mm256_sub_epi16(_mm256_cvtepu8_epi16(u_vals8), const_128);
				__m256i v_vals = _mm256_sub_epi16(_mm256_cvtepu8_epi16(v_vals8), const_128);

				__m256i u_valsl, u_valsh;
				Upscale(u_vals, u_valsl, u_valsh);

				__m256i v_valsl, v_valsh;
				Upscale(v_vals, v_valsl, v_valsh);

				// Multiply UV with scaling coefficients
				__m256i u_vals_ugl = _mm256_srai_epi16(_mm256_mullo_epi16(u_valsl, u_to_g_coeff_vec), 6);
				__m256i u_vals_ubl = _mm256_srai_epi16(_mm256_mullo_epi16(u_valsl, u_to_b_coeff_vec), 6);
				__m256i v_vals_vgl = _mm256_srai_epi16(_mm256_mullo_epi16(v_valsl, v_to_g_coeff_vec), 6);
				__m256i v_vals_vrl = _mm256_srai_epi16(_mm256_mullo_epi16(v_valsl, v_to_r_coeff_vec), 6);

				__m256i u_vals_ugh = _mm256_srai_epi16(_mm256_mullo_epi16(u_valsh, u_to_g_coeff_vec), 6);
				__m256i u_vals_ubh = _mm256_srai_epi16(_mm256_mullo_epi16(u_valsh, u_to_b_coeff_vec), 6);
				__m256i v_vals_vgh = _mm256_srai_epi16(_mm256_mullo_epi16(v_valsh, v_to_g_coeff_vec), 6);
				__m256i v_vals_vrh = _mm256_srai_epi16(_mm256_mullo_epi16(v_valsh, v_to_r_coeff_vec), 6);

				// Load 32 Y values for two rows
				__m256i y_vals1 = _mm256_loadu_si256((__m256i*)(y_row1 + x));
				__m256i y_vals2 = _mm256_loadu_si256((__m256i*)(y_row2 + x));

				// Convert Y to 16-bit and adjust range
				__m256i y_vals_16_1l = _mm256_sub_epi16(_mm256_cvtepu8_epi16(_mm256_castsi256_si128(y_vals1)), const_16);
				__m256i y_vals_16_1h = _mm256_sub_epi16(_mm256_cvtepu8_epi16(_mm256_extracti128_si256(y_vals1, 1)), const_16);
				__m256i y_vals_16_2l = _mm256_sub_epi16(_mm256_cvtepu8_epi16(_mm256_castsi256_si128(y_vals2)), const_16);
				__m256i y_vals_16_2h = _mm256_sub_epi16(_mm256_cvtepu8_epi16(_mm256_extracti128_si256(y_vals2, 1)), const_16);

				// Scale Y (-16 and multiply by 1.164)
				y_vals_16_1l = _mm256_srai_epi16(_mm256_mullo_epi16(y_vals_16_1l, y_factor_vec), 7);
				y_vals_16_1h = _mm256_srai_epi16(_mm256_mullo_epi16(y_vals_16_1h, y_factor_vec), 7);
				y_vals_16_2l = _mm256_srai_epi16(_mm256_mullo_epi16(y_vals_16_2l, y_factor_vec), 7);
				y_vals_16_2h = _mm256_srai_epi16(_mm256_mullo_epi16(y_vals_16_2h, y_factor_vec), 7);

				// Calculate RGB for first 16 pixels
				__m256i r1l = _mm256_add_epi16(y_vals_16_1l, v_vals_vrl);
				__m256i g1l = _mm256_sub_epi16(_mm256_sub_epi16(y_vals_16_1l, u_vals_ugl), v_vals_vgl);
				__m256i b1l = _mm256_add_epi16(y_vals_16_1l, u_vals_ubl);

				__m256i r1h = _mm256_add_epi16(y_vals_16_1h, v_vals_vrh);
				__m256i g1h = _mm256_sub_epi16(_mm256_sub_epi16(y_vals_16_1h, u_vals_ugh), v_vals_vgh);
				__m256i b1h = _mm256_add_epi16(y_vals_16_1h, u_vals_ubh);

				__m256i r = _mm256_packus_epi16(r1l, r1h);
				__m256i g = _mm256_packus_epi16(g1l, g1h);
				__m256i b = _mm256_packus_epi16(b1l, b1h);

				r = _mm256_permute4x64_epi64(r, _MM_SHUFFLE(3, 1, 2, 0));
				g = _mm256_permute4x64_epi64(g, _MM_SHUFFLE(3, 1, 2, 0));
				b = _mm256_permute4x64_epi64(b, _MM_SHUFFLE(3, 1, 2, 0));

				Store(r, g, b, (rgb_row1 + x * 3));


				// Calculate RGB for second row
				__m256i r2l = _mm256_add_epi16(y_vals_16_2l, v_vals_vrl);
				__m256i g2l = _mm256_sub_epi16(_mm256_sub_epi16(y_vals_16_2l, u_vals_ugl), v_vals_vgl);
				__m256i b2l = _mm256_add_epi16(y_vals_16_2l, u_vals_ubl);

				__m256i r2h = _mm256_add_epi16(y_vals_16_2h, v_vals_vrh);
				__m256i g2h = _mm256_sub_epi16(_mm256_sub_epi16(y_vals_16_2h, u_vals_ugh), v_vals_vgh);
				__m256i b2h = _mm256_add_epi16(y_vals_16_2h, u_vals_ubh);

				__m256i r1 = _mm256_packus_epi16(r2l, r2h);
				__m256i g1 = _mm256_packus_epi16(g2l, g2h);
				__m256i b1 = _mm256_packus_epi16(b2l, b2h);

				r1 = _mm256_permute4x64_epi64(r1, _MM_SHUFFLE(3, 1, 2, 0));
				g1 = _mm256_permute4x64_epi64(g1, _MM_SHUFFLE(3, 1, 2, 0));
				b1 = _mm256_permute4x64_epi64(b1, _MM_SHUFFLE(3, 1, 2, 0));

				Store(r1, g1, b1, (rgb_row2 + x * 3));

			}
		}
	}

	inline void Upscale(__m256i u_vals, __m256i& low, __m256i& high) {

		__m128i a_low = _mm256_castsi256_si128(u_vals); // Lower 128 bits
		// Interleave the lower and upper halves
		__m128i result_low = _mm_unpacklo_epi16(a_low, a_low);
		__m128i result_low1 = _mm_unpackhi_epi16(a_low, a_low);
		// combine
		low = _mm256_set_m128i(result_low1, result_low);

		__m128i a_high = _mm256_extracti128_si256(u_vals, 1); // Upper 128 bits
		__m128i result_high = _mm_unpacklo_epi16(a_high, a_high);
		__m128i result_high1 = _mm_unpackhi_epi16(a_high, a_high);
		high = _mm256_set_m128i(result_high1, result_high);
	}

	const __m256i ymm4_const = _mm256_setr_epi8(
		0, 11, 6, 1, 12, 7, 2, 13, 8, 3, 14, 9, 4, 15, 10, 5,
		0, 11, 6, 1, 12, 7, 2, 13, 8, 3, 14, 9, 4, 15, 10, 5
	);

	inline void Store(__m256i r1, __m256i g1, __m256i b1, uint8_t* dst) {
		// Load the vectors into ymm registers
		__m256i ymm0 = r1;
		__m256i ymm1 = g1;
		__m256i ymm2 = b1;

		// Perform the vpalignr operations
		ymm2 = _mm256_alignr_epi8(ymm2, ymm2, 6);
		__m256i ymm3 = _mm256_alignr_epi8(ymm1, ymm1, 11);
		__m256i ymm4 = _mm256_alignr_epi8(ymm0, ymm2, 5);
		ymm2 = _mm256_alignr_epi8(ymm2, ymm3, 5);
		ymm0 = _mm256_alignr_epi8(ymm3, ymm0, 5);
		ymm1 = _mm256_alignr_epi8(ymm1, ymm4, 5);
		ymm2 = _mm256_alignr_epi8(ymm0, ymm2, 5);
		ymm0 = _mm256_alignr_epi8(ymm4, ymm0, 5);

		// Perform the vinserti128 operation
		__m256i ymm3_combined = _mm256_inserti128_si256(ymm1, _mm256_castsi256_si128(ymm2), 1);

		// Perform the vpshufb operations
		ymm3_combined = _mm256_shuffle_epi8(ymm3_combined, ymm4_const);
		ymm1 = _mm256_blend_epi32(ymm0, ymm1, 0xF0); // vpblendd
		ymm1 = _mm256_shuffle_epi8(ymm1, ymm4_const);
		ymm0 = _mm256_permute2x128_si256(ymm2, ymm0, 0x31); // vperm2i128
		ymm0 = _mm256_shuffle_epi8(ymm0, ymm4_const);

		// Store the results back to memory
		_mm256_storeu_si256((__m256i*)(dst + 64), ymm0);
		_mm256_storeu_si256((__m256i*)(dst + 32), ymm1);
		_mm256_storeu_si256((__m256i*)dst, ymm3_combined);


	}
}
#endif


