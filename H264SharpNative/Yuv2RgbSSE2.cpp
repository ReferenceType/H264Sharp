

#include "Yuv2Rgb.h"
#ifndef __arm__
#include <immintrin.h>
#include <smmintrin.h>
#include <emmintrin.h>
#include <stdint.h>
namespace H264Sharp
{
	template<int NUM_CH, bool RGB>
	void ConvertYUVToRGB_SSE_Body(const uint8_t* RESTRICT y_plane, const uint8_t* RESTRICT u_plane, const uint8_t* RESTRICT v_plane,
		uint8_t* RESTRICT rgb_buffer,int32_t width,int32_t Y_stride,int32_t UV_stride,int32_t RGB_stride,int32_t begin,int32_t end);

	template <int NUM_CH, bool RGB>
	void Yuv2Rgb::yuv420_rgb24_sse(int32_t width,int32_t height,const uint8_t* RESTRICT Y,const uint8_t* RESTRICT U,const uint8_t* RESTRICT V,
		int32_t Y_stride,int32_t UV_stride,uint8_t* RESTRICT Rgb,int32_t RGB_stride,int32_t numThreads)
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

					ConvertYUVToRGB_SSE_Body<NUM_CH, RGB>(Y, U, V, Rgb, width, Y_stride, UV_stride, RGB_stride, bgn, end);

				});
		}
		else
		{
			ConvertYUVToRGB_SSE_Body<NUM_CH, RGB>(Y, U, V, Rgb, width, Y_stride, UV_stride, RGB_stride, 0, height);
		}
	}

	inline void Store3(__m128i c, __m128i b, __m128i a, uint8_t* ptr);
	inline void Store4(const __m128i& vec0, const __m128i& vec1, const __m128i& vec2, unsigned char* dst);

	inline void LoadAndUpscale(const uint8_t* plane, __m128i& low, __m128i& high);
	inline void Convert(__m128i y_vals1, __m128i y_vals2, __m128i u_valsl, __m128i u_valsh, __m128i v_valsl, __m128i v_valsh,
		__m128i& r, __m128i& g, __m128i& b, __m128i& r1, __m128i& g1, __m128i& b1);

	/*
	*   R = CLAMP((Y-16)*1.164 +           1.596*V)
		G = CLAMP((Y-16)*1.164 - 0.391*U - 0.813*V)
		B = CLAMP((Y-16)*1.164 + 2.018*U          )
	*/
	const __m128i y_factor_vec = _mm_set1_epi16(149);      // 1.164 * 64
	const __m128i v_to_r_coeff_vec = _mm_set1_epi16(102);  // 1.596 * 64
	const __m128i u_to_g_coeff_vec = _mm_set1_epi16(25);   // 0.391 * 64
	const __m128i v_to_g_coeff_vec = _mm_set1_epi16(52);   // 0.813 * 64
	const __m128i u_to_b_coeff_vec = _mm_set1_epi16(129);  // 2.018 * 64
	const __m128i const_16 = _mm_set1_epi8(16);
	const __m128i const_128 = _mm_set1_epi16(128);

	template<int NUM_CH, bool RGB>
	void ConvertYUVToRGB_SSE_Body(
		const uint8_t* RESTRICT y_plane,
		const uint8_t* RESTRICT u_plane,
		const uint8_t* RESTRICT v_plane,
		uint8_t* RESTRICT rgb_buffer,
		int32_t width,
		int32_t Y_stride,
		int32_t UV_stride,
		int32_t RGB_stride,
		int32_t begin,
		int32_t end) {

		for (int y = begin; y < end; y += 2) {
			const uint8_t* y_row1 = y_plane + y * Y_stride;
			const uint8_t* y_row2 = y_row1 + Y_stride;
			const uint8_t* u_row = u_plane + (y / 2) * UV_stride;
			const uint8_t* v_row = v_plane + (y / 2) * UV_stride;
			uint8_t* rgb_row1 = rgb_buffer + y * RGB_stride;
			uint8_t* rgb_row2 = rgb_row1 + RGB_stride;

			for (int x = 0; x < width; x += 16) {
				// Load 32 Y values for two rows
				__m128i y_vals1 = _mm_loadu_si128((__m128i*)(y_row1 + x));
				__m128i y_vals2 = _mm_loadu_si128((__m128i*)(y_row2 + x));

				__m128i u_valsl, u_valsh, v_valsl, v_valsh;

				LoadAndUpscale((u_row + (x / 2)), u_valsl, u_valsh);
				LoadAndUpscale((v_row + (x / 2)), v_valsl, v_valsh);

				__m128i r, g, b, r1, g1, b1;
				Convert(y_vals1, y_vals2, u_valsl, u_valsh, v_valsl, v_valsh,
					r, g, b, r1, g1, b1);

				if constexpr (NUM_CH < 4)
					if constexpr (RGB)
					{
						Store3(r, g, b, (rgb_row1 + (x * 3)));
						Store3(r1, g1, b1, (rgb_row2 + (x * 3)));

					}
					else 
					{
						Store3(b, g, r, (rgb_row1 + (x * 3)));
						Store3(b1, g1, r1, (rgb_row2 + (x * 3)));
					}
				else
					if constexpr (RGB)
					{
						Store4(r, g, b, (rgb_row1 + (x * 4)));
						Store4(r1, g1, b1, (rgb_row2 + (x * 4)));
					}
					else
					{
						Store4(b, g, r, (rgb_row1 + (x * 4)));
						Store4(b1, g1, r1, (rgb_row2 + (x * 4)));
					}



				//// Multiply UV with scaling coefficients
				//__m128i u_vals_ugl = _mm_srai_epi16(_mm_mullo_epi16(u_valsl, u_to_g_coeff_vec),6);
				//__m128i u_vals_ubl = _mm_srai_epi16(_mm_mullo_epi16(u_valsl, u_to_b_coeff_vec), 6);
				//__m128i v_vals_vgl = _mm_srai_epi16(_mm_mullo_epi16(v_valsl, v_to_g_coeff_vec),6);
				//__m128i v_vals_vrl = _mm_srai_epi16(_mm_mullo_epi16(v_valsl, v_to_r_coeff_vec), 6);

				//__m128i u_vals_ugh = _mm_srai_epi16(_mm_mullo_epi16(u_valsh, u_to_g_coeff_vec),6);
				//__m128i u_vals_ubh = _mm_srai_epi16(_mm_mullo_epi16(u_valsh, u_to_b_coeff_vec), 6);
				//__m128i v_vals_vgh = _mm_srai_epi16(_mm_mullo_epi16(v_valsh, v_to_g_coeff_vec),6);
				//__m128i v_vals_vrh = _mm_srai_epi16(_mm_mullo_epi16(v_valsh, v_to_r_coeff_vec), 6);

				////-16
				//y_vals1 = _mm_subs_epu8(y_vals1, const_16);
				//y_vals2 = _mm_subs_epu8(y_vals2, const_16);

				//// 0 extend 16 to bit
				//__m128i y_vals_16_1l = _mm_unpacklo_epi8(y_vals1, _mm_set1_epi8(0x00));
				//__m128i y_vals_16_1h = _mm_unpackhi_epi8(y_vals1, _mm_set1_epi8(0x00));
				//__m128i y_vals_16_2l = _mm_unpacklo_epi8(y_vals2, _mm_set1_epi8(0x00));
				//__m128i y_vals_16_2h = _mm_unpackhi_epi8(y_vals2, _mm_set1_epi8(0x00));

				//// Y*1.164
				//y_vals_16_1l = _mm_srli_epi16(_mm_mullo_epi16(y_vals_16_1l, y_factor_vec), 7);
				//y_vals_16_1h = _mm_srli_epi16(_mm_mullo_epi16(y_vals_16_1h, y_factor_vec), 7);
				//y_vals_16_2l = _mm_srli_epi16(_mm_mullo_epi16(y_vals_16_2l, y_factor_vec), 7);
				//y_vals_16_2h = _mm_srli_epi16(_mm_mullo_epi16(y_vals_16_2h, y_factor_vec), 7);
				///*
				//	  R = CLAMP((Y-16)*1.164 +           1.596*V)
				//	  G = CLAMP((Y-16)*1.164 - 0.391*U - 0.813*V)
				//	  B = CLAMP((Y-16)*1.164 + 2.018*U          )
				//*/

				//__m128i r1l = _mm_add_epi16(y_vals_16_1l, v_vals_vrl);
				//__m128i g1l = _mm_sub_epi16(_mm_sub_epi16(y_vals_16_1l, u_vals_ugl), v_vals_vgl);
				//__m128i b1l = _mm_add_epi16(y_vals_16_1l, u_vals_ubl);

				//__m128i r1h = _mm_add_epi16(y_vals_16_1h, v_vals_vrh);
				//__m128i g1h = _mm_sub_epi16(_mm_sub_epi16(y_vals_16_1h, u_vals_ugh), v_vals_vgh);
				//__m128i b1h = _mm_add_epi16(y_vals_16_1h, u_vals_ubh);

				//__m128i r = _mm_packus_epi16(r1l, r1h);
				//__m128i g = _mm_packus_epi16(g1l, g1h);
				//__m128i b = _mm_packus_epi16(b1l, b1h);

				//if constexpr (NUM_CH < 4)
				//	if constexpr (RGB)
				//		Store3(r, g, b, (rgb_row1 + (x * 3)));
				//	else
				//		Store3(b, g, r, (rgb_row1 + (x * 3)));
				//else
				//	if constexpr (RGB)
				//		Store4(r, g, b, (rgb_row1 + (x * 4)));
				//	else
				//		Store4(b, g, r, (rgb_row1 + (x * 4)));

				//// Calculate RGB for second row
				//__m128i r2l = _mm_add_epi16(y_vals_16_2l, v_vals_vrl);
				//__m128i g2l = _mm_sub_epi16(_mm_sub_epi16(y_vals_16_2l, u_vals_ugl), v_vals_vgl);
				//__m128i b2l = _mm_add_epi16(y_vals_16_2l, u_vals_ubl);

				//__m128i r2h = _mm_add_epi16(y_vals_16_2h, v_vals_vrh);
				//__m128i g2h = _mm_sub_epi16(_mm_sub_epi16(y_vals_16_2h, u_vals_ugh), v_vals_vgh);
				//__m128i b2h = _mm_add_epi16(y_vals_16_2h, u_vals_ubh);

				//__m128i r1 = _mm_packus_epi16(r2l, r2h);
				//__m128i g1 = _mm_packus_epi16(g2l, g2h);
				//__m128i b1 = _mm_packus_epi16(b2l, b2h);

				//if constexpr (NUM_CH < 4)
				//	if constexpr (RGB)
				//		Store3(r1, g1, b1, (rgb_row2 + (x * 3)));
				//	else
				//		Store3(b1, g1, r1, (rgb_row2 + (x * 3)));
				//else
				//	if constexpr (RGB)
				//		Store4(r1, g1, b1, (rgb_row2 + (x * 4)));
				//	else
				//		Store4(b1, g1, r1, (rgb_row2 + (x * 4)));
			}
		}
	}

	inline void Convert(__m128i y_vals1, __m128i y_vals2, __m128i u_valsl, __m128i u_valsh, __m128i v_valsl, __m128i v_valsh,
		__m128i& r, __m128i& g, __m128i& b, __m128i& r1, __m128i& g1, __m128i& b1)
	{
		// Multiply UV with scaling coefficients
		__m128i u_vals_ugl = _mm_srai_epi16(_mm_mullo_epi16(u_valsl, u_to_g_coeff_vec), 6);
		__m128i u_vals_ubl = _mm_srai_epi16(_mm_mullo_epi16(u_valsl, u_to_b_coeff_vec), 6);
		__m128i v_vals_vgl = _mm_srai_epi16(_mm_mullo_epi16(v_valsl, v_to_g_coeff_vec), 6);
		__m128i v_vals_vrl = _mm_srai_epi16(_mm_mullo_epi16(v_valsl, v_to_r_coeff_vec), 6);

		__m128i u_vals_ugh = _mm_srai_epi16(_mm_mullo_epi16(u_valsh, u_to_g_coeff_vec), 6);
		__m128i u_vals_ubh = _mm_srai_epi16(_mm_mullo_epi16(u_valsh, u_to_b_coeff_vec), 6);
		__m128i v_vals_vgh = _mm_srai_epi16(_mm_mullo_epi16(v_valsh, v_to_g_coeff_vec), 6);
		__m128i v_vals_vrh = _mm_srai_epi16(_mm_mullo_epi16(v_valsh, v_to_r_coeff_vec), 6);

		//-16
		y_vals1 = _mm_subs_epu8(y_vals1, const_16);
		y_vals2 = _mm_subs_epu8(y_vals2, const_16);

		// 0 extend 16 to bit
		__m128i y_vals_16_1l = _mm_unpacklo_epi8(y_vals1, _mm_set1_epi8(0x00));
		__m128i y_vals_16_1h = _mm_unpackhi_epi8(y_vals1, _mm_set1_epi8(0x00));
		__m128i y_vals_16_2l = _mm_unpacklo_epi8(y_vals2, _mm_set1_epi8(0x00));
		__m128i y_vals_16_2h = _mm_unpackhi_epi8(y_vals2, _mm_set1_epi8(0x00));

		// Y*1.164
		y_vals_16_1l = _mm_srli_epi16(_mm_mullo_epi16(y_vals_16_1l, y_factor_vec), 7);
		y_vals_16_1h = _mm_srli_epi16(_mm_mullo_epi16(y_vals_16_1h, y_factor_vec), 7);
		y_vals_16_2l = _mm_srli_epi16(_mm_mullo_epi16(y_vals_16_2l, y_factor_vec), 7);
		y_vals_16_2h = _mm_srli_epi16(_mm_mullo_epi16(y_vals_16_2h, y_factor_vec), 7);
		/*
			  R = CLAMP((Y-16)*1.164 +           1.596*V)
			  G = CLAMP((Y-16)*1.164 - 0.391*U - 0.813*V)
			  B = CLAMP((Y-16)*1.164 + 2.018*U          )
		*/

		__m128i r1l = _mm_add_epi16(y_vals_16_1l, v_vals_vrl);
		__m128i g1l = _mm_sub_epi16(_mm_sub_epi16(y_vals_16_1l, u_vals_ugl), v_vals_vgl);
		__m128i b1l = _mm_add_epi16(y_vals_16_1l, u_vals_ubl);

		__m128i r1h = _mm_add_epi16(y_vals_16_1h, v_vals_vrh);
		__m128i g1h = _mm_sub_epi16(_mm_sub_epi16(y_vals_16_1h, u_vals_ugh), v_vals_vgh);
		__m128i b1h = _mm_add_epi16(y_vals_16_1h, u_vals_ubh);

		 r = _mm_packus_epi16(r1l, r1h);
		 g = _mm_packus_epi16(g1l, g1h);
		 b = _mm_packus_epi16(b1l, b1h);

		// Calculate RGB for second row
		__m128i r2l = _mm_add_epi16(y_vals_16_2l, v_vals_vrl);
		__m128i g2l = _mm_sub_epi16(_mm_sub_epi16(y_vals_16_2l, u_vals_ugl), v_vals_vgl);
		__m128i b2l = _mm_add_epi16(y_vals_16_2l, u_vals_ubl);

		__m128i r2h = _mm_add_epi16(y_vals_16_2h, v_vals_vrh);
		__m128i g2h = _mm_sub_epi16(_mm_sub_epi16(y_vals_16_2h, u_vals_ugh), v_vals_vgh);
		__m128i b2h = _mm_add_epi16(y_vals_16_2h, u_vals_ubh);

		 r1 = _mm_packus_epi16(r2l, r2h);
		 g1 = _mm_packus_epi16(g2l, g2h);
		 b1 = _mm_packus_epi16(b2l, b2h);
	}

	__m128i uvmask_sse = _mm_setr_epi8(0, 0, 1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 6, 6, 7, 7);

	inline void LoadAndUpscale(const uint8_t* plane, __m128i& low, __m128i& high)
	{
		// Load 8 UV values
		__m128i u = _mm_loadl_epi64((__m128i*)plane);
		__m128i ud = _mm_shuffle_epi8(u, uvmask_sse);

		// 0 extend
		auto udl = _mm_unpacklo_epi8(ud, _mm_set1_epi8(0x00));
		auto udh = _mm_unpackhi_epi8(ud, _mm_set1_epi8(0x00));

		low = _mm_sub_epi16(udl, _mm_set1_epi16(128));
		high = _mm_sub_epi16(udh, _mm_set1_epi16(128));
	}



	inline void Store3(__m128i c, __m128i b, __m128i a, uint8_t* ptr)
	{
		const __m128i sh_a = _mm_setr_epi8(0, 11, 6, 1, 12, 7, 2, 13, 8, 3, 14, 9, 4, 15, 10, 5);
		const __m128i sh_b = _mm_setr_epi8(5, 0, 11, 6, 1, 12, 7, 2, 13, 8, 3, 14, 9, 4, 15, 10);
		const __m128i sh_c = _mm_setr_epi8(10, 5, 0, 11, 6, 1, 12, 7, 2, 13, 8, 3, 14, 9, 4, 15);
		__m128i a0 = _mm_shuffle_epi8(a, sh_a);
		__m128i b0 = _mm_shuffle_epi8(b, sh_b);
		__m128i c0 = _mm_shuffle_epi8(c, sh_c);

		const __m128i m0 = _mm_setr_epi8(0, 0, -1, 0, 0, -1, 0, 0, -1, 0, 0, -1, 0, 0, -1, 0);
		const __m128i m1 = _mm_setr_epi8(0, -1, 0, 0, -1, 0, 0, -1, 0, 0, -1, 0, 0, -1, 0, 0);
		__m128i v0 = _mm_blendv_epi8(_mm_blendv_epi8(a0, b0, m1), c0, m0);
		__m128i v1 = _mm_blendv_epi8(_mm_blendv_epi8(b0, c0, m1), a0, m0);
		__m128i v2 = _mm_blendv_epi8(_mm_blendv_epi8(c0, a0, m1), b0, m0);

		_mm_storeu_si128((__m128i*)(ptr), v0);
		_mm_storeu_si128((__m128i*)(ptr + 16), v1);
		_mm_storeu_si128((__m128i*)(ptr + 32), v2);
	}
	inline void Store4(const __m128i& vec0, const __m128i& vec1, const __m128i& vec2, unsigned char* dst ) {
		__m128i ones = _mm_set1_epi32(-1);

		__m128i unpck_low_21 = _mm_unpacklo_epi8(vec2, vec1);
		__m128i unpck_high_21 = _mm_unpackhi_epi8(vec2, vec1);
		__m128i unpck_low_03 = _mm_unpacklo_epi8(vec0, ones);
		__m128i unpck_high_03 = _mm_unpackhi_epi8(vec0, ones);

		__m128i result0 = _mm_unpacklo_epi16(unpck_low_21, unpck_low_03);
		__m128i result1 = _mm_unpackhi_epi16(unpck_low_21, unpck_low_03);
		__m128i result2 = _mm_unpacklo_epi16(unpck_high_21, unpck_high_03);
		__m128i result3 = _mm_unpackhi_epi16(unpck_high_21, unpck_high_03);

		// Store results
		_mm_storeu_si128((__m128i*)(dst + 0), result0);
		_mm_storeu_si128((__m128i*)(dst + 16), result1);
		_mm_storeu_si128((__m128i*)(dst + 32), result2);
		_mm_storeu_si128((__m128i*)(dst + 48), result3);
	}
	

	template void Yuv2Rgb::yuv420_rgb24_sse<3, true>(
int32_t width,
		int32_t height,
		const uint8_t* Y,
		const uint8_t* U,
		const uint8_t* V,
		int32_t Y_stride,
		int32_t UV_stride,
		uint8_t* Rgb,
		int32_t RGB_stride,
		int32_t numThreads);

	template void Yuv2Rgb::yuv420_rgb24_sse<4, true>(
		int32_t width,
		int32_t height,
		const uint8_t* Y,
		const uint8_t* U,
		const uint8_t* V,
		int32_t Y_stride,
		int32_t UV_stride,
		uint8_t* Rgb,
		int32_t RGB_stride,
		int32_t numThreads);

	template void Yuv2Rgb::yuv420_rgb24_sse<3, false>(
		int32_t width,
		int32_t height,
		const uint8_t* Y,
		const uint8_t* U,
		const uint8_t* V,
		int32_t Y_stride,
		int32_t UV_stride,
		uint8_t* Rgb,
		int32_t RGB_stride,
		int32_t numThreads);

	template void Yuv2Rgb::yuv420_rgb24_sse<4, false>(
		int32_t width,
		int32_t height,
		const uint8_t* Y,
		const uint8_t* U,
		const uint8_t* V,
		int32_t Y_stride,
		int32_t UV_stride,
		uint8_t* Rgb,
		int32_t RGB_stride,
		int32_t numThreads);

}
#endif


