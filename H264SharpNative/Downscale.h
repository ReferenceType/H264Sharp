#include "pch.h"
#ifndef ARM

#include <emmintrin.h>
#include <immintrin.h>
#include <smmintrin.h>

#include <stdint.h>
#include <cstdint>

__m128i Gather2(uint8_t* src, int mul)
{
    __m256i indices = _mm256_setr_epi32(0, 4, 8, 12, 16, 20, 22, 24); // Indices to gather

    indices = _mm256_mullo_epi32(indices, _mm256_set1_epi32(mul));
    __m256i result = _mm256_i32gather_epi32((int*)src, indices, 1);   // Scale = 4 (sizeof(int))// just mull enough here also

    __m256i low16 = _mm256_and_si256(result, _mm256_set1_epi32(0xFFFF));
    __m256i packed = _mm256_packs_epi32(low16, low16);

    result = _mm256_permute4x64_epi64(packed, 0b11011000);

    return _mm256_castsi256_si128(result);//x16
}

__m256i Gather3(uint8_t* src, int mul)
{
    __m256i indices = _mm256_setr_epi32(0, 6, 12, 18, 24, 30, 36, 42); // Indices to gather
    indices = _mm256_mullo_epi32(indices, _mm256_set1_epi32(mul));

    __m256i result = _mm256_i32gather_epi32((int*)src, indices, 1);   // Scale = 4 (sizeof(int))// just mull enough here also

    result = _mm256_shuffle_epi8(result, _mm256_setr_epi8(0, 1, 2, 4, 5, 6, 8, 9, 10, 12, 13, 14, -1, -1, -1, -1,
        0, 1, 2, 4, 5, 6, 8, 9, 10, 12, 13, 14, -1, -1, -1, -1));

    result = _mm256_permutevar8x32_epi32(result, _mm256_setr_epi32(0, 1, 2, 4, 5, 6, 7, 3));
    return result;//x24
}

__m256i Gather4(uint8_t* src, int mul)
{
    __m256i indices = _mm256_setr_epi32(0, 8, 16, 24, 32, 40, 48, 56); // Indices to gather
    indices = _mm256_mullo_epi32(indices, _mm256_set1_epi32(mul));

    __m256i result = _mm256_i32gather_epi32((int*)src, indices, 1);   // Scale = 4 (sizeof(int))// just mull enough here also

    return result;
}

void Downscale3(uint8_t* RESTRICT rgbSrc, int32_t width, int32_t height, int32_t stride, uint8_t* RESTRICT dst, int32_t multiplier)
{
    int div = 1 << multiplier;  // Scaling factor (2^multiplier)
    int destShift = 0;

    for (int h = 0; h < height / div; h++)
    {
        uint8_t* row = rgbSrc + (h * stride * div);

        for (int w = 0; w < width / div; w += 8)
        {
            int shift = w * 3 * div;

            auto res = Gather3(row + shift, multiplier);

            _mm256_storeu_si256((__m256i*)(dst + destShift), res);
            destShift += 24;
        }
    }
}

void Downscale4(uint8_t* RESTRICT rgbSrc, int32_t width, int32_t height, int32_t stride, uint8_t* RESTRICT dst, int32_t multiplier)
{
    int div = 1 << multiplier;
    int destShift = 0;

    for (int h = 0; h < height / div; h++)
    {
        uint8_t* row = rgbSrc + (h * stride * div);

        for (int w = 0; w < width / div; w += 8)
        {
            int shift = w * 4 * div;

            auto res = Gather4(row + shift, multiplier);

            _mm256_storeu_si256((__m256i*)(dst + destShift), res);
            destShift += 32;
        }
    }
}


#endif // !__arm__
