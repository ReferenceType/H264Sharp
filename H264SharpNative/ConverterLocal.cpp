#include "ConverterLocal.h"

// TODO
/*
* parallelisation for linux(openmp?)
* ARM intrinsics
*/
enum
{
    FLAGS = 0x40080100
};

#ifdef _WIN32
Concurrency::affinity_partitioner affin;
Concurrency::affinity_partitioner affin2;
#endif

void Yuv2Rgb(unsigned char* dst_ptr,
    const unsigned char* y_ptr,
    const unsigned char* u_ptr,
    const unsigned char* v_ptr,
    signed   int   width,
    signed   int   height,
    signed   int   y_span,
    signed   int   uv_span,
    signed   int   dst_span,

    int numThreads);
void Yuv2RgbNaive_PB(int k, unsigned char* dst_ptr,
    const unsigned char* y_ptr,
    const unsigned char* u_ptr,
    const unsigned char* v_ptr,
    signed   int   width,
    signed   int   height,
    signed   int   y_span,
    signed   int   uv_span,
    signed   int   dst_span);

#define READUV(U,V) (yuv2rgb565_table1[256 + (U)] + yuv2rgb565_table1[512 + (V)])
#define READY(Y)    yuv2rgb565_table1[Y]
#define FIXUP(Y)                 \
do {                             \
    int tmp = (Y) & FLAGS;       \
    if (tmp != 0)                \
    {                            \
        tmp  -= tmp>>8;          \
        (Y)  |= tmp;             \
        tmp   = FLAGS & ~(Y>>1); \
        (Y)  += tmp>>8;          \
    }                            \
} while (0 == 1)

#define STORE(Y,DSTPTR)           \
do {                              \
    unsigned int Y2       = (Y);      \
    unsigned char  *DSTPTR2 = (DSTPTR); \
    (DSTPTR2)[0] = (Y2);          \
    (DSTPTR2)[1] = (Y2)>>22;      \
    (DSTPTR2)[2] = (Y2)>>11;      \
} while (0 == 1)

void Yuv420P2RGB(unsigned char* dst_ptr,
    const unsigned char* y_ptr,
    const unsigned char* u_ptr,
    const unsigned char* v_ptr,
    signed   int   width,
    signed   int   height,
    signed   int   y_span,
    signed   int   uv_span,
    signed   int   dst_span,
    bool useSSE,
    int numThreads)
{
    if (useSSE && width % 32 == 0)
    {
#ifndef __arm__

        Yuv2Rgb(dst_ptr,
            y_ptr,
            u_ptr,
            v_ptr,
            width,
            height,
            y_span,
            uv_span,
            dst_span, numThreads);
        return;
#else
        for (size_t i = 0; i < height / 2; i++) {
            [[clang::always_inline]] Yuv2RgbNaive_PB(i, dst_ptr,
                y_ptr,
                u_ptr,
                v_ptr,
                width,
                height,
                y_span,
                uv_span,
                dst_span);

        }
#endif

    }
    else
    {
        if (width * height > 1280 * 720 && numThreads > 1)
        {
#ifdef _WIN32

            Concurrency::parallel_for(0, numThreads, [&](int j)
                {
                    int hi = height / 2;
                    int bgn = ((hi / numThreads) * (j));
                    int end = ((hi / numThreads) * (j + 1));
                    if (j == numThreads - 1)
                    {
                        end = hi;
                    }

                    for (int i = bgn; i < end; i++)
                    {
                        [[clang::always_inline]] Yuv2RgbNaive_PB(i, dst_ptr,
                            y_ptr,
                            u_ptr,
                            v_ptr,
                            width,
                            height,
                            y_span,
                            uv_span,
                            dst_span);

                    }
                }, affin2);
#else
            for (size_t i = 0; i < height / 2; i++) {
                [[clang::always_inline]] Yuv2RgbNaive_PB(i, dst_ptr,
                    y_ptr,
                    u_ptr,
                    v_ptr,
                    width,
                    height,
                    y_span,
                    uv_span,
                    dst_span);
            }
#endif
        }
        else {
            for (size_t i = 0; i < height / 2; i++) {
                [[clang::always_inline]] Yuv2RgbNaive_PB(i, dst_ptr,
                    y_ptr,
                    u_ptr,
                    v_ptr,
                    width,
                    height,
                    y_span,
                    uv_span,
                    dst_span);
            }

        }
    }


};

