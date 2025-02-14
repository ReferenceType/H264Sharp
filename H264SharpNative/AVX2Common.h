#pragma once
#ifndef AVX_COMMON
#define AVX_COMMON

#include "pch.h"
#ifndef __arm__

#include <stdint.h>
#include <cstdint>

#ifdef _MSC_VER
#include <intrin.h>  // MSVC
#else
#include <cpuid.h>   // GCC/Clang
#endif
#include <array>

static bool hasAVX512() {
	std::array<int, 4> cpuInfo = {};

#ifdef _MSC_VER
	__cpuidex(cpuInfo.data(), 7, 0);
#else
	__cpuid_count(7, 0, cpuInfo[0], cpuInfo[1], cpuInfo[2], cpuInfo[3]);
#endif

	return (cpuInfo[1] & (1 << 16)) != 0;  // Check EBX bit 16 for AVX-512F
}

static bool hasAVX2() {
	int cpuInfo[4] = { 0 };  

#ifdef _MSC_VER
	__cpuidex(cpuInfo, 7, 0);
#else
	__cpuid_count(7, 0, cpuInfo[0], cpuInfo[1], cpuInfo[2], cpuInfo[3]);
#endif

	return (cpuInfo[1] & (1 << 5)) != 0;  // AVX2 is bit 5 of EBX (EAX=7, ECX=0)
}


const __m256i blendMask0 = _mm256_setr_epi8(
	0, 0, -1, 0, 0, -1, 0, 0, -1, 0, 0, -1, 0, 0, -1, 0,
	0, -1, 0, 0, -1, 0, 0, -1, 0, 0, -1, 0, 0, -1, 0, 0);

const __m256i blendMask1 = _mm256_setr_epi8(
	0, -1, 0, 0, -1, 0, 0, -1, 0, 0, -1, 0, 0, -1, 0, 0,
	-1, 0, 0, -1, 0, 0, -1, 0, 0, -1, 0, 0, -1, 0, 0, -1);

const __m256i shuffleMaskR = _mm256_setr_epi8(
	0, 3, 6, 9, 12, 15, 2, 5, 8, 11, 14, 1, 4, 7, 10, 13,
	0, 3, 6, 9, 12, 15, 2, 5, 8, 11, 14, 1, 4, 7, 10, 13);

const __m256i shuffleMaskG = _mm256_setr_epi8(
	1, 4, 7, 10, 13, 0, 3, 6, 9, 12, 15, 2, 5, 8, 11, 14,
	1, 4, 7, 10, 13, 0, 3, 6, 9, 12, 15, 2, 5, 8, 11, 14);

const __m256i shuffleMaskB = _mm256_setr_epi8(
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

const __m256i rgbaShuffleMask = _mm256_setr_epi8(0, 4, 8, 12, 1, 5, 9, 13, 2, 6, 10, 14, 3, 7, 11, 15,
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

const __m256i shuffleMaskB_s = _mm256_setr_epi8(
	0, 11, 6, 1, 12, 7, 2, 13, 8, 3, 14, 9, 4, 15, 10, 5,
	0, 11, 6, 1, 12, 7, 2, 13, 8, 3, 14, 9, 4, 15, 10, 5);

const __m256i shuffleMaskG_s = _mm256_setr_epi8(
	5, 0, 11, 6, 1, 12, 7, 2, 13, 8, 3, 14, 9, 4, 15, 10,
	5, 0, 11, 6, 1, 12, 7, 2, 13, 8, 3, 14, 9, 4, 15, 10);

const __m256i shuffleMaskR_s = _mm256_setr_epi8(
	10, 5, 0, 11, 6, 1, 12, 7, 2, 13, 8, 3, 14, 9, 4, 15,
	10, 5, 0, 11, 6, 1, 12, 7, 2, 13, 8, 3, 14, 9, 4, 15);

const __m256i blendMask0_s = _mm256_setr_epi8(
	0, -1, 0, 0, -1, 0, 0, -1, 0, 0, -1, 0, 0, -1, 0, 0,
    0, -1, 0, 0, -1, 0, 0, -1, 0, 0, -1, 0, 0, -1, 0, 0);

const __m256i blendMask1_s = _mm256_setr_epi8(
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

#endif
#endif