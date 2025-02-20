

#include "Yuv2Rgb.h"
#ifndef __arm__
#include "AVX2Common.h"
namespace H264Sharp
{
	template<int NUM_CH, bool RGB>
	void ConvertYUVToRGB_AVX2_Body(
		const uint8_t* RESTRICT y_plane,
		const uint8_t* RESTRICT u_plane,
		const uint8_t* RESTRICT v_plane,
		uint8_t* RESTRICT rgb_buffer,
		int32_t width,
		int32_t Y_stride,
		int32_t UV_stride,
		int32_t RGB_stride,
		int32_t begin,
		int32_t end);

	template<int NUM_CH, bool RGB>
	void ConvertYUVNV12ToRGB_AVX2_Body(
		const uint8_t* RESTRICT y_plane,
		const uint8_t* RESTRICT uv_plane,
		uint8_t* RESTRICT rgb_buffer,
		int32_t width,
		int32_t Y_stride,
		int32_t UV_stride,
		int32_t RGB_stride,
		int32_t begin,
		int32_t end);

	template<int NUM_CH, bool RGB>
	void Yuv2Rgb::ConvertYUVToRGB_AVX2(
		int32_t width,
		int32_t height,
		const uint8_t* RESTRICT Y,
		const uint8_t* RESTRICT U,
		const uint8_t* RESTRICT V,
		int32_t Y_stride,
		int32_t UV_stride,
		uint8_t* RESTRICT Rgb,
		int32_t RGB_stride,
		int32_t numThreads)
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

	template<int NUM_CH, bool RGB>
	void Yuv2Rgb::ConvertYUVNV12ToRGB_AVX2(
		int32_t width,
		int32_t height,
		const uint8_t* RESTRICT Y,
		const uint8_t* RESTRICT UV,
		int32_t Y_stride,
		int32_t UV_stride,
		uint8_t* RESTRICT Rgb,
		int32_t RGB_stride,
		int32_t numThreads)
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

