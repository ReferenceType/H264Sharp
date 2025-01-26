#include "Yuv2Rgb.h"
#ifndef __arm__
namespace H264Sharp
{
	
	typedef enum
	{
		YCBCR_JPEG,
		YCBCR_601,
		YCBCR_709,
		YCBCR_2020
	} YCbCrType;


	typedef struct
	{
		uint8_t cb_factor;   // [(255*CbNorm)/CbRange]
		uint8_t cr_factor;   // [(255*CrNorm)/CrRange]
		uint8_t g_cb_factor; // [Bf/Gf*(255*CbNorm)/CbRange]
		uint8_t g_cr_factor; // [Rf/Gf*(255*CrNorm)/CrRange]
		uint8_t y_factor;    // [(YMax-YMin)/255]
		uint8_t y_offset;    // YMin
	} YUV2RGBParam;
	#define FIXED_POINT_VALUE(value, precision) ((int)(((value)*(1<<precision))+0.5))

	#define YUV2RGB_PARAM(Rf, Bf, YMin, YMax, CbCrRange) \
	{.cb_factor=FIXED_POINT_VALUE(255.0*(2.0*(1-Bf))/CbCrRange, 6), \
	.cr_factor=FIXED_POINT_VALUE(255.0*(2.0*(1-Rf))/CbCrRange, 6), \
	.g_cb_factor=FIXED_POINT_VALUE(Bf/(1.0-Bf-Rf)*255.0*(2.0*(1-Bf))/CbCrRange, 7), \
	.g_cr_factor=FIXED_POINT_VALUE(Rf/(1.0-Bf-Rf)*255.0*(2.0*(1-Rf))/CbCrRange, 7), \
	.y_factor=FIXED_POINT_VALUE(255.0/(YMax-YMin), 7), \
	.y_offset=YMin}


	static const YUV2RGBParam YUV2RGB[4] = {
		// ITU-T T.871 (JPEG)
		// ITU-T T.871 (JPEG)
		YUV2RGB_PARAM(0.299, 0.114, 0, 255.0, 255.0),
		// ITU-R BT.601-7
		YUV2RGB_PARAM(0.299, 0.114, 16, 235.0, 224.0),
		// ITU-R BT.709-6
		YUV2RGB_PARAM(0.2126, 0.0722, 16, 235.0, 224.0),
		// ITU-R BT.2020
		YUV2RGB_PARAM(0.2627, 0.0593, 0, 255, 224)
	};
	const YUV2RGBParam* const param = &(YUV2RGB[1]);
	#define LOAD_UV_PLANAR \
		__m128i u = LOAD_SI128((const __m128i*)(u_ptr)); \
		__m128i v = LOAD_SI128((const __m128i*)(v_ptr)); \

	#define UV2RGB_16(U,V,B1,G1,R1,B2,G2,R2) \
		r_tmp = _mm_srai_epi16(_mm_mullo_epi16(V, _mm_set1_epi16(param->cr_factor)), 6); \
		g_tmp = _mm_srai_epi16(_mm_add_epi16( \
			_mm_mullo_epi16(U, _mm_set1_epi16(param->g_cb_factor)), \
			_mm_mullo_epi16(V, _mm_set1_epi16(param->g_cr_factor))), 7); \
		b_tmp = _mm_srai_epi16(_mm_mullo_epi16(U, _mm_set1_epi16(param->cb_factor)), 6); \
		R1 = _mm_unpacklo_epi16(r_tmp, r_tmp); \
		G1 = _mm_unpacklo_epi16(g_tmp, g_tmp); \
		B1 = _mm_unpacklo_epi16(b_tmp, b_tmp); \
		R2 = _mm_unpackhi_epi16(r_tmp, r_tmp); \
		G2 = _mm_unpackhi_epi16(g_tmp, g_tmp); \
		B2 = _mm_unpackhi_epi16(b_tmp, b_tmp); \


