#include "Rgb2Yuv.h"
#include <immintrin.h>
#include <stdint.h>
namespace H264Sharp {


// PixelFormat struct templates for different formats
struct RGB {
    static constexpr int BytesPerPixel = 3;
    static __m256i ExtractR(__m256i pixels) { return _mm256_shuffle_epi8(pixels, _mm256_setr_epi8(0, -1, -1, 3, 6, -1, -1, 9, 12, -1, -1, 15, 18, -1, -1, 21, 24, -1, -1, 27, 30, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1)); }
    static __m256i ExtractG(__m256i pixels) { return _mm256_shuffle_epi8(pixels, _mm256_setr_epi8(1, -1, -1, 4, 7, -1, -1, 10, 13, -1, -1, 16, 19, -1, -1, 22, 25, -1, -1, 28, 31, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1)); }
    static __m256i ExtractB(__m256i pixels) { return _mm256_shuffle_epi8(pixels, _mm256_setr_epi8(2, -1, -1, 5, 8, -1, -1, 11, 14, -1, -1, 17, 20, -1, -1, 23, 26, -1, -1, 29, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1)); }
};

struct BGR {
    static constexpr int BytesPerPixel = 3;
    static __m256i ExtractB(__m256i pixels) { return RGB::ExtractR(pixels); }
    static __m256i ExtractG(__m256i pixels) { return RGB::ExtractG(pixels); }
    static __m256i ExtractR(__m256i pixels) { return RGB::ExtractB(pixels); }
};

struct RGBA {
    static constexpr int BytesPerPixel = 4;
    static __m256i ExtractR(__m256i pixels) { return _mm256_shuffle_epi8(pixels, _mm256_setr_epi8(0, -1, -1, -1, 4, -1, -1, -1, 8, -1, -1, -1, 12, -1, -1, -1, 16, -1, -1, -1, 20, -1, -1, -1, 24, -1, -1, -1, 28, -1, -1, -1)); }
    static __m256i ExtractG(__m256i pixels) { return _mm256_shuffle_epi8(pixels, _mm256_setr_epi8(1, -1, -1, -1, 5, -1, -1, -1, 9, -1, -1, -1, 13, -1, -1, -1, 17, -1, -1, -1, 21, -1, -1, -1, 25, -1, -1, -1, 29, -1, -1, -1)); }
    static __m256i ExtractB(__m256i pixels) { return _mm256_shuffle_epi8(pixels, _mm256_setr_epi8(2, -1, -1, -1, 6, -1, -1, -1, 10, -1, -1, -1, 14, -1, -1, -1, 18, -1, -1, -1, 22, -1, -1, -1, 26, -1, -1, -1, 30, -1, -1, -1)); }
};

struct BGRA {
    static constexpr int BytesPerPixel = 4;
    static __m256i ExtractB(__m256i pixels) { return RGBA::ExtractR(pixels); }
    static __m256i ExtractG(__m256i pixels) { return RGBA::ExtractG(pixels); }
    static __m256i ExtractR(__m256i pixels) { return RGBA::ExtractB(pixels); }
};
// Not working may abandon too much suffle..

template <typename PixelFormat>
void RGBToI420_AVX2(const uint8_t* src, uint8_t* y_plane, int width, int height) {
    const int chroma_width = width / 2;
    const int chroma_height = height / 2;
    const int src_stride = width * PixelFormat::BytesPerPixel;
    const int y_stride = width;
    const int chroma_stride = chroma_width;
    uint8_t* u_plane = y_plane + (width * height);
    uint8_t* v_plane = u_plane + (width * height) / 4;

    for (int y = 0; y < height; y += 2) {
        for (int x = 0; x < width; x += 16) {
            __m256i r0, g0, b0, r1, g1, b1;

            uint8_t* src_row1 = (uint8_t*)src + y * src_stride + x * PixelFormat::BytesPerPixel;
            uint8_t* src_row2 = (uint8_t*)src + (y + 1) * src_stride + x * PixelFormat::BytesPerPixel;

            __m256i row1 = _mm256_loadu_si256((__m256i*)src_row1);
            __m256i row2 = _mm256_loadu_si256((__m256i*)src_row2);

            r0 = PixelFormat::ExtractR(row1);
            g0 = PixelFormat::ExtractG(row1);
            b0 = PixelFormat::ExtractB(row1);

            r1 = PixelFormat::ExtractR(row2);
            g1 = PixelFormat::ExtractG(row2);
            b1 = PixelFormat::ExtractB(row2);

            /*
            * const int16_t YB = 25;
             int16_t YG = 129;
            const int16_t YR = 66;
            */

            auto ry0 = _mm256_mullo_epi16(r0, _mm256_set1_epi16(66));
            auto gy0 = _mm256_mullo_epi16(g0, _mm256_set1_epi16(129));
            auto by0 = _mm256_mullo_epi16(b0, _mm256_set1_epi16(25));

            auto ry1 = _mm256_mullo_epi16(r1, _mm256_set1_epi16(66));
            auto gy1 = _mm256_mullo_epi16(g1, _mm256_set1_epi16(129));
            auto by1 = _mm256_mullo_epi16(b1, _mm256_set1_epi16(25));


            __m256i y0 = _mm256_srli_epi16(_mm256_add_epi16(_mm256_add_epi16(ry0, gy0), by0), 8);
            __m256i y1 = _mm256_srli_epi16(_mm256_add_epi16(_mm256_add_epi16(ry1, gy1), by1), 8);

          

            y0 = _mm256_add_epi16(y0, _mm256_set1_epi16(16));
            y1 = _mm256_add_epi16(y1, _mm256_set1_epi16(16));

            auto y0l = _mm256_castsi256_si128(y0);
            auto y0h = _mm256_extracti128_si256(y0, 1);
            auto y1l = _mm256_castsi256_si128(y1);
            auto y1h = _mm256_extracti128_si256(y1, 1);

            y0 = _mm256_set_m128i(y0h, y0l);
            y1 = _mm256_set_m128i(y1h, y1l);

            _mm256_storeu_si256((__m256i*)(y_plane + y * y_stride + x), y0);
            _mm256_storeu_si256((__m256i*)(y_plane + (y + 1) * y_stride + x), y1);

            /*
            * const int16_t UR = 112;
    const int16_t UG = -94;
    const int16_t UB = -18;

    const int16_t VR = -38;
    const int16_t VG = -74;
    const int16_t VB = 112;
            */
           
            auto ru = _mm256_mullo_epi16(r0, _mm256_set1_epi16(112));
            auto gu = _mm256_mullo_epi16(g0, _mm256_set1_epi16(-94));
            auto bu = _mm256_mullo_epi16(b0, _mm256_set1_epi16(-18));

            auto rv = _mm256_mullo_epi16(r0, _mm256_set1_epi16(-38));
            auto gv = _mm256_mullo_epi16(g0, _mm256_set1_epi16(-74));
            auto bv = _mm256_mullo_epi16(b0, _mm256_set1_epi16(112));



            __m256i u = _mm256_srli_epi16(_mm256_add_epi16(bu, _mm256_add_epi16(ru, gu)), 8);
            __m256i v = _mm256_srli_epi16(_mm256_add_epi16(bv, _mm256_add_epi16(rv, gv)), 8);

            u = _mm256_add_epi16(u, _mm256_set1_epi16(128));
            v = _mm256_add_epi16(v, _mm256_set1_epi16(128));

            auto us = _mm256_castsi256_si128(u);
            auto vs = _mm256_castsi256_si128(v);

            _mm_storeu_si128((__m128i*)(u_plane + (y / 2) * chroma_stride + x / 2), us);
            _mm_storeu_si128((__m128i*)(v_plane + (y / 2) * chroma_stride + x / 2), vs);
        }
    }
}

void Rgb2Yuv::RGBToI420_AVX2_(const uint8_t* src, uint8_t* y_plane, int width, int height)
{
    RGBToI420_AVX2<RGB>(src, y_plane, width, height);
}

void Rgb2Yuv::RGBAToI420_AVX2_(const uint8_t* src, uint8_t* y_plane, int width, int height)
{
    RGBToI420_AVX2<RGBA>(src, y_plane, width, height);
}
void Rgb2Yuv::BGRToI420_AVX2_(const uint8_t* src, uint8_t* y_plane, int width, int height)
{
    RGBToI420_AVX2<BGR>(src, y_plane, width, height);
}
void Rgb2Yuv::BGRAToI420_AVX2_(const uint8_t* src, uint8_t* y_plane, int width, int height)
{
    RGBToI420_AVX2<BGRA>(src, y_plane, width, height);
}
}