inline void Yuv2RgbNaive_PB(int k, unsigned char* dst_ptr,
    const unsigned char* y_ptr,
    const unsigned char* u_ptr,
    const unsigned char* v_ptr,
    signed   int   width,
    signed   int   height,
    signed   int   y_span,
    signed   int   uv_span,
    signed   int   dst_span)
{
    auto dst_ptr1 = dst_ptr + (dst_span * 2) * k;
    auto y_ptr1 = y_ptr + (y_span * 2) * k;
    auto u_ptr1 = u_ptr + (uv_span)*k;
    auto v_ptr1 = v_ptr + (uv_span)*k;

#pragma clang loop vectorize(assume_safety)
    for (size_t j = 0; j < width / 2; j++)
    {
        /* Do 2 column pairs */
        unsigned int uv, y0, y1;

        uv = READUV(*u_ptr1++, *v_ptr1++);
        y1 = uv + READY(y_ptr1[y_span]);
        y0 = uv + READY(*y_ptr1++);
        FIXUP(y1);
        FIXUP(y0);
        STORE(y1, &dst_ptr1[dst_span]);
        STORE(y0, dst_ptr1);
        dst_ptr1 += 3;
        y1 = uv + READY(y_ptr1[y_span]);
        y0 = uv + READY(*y_ptr1++);
        FIXUP(y1);
        FIXUP(y0);
        STORE(y1, &dst_ptr1[dst_span]);
        STORE(y0, dst_ptr1);
        dst_ptr1 += 3;

        //uptr 1 vptr 1 yptr 2 dst 6
    }

}

//------------SSE--------------------
#ifndef __arm__

typedef enum
{
    YCBCR_JPEG,
    YCBCR_601,
    YCBCR_709,
    YCBCR_2020
} YCbCrType;
void yuv420_rgb24_sse(
    uint32_t width, uint32_t height,
    const uint8_t* Y, const uint8_t* U, const uint8_t* V, uint32_t Y_stride, uint32_t UV_stride,
    uint8_t* RGB, uint32_t RGB_stride, int numThreads);
void Yuv2Rgb(unsigned char* dst_ptr,
    const unsigned char* y_ptr,
    const unsigned char* u_ptr,
    const unsigned char* v_ptr,
    signed   int   width,
    signed   int   height,
    signed   int   y_span,
    signed   int   uv_span,
    signed   int   dst_span,

    int numThreads)
{


    yuv420_rgb24_sse(
        width, height,
        y_ptr, u_ptr, v_ptr, y_span, uv_span,
        dst_ptr, dst_span,
        numThreads);
}

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




inline void Yuv2Bgr_ParallelBody(uint32_t y, uint32_t width, uint32_t height,
    const uint8_t* Y, const uint8_t* U, const uint8_t* V, uint32_t Y_stride, uint32_t UV_stride,
    uint8_t* RGB, uint32_t RGB_stride);


void yuv420_rgb24_sse(
    uint32_t width, uint32_t height,
    const uint8_t* Y, const uint8_t* U, const uint8_t* V, uint32_t Y_stride, uint32_t UV_stride,
    uint8_t* RGB, uint32_t RGB_stride, int numThreads)
{
    
    if (numThreads > 1 && (width * height > 1280 * 720)) {
#ifdef _WIN32

        concurrency::parallel_for(int(0), numThreads, [&width, &height, &Y, &U, &V, &Y_stride, &UV_stride, &RGB, &RGB_stride, &numThreads](int j)
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
                    Yuv2Bgr_ParallelBody(j, width, height,
                        Y, U, V, Y_stride, UV_stride,
                        RGB, RGB_stride);
                }
            });
#else
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
#endif

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


inline void Yuv2Bgr_ParallelBody(uint32_t y, uint32_t width, uint32_t height,
    const uint8_t* Y, const uint8_t* U, const uint8_t* V, uint32_t Y_stride, uint32_t UV_stride,
    uint8_t* RGB, uint32_t RGB_stride)
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
#endif // !__arm__
//----------END SSE------------------

#pragma endregion