	#define ADD_Y2RGB_16(Y1,Y2,B1,G1,R1,B2,G2,R2) \
		Y1 = _mm_srli_epi16(_mm_mullo_epi16(Y1, _mm_set1_epi16(param->y_factor)), 7); \
		Y2 = _mm_srli_epi16(_mm_mullo_epi16(Y2, _mm_set1_epi16(param->y_factor)), 7); \
		\
		R1 = _mm_add_epi16(Y1, R1); \
		G1 = _mm_sub_epi16(Y1, G1); \
		B1 = _mm_add_epi16(Y1, B1); \
		R2 = _mm_add_epi16(Y2, R2); \
		G2 = _mm_sub_epi16(Y2, G2); \
		B2 = _mm_add_epi16(Y2, B2); \

	#define PACK_RGB24_32(R1, R2, G1, G2, B1, B2, RGB1, RGB2, RGB3, RGB4, RGB5, RGB6) \
	PACK_RGB24_32_STEP(R1, R2, G1, G2, B1, B2, RGB1, RGB2, RGB3, RGB4, RGB5, RGB6) \
	PACK_RGB24_32_STEP(RGB1, RGB2, RGB3, RGB4, RGB5, RGB6, R1, R2, G1, G2, B1, B2) \
	PACK_RGB24_32_STEP(R1, R2, G1, G2, B1, B2, RGB1, RGB2, RGB3, RGB4, RGB5, RGB6) \
	PACK_RGB24_32_STEP(RGB1, RGB2, RGB3, RGB4, RGB5, RGB6, R1, R2, G1, G2, B1, B2) \
	PACK_RGB24_32_STEP(R1, R2, G1, G2, B1, B2, RGB1, RGB2, RGB3, RGB4, RGB5, RGB6) \


	#define PACK_RGB24_32_STEP(RS1, RS2, RS3, RS4, RS5, RS6, RD1, RD2, RD3, RD4, RD5, RD6) \
	RD1 = _mm_packus_epi16(_mm_and_si128(RS1,_mm_set1_epi16(0xFF)), _mm_and_si128(RS2,_mm_set1_epi16(0xFF))); \
	RD2 = _mm_packus_epi16(_mm_and_si128(RS3,_mm_set1_epi16(0xFF)), _mm_and_si128(RS4,_mm_set1_epi16(0xFF))); \
	RD3 = _mm_packus_epi16(_mm_and_si128(RS5,_mm_set1_epi16(0xFF)), _mm_and_si128(RS6,_mm_set1_epi16(0xFF))); \
	RD4 = _mm_packus_epi16(_mm_srli_epi16(RS1,8), _mm_srli_epi16(RS2,8)); \
	RD5 = _mm_packus_epi16(_mm_srli_epi16(RS3,8), _mm_srli_epi16(RS4,8)); \
	RD6 = _mm_packus_epi16(_mm_srli_epi16(RS5,8), _mm_srli_epi16(RS6,8)); \


