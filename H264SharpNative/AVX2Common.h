#pragma once
#ifndef AVX_COMMON
#define AVX_COMMON

#include "pch.h"
#ifndef __arm__

#include <emmintrin.h>
#include <immintrin.h>

#include <stdint.h>
#include <cstdint>

#ifdef _MSC_VER
#include <intrin.h>  // MSVC
#else
#include <cpuid.h>   // GCC/Clang
#endif



enum class AlignmentFlags : uint8_t {
	None = 0,        // No buffer is aligned
	YAligned = 1 << 0,   // 0001
	UAligned = 1 << 1,   // 0010
	VAligned = 1 << 2,   // 0100
	RGBAligned = 1 << 3    // 1000
};

// Enable bitwise operators for AlignmentFlags
inline AlignmentFlags operator|(AlignmentFlags a, AlignmentFlags b) {
	return static_cast<AlignmentFlags>(static_cast<uint8_t>(a) | static_cast<uint8_t>(b));
}

// Use constexpr function for compile-time flag checking
constexpr bool hasFlag(AlignmentFlags allFlags, AlignmentFlags flag) {
	return (static_cast<uint8_t>(allFlags) & static_cast<uint8_t>(flag)) != 0;
}


inline bool isAligned32(void* ptr) {
	return (reinterpret_cast<std::uintptr_t>(ptr) & 31) == 0;
}

template <bool alligned>
inline __m256i Load(const void* ptr)
{
	if constexpr (alligned)
		return loadAligned(ptr);
	else
		return loadUnaligned(ptr);
}

inline __m256i loadAligned(const void* ptr) {
	const __m256i* aligned_ptr = (const __m256i*)__builtin_assume_aligned(ptr, 32);
	return _mm256_load_si256(aligned_ptr);
}

inline __m256i loadUnaligned(const void* ptr) {
	return _mm256_loadu_si256((const __m256i*)ptr);
}


static const __m256i blendMask0 = _mm256_setr_epi8(
	0, 0, -1, 0, 0, -1, 0, 0, -1, 0, 0, -1, 0, 0, -1, 0,
	0, -1, 0, 0, -1, 0, 0, -1, 0, 0, -1, 0, 0, -1, 0, 0);

static const __m256i blendMask1 = _mm256_setr_epi8(
	0, -1, 0, 0, -1, 0, 0, -1, 0, 0, -1, 0, 0, -1, 0, 0,
	-1, 0, 0, -1, 0, 0, -1, 0, 0, -1, 0, 0, -1, 0, 0, -1);

static const __m256i shuffleMaskR = _mm256_setr_epi8(
	0, 3, 6, 9, 12, 15, 2, 5, 8, 11, 14, 1, 4, 7, 10, 13,
	0, 3, 6, 9, 12, 15, 2, 5, 8, 11, 14, 1, 4, 7, 10, 13);

static const __m256i shuffleMaskG = _mm256_setr_epi8(
	1, 4, 7, 10, 13, 0, 3, 6, 9, 12, 15, 2, 5, 8, 11, 14,
	1, 4, 7, 10, 13, 0, 3, 6, 9, 12, 15, 2, 5, 8, 11, 14);

static const __m256i shuffleMaskB = _mm256_setr_epi8(
	2, 5, 8, 11, 14, 1, 4, 7, 10, 13, 0, 3, 6, 9, 12, 15,
	2, 5, 8, 11, 14, 1, 4, 7, 10, 13, 0, 3, 6, 9, 12, 15);