//------------------------------------BRRx->Yuv-------------------------------------------------
inline void BGRA2YUVP_ParallelBody(const unsigned char* bgra, unsigned char* dst, const int width, const int height, const int stride, const int begin)
{
    const int wi = width / 2;
    const int uvlineBegin = begin * wi;
    const int yPlaneSize = width * height;

    int vIndex = yPlaneSize + uvlineBegin;
    int uIndex = yPlaneSize + (yPlaneSize / 4) + (uvlineBegin);
    const int readBegin = 2 * stride * begin;
    int index = readBegin;
    int yIndex = width * 2 * begin;

    unsigned char* buffer = dst;
    //  for (int j = begin; j < end; j++)
      //{
#pragma clang loop unroll_count(2)
#pragma clang loop vectorize(assume_safety)
#pragma clang loop vectorize_width(32) interleave_count(1)
#pragma clang loop vectorize_predicate(enable)

    for (int i = 0; i < wi; ++i)
    {
        const auto b = bgra[index++];
        const auto g = bgra[index++];
        const auto r = bgra[index++];
        index++;

        const auto b1 = bgra[index++];
        const auto g1 = bgra[index++];
        const auto r1 = bgra[index++];
        index++;

        buffer[yIndex++] = ((25 * b + 129 * g + 66 * r) >> 8) + 16;
        buffer[yIndex++] = ((25 * b1 + 129 * g1 + 66 * r1) >> 8) + 16;

        buffer[uIndex++] = ((112 * r + -94 * g + -18 * b) >> 8) + 128;
        buffer[vIndex++] = ((-38 * r + -74 * g + 112 * b) >> 8) + 128;
    }

    int indexNext = (readBegin)+(stride);
#pragma clang loop unroll_count(2)
#pragma clang loop vectorize(assume_safety)
#pragma clang loop vectorize_width(32) interleave_count(1)
    for (int i = 0; i < width; ++i)
    {
        const auto b2 = bgra[indexNext++];
        const auto g2 = bgra[indexNext++];
        const auto r2 = bgra[indexNext++];
        indexNext++;
        buffer[yIndex++] = ((25 * b2 + 129 * g2 + 66 * r2) >> 8) + 16;
    }

    // }
}
inline void BGR2YUVP_ParallelBody(const unsigned char* bgra, unsigned char* dst, const int width, const int height, const int stride, const int begin) {

    //begin = begin / 2;
    //end = end / 2;
    const int wi = width / 2;
    const int uvlineBegin = begin * wi;
    const int yPlaneSize = width * height;

    int vIndex = yPlaneSize + uvlineBegin;
    int uIndex = yPlaneSize + (yPlaneSize / 4) + (uvlineBegin);
    const int readBegin = 2 * stride * begin;
    int index = readBegin;
    int yIndex = width * 2 * begin;

    unsigned char* buffer = dst;
    //  for (int j = begin; j < end; j++)
      //{
#pragma clang loop unroll_count(2)
#pragma clang loop vectorize(assume_safety)
#pragma clang loop vectorize_width(32) interleave_count(1)
#pragma clang loop vectorize_predicate(enable)

    for (int i = 0; i < wi; ++i)
    {
        const auto b = bgra[index++];
        const auto g = bgra[index++];
        const auto r = bgra[index++];

        const auto b1 = bgra[index++];
        const auto g1 = bgra[index++];
        const auto r1 = bgra[index++];

        buffer[yIndex++] = ((25 * b + 129 * g + 66 * r) >> 8) + 16;
        buffer[yIndex++] = ((25 * b1 + 129 * g1 + 66 * r1) >> 8) + 16;

        buffer[uIndex++] = ((112 * r + -94 * g + -18 * b) >> 8) + 128;
        buffer[vIndex++] = ((-38 * r + -74 * g + 112 * b) >> 8) + 128;
    }

    int indexNext = (readBegin)+(stride);
#pragma clang loop unroll_count(2)
#pragma clang loop vectorize(assume_safety)
#pragma clang loop vectorize_width(32) interleave_count(1)
    for (int i = 0; i < width; ++i)
    {
        const auto b2 = bgra[indexNext++];
        const auto g2 = bgra[indexNext++];
        const auto r2 = bgra[indexNext++];
        buffer[yIndex++] = ((25 * b2 + 129 * g2 + 66 * r2) >> 8) + 16;
    }
}

