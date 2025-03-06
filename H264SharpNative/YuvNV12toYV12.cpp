
#include "pch.h"
#include "Converter.h"
#include <stdint.h>
#include <cstdint>

namespace H264Sharp
{

	inline void Deinterleave(uint8_t* UV,int widthUV, int heightUV, int srcUVStride,
					uint8_t* U, uint8_t*V)
	{
		int idxU = 0, idxV = 0;

		for (int h = 0; h < heightUV; h++)
		{
			for (int w = 0; w < widthUV; w += 2) 
			{  
				U[idxU++] = UV[(h * srcUVStride) + w];
				V[idxV++] = UV[(h * srcUVStride) + w + 1];
			}
		}
	}
#if defined(__arm__)

#include <arm_neon.h>

	inline void DeinterleaveNEON(uint8_t* UV, int widthUV, int heightUV, int srcUVStride,
		uint8_t* U, uint8_t* V)
	{
		for (size_t h = 0; h < heightUV; h++)
		{
			for (int w = 0; w < widthUV; w += 32)
			{
				uint8x16x2_t uv = vld2q_u8(UV + (h * srcUVStride) + w);
				vst1q_u8(U + (h * (widthUV / 2)) + (w / 2), uv.val[0]);
				vst1q_u8(V + (h * (widthUV / 2)) + (w / 2), uv.val[1]);
			}
		}

	}

#else

#include <immintrin.h>
#include <smmintrin.h>
#include <emmintrin.h>


	inline void DeinterleaveAVX(uint8_t* UV, int widthUV, int heightUV, int srcUVStride,
					uint8_t* U, uint8_t* V)
	{

		for (size_t h = 0; h < heightUV; h++)
		{
			for (int w = 0; w < widthUV; w += 64)
			{
				__m256i uv1 = _mm256_loadu_si256((const __m256i*)(UV + (h * srcUVStride) + w));
				__m256i uv2 = _mm256_loadu_si256((const __m256i*)(UV + (h * srcUVStride) + w + 32));

				const __m256i sh = _mm256_setr_epi8(0, 2, 4, 6, 8, 10, 12, 14, 1, 3, 5, 7, 9, 11, 13, 15,
					0, 2, 4, 6, 8, 10, 12, 14, 1, 3, 5, 7, 9, 11, 13, 15);
				__m256i p0 = _mm256_shuffle_epi8(uv1, sh);
				__m256i p1 = _mm256_shuffle_epi8(uv2, sh);
				__m256i pl = _mm256_permute2x128_si256(p0, p1, 0 + 2 * 16);
				__m256i ph = _mm256_permute2x128_si256(p0, p1, 1 + 3 * 16);
				__m256i u = _mm256_unpacklo_epi64(pl, ph);
				__m256i v = _mm256_unpackhi_epi64(pl, ph);

				_mm256_storeu_si256((__m256i*)(U + (h * (widthUV / 2)) + (w / 2)), u);
				_mm256_storeu_si256((__m256i*)(V + (h * (widthUV / 2)) + (w / 2)), v);
			}
		}

	}

	inline void DeinterleaveSSE(uint8_t* UV, int widthUV, int heightUV, int srcUVStride,
		uint8_t* U, uint8_t* V)
	{
		for (size_t h = 0; h < heightUV; h++)
		{
			for (int w = 0; w < widthUV; w += 32)
			{
				__m128i uv0 = _mm_loadu_si128((const __m128i*)(UV + (h * srcUVStride) + w));
				__m128i uv1 = _mm_loadu_si128((const __m128i*)(UV + (h * srcUVStride) + w + 16));

				
				__m128i t10 = _mm_unpacklo_epi8(uv0, uv1);
				__m128i t11 = _mm_unpackhi_epi8(uv0, uv1);

				__m128i t20 = _mm_unpacklo_epi8(t10, t11);
				__m128i t21 = _mm_unpackhi_epi8(t10, t11);

				__m128i t30 = _mm_unpacklo_epi8(t20, t21);
				__m128i t31 = _mm_unpackhi_epi8(t20, t21);

				__m128i u = _mm_unpacklo_epi8(t30, t31);
				__m128i v = _mm_unpackhi_epi8(t30, t31);

				_mm_storeu_si128((__m128i*)(U +(h* (widthUV / 2)) +  (w / 2)), u);
				_mm_storeu_si128((__m128i*)(V + (h * (widthUV / 2)) + (w / 2)), v);
			}
		}

	}

#endif


	void Converter::Yuv_NV12ToYV12(const YuvNV12Native& from, YuvNative& to, uint8_t* buffer)
	{
		to.Y = from.Y; 
		to.U = buffer;
		to.V = buffer + (from.height * from.width) / 4; 

		to.height = from.height;
		to.width = from.width;
		to.yStride = from.width;
		to.uvStride = from.width / 2;
		

#if defined(__arm__)
		if (Converter::Config.EnableNeon)
		{
			DeinterleaveNEON(from.UV, from.width, from.height / 2, from.uvStride, to.U, to.V);
		}
#else

		if (Converter::Config.EnableAvx2) 
		{
			DeinterleaveAVX(from.UV, from.width, from.height / 2, from.uvStride, to.U, to.V);
		}
		else if(Converter::Config.EnableSSE)
		{
			DeinterleaveSSE(from.UV, from.width, from.height / 2, from.uvStride, to.U, to.V);
		}
#endif
		else 
		{
			Deinterleave(from.UV, from.width, from.height / 2, from.uvStride, to.U, to.V);
		}
		

	}

	void Converter::Yuv_NV12ToYV12(const YuvNV12Native& from, YuvNative& to)
	{
		memcpy(to.Y, from.Y, (from.height * from.width));
		to.U = to.Y+(from.height * from.width);
		to.V = to.U + (from.height * from.width) / 4;

		to.height = from.height;
		to.width = from.width;
		to.yStride = from.width;
		to.uvStride = from.width / 2;


#if defined(__arm__)
		if (Converter::Config.EnableNeon)
		{
			DeinterleaveNEON(from.UV, from.width, from.height / 2, from.uvStride, to.U, to.V);
		}
#else

		if (Converter::Config.EnableAvx2)
		{
			DeinterleaveAVX(from.UV, from.width, from.height / 2, from.uvStride, to.U, to.V);
		}
		else if (Converter::Config.EnableSSE)
		{
			DeinterleaveSSE(from.UV, from.width, from.height / 2, from.uvStride, to.U, to.V);
		}
#endif
		else
		{
			Deinterleave(from.UV, from.width, from.height / 2, from.uvStride, to.U, to.V);
		}


	}

}