inline void GetChannels3_16x16_2(uint8_t* ptr, __m256i& rl, __m256i& gl, __m256i& bl, __m256i& rh, __m256i& gh, __m256i& bh)
{
	__m256i rgb1 = _mm256_loadu_si256((const __m256i*)ptr);
	__m256i rgb2 = _mm256_loadu_si256((const __m256i*)(ptr + 32));
	__m256i rgb3 = _mm256_loadu_si256((const __m256i*)(ptr + 64));

	__m256i perm_low = _mm256_permute2x128_si256(rgb1, rgb3, 0 + 2 * 16);
	__m256i perm_high = _mm256_permute2x128_si256(rgb1, rgb3, 1 + 3 * 16);

	__m256i r0 = _mm256_blendv_epi8(_mm256_blendv_epi8(perm_low, perm_high, blendMask0), rgb2, blendMask1);
	__m256i g0 = _mm256_blendv_epi8(_mm256_blendv_epi8(perm_high, perm_low, blendMask1), rgb2, blendMask0);
	__m256i b0 = _mm256_blendv_epi8(_mm256_blendv_epi8(rgb2, perm_low, blendMask0), perm_high, blendMask1);

	__m256i r = _mm256_shuffle_epi8(r0, shuffleMaskR);
	__m256i g = _mm256_shuffle_epi8(g0, shuffleMaskG);
	__m256i b = _mm256_shuffle_epi8(b0, shuffleMaskB);

	rl = _mm256_cvtepu8_epi16(_mm256_castsi256_si128(r));
	rh = _mm256_cvtepu8_epi16(_mm256_extracti128_si256(r, 1));

	gl = _mm256_cvtepu8_epi16(_mm256_castsi256_si128(g));
	gh = _mm256_cvtepu8_epi16(_mm256_extracti128_si256(g, 1));

	bl = _mm256_cvtepu8_epi16(_mm256_castsi256_si128(b));
	bh = _mm256_cvtepu8_epi16(_mm256_extracti128_si256(b, 1));

}

static const __m256i rgbaShuffleMask = _mm256_setr_epi8(0, 4, 8, 12, 1, 5, 9, 13, 2, 6, 10, 14, 3, 7, 11, 15,
												 0, 4, 8, 12, 1, 5, 9, 13, 2, 6, 10, 14, 3, 7, 11, 15);
// no need for aplha
inline void GetChannels4_16x16_2(const uint8_t* ptr, __m256i& rl, __m256i& gl, __m256i& bl, __m256i& rh, __m256i& gh, __m256i& bh)
{
	__m256i rgb1 = _mm256_loadu_si256((const __m256i*)ptr);
	__m256i rgb2 = _mm256_loadu_si256((const __m256i*)(ptr + 32));
	__m256i rgb3 = _mm256_loadu_si256((const __m256i*)(ptr + 64));
	__m256i rgb4 = _mm256_loadu_si256((const __m256i*)(ptr + 96));
	

	__m256i sv0 = _mm256_shuffle_epi8(rgb1, rgbaShuffleMask);
	__m256i sv1 = _mm256_shuffle_epi8(rgb2, rgbaShuffleMask);
	__m256i sv2 = _mm256_shuffle_epi8(rgb3, rgbaShuffleMask);
	__m256i sv4 = _mm256_shuffle_epi8(rgb4, rgbaShuffleMask);

	__m256i sv01l = _mm256_unpacklo_epi32(sv0, sv1);
	__m256i sv01h = _mm256_unpackhi_epi32(sv0, sv1);
	__m256i sv23l = _mm256_unpacklo_epi32(sv2, sv4);
	__m256i sv23h = _mm256_unpackhi_epi32(sv2, sv4);

	__m256i permll = _mm256_permute2x128_si256(sv01l, sv23l, 0 + 2 * 16);
	__m256i permlh = _mm256_permute2x128_si256(sv01l, sv23l, 1 + 3 * 16);
	__m256i permhl = _mm256_permute2x128_si256(sv01h, sv23h, 0 + 2 * 16);
	__m256i permhh = _mm256_permute2x128_si256(sv01h, sv23h, 1 + 3 * 16);

	__m256i r = _mm256_unpacklo_epi32(permll, permlh);
	__m256i g = _mm256_unpackhi_epi32(permll, permlh);
	__m256i b = _mm256_unpacklo_epi32(permhl, permhh);
	//__m256i a0 = _mm256_unpackhi_epi32(phl, phh);


	rl = _mm256_cvtepu8_epi16(_mm256_castsi256_si128(r));
	rh = _mm256_cvtepu8_epi16(_mm256_extracti128_si256(r, 1));

	gl = _mm256_cvtepu8_epi16(_mm256_castsi256_si128(g));
	gh = _mm256_cvtepu8_epi16(_mm256_extracti128_si256(g, 1));

	bl = _mm256_cvtepu8_epi16(_mm256_castsi256_si128(b));
	bh = _mm256_cvtepu8_epi16(_mm256_extracti128_si256(b, 1));

}