inline void RGBA2YUVP_ParallelBody(const unsigned char* bgra, unsigned char* dst, const int width, const int height, const int stride, const int begin) {
    //begin = begin / 2;
   //end = end / 2;
    const int wi = width / 2;
    const int uvlineBegin = begin * wi;
    const int yPlaneSize = width * height;

    int vIndex = yPlaneSize + uvlineBegin;
    int uIndex = yPlaneSize + (yPlaneSize / 4) + (uvlineBegin);
    const int readBegin = 2 * stride * begin;
    int index = readBegin;
    int yIndex = width * 2 * begin;

    unsigned char* buffer = dst;
    //  for (int j = begin; j < end; j++)
      //{
#pragma clang loop unroll_count(2)
#pragma clang loop vectorize(assume_safety)
#pragma clang loop vectorize_width(32) interleave_count(1)
#pragma clang loop vectorize_predicate(enable)

    for (int i = 0; i < wi; ++i)
    {
        const auto r = bgra[index++];
        const auto g = bgra[index++];
        const auto b = bgra[index++];
        index++;

        const auto r1 = bgra[index++];
        const auto g1 = bgra[index++];
        const auto b1 = bgra[index++];
        index++;

        buffer[yIndex++] = ((25 * b + 129 * g + 66 * r) >> 8) + 16;
        buffer[yIndex++] = ((25 * b1 + 129 * g1 + 66 * r1) >> 8) + 16;

        buffer[uIndex++] = ((112 * r + -94 * g + -18 * b) >> 8) + 128;
        buffer[vIndex++] = ((-38 * r + -74 * g + 112 * b) >> 8) + 128;
    }

    int indexNext = (readBegin)+(stride);
#pragma clang loop unroll_count(2)
#pragma clang loop vectorize(assume_safety)
#pragma clang loop vectorize_width(32) interleave_count(1)
    for (int i = 0; i < width; ++i)
    {
        const auto r2 = bgra[indexNext++];
        const auto g2 = bgra[indexNext++];
        const auto b2 = bgra[indexNext++];
        indexNext++;
        buffer[yIndex++] = ((25 * b2 + 129 * g2 + 66 * r2) >> 8) + 16;
    }
}
inline void RGB2YUVP_ParallelBody(const unsigned char* bgra, unsigned char* dst, const int width, const int height, const int stride, const int begin)
{
    //begin = begin / 2;
    //end = end / 2;
    const int wi = width / 2;
    const int uvlineBegin = begin * wi;
    const int yPlaneSize = width * height;

    int vIndex = yPlaneSize + uvlineBegin;
    int uIndex = yPlaneSize + (yPlaneSize / 4) + (uvlineBegin);
    const int readBegin = 2 * stride * begin;
    int index = readBegin;
    int yIndex = width * 2 * begin;

    unsigned char* buffer = dst;
    //  for (int j = begin; j < end; j++)
      //{
#pragma clang loop unroll_count(2)
#pragma clang loop vectorize(assume_safety)
#pragma clang loop vectorize_width(32) interleave_count(1)
#pragma clang loop vectorize_predicate(enable)

    for (int i = 0; i < wi; ++i)
    {
        const auto r = bgra[index++];
        const auto g = bgra[index++];
        const auto b = bgra[index++];

        const auto r1 = bgra[index++];
        const auto g1 = bgra[index++];
        const auto b1 = bgra[index++];

        buffer[yIndex++] = ((25 * b + 129 * g + 66 * r) >> 8) + 16;
        buffer[yIndex++] = ((25 * b1 + 129 * g1 + 66 * r1) >> 8) + 16;

        buffer[uIndex++] = ((112 * r + -94 * g + -18 * b) >> 8) + 128;
        buffer[vIndex++] = ((-38 * r + -74 * g + 112 * b) >> 8) + 128;
    }

    int indexNext = (readBegin)+(stride);
#pragma clang loop unroll_count(2)
#pragma clang loop vectorize(assume_safety)
#pragma clang loop vectorize_width(32) interleave_count(1)
    for (int i = 0; i < width; ++i)
    {
        const auto r2 = bgra[indexNext++];
        const auto g2 = bgra[indexNext++];
        const auto b2 = bgra[indexNext++];
        buffer[yIndex++] = ((25 * b2 + 129 * g2 + 66 * r2) >> 8) + 16;
    }
}

void BGRAtoYUV420Planar(const unsigned char* bgra, unsigned char* dst, const int width, const int height, const int stride, int numThreads)
{
    const int hi = height / 2;
    if (width * height > 1280 * 720 && numThreads > 1)
    {
#ifdef _WIN32
        concurrency::parallel_for(int(0), numThreads, [&bgra, &dst, &width, &height, &stride, &hi, &numThreads](int j)
            {
                int bgn = ((hi / numThreads) * (j));
                int end = ((hi / numThreads) * (j + 1));
                if (j == numThreads - 1)
                {
                    end = hi;
                }

                for (int i = bgn; i < end; i++)
                {
                    [[clang::always_inline]] BGRA2YUVP_ParallelBody(bgra, dst, width, height, stride, i);
                }
            }, affin);
#else
        for (int j = 0; j < hi; j++)
        {
            [[clang::always_inline]] BGRA2YUVP_ParallelBody(bgra, dst, width, height, stride, j);
        }
#endif


    }
    else
    {
        for (int j = 0; j < hi; j++)
        {
            [[clang::always_inline]] BGRA2YUVP_ParallelBody(bgra, dst, width, height, stride, j);
        }
    }
}

