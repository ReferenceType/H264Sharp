

#include "Yuv2Rgb.h"
#ifndef __arm__
#include "AVX2Common.h"
#include <immintrin.h>
#include <stdint.h>
namespace H264Sharp
{
	template<int NUM_CH, bool RGB>
	void ConvertYUVToRGB_AVX2_Body(
		const uint8_t* y_plane,
		const uint8_t* u_plane,
		const uint8_t* v_plane,
		uint8_t* rgb_buffer,
		int width,
		int Y_stride,
		int UV_stride,
		int RGB_stride,
		int begin,
		int end);

	template<int NUM_CH, bool RGB>
	void Yuv2Rgb::ConvertYUVToRGB_AVX2(
		uint32_t width,
		uint32_t height,
		const uint8_t* Y,
		const uint8_t* U,
		const uint8_t* V,
		uint32_t Y_stride,
		uint32_t UV_stride,
		uint8_t* Rgb,
		uint32_t RGB_stride,
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

					ConvertYUVToRGB_AVX2_Body<NUM_CH, RGB>(Y, U, V, Rgb, width, Y_stride, UV_stride, RGB_stride, bgn, end);

				});
		}
		else
		{
			ConvertYUVToRGB_AVX2_Body<NUM_CH, RGB>(Y, U, V, Rgb, width, Y_stride, UV_stride, RGB_stride, 0, height);
		}
	}

	inline void Store(__m256i r1, __m256i g1, __m256i b1, uint8_t* dst);

	inline void Upscale(__m256i u_vals, __m256i& low, __m256i& high);
	inline void LoadAndUpscale(const uint8_t* plane, __m256i& low, __m256i& high);

	const __m256i const_16 = _mm256_set1_epi8(16);
	const __m256i const_128 = _mm256_set1_epi16(128);
	/*
	* R = CLAMP((Y-16)*1.164 +           1.596*V)
		G = CLAMP((Y-16)*1.164 - 0.391*U - 0.813*V)
		B = CLAMP((Y-16)*1.164 + 2.018*U          )
	*/
	const __m256i y_factor_vec = _mm256_set1_epi16(149);      // 1.164 * 64
	const __m256i v_to_r_coeff_vec = _mm256_set1_epi16(102);  // 1.596 * 64
	const __m256i u_to_g_coeff_vec = _mm256_set1_epi16(25);   // 0.391 * 64
	const __m256i v_to_g_coeff_vec = _mm256_set1_epi16(52);   // 0.813 * 64
	const __m256i u_to_b_coeff_vec = _mm256_set1_epi16(129);  // 2.018 * 64

	template<int NUM_CH, bool RGB>
	void ConvertYUVToRGB_AVX2_Body(
		const uint8_t* y_plane,
		const uint8_t* u_plane,
		const uint8_t* v_plane,
		uint8_t* rgb_buffer,
		int width,
		int Y_stride,
		int UV_stride,
		int RGB_stride,
		int begin,
		int end) {

		for (int y = begin; y < end; y += 2) {
			const uint8_t* y_row1 = y_plane + y * Y_stride;
			const uint8_t* y_row2 = y_row1 + Y_stride;
			const uint8_t* u_row = u_plane + (y / 2) * UV_stride;
			const uint8_t* v_row = v_plane + (y / 2) * UV_stride;
			uint8_t* rgb_row1 = rgb_buffer + y * RGB_stride;
			uint8_t* rgb_row2 = rgb_row1 + RGB_stride;

			for (int x = 0; x < width; x += 32) {
				// Load 32 Y values for two rows
				__m256i y_vals1 = _mm256_loadu_si256((__m256i*)(y_row1 + x));
				__m256i y_vals2 = _mm256_loadu_si256((__m256i*)(y_row2 + x));

				__m256i u_valsl, u_valsh, v_valsl, v_valsh;

				LoadAndUpscale((u_row + (x / 2)), u_valsl, u_valsh);
				LoadAndUpscale((v_row + (x / 2)), v_valsl, v_valsh);

				// Multiply UV with scaling coefficients
				__m256i u_vals_ugl = (_mm256_mullo_epi16(u_valsl, u_to_g_coeff_vec));
				__m256i u_vals_ubl = _mm256_srai_epi16(_mm256_mullo_epi16(u_valsl, u_to_b_coeff_vec), 6);
				__m256i v_vals_vgl = (_mm256_mullo_epi16(v_valsl, v_to_g_coeff_vec));
				__m256i v_vals_vrl = _mm256_srai_epi16(_mm256_mullo_epi16(v_valsl, v_to_r_coeff_vec), 6);

				__m256i u_vals_ugh = (_mm256_mullo_epi16(u_valsh, u_to_g_coeff_vec));
				__m256i u_vals_ubh = _mm256_srai_epi16(_mm256_mullo_epi16(u_valsh, u_to_b_coeff_vec), 6);
				__m256i v_vals_vgh = (_mm256_mullo_epi16(v_valsh, v_to_g_coeff_vec));
				__m256i v_vals_vrh = _mm256_srai_epi16(_mm256_mullo_epi16(v_valsh, v_to_r_coeff_vec), 6);

				//-16
				y_vals1 = _mm256_subs_epu8(y_vals1, const_16);
				y_vals2 = _mm256_subs_epu8(y_vals2, const_16);

				// 0 extend 16 to bit
				__m256i y_vals_16_1l = _mm256_cvtepu8_epi16(_mm256_castsi256_si128(y_vals1));
				__m256i y_vals_16_1h = _mm256_cvtepu8_epi16(_mm256_extracti128_si256(y_vals1, 1));
				__m256i y_vals_16_2l = _mm256_cvtepu8_epi16(_mm256_castsi256_si128(y_vals2));
				__m256i y_vals_16_2h = _mm256_cvtepu8_epi16(_mm256_extracti128_si256(y_vals2, 1));

				// Y*1.164
				y_vals_16_1l = _mm256_srli_epi16(_mm256_mullo_epi16(y_vals_16_1l, y_factor_vec), 7);
				y_vals_16_1h = _mm256_srli_epi16(_mm256_mullo_epi16(y_vals_16_1h, y_factor_vec), 7);
				y_vals_16_2l = _mm256_srli_epi16(_mm256_mullo_epi16(y_vals_16_2l, y_factor_vec), 7);
				y_vals_16_2h = _mm256_srli_epi16(_mm256_mullo_epi16(y_vals_16_2h, y_factor_vec), 7);
				/*
					* R = CLAMP((Y-16)*1.164 +           1.596*V)
					  G = CLAMP((Y-16)*1.164 - 0.391*U - 0.813*V)
					  B = CLAMP((Y-16)*1.164 + 2.018*U          )
					*/
				
				__m256i r1l = _mm256_add_epi16(y_vals_16_1l, v_vals_vrl);
				__m256i g1l = _mm256_sub_epi16(y_vals_16_1l, _mm256_srai_epi16(_mm256_add_epi16(u_vals_ugl, v_vals_vgl),6));
				__m256i b1l = _mm256_add_epi16(y_vals_16_1l, u_vals_ubl);

				__m256i r1h = _mm256_add_epi16(y_vals_16_1h, v_vals_vrh);
				__m256i g1h = _mm256_sub_epi16(y_vals_16_1h, _mm256_srai_epi16(_mm256_add_epi16(u_vals_ugh, v_vals_vgh), 6));
				__m256i b1h = _mm256_add_epi16(y_vals_16_1h, u_vals_ubh);

				__m256i r = _mm256_packus_epi16(r1l, r1h);
				__m256i g = _mm256_packus_epi16(g1l, g1h);
				__m256i b = _mm256_packus_epi16(b1l, b1h);

				r = _mm256_permute4x64_epi64(r, _MM_SHUFFLE(3, 1, 2, 0));
				g = _mm256_permute4x64_epi64(g, _MM_SHUFFLE(3, 1, 2, 0));
				b = _mm256_permute4x64_epi64(b, _MM_SHUFFLE(3, 1, 2, 0));

				//Store(r, g, b, (rgb_row1 + (x * 3)));
				if constexpr (NUM_CH<4)
					if constexpr (RGB)
						Store3Interleave((rgb_row1 + (x * 3)), r, g, b);
					else
						Store3Interleave((rgb_row1 + (x * 3)), b, g, r);
				else
					if constexpr (RGB)
						Store4Interleave((rgb_row1 + (x * 4)), r, g, b, _mm256_set1_epi8(0xff));
					else
						Store4Interleave((rgb_row1 + (x * 4)), b, g, r, _mm256_set1_epi8(0xff));

				// Calculate RGB for second row
				__m256i r2l = _mm256_add_epi16(y_vals_16_2l, v_vals_vrl);
				__m256i g2l = _mm256_sub_epi16(y_vals_16_2l, _mm256_srai_epi16(_mm256_add_epi16(u_vals_ugl, v_vals_vgl), 6));
				__m256i b2l = _mm256_add_epi16(y_vals_16_2l, u_vals_ubl);

				__m256i r2h = _mm256_add_epi16(y_vals_16_2h, v_vals_vrh);
				__m256i g2h = _mm256_sub_epi16(y_vals_16_2h, _mm256_srai_epi16(_mm256_add_epi16(u_vals_ugh, v_vals_vgh), 6));
				__m256i b2h = _mm256_add_epi16(y_vals_16_2h, u_vals_ubh);

				__m256i r1 = _mm256_packus_epi16(r2l, r2h);
				__m256i g1 = _mm256_packus_epi16(g2l, g2h);
				__m256i b1 = _mm256_packus_epi16(b2l, b2h);

				r1 = _mm256_permute4x64_epi64(r1, _MM_SHUFFLE(3, 1, 2, 0));
				g1 = _mm256_permute4x64_epi64(g1, _MM_SHUFFLE(3, 1, 2, 0));
				b1 = _mm256_permute4x64_epi64(b1, _MM_SHUFFLE(3, 1, 2, 0));

				//Store(r1, g1, b1, (rgb_row2 + (x * 3)));
				if constexpr (NUM_CH < 4)
					if constexpr (RGB)
						Store3Interleave((rgb_row2 + (x * 3)), r1, g1, b1);
					else
						Store3Interleave((rgb_row2 + (x * 3)), b1, g1, r1);
				else
					if constexpr (RGB)
						Store4Interleave((rgb_row2 + (x * 4)), r1, g1, b1, _mm256_set1_epi8(0xff));
					else
						Store4Interleave((rgb_row2 + (x * 4)), b1, g1, r1, _mm256_set1_epi8(0xff));

			}
		}
	}

	__m256i uvmask = _mm256_setr_epi8(0, 0, 2, 2, 4, 4, 6, 6, 8, 8, 10, 10, 12, 12, 14, 14, 16, 16, 18, 18, 20, 20, 22, 22, 24, 24, 26, 26, 28, 28, 30, 30);
	inline void LoadAndUpscale(const uint8_t* plane, __m256i& low, __m256i& high)
	{
		__m128i u = _mm_loadu_si128((__m128i*)plane);
		__m256i u0 = _mm256_cvtepu8_epi16(u);

		auto ud = _mm256_shuffle_epi8(u0, uvmask);
		__m128i ul8 = _mm256_castsi256_si128(ud);
		__m128i uh8 = _mm256_extracti128_si256(ud, 1);

		low = _mm256_sub_epi16(_mm256_cvtepu8_epi16(ul8), const_128);
		high = _mm256_sub_epi16( _mm256_cvtepu8_epi16(uh8), const_128);

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


	template void Yuv2Rgb::ConvertYUVToRGB_AVX2<3, true>(uint32_t width,
		uint32_t height,
		const uint8_t* Y,
		const uint8_t* U,
		const uint8_t* V,
		uint32_t Y_stride,
		uint32_t UV_stride,
		uint8_t* Rgb,
		uint32_t RGB_stride,
		int numThreads);

	template void Yuv2Rgb::ConvertYUVToRGB_AVX2<4, true>(uint32_t width,
		uint32_t height,
		const uint8_t* Y,
		const uint8_t* U,
		const uint8_t* V,
		uint32_t Y_stride,
		uint32_t UV_stride,
		uint8_t* Rgb,
		uint32_t RGB_stride,
		int numThreads);

	template void Yuv2Rgb::ConvertYUVToRGB_AVX2<3, false>(uint32_t width,
		uint32_t height,
		const uint8_t* Y,
		const uint8_t* U,
		const uint8_t* V,
		uint32_t Y_stride,
		uint32_t UV_stride,
		uint8_t* Rgb,
		uint32_t RGB_stride,
		int numThreads);

	template void Yuv2Rgb::ConvertYUVToRGB_AVX2<4, false>(uint32_t width,
		uint32_t height,
		const uint8_t* Y,
		const uint8_t* U,
		const uint8_t* V,
		uint32_t Y_stride,
		uint32_t UV_stride,
		uint8_t* Rgb,
		uint32_t RGB_stride,
		int numThreads);
}
#endif