static const __m256i shuffleMaskB_s = _mm256_setr_epi8(
	0, 11, 6, 1, 12, 7, 2, 13, 8, 3, 14, 9, 4, 15, 10, 5,
	0, 11, 6, 1, 12, 7, 2, 13, 8, 3, 14, 9, 4, 15, 10, 5);

static const __m256i shuffleMaskG_s = _mm256_setr_epi8(
	5, 0, 11, 6, 1, 12, 7, 2, 13, 8, 3, 14, 9, 4, 15, 10,
	5, 0, 11, 6, 1, 12, 7, 2, 13, 8, 3, 14, 9, 4, 15, 10);

static const __m256i shuffleMaskR_s = _mm256_setr_epi8(
	10, 5, 0, 11, 6, 1, 12, 7, 2, 13, 8, 3, 14, 9, 4, 15,
	10, 5, 0, 11, 6, 1, 12, 7, 2, 13, 8, 3, 14, 9, 4, 15);

static const __m256i blendMask0_s = _mm256_setr_epi8(
	0, -1, 0, 0, -1, 0, 0, -1, 0, 0, -1, 0, 0, -1, 0, 0,
    0, -1, 0, 0, -1, 0, 0, -1, 0, 0, -1, 0, 0, -1, 0, 0);

static const __m256i blendMask1_s = _mm256_setr_epi8(
	0, 0, -1, 0, 0, -1, 0, 0, -1, 0, 0, -1, 0, 0, -1, 0,
	0, 0, -1, 0, 0, -1, 0, 0, -1, 0, 0, -1, 0, 0, -1, 0);


inline void Store3Interleave(uint8_t* ptr, const __m256i& r, const __m256i& g, const __m256i& b)
{
	__m256i rs = _mm256_shuffle_epi8(b, shuffleMaskB_s);
	__m256i gs = _mm256_shuffle_epi8(g, shuffleMaskG_s);
	__m256i bs = _mm256_shuffle_epi8(r, shuffleMaskR_s);
	
	__m256i bl0 = _mm256_blendv_epi8(_mm256_blendv_epi8(rs, gs, blendMask0_s), bs, blendMask1_s);
	__m256i bl1 = _mm256_blendv_epi8(_mm256_blendv_epi8(gs, bs, blendMask0_s), rs, blendMask1_s);
	__m256i bl2 = _mm256_blendv_epi8(_mm256_blendv_epi8(bs, rs, blendMask0_s), gs, blendMask1_s);

	__m256i rgb1 = _mm256_permute2x128_si256(bl0, bl1, 0 + 2 * 16);
	__m256i rgb2 = _mm256_permute2x128_si256(bl2, bl0, 0 + 3 * 16);
	__m256i rgb3 = _mm256_permute2x128_si256(bl1, bl2, 1 + 3 * 16);

	_mm256_storeu_si256((__m256i*)ptr, rgb1);
	_mm256_storeu_si256((__m256i*)(ptr + 32), rgb2);
	_mm256_storeu_si256((__m256i*)(ptr + 64), rgb3);
}
// check this later
inline void Store4Interleave(uint8_t* ptr, const __m256i& r, const __m256i& g, const __m256i& b, const __m256i& a)
{
	__m256i rgl = _mm256_unpacklo_epi8(r, g);
	__m256i rgh = _mm256_unpackhi_epi8(r, g);
	__m256i bal = _mm256_unpacklo_epi8(b, a);
	__m256i bah = _mm256_unpackhi_epi8(b, a);

	__m256i rgba_l0 = _mm256_unpacklo_epi16(rgl, bal);
	__m256i rgba_l1 = _mm256_unpackhi_epi16(rgl, bal);
	__m256i rgba_h0 = _mm256_unpacklo_epi16(rgh, bah);
	__m256i rgba_h1 = _mm256_unpackhi_epi16(rgh, bah);

	__m256i rgba0 = _mm256_permute2x128_si256(rgba_l0, rgba_l1, 0 + 2 * 16);
	__m256i rgba1 = _mm256_permute2x128_si256(rgba_l0, rgba_l1, 1 + 3 * 16);
	__m256i rgba2 = _mm256_permute2x128_si256(rgba_h0, rgba_h1, 0 + 2 * 16);
	__m256i rgba3 = _mm256_permute2x128_si256(rgba_h0, rgba_h1, 1 + 3 * 16);

	_mm256_storeu_si256((__m256i*)ptr, rgba0);
	_mm256_storeu_si256((__m256i*)(ptr + 32), rgba1);
	_mm256_storeu_si256((__m256i*)(ptr + 64), rgba2);
	_mm256_storeu_si256((__m256i*)(ptr + 96), rgba3);
}