void RGBAtoYUV420Planar(unsigned char* bgra, unsigned char* dst, int width, int height, int stride, int numThreads)
{
    const int hi = height / 2;
    if (width * height > 1280 * 720 && numThreads > 1)
    {
#ifdef _WIN32

        concurrency::parallel_for(int(0), numThreads, [&bgra, &dst, &width, &height, &stride, &hi, &numThreads](int j)
            {
                int bgn = ((hi / numThreads) * (j));
                int end = ((hi / numThreads) * (j + 1));
                if (j == numThreads - 1)
                {
                    end = hi;
                }

                for (int i = bgn; i < end; i++)
                {
                    [[clang::always_inline]] RGBA2YUVP_ParallelBody(bgra, dst, width, height, stride, i);
                }
            }, affin);
#else
        for (int j = 0; j < hi; j++)
        {
            [[clang::always_inline]] RGBA2YUVP_ParallelBody(bgra, dst, width, height, stride, j);
        }
#endif

    }
    else
    {
        for (int j = 0; j < hi; j++)
        {
            [[clang::always_inline]] RGBA2YUVP_ParallelBody(bgra, dst, width, height, stride, j);
        }
    }

}

void BGRtoYUV420Planar(unsigned char* bgra, unsigned char* dst, int width, int height, int stride, int numThreads)
{
    const int hi = height / 2;
    if (width * height > 1280 * 720 && numThreads > 1)
    {
#ifdef _WIN32

        concurrency::parallel_for(int(0), numThreads, [&bgra, &dst, &width, &height, &stride, &hi, &numThreads](int j)
            {
                int bgn = ((hi / numThreads) * (j));
                int end = ((hi / numThreads) * (j + 1));
                if (j == numThreads - 1)
                {
                    end = hi;
                }

                for (int i = bgn; i < end; i++)
                {
                    [[clang::always_inline]] BGR2YUVP_ParallelBody(bgra, dst, width, height, stride, i);
                }
            }, affin);
#else
        for (int j = 0; j < hi; j++)
        {
            [[clang::always_inline]] BGR2YUVP_ParallelBody(bgra, dst, width, height, stride, j);
        }
#endif

    }
    else
    {
        for (int j = 0; j < hi; j++)
        {
            [[clang::always_inline]] BGR2YUVP_ParallelBody(bgra, dst, width, height, stride, j);
        }
    }
}

void RGBtoYUV420Planar(unsigned char* bgra, unsigned char* dst, int width, int height, int stride, int numThreads)
{
    const int hi = height / 2;
    if (width * height > 1280 * 720 && numThreads > 1)
    {
#ifdef _WIN32
        concurrency::parallel_for(int(0), numThreads, [&bgra, &dst, &width, &height, &stride, &hi, &numThreads](int j)
            {
                int bgn = ((hi / numThreads) * (j));
                int end = ((hi / numThreads) * (j + 1));
                if (j == numThreads - 1)
                {
                    end = hi;
                }

                for (int i = bgn; i < end; i++)
                {
                    [[clang::always_inline]] RGB2YUVP_ParallelBody(bgra, dst, width, height, stride, i);
                }
            }, affin);
#else
        for (int j = 0; j < hi; j++)
        {
            [[clang::always_inline]] RGB2YUVP_ParallelBody(bgra, dst, width, height, stride, j);
        }
#endif
    }
    else
    {
        for (int j = 0; j < hi; j++)
        {
            [[clang::always_inline]] RGB2YUVP_ParallelBody(bgra, dst, width, height, stride, j);
        }
    }

}

//-------------------------------Downscale------------------------------------------------

void Downscale24(unsigned char* rgbSrc, int width, int height, int stride, unsigned char* dst, int multiplier)
{
    int index = 0;
    int dinx = 0;
    for (size_t i = 0; i < height / multiplier; i++)
    {
#pragma clang loop vectorize(assume_safety)
        for (size_t j = 0; j < width / multiplier; j++)
        {

            dst[dinx++] = rgbSrc[index];
            dst[dinx++] = rgbSrc[index + 1];
            dst[dinx++] = rgbSrc[index + 2];

            index += 3 * multiplier;
        }
        index = stride * multiplier * (i + 1);
    }
}

void Downscale32(unsigned char* rgbaSrc, int width, int height, int stride, unsigned char* dst, int multiplier)
{
    int index = 0;
    int dinx = 0;
    for (size_t i = 0; i < height / multiplier; i++)
    {
#pragma clang loop vectorize(assume_safety)

        for (size_t j = 0; j < width / multiplier; j++)
        {
            dst[dinx++] = rgbaSrc[index];
            dst[dinx++] = rgbaSrc[index + 1];
            dst[dinx++] = rgbaSrc[index + 2];

            index += 4 * multiplier;
        }
        index = stride * multiplier * (i + 1);
    }
}