	#define YUV2RGB_32 \
		__m128i r_tmp, g_tmp, b_tmp; \
		__m128i r_16_1, g_16_1, b_16_1, r_16_2, g_16_2, b_16_2; \
		__m128i r_uv_16_1, g_uv_16_1, b_uv_16_1, r_uv_16_2, g_uv_16_2, b_uv_16_2; \
		__m128i y_16_1, y_16_2; \
		\
		u = _mm_add_epi8(u, _mm_set1_epi8(-128)); \
		v = _mm_add_epi8(v, _mm_set1_epi8(-128)); \
		\
		/* process first 16 pixels of first line */\
		__m128i u_16 = _mm_srai_epi16(_mm_unpacklo_epi8(u, u), 8); \
		__m128i v_16 = _mm_srai_epi16(_mm_unpacklo_epi8(v, v), 8); \
		\
		UV2RGB_16(u_16, v_16, r_uv_16_1, g_uv_16_1, b_uv_16_1, r_uv_16_2, g_uv_16_2, b_uv_16_2) \
		r_16_1=r_uv_16_1; g_16_1=g_uv_16_1; b_16_1=b_uv_16_1; \
		r_16_2=r_uv_16_2; g_16_2=g_uv_16_2; b_16_2=b_uv_16_2; \
		\
		__m128i y = LOAD_SI128((const __m128i*)(y_ptr1)); \
		y = _mm_subs_epu8(y, _mm_set1_epi8(param->y_offset)); \
		y_16_1 = _mm_unpacklo_epi8(y, _mm_setzero_si128()); \
		y_16_2 = _mm_unpackhi_epi8(y, _mm_setzero_si128()); \
		\
		ADD_Y2RGB_16(y_16_1, y_16_2, r_16_1, g_16_1, b_16_1, r_16_2, g_16_2, b_16_2) \
		\
		__m128i r_8_11 = _mm_packus_epi16(r_16_1, r_16_2); \
		__m128i g_8_11 = _mm_packus_epi16(g_16_1, g_16_2); \
		__m128i b_8_11 = _mm_packus_epi16(b_16_1, b_16_2); \
		\
		/* process first 16 pixels of second line */\
		r_16_1=r_uv_16_1; g_16_1=g_uv_16_1; b_16_1=b_uv_16_1; \
		r_16_2=r_uv_16_2; g_16_2=g_uv_16_2; b_16_2=b_uv_16_2; \
		\
		y = LOAD_SI128((const __m128i*)(y_ptr2)); \
		y = _mm_subs_epu8(y, _mm_set1_epi8(param->y_offset)); \
		y_16_1 = _mm_unpacklo_epi8(y, _mm_setzero_si128()); \
		y_16_2 = _mm_unpackhi_epi8(y, _mm_setzero_si128()); \
		\
		ADD_Y2RGB_16(y_16_1, y_16_2, r_16_1, g_16_1, b_16_1, r_16_2, g_16_2, b_16_2) \
		\
		__m128i r_8_21 = _mm_packus_epi16(r_16_1, r_16_2); \
		__m128i g_8_21 = _mm_packus_epi16(g_16_1, g_16_2); \
		__m128i b_8_21 = _mm_packus_epi16(b_16_1, b_16_2); \
		\
		/* process last 16 pixels of first line */\
		u_16 = _mm_srai_epi16(_mm_unpackhi_epi8(u, u), 8); \
		v_16 = _mm_srai_epi16(_mm_unpackhi_epi8(v, v), 8); \
		\
		UV2RGB_16(u_16, v_16, r_uv_16_1, g_uv_16_1, b_uv_16_1, r_uv_16_2, g_uv_16_2, b_uv_16_2) \
		r_16_1=r_uv_16_1; g_16_1=g_uv_16_1; b_16_1=b_uv_16_1; \
		r_16_2=r_uv_16_2; g_16_2=g_uv_16_2; b_16_2=b_uv_16_2; \
		\
		y = LOAD_SI128((const __m128i*)(y_ptr1+16)); \
		y = _mm_subs_epu8(y, _mm_set1_epi8(param->y_offset)); \
		y_16_1 = _mm_unpacklo_epi8(y, _mm_setzero_si128()); \
		y_16_2 = _mm_unpackhi_epi8(y, _mm_setzero_si128()); \
		\
		ADD_Y2RGB_16(y_16_1, y_16_2, r_16_1, g_16_1, b_16_1, r_16_2, g_16_2, b_16_2) \
		\
		__m128i r_8_12 = _mm_packus_epi16(r_16_1, r_16_2); \
		__m128i g_8_12 = _mm_packus_epi16(g_16_1, g_16_2); \
		__m128i b_8_12 = _mm_packus_epi16(b_16_1, b_16_2); \
		\
		/* process last 16 pixels of second line */\
		r_16_1=r_uv_16_1; g_16_1=g_uv_16_1; b_16_1=b_uv_16_1; \
		r_16_2=r_uv_16_2; g_16_2=g_uv_16_2; b_16_2=b_uv_16_2; \
		\
		y = LOAD_SI128((const __m128i*)(y_ptr2+16)); \
		y = _mm_subs_epu8(y, _mm_set1_epi8(param->y_offset)); \
		y_16_1 = _mm_unpacklo_epi8(y, _mm_setzero_si128()); \
		y_16_2 = _mm_unpackhi_epi8(y, _mm_setzero_si128()); \
		\
		ADD_Y2RGB_16(y_16_1, y_16_2, r_16_1, g_16_1, b_16_1, r_16_2, g_16_2, b_16_2) \
		\
		__m128i r_8_22 = _mm_packus_epi16(r_16_1, r_16_2); \
		__m128i g_8_22 = _mm_packus_epi16(g_16_1, g_16_2); \
		__m128i b_8_22 = _mm_packus_epi16(b_16_1, b_16_2); \
		\
		__m128i rgb_1, rgb_2, rgb_3, rgb_4, rgb_5, rgb_6; \
		\
		PACK_RGB24_32(r_8_11, r_8_12, g_8_11, g_8_12, b_8_11, b_8_12, rgb_1, rgb_2, rgb_3, rgb_4, rgb_5, rgb_6) \
		SAVE_SI128((__m128i*)(rgb_ptr1), rgb_1); \
		SAVE_SI128((__m128i*)(rgb_ptr1+16), rgb_2); \
		SAVE_SI128((__m128i*)(rgb_ptr1+32), rgb_3); \
		SAVE_SI128((__m128i*)(rgb_ptr1+48), rgb_4); \
		SAVE_SI128((__m128i*)(rgb_ptr1+64), rgb_5); \
		SAVE_SI128((__m128i*)(rgb_ptr1+80), rgb_6); \
		\
		PACK_RGB24_32(r_8_21, r_8_22, g_8_21, g_8_22, b_8_21, b_8_22, rgb_1, rgb_2, rgb_3, rgb_4, rgb_5, rgb_6) \
		SAVE_SI128((__m128i*)(rgb_ptr2), rgb_1); \
		SAVE_SI128((__m128i*)(rgb_ptr2+16), rgb_2); \
		SAVE_SI128((__m128i*)(rgb_ptr2+32), rgb_3); \
		SAVE_SI128((__m128i*)(rgb_ptr2+48), rgb_4); \
		SAVE_SI128((__m128i*)(rgb_ptr2+64), rgb_5); \
		SAVE_SI128((__m128i*)(rgb_ptr2+80), rgb_6); \

	int i = 0;
	#define YUV2RGB_32_PLANAR \
		LOAD_UV_PLANAR \
		YUV2RGB_32

	inline void Yuv2BgrSSE_ParallelBody(uint32_t y,
		uint32_t width,
		uint32_t height,
		const uint8_t* Y,
		const uint8_t* U,
		const uint8_t* V,
		uint32_t Y_stride,
		uint32_t UV_stride,
		uint8_t* RGB,
		uint32_t RGB_stride);

	void Yuv2Rgb::yuv420_rgb24_sse(YuvNative& yuv, unsigned char* dest, int numThreads)
	{
		
		if (numThreads > 1)
		{
			ThreadPool::For(0, numThreads, [&yuv,&dest, numThreads](int j)
				{
					

					int hi = (yuv.height / 2);
					int bgn = (hi / numThreads) * (j);
					int end = (hi / numThreads) * (j + 1);
					if (j == numThreads - 1)
					{
						end = hi;
					}

					for (int i = bgn; i < end; i++)
					{
						j = i * 2;
						if (j == yuv.height - 1)
							return;
						Yuv2BgrSSE_ParallelBody(j, yuv.width, yuv.height,
							yuv.Y, yuv.U, yuv.V, yuv.yStride, yuv.uvStride,
							dest, yuv.width*3);
					}
				});


		}
		else
		{
#define LOAD_SI128 _mm_load_si128
#define SAVE_SI128 _mm_stream_si128

			uint32_t width = yuv.width;
			uint32_t height = yuv.height;
			const uint8_t* Y = yuv.Y;
			const uint8_t* U = yuv.U;
			const uint8_t* V = yuv.V;
			uint32_t Y_stride = yuv.yStride;
			uint32_t UV_stride = yuv.uvStride;
			uint8_t* RGB = dest;
			uint32_t RGB_stride = yuv.width*3;

			uint32_t y;
			for (y = 0; y < (height - 1); y += 2)
			{
				const uint8_t* y_ptr1 = Y + y * Y_stride,
					* y_ptr2 = Y + (y + 1) * Y_stride,
					* u_ptr = U + (y / 2) * UV_stride,
					* v_ptr = V + (y / 2) * UV_stride;

				uint8_t* rgb_ptr1 = RGB + y * RGB_stride,
					* rgb_ptr2 = RGB + (y + 1) * RGB_stride;

				uint32_t x;
				for (x = 0; x < (width - 31); x += 32)
				{
					YUV2RGB_32_PLANAR

						y_ptr1 += 32;
					y_ptr2 += 32;
					u_ptr += 16;
					v_ptr += 16;
					rgb_ptr1 += 96;
					rgb_ptr2 += 96;
				}
			}
#undef LOAD_SI128
#undef SAVE_SI128
		}
	}