static const __m256i const_128_16b = _mm256_set1_epi16(128);
static const __m256i uv_mask = _mm256_setr_epi8(0, 0, 2, 2, 4, 4, 6, 6, 8, 8, 10, 10, 12, 12, 14, 14, 16, 16, 18, 18, 20, 20, 22, 22, 24, 24, 26, 26, 28, 28, 30, 30);
inline void LoadAndUpscale(const uint8_t* plane, __m256i& low, __m256i& high)
{
	__m128i u = _mm_loadu_si128((__m128i*)plane);
	__m256i u0 = _mm256_cvtepu8_epi16(u);

	auto ud = _mm256_shuffle_epi8(u0, uv_mask);
	__m128i ul8 = _mm256_castsi256_si128(ud);
	__m128i uh8 = _mm256_extracti128_si256(ud, 1);

	low = _mm256_sub_epi16(_mm256_cvtepu8_epi16(ul8), const_128_16b);
	high = _mm256_sub_epi16(_mm256_cvtepu8_epi16(uh8), const_128_16b);

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

static const __m256i ymm4_const = _mm256_setr_epi8(
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


alignas(32) static const uint8_t shuffle_pattern[16] = {
	0, 3, 6, 9, 12, 15, 2, 5,
	8, 11, 14, 1, 4, 7, 10, 13
};

alignas(32) static const uint8_t blend_mask[16] = {
	255, 255, 255, 255, 255, 255, 255, 255,
	255, 255, 255, 0, 0, 0, 0, 0
};
static const __m256i shuffleMask = _mm256_broadcastsi128_si256(*(__m128i*)shuffle_pattern);
static const __m256i blendMask = _mm256_broadcastsi128_si256(*(__m128i*)blend_mask);

inline void GetChannels3_16x16(uint8_t* RESTRICT input, __m256i& rl, __m256i& gl, __m256i& bl, __m256i& rh, __m256i& gh, __m256i& bh)
{

	// Load 96 bytes of input data into three YMM registers
	__m256i ymm0 = _mm256_inserti128_si256(_mm256_loadu_si256((__m256i*) & input[0]),
		_mm_loadu_si128((__m128i*) & input[48]), 1);

	__m256i ymm1 = _mm256_inserti128_si256(_mm256_loadu_si256((__m256i*) & input[16]),
		_mm_loadu_si128((__m128i*) & input[64]), 1);

	__m256i ymm2 = _mm256_inserti128_si256(_mm256_loadu_si256((__m256i*) & input[32]),
		_mm_loadu_si128((__m128i*) & input[80]), 1);


	// Shuffle bytes according to pattern
	ymm0 = _mm256_shuffle_epi8(ymm0, shuffleMask);
	ymm1 = _mm256_shuffle_epi8(ymm1, shuffleMask);
	ymm2 = _mm256_shuffle_epi8(ymm2, shuffleMask);

	// Align registers using palignr
	__m256i ymm3 = _mm256_alignr_epi8(ymm0, ymm2, 11);
	ymm0 = _mm256_alignr_epi8(ymm1, ymm0, 11);
	ymm1 = _mm256_alignr_epi8(ymm2, ymm1, 11);
	ymm2 = _mm256_alignr_epi8(ymm1, ymm3, 11);


	ymm1 = _mm256_blendv_epi8(ymm1, ymm0, blendMask);

	ymm0 = _mm256_alignr_epi8(ymm3, ymm0, 11);
	ymm0 = _mm256_alignr_epi8(ymm0, ymm0, 10);

	rl = _mm256_cvtepu8_epi16(_mm256_castsi256_si128(ymm0));
	rh = _mm256_cvtepu8_epi16(_mm256_extracti128_si256(ymm0, 1));

	gl = _mm256_cvtepu8_epi16(_mm256_castsi256_si128(ymm1));
	gh = _mm256_cvtepu8_epi16(_mm256_extracti128_si256(ymm1, 1));

	bl = _mm256_cvtepu8_epi16(_mm256_castsi256_si128(ymm2));
	bh = _mm256_cvtepu8_epi16(_mm256_extracti128_si256(ymm2, 1));

}

inline void pack_16x16(__m256i a, __m256i b, __m256i c, __m256i d, __m256i& low, __m256i& high)
{
	__m256i packed16_0 = _mm256_packs_epi32(a, b);
	low = _mm256_permute4x64_epi64(packed16_0, _MM_SHUFFLE(3, 1, 2, 0));

	__m256i packed16_1 = _mm256_packs_epi32(c, d);
	high = _mm256_permute4x64_epi64(packed16_1, _MM_SHUFFLE(3, 1, 2, 0));

}

static const auto rmask = _mm256_setr_epi8(0, -1, -1, -1, 4, -1, -1, -1, 8, -1, -1, -1, 12, -1, -1, -1, 16, -1, -1, -1, 20, -1, -1, -1, 24, -1, -1, -1, 28, -1, -1, -1);
static const auto gmask = _mm256_setr_epi8(1, -1, -1, -1, 5, -1, -1, -1, 9, -1, -1, -1, 13, -1, -1, -1, 17, -1, -1, -1, 21, -1, -1, -1, 25, -1, -1, -1, 29, -1, -1, -1);
static const auto bmask = _mm256_setr_epi8(2, -1, -1, -1, 6, -1, -1, -1, 10, -1, -1, -1, 14, -1, -1, -1, 18, -1, -1, -1, 22, -1, -1, -1, 26, -1, -1, -1, 30, -1, -1, -1);

inline void GetChannels4_16x16(uint8_t* RESTRICT src, __m256i& rl, __m256i& gl, __m256i& bl, __m256i& rh, __m256i& gh, __m256i& bh)
{

	__m256i rgb1 = _mm256_loadu_si256((__m256i*)src);
	__m256i rgb2 = _mm256_loadu_si256((__m256i*)(src + 32));
	__m256i rgb3 = _mm256_loadu_si256((__m256i*)(src + 64));
	__m256i rgb4 = _mm256_loadu_si256((__m256i*)(src + 96));

	auto r1 = _mm256_shuffle_epi8(rgb1, rmask);
	auto g1 = _mm256_shuffle_epi8(rgb1, gmask);
	auto b1 = _mm256_shuffle_epi8(rgb1, bmask);

	auto r2 = _mm256_shuffle_epi8(rgb2, rmask);
	auto g2 = _mm256_shuffle_epi8(rgb2, gmask);
	auto b2 = _mm256_shuffle_epi8(rgb2, bmask);

	auto r3 = _mm256_shuffle_epi8(rgb3, rmask);
	auto g3 = _mm256_shuffle_epi8(rgb3, gmask);
	auto b3 = _mm256_shuffle_epi8(rgb3, bmask);

	auto r4 = _mm256_shuffle_epi8(rgb4, rmask);
	auto g4 = _mm256_shuffle_epi8(rgb4, gmask);
	auto b4 = _mm256_shuffle_epi8(rgb4, bmask);

	pack_16x16(r1, r2, r3, r4, rl, rh);
	pack_16x16(g1, g2, g3, g4, gl, gh);
	pack_16x16(b1, b2, b3, b4, bl, bh);
}

#endif
#endif