					ConvertYUVNV12ToRGB_AVX2_Body<NUM_CH, RGB>(Y, UV, Rgb, width, Y_stride, UV_stride, RGB_stride, bgn, end);

				});
		}
		else
		{
			ConvertYUVNV12ToRGB_AVX2_Body<NUM_CH, RGB>(Y, UV, Rgb, width, Y_stride, UV_stride, RGB_stride, 0, height);
		}
	}

	template<int NUM_CH, bool RGB>
	void ConvertYUVNV12ToRGB_AVX2_Body(
		const uint8_t* RESTRICT y_plane,
		const uint8_t* RESTRICT uv_plane,
		uint8_t* RESTRICT rgb_buffer,
		int32_t width,
		int32_t Y_stride,
		int32_t UV_stride,
		int32_t RGB_stride,
		int32_t begin,
		int32_t end)
	{
		for (int y = begin; y < end; y += 2) {
			const uint8_t* y_row1 = y_plane + y * Y_stride;
			const uint8_t* y_row2 = y_row1 + Y_stride;
			const uint8_t* u_row = uv_plane + (y)*UV_stride;
			//const uint8_t* v_row = v_plane + (y / 2) * UV_stride;
			uint8_t* rgb_row1 = rgb_buffer + y * RGB_stride;
			uint8_t* rgb_row2 = rgb_row1 + RGB_stride;

			for (int x = 0; x < width; x += 32) 
			{
				__m256i UV = _mm256_loadu_si256((__m256i*)(u_row + x));
				__m256i y_vals1 = _mm256_loadu_si256((__m256i*)(y_row1 + x));
				__m256i y_vals2 = _mm256_loadu_si256((__m256i*)(y_row2 + x));

				const __m256i masku = _mm256_set1_epi16(0x00FF);
				const __m256i maskv = _mm256_set1_epi16(0xFF00);

				auto u_vals = _mm256_and_si256(UV, masku);
				auto v_vals = _mm256_srli_epi16(_mm256_and_si256(UV, maskv), 8);

				u_vals = _mm256_sub_epi16(u_vals, _mm256_set1_epi16(128));
				v_vals = _mm256_sub_epi16(v_vals, _mm256_set1_epi16(128));

				__m256i u_valsl, u_valsh;
				Upscale(u_vals, u_valsl, u_valsh);

				__m256i v_valsl, v_valsh;
				Upscale(v_vals, v_valsl, v_valsh);

				__m256i r, g, b, r1, g1, b1;
				Convert(y_vals1, y_vals2, u_valsl, u_valsh, v_valsl, v_valsh,
					r, g, b, r1, g1, b1);

				if constexpr (NUM_CH < 4)
					if constexpr (RGB)
					{
						Store3Interleave((rgb_row1 + (x * 3)), r, g, b);
						Store3Interleave((rgb_row2 + (x * 3)), r1, g1, b1);
					}
					else
					{
						Store3Interleave((rgb_row1 + (x * 3)), b, g, r);
						Store3Interleave((rgb_row2 + (x * 3)), b1, g1, r1);
					}
				else
					if constexpr (RGB)
					{
						Store4Interleave((rgb_row1 + (x * 4)), r, g, b);
						Store4Interleave((rgb_row2 + (x * 4)), r1, g1, b1);
					}
					else
					{
						Store4Interleave((rgb_row1 + (x * 4)), b, g, r);
						Store4Interleave((rgb_row2 + (x * 4)), b1, g1, r1);
					}
			}
		}
	}

	inline void Convert(__m256i y_vals1, __m256i y_vals2, __m256i u_valsl, __m256i u_valsh, __m256i v_valsl, __m256i v_valsh,
		__m256i& r, __m256i& g, __m256i& b, __m256i& r1, __m256i& g1, __m256i& b1);


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
	const __m256i const_16_8b = _mm256_set1_epi8(16);

	template<int NUM_CH, bool RGB>
	void ConvertYUVToRGB_AVX2_Body(
		const uint8_t* RESTRICT y_plane,
		const uint8_t* RESTRICT u_plane,
		const uint8_t* RESTRICT v_plane,
		uint8_t* RESTRICT rgb_buffer,
		int32_t width,
		int32_t Y_stride,
		int32_t UV_stride,
		int32_t RGB_stride,
		int32_t begin,
		int32_t end) 
	{

		for (int y = begin; y < end; y += 2) 
		{
			const uint8_t* y_row1 = y_plane + y * Y_stride;
			const uint8_t* y_row2 = y_row1 + Y_stride;
			const uint8_t* u_row = u_plane + (y / 2) * UV_stride;
			const uint8_t* v_row = v_plane + (y / 2) * UV_stride;
			uint8_t* rgb_row1 = rgb_buffer + y * RGB_stride;
			uint8_t* rgb_row2 = rgb_row1 + RGB_stride;

			for (int x = 0; x < width; x += 32)
			{
				// Load 32 Y values for two rows
				/*
				__m256i y_vals1, y_vals2;
				y_vals1 = loadY((__m256i*)(y_row1 + x));
				y_vals2 = loadY((__m256i*)(y_row2 + x));
*/

				__m256i y_vals1 = _mm256_loadu_si256((__m256i*)(y_row1 + x));
				__m256i y_vals2 = _mm256_loadu_si256((__m256i*)(y_row2 + x));

				__m256i u_valsl, u_valsh, v_valsl, v_valsh;

				LoadAndUpscale((u_row + (x / 2)), u_valsl, u_valsh);
				LoadAndUpscale((v_row + (x / 2)), v_valsl, v_valsh);

				__m256i r, g, b, r1, g1, b1;
				Convert(y_vals1, y_vals2, u_valsl, u_valsh, v_valsl, v_valsh,
					r, g, b, r1, g1, b1);

				if constexpr (NUM_CH < 4)
					if constexpr (RGB)
					{
						Store3Interleave((rgb_row1 + (x * 3)), r, g, b);
						Store3Interleave((rgb_row2 + (x * 3)), r1, g1, b1);
					}
					else
					{
						Store3Interleave((rgb_row1 + (x * 3)), b, g, r);
						Store3Interleave((rgb_row2 + (x * 3)), b1, g1, r1);
					}
				else
					if constexpr (RGB)
					{
						Store4Interleave((rgb_row1 + (x * 4)), r, g, b);
						Store4Interleave((rgb_row2 + (x * 4)), r1, g1, b1);
					}	
					else
					{
						Store4Interleave((rgb_row1 + (x * 4)), b, g, r);
						Store4Interleave((rgb_row2 + (x * 4)), b1, g1, r1);
					}
						

				//// Multiply UV with scaling coefficients
				//__m256i u_vals_ugl = _mm256_srai_epi16(_mm256_mullo_epi16(u_valsl, u_to_g_coeff_vec), 6);
				//__m256i u_vals_ubl = _mm256_srai_epi16(_mm256_mullo_epi16(u_valsl, u_to_b_coeff_vec), 6);
				//__m256i v_vals_vgl = _mm256_srai_epi16(_mm256_mullo_epi16(v_valsl, v_to_g_coeff_vec), 6);
				//__m256i v_vals_vrl = _mm256_srai_epi16(_mm256_mullo_epi16(v_valsl, v_to_r_coeff_vec), 6);

				//__m256i u_vals_ugh = _mm256_srai_epi16(_mm256_mullo_epi16(u_valsh, u_to_g_coeff_vec), 6);
				//__m256i u_vals_ubh = _mm256_srai_epi16(_mm256_mullo_epi16(u_valsh, u_to_b_coeff_vec), 6);
				//__m256i v_vals_vgh = _mm256_srai_epi16(_mm256_mullo_epi16(v_valsh, v_to_g_coeff_vec), 6);
				//__m256i v_vals_vrh = _mm256_srai_epi16(_mm256_mullo_epi16(v_valsh, v_to_r_coeff_vec), 6);

				////-16
				//y_vals1 = _mm256_subs_epu8(y_vals1, const_16_8b);
				//y_vals2 = _mm256_subs_epu8(y_vals2, const_16_8b);

				//// 0 extend 16 to bit
				//__m256i y_vals_16_1l = _mm256_cvtepu8_epi16(_mm256_castsi256_si128(y_vals1));
				//__m256i y_vals_16_1h = _mm256_cvtepu8_epi16(_mm256_extracti128_si256(y_vals1, 1));
				//__m256i y_vals_16_2l = _mm256_cvtepu8_epi16(_mm256_castsi256_si128(y_vals2));
				//__m256i y_vals_16_2h = _mm256_cvtepu8_epi16(_mm256_extracti128_si256(y_vals2, 1));

				//// Y*1.164
				//y_vals_16_1l = _mm256_srli_epi16(_mm256_mullo_epi16(y_vals_16_1l, y_factor_vec), 7);
				//y_vals_16_1h = _mm256_srli_epi16(_mm256_mullo_epi16(y_vals_16_1h, y_factor_vec), 7);
				//y_vals_16_2l = _mm256_srli_epi16(_mm256_mullo_epi16(y_vals_16_2l, y_factor_vec), 7);
				//y_vals_16_2h = _mm256_srli_epi16(_mm256_mullo_epi16(y_vals_16_2h, y_factor_vec), 7);
				///*
				//	* R = CLAMP((Y-16)*1.164 +           1.596*V)
				//	  G = CLAMP((Y-16)*1.164 - 0.391*U - 0.813*V)
				//	  B = CLAMP((Y-16)*1.164 + 2.018*U          )
				//	*/
				//
				//__m256i r1l = _mm256_add_epi16(y_vals_16_1l, v_vals_vrl);
				//__m256i g1l = _mm256_sub_epi16(_mm256_sub_epi16(y_vals_16_1l, u_vals_ugl), v_vals_vgl);
				//__m256i b1l = _mm256_add_epi16(y_vals_16_1l, u_vals_ubl);

				//__m256i r1h = _mm256_add_epi16(y_vals_16_1h, v_vals_vrh);
				//__m256i g1h = _mm256_sub_epi16(_mm256_sub_epi16(y_vals_16_1h, u_vals_ugh), v_vals_vgh);
				//__m256i b1h = _mm256_add_epi16(y_vals_16_1h, u_vals_ubh);

				//__m256i r = _mm256_packus_epi16(r1l, r1h);
				//__m256i g = _mm256_packus_epi16(g1l, g1h);
				//__m256i b = _mm256_packus_epi16(b1l, b1h);

				//r = _mm256_permute4x64_epi64(r, _MM_SHUFFLE(3, 1, 2, 0));
				//g = _mm256_permute4x64_epi64(g, _MM_SHUFFLE(3, 1, 2, 0));
				//b = _mm256_permute4x64_epi64(b, _MM_SHUFFLE(3, 1, 2, 0));

				////Store(r, g, b, (rgb_row1 + (x * 3)));
				//if constexpr (NUM_CH<4)
				//	if constexpr (RGB)
				//		Store3Interleave((rgb_row1 + (x * 3)), r, g, b);
				//	else
				//		Store3Interleave((rgb_row1 + (x * 3)), b, g, r);
				//else
				//	if constexpr (RGB)
				//		Store4Interleave((rgb_row1 + (x * 4)), r, g, b);
				//	else
				//		Store4Interleave((rgb_row1 + (x * 4)), b, g, r);

				//// Calculate RGB for second row
				//__m256i r2l = _mm256_add_epi16(y_vals_16_2l, v_vals_vrl);
				//__m256i g2l = _mm256_sub_epi16(_mm256_sub_epi16(y_vals_16_2l, u_vals_ugl), v_vals_vgl);
				//__m256i b2l = _mm256_add_epi16(y_vals_16_2l, u_vals_ubl);

				//__m256i r2h = _mm256_add_epi16(y_vals_16_2h, v_vals_vrh);
				//__m256i g2h = _mm256_sub_epi16(_mm256_sub_epi16(y_vals_16_2h, u_vals_ugh), v_vals_vgh);
				//__m256i b2h = _mm256_add_epi16(y_vals_16_2h, u_vals_ubh);

				//__m256i r1 = _mm256_packus_epi16(r2l, r2h);
				//__m256i g1 = _mm256_packus_epi16(g2l, g2h);
				//__m256i b1 = _mm256_packus_epi16(b2l, b2h);

				//r1 = _mm256_permute4x64_epi64(r1, _MM_SHUFFLE(3, 1, 2, 0));
				//g1 = _mm256_permute4x64_epi64(g1, _MM_SHUFFLE(3, 1, 2, 0));
				//b1 = _mm256_permute4x64_epi64(b1, _MM_SHUFFLE(3, 1, 2, 0));

				////Store(r1, g1, b1, (rgb_row2 + (x * 3)));
				//if constexpr (NUM_CH < 4)
				//	if constexpr (RGB)
				//		Store3Interleave((rgb_row2 + (x * 3)), r1, g1, b1);
				//	else
				//		Store3Interleave((rgb_row2 + (x * 3)), b1, g1, r1);
				//else
				//	if constexpr (RGB)
				//		Store4Interleave((rgb_row2 + (x * 4)), r1, g1, b1);
				//	else
				//		Store4Interleave((rgb_row2 + (x * 4)), b1, g1, r1);

			}
		}
	}

	inline void Convert( __m256i y_vals1, __m256i y_vals2,__m256i u_valsl, __m256i u_valsh, __m256i v_valsl, __m256i v_valsh,
		__m256i& r, __m256i& g, __m256i& b, __m256i& r1, __m256i& g1, __m256i& b1)
	{
		// Multiply UV with scaling coefficients
		__m256i u_vals_ugl = _mm256_srai_epi16(_mm256_mullo_epi16(u_valsl, u_to_g_coeff_vec), 6);
		__m256i u_vals_ubl = _mm256_srai_epi16(_mm256_mullo_epi16(u_valsl, u_to_b_coeff_vec), 6);
		__m256i v_vals_vgl = _mm256_srai_epi16(_mm256_mullo_epi16(v_valsl, v_to_g_coeff_vec), 6);
		__m256i v_vals_vrl = _mm256_srai_epi16(_mm256_mullo_epi16(v_valsl, v_to_r_coeff_vec), 6);

		__m256i u_vals_ugh = _mm256_srai_epi16(_mm256_mullo_epi16(u_valsh, u_to_g_coeff_vec), 6);
		__m256i u_vals_ubh = _mm256_srai_epi16(_mm256_mullo_epi16(u_valsh, u_to_b_coeff_vec), 6);
		__m256i v_vals_vgh = _mm256_srai_epi16(_mm256_mullo_epi16(v_valsh, v_to_g_coeff_vec), 6);
		__m256i v_vals_vrh = _mm256_srai_epi16(_mm256_mullo_epi16(v_valsh, v_to_r_coeff_vec), 6);

		//-16
		y_vals1 = _mm256_subs_epu8(y_vals1, const_16_8b);
		y_vals2 = _mm256_subs_epu8(y_vals2, const_16_8b);

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
		__m256i g1l = _mm256_sub_epi16(_mm256_sub_epi16(y_vals_16_1l, u_vals_ugl), v_vals_vgl);
		__m256i b1l = _mm256_add_epi16(y_vals_16_1l, u_vals_ubl);

		__m256i r1h = _mm256_add_epi16(y_vals_16_1h, v_vals_vrh);
		__m256i g1h = _mm256_sub_epi16(_mm256_sub_epi16(y_vals_16_1h, u_vals_ugh), v_vals_vgh);
		__m256i b1h = _mm256_add_epi16(y_vals_16_1h, u_vals_ubh);

		r = _mm256_packus_epi16(r1l, r1h);
		g = _mm256_packus_epi16(g1l, g1h);
		b = _mm256_packus_epi16(b1l, b1h);

		r = _mm256_permute4x64_epi64(r, _MM_SHUFFLE(3, 1, 2, 0));
		g = _mm256_permute4x64_epi64(g, _MM_SHUFFLE(3, 1, 2, 0));
		b = _mm256_permute4x64_epi64(b, _MM_SHUFFLE(3, 1, 2, 0));

		// Calculate RGB for second row
		__m256i r2l = _mm256_add_epi16(y_vals_16_2l, v_vals_vrl);
		__m256i g2l = _mm256_sub_epi16(_mm256_sub_epi16(y_vals_16_2l, u_vals_ugl), v_vals_vgl);
		__m256i b2l = _mm256_add_epi16(y_vals_16_2l, u_vals_ubl);

		__m256i r2h = _mm256_add_epi16(y_vals_16_2h, v_vals_vrh);
		__m256i g2h = _mm256_sub_epi16(_mm256_sub_epi16(y_vals_16_2h, u_vals_ugh), v_vals_vgh);
		__m256i b2h = _mm256_add_epi16(y_vals_16_2h, u_vals_ubh);

		r1 = _mm256_packus_epi16(r2l, r2h);
		g1 = _mm256_packus_epi16(g2l, g2h);
		b1 = _mm256_packus_epi16(b2l, b2h);

		r1 = _mm256_permute4x64_epi64(r1, _MM_SHUFFLE(3, 1, 2, 0));
		g1 = _mm256_permute4x64_epi64(g1, _MM_SHUFFLE(3, 1, 2, 0));
		b1 = _mm256_permute4x64_epi64(b1, _MM_SHUFFLE(3, 1, 2, 0));

		
	}

	template void Yuv2Rgb::ConvertYUVToRGB_AVX2<3, true>(
		int32_t width,
		int32_t height,
		const uint8_t* RESTRICT Y,
		const uint8_t* RESTRICT U,
		const uint8_t* RESTRICT V,
		int32_t Y_stride,
		int32_t UV_stride,
		uint8_t* RESTRICT Rgb,
		int32_t RGB_stride,
		int32_t numThreads);

	template void Yuv2Rgb::ConvertYUVToRGB_AVX2<4, true>(
		int32_t width,
		int32_t height,
		const uint8_t* RESTRICT Y,
		const uint8_t* RESTRICT U,
		const uint8_t* RESTRICT V,
		int32_t Y_stride,
		int32_t UV_stride,
		uint8_t* RESTRICT Rgb,
		int32_t RGB_stride,
		int32_t numThreads);

	template void Yuv2Rgb::ConvertYUVToRGB_AVX2<3, false>(
		int32_t width,
		int32_t height,
		const uint8_t* RESTRICT Y,
		const uint8_t* RESTRICT U,
		const uint8_t* RESTRICT V,
		int32_t Y_stride,
		int32_t UV_stride,
		uint8_t* RESTRICT Rgb,
		int32_t RGB_stride,
		int32_t numThreads);

	template void Yuv2Rgb::ConvertYUVToRGB_AVX2<4, false>(
		int32_t width,
		int32_t height,
		const uint8_t* RESTRICT Y,
		const uint8_t* RESTRICT U,
		const uint8_t* RESTRICT V,
		int32_t Y_stride,
		int32_t UV_stride,
		uint8_t* RESTRICT Rgb,
		int32_t RGB_stride,
		int32_t numThreads);

	//--
	template void Yuv2Rgb::ConvertYUVNV12ToRGB_AVX2<3, true>(
		int32_t width,
		int32_t height,
		const uint8_t* RESTRICT Y,
		const uint8_t* RESTRICT UV,
		int32_t Y_stride,
		int32_t UV_stride,
		uint8_t* RESTRICT Rgb,
		int32_t RGB_stride,
		int32_t numThreads);

	template void Yuv2Rgb::ConvertYUVNV12ToRGB_AVX2<4, true>(
		int32_t width,
		int32_t height,
		const uint8_t* RESTRICT Y,
		const uint8_t* RESTRICT UV,
		int32_t Y_stride,
		int32_t UV_stride,
		uint8_t* RESTRICT Rgb,
		int32_t RGB_stride,
		int32_t numThreads);

	template void Yuv2Rgb::ConvertYUVNV12ToRGB_AVX2<3, false>(
		int32_t width,
		int32_t height,
		const uint8_t* RESTRICT Y,
		const uint8_t* RESTRICT UV,
		int32_t Y_stride,
		int32_t UV_stride,
		uint8_t* RESTRICT Rgb,
		int32_t RGB_stride,
		int32_t numThreads);

	template void Yuv2Rgb::ConvertYUVNV12ToRGB_AVX2<4, false>(
		int32_t width,
		int32_t height,
		const uint8_t* RESTRICT Y,
		const uint8_t* RESTRICT UV,
		int32_t Y_stride,
		int32_t UV_stride,
		uint8_t* RESTRICT Rgb,
		int32_t RGB_stride,
		int32_t numThreads);
}
#endif