	void Yuv2Rgb::yuv420_rgb24_sse(uint32_t width,
		uint32_t height,
		const uint8_t* Y,
		const uint8_t* U,
		const uint8_t* V,
		uint32_t Y_stride,
		uint32_t UV_stride,
		uint8_t* RGB,
		uint32_t RGB_stride,
		int numThreads)
	{

		if (numThreads > 1)
		{

			ThreadPool::For(0, numThreads, [width, height, Y, U, V, Y_stride, UV_stride, RGB, RGB_stride, numThreads](int j)
				{
					int hi = (height / 2);
					int bgn = (hi / numThreads) * (j);
					int end = (hi / numThreads) * (j + 1);
					if (j == numThreads - 1)
					{
						end = hi;
					}

					for (int i = bgn; i < end; i++)
					{
						j = i * 2;
						if (j == height - 1)
							return;
						Yuv2BgrSSE_ParallelBody(j, width, height,
							Y, U, V, Y_stride, UV_stride,
							RGB, RGB_stride);
					}
				});


		}
		else
		{
	#define LOAD_SI128 _mm_load_si128
	#define SAVE_SI128 _mm_stream_si128
			uint32_t y;
			for (y = 0; y < (height - 1); y += 2)
			{
				const uint8_t* y_ptr1 = Y + y * Y_stride,
					* y_ptr2 = Y + (y + 1) * Y_stride,
					* u_ptr = U + (y / 2) * UV_stride,
					* v_ptr = V + (y / 2) * UV_stride;

				uint8_t* rgb_ptr1 = RGB + y * RGB_stride,
					* rgb_ptr2 = RGB + (y + 1) * RGB_stride;

				uint32_t x;
				for (x = 0; x < (width - 31); x += 32)
				{
					YUV2RGB_32_PLANAR

						y_ptr1 += 32;
					y_ptr2 += 32;
					u_ptr += 16;
					v_ptr += 16;
					rgb_ptr1 += 96;
					rgb_ptr2 += 96;
				}
			}
	#undef LOAD_SI128
	#undef SAVE_SI128
		}

	}

	


	inline void Yuv2BgrSSE_ParallelBody(uint32_t y,
		uint32_t width,
		uint32_t height,
		const uint8_t* Y,
		const uint8_t* U,
		const uint8_t* V,
		uint32_t Y_stride,
		uint32_t UV_stride,
		uint8_t* RGB,
		uint32_t RGB_stride)
	{
	#define LOAD_SI128 _mm_load_si128
	#define SAVE_SI128 _mm_stream_si128
		//const YUV2RGBParam* const param = &(YUV2RGB[yuv_type]);

		const uint8_t* y_ptr1 = Y + y * Y_stride,
			* y_ptr2 = Y + (y + 1) * Y_stride,
			* u_ptr = U + (y / 2) * UV_stride,
			* v_ptr = V + (y / 2) * UV_stride;

		uint8_t* rgb_ptr1 = RGB + y * RGB_stride,
			* rgb_ptr2 = RGB + (y + 1) * RGB_stride;

		for (int x = 0; x < (width - 31); x += 32)
		{
			YUV2RGB_32_PLANAR

				y_ptr1 += 32;
			y_ptr2 += 32;
			u_ptr += 16;
			v_ptr += 16;
			rgb_ptr1 += 96;
			rgb_ptr2 += 96;
		}
	#undef LOAD_SI128
	#undef SAVE_SI128
	}
}
#endif // !__arm__
