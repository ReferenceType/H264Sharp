#include "Rgb2Yuv.h"
#include <immintrin.h>
#include <stdint.h>
namespace H264Sharp {


    inline void GetChannels3_16x16(uint8_t* src, __m256i& rl, __m256i& gl, __m256i& bl, __m256i& rh, __m256i& gh, __m256i& bh);
    inline void GetChannels4_16x16(uint8_t* src, __m256i& rl, __m256i& gl, __m256i& bl, __m256i& rh, __m256i& gh, __m256i& bh);
   

    const __m256i constv_56 = _mm256_set1_epi16(56);
    const __m256i constv_47 = _mm256_set1_epi16(47);
    const __m256i constv_9 = _mm256_set1_epi16(9);
    const __m256i constv_19 = _mm256_set1_epi16(19);
    const __m256i constv_37 = _mm256_set1_epi16(37);
    const __m256i constv_16 = _mm256_set1_epi16(16);
    const __m256i constv_128 = _mm256_set1_epi16(128);
    const __m256i constv_66 = _mm256_set1_epi16(66);
    const __m256i constv_129 = _mm256_set1_epi16(129);
    const __m256i constv_25 = _mm256_set1_epi16(25);

    template <bool RGB, int NUM_CH>
    inline void RGBToI420_AVX2_(const uint8_t* src, uint8_t* y_plane, int width, int height, int stride, int begin, int end) {
        const int chroma_width = width / 2;
        const int src_stride = stride;
        const int y_stride = width;
        const int chroma_stride = chroma_width;
        uint8_t* u_plane = y_plane + (y_stride * height);
        uint8_t* v_plane = u_plane + (y_stride * height) / 4;

        for (int y = begin; y < end; y += 2)
        {
            for (int x = 0; x < width; x += 32) 
            {

                __m256i r0l, g0l, b0l, r0h, g0h, b0h; //first row
                __m256i r1l, g1l, b1l, r1h, g1h, b1h; //second row
                uint8_t* src_row1 = (uint8_t*)src + y * src_stride + (x * NUM_CH);
                uint8_t* src_row2 = (uint8_t*)src + ((y + 1) * src_stride) + (x * NUM_CH);

                if constexpr(NUM_CH > 3) 
                {
                    if constexpr(RGB)
                        GetChannels4_16x16(src_row1, r0l, g0l, b0l, r0h, g0h, b0h);
                    else
                        GetChannels4_16x16(src_row1, b0l, g0l, r0l, b0h, g0h, r0h);

                    if constexpr(RGB)
                        GetChannels4_16x16(src_row2, r1l, g1l, b1l, r1h, g1h, b1h);
                    else
                        GetChannels4_16x16(src_row2, b1l, g1l, r1l, b1h, g1h, r1h);
                }
                else 
                {
                    if constexpr (RGB)
                        GetChannels3_16x16(src_row1, r0l, g0l, b0l, r0h, g0h, b0h);
                    else
                        GetChannels3_16x16(src_row1, b0l, g0l, r0l, b0h, g0h, r0h);

                    if constexpr (RGB)
                        GetChannels3_16x16(src_row2, r1l, g1l, b1l, r1h, g1h, b1h);
                    else
                        GetChannels3_16x16(src_row2, b1l, g1l, r1l, b1h, g1h, r1h);
                    
                }

                auto ry0l = _mm256_mullo_epi16(r0l, constv_66);
                auto gy0l = _mm256_mullo_epi16(g0l, constv_129);
                auto by0l = _mm256_mullo_epi16(b0l, constv_25);

                auto ry0h = _mm256_mullo_epi16(r0h, constv_66);
                auto gy0h = _mm256_mullo_epi16(g0h, constv_129);
                auto by0h = _mm256_mullo_epi16(b0h, constv_25);

                auto ry1l = _mm256_mullo_epi16(r1l, constv_66);
                auto gy1l = _mm256_mullo_epi16(g1l, constv_129);
                auto by1l = _mm256_mullo_epi16(b1l, constv_25);

                auto ry1h = _mm256_mullo_epi16(r1h, constv_66);
                auto gy1h = _mm256_mullo_epi16(g1h, constv_129);
                auto by1h = _mm256_mullo_epi16(b1h, constv_25);


                __m256i y0l = _mm256_srli_epi16(_mm256_add_epi16(_mm256_add_epi16(ry0l, gy0l), by0l), 8);
                __m256i y0h = _mm256_srli_epi16(_mm256_add_epi16(_mm256_add_epi16(ry0h, gy0h), by0h), 8);

                __m256i y1l = _mm256_srli_epi16(_mm256_add_epi16(_mm256_add_epi16(ry1l, gy1l), by1l), 8);
                __m256i y1h = _mm256_srli_epi16(_mm256_add_epi16(_mm256_add_epi16(ry1h, gy1h), by1h), 8);

                y0l = _mm256_add_epi16(y0l, constv_16);
                y0h = _mm256_add_epi16(y0h, constv_16);
                y1l = _mm256_add_epi16(y1l, constv_16);
                y1h = _mm256_add_epi16(y1h, constv_16);

                __m256i packed = _mm256_packus_epi16(y0l, y0h);
                __m256i y0 = _mm256_permute4x64_epi64(packed, _MM_SHUFFLE(3, 1, 2, 0));

                packed = _mm256_packus_epi16(y1l, y1h);
                __m256i y1 = _mm256_permute4x64_epi64(packed, _MM_SHUFFLE(3, 1, 2, 0));


                _mm256_storeu_si256((__m256i*)(y_plane + y * y_stride + x), y0);
                _mm256_storeu_si256((__m256i*)(y_plane + (y + 1) * y_stride + x), y1);

                __m256i rul = _mm256_srli_epi16( _mm256_mullo_epi16(r0l, constv_56), 7);
                __m256i gul = _mm256_srli_epi16(_mm256_mullo_epi16(g0l, constv_47), 7);
                __m256i bul = _mm256_srli_epi16(_mm256_mullo_epi16(b0l, constv_9), 7);

                __m256i ruh = _mm256_srli_epi16(_mm256_mullo_epi16(r0h, constv_56), 7);
                __m256i guh = _mm256_srli_epi16(_mm256_mullo_epi16(g0h, constv_47), 7);
                __m256i buh = _mm256_srli_epi16(_mm256_mullo_epi16(b0h, constv_9), 7);

                __m256i rvl = _mm256_srli_epi16(_mm256_mullo_epi16(r0l, constv_19), 7);
                __m256i gvl = _mm256_srli_epi16(_mm256_mullo_epi16(g0l, constv_37), 7);
                __m256i bvl = _mm256_srli_epi16(_mm256_mullo_epi16(b0l, constv_56), 7);

                __m256i rvh = _mm256_srli_epi16(_mm256_mullo_epi16(r0h, constv_19), 7);
                __m256i gvh = _mm256_srli_epi16(_mm256_mullo_epi16(g0h, constv_37), 7);
                __m256i bvh = _mm256_srli_epi16(_mm256_mullo_epi16(b0h, constv_56), 7);

                __m256i ul = _mm256_sub_epi16(_mm256_sub_epi16(rul, gul),bul);
                __m256i uh = _mm256_sub_epi16(_mm256_sub_epi16(ruh, guh),buh);

                __m256i vl = _mm256_sub_epi16(_mm256_sub_epi16(bvl, gvl),rvl);
                __m256i vh = _mm256_sub_epi16(_mm256_sub_epi16(bvh, gvh),rvh);


                ul = _mm256_add_epi16(ul, constv_128);
                uh = _mm256_add_epi16(uh, constv_128);

                vl = _mm256_add_epi16(vl, constv_128);
                vh = _mm256_add_epi16(vh, constv_128);

                __m256i packed1 = _mm256_packus_epi16(ul, uh);
                __m256i U = _mm256_permute4x64_epi64(packed1, _MM_SHUFFLE(3, 1, 2, 0));
                
                __m256i packed2 = _mm256_packus_epi16(vl, vh);
                __m256i V = _mm256_permute4x64_epi64(packed2, _MM_SHUFFLE(3, 1, 2, 0));

                
                __m256i maskuv = _mm256_set1_epi16(0x00FF);  // Mask to keep only lower 8 bits
                U = _mm256_and_si256(U, maskuv);

                U = _mm256_packus_epi16(U, U);
                U = _mm256_permute4x64_epi64(U, _MM_SHUFFLE(3, 1, 2, 0));
                __m128i us = _mm256_castsi256_si128(U);

                maskuv = _mm256_set1_epi16(0x00FF);
                V = _mm256_and_si256(V, maskuv);

                V = _mm256_packus_epi16(V, V);
                V = _mm256_permute4x64_epi64(V, _MM_SHUFFLE(3, 1, 2, 0));
                __m128i vs = _mm256_castsi256_si128(V);
               
                _mm_storeu_si128((__m128i*)(v_plane + (y / 2) * chroma_stride + x / 2), us);
                _mm_storeu_si128((__m128i*)(u_plane + (y / 2) * chroma_stride + x / 2), vs);
            }
        }
    }

    void Rgb2Yuv::RGBToI420_AVX2(const uint8_t* src, uint8_t* y_plane, int width, int height, int stride, int numThreads)
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

                    RGBToI420_AVX2_<true, 3>(src, y_plane, width, height, stride, bgn, end);

                });
        }
        else 
        
        RGBToI420_AVX2_<true,3>(src, y_plane, width, height, stride,0,height);
    }

    void Rgb2Yuv::RGBAToI420_AVX2(const uint8_t* src, uint8_t* y_plane, int width, int height, int stride, int numThreads)
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

                    RGBToI420_AVX2_<true, 4>(src, y_plane, width, height, stride, bgn, end);

                });
        }
        else
            RGBToI420_AVX2_<true,4>(src, y_plane, width, height, stride, 0, height);
    }
    void Rgb2Yuv::BGRToI420_AVX2(const uint8_t* src, uint8_t* y_plane, int width, int height, int stride, int numThreads)
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

                    RGBToI420_AVX2_<false, 3>(src, y_plane, width, height, stride, bgn, end);

                });
        }
        else
            RGBToI420_AVX2_<false,3>(src, y_plane, width, height, stride, 0, height);
    }
    void Rgb2Yuv::BGRAToI420_AVX2(const uint8_t* src, uint8_t* y_plane, int width, int height, int stride, int numThreads)
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

                    RGBToI420_AVX2_<false, 4>(src, y_plane, width, height, stride, bgn, end);
                

                });
        }
        else
            RGBToI420_AVX2_<false,4>(src, y_plane, width, height, stride, 0, height);
    }




    alignas(32) const uint8_t shuffle_pattern[16] = {
        0, 3, 6, 9, 12, 15, 2, 5,
        8, 11, 14, 1, 4, 7, 10, 13
    };

    alignas(32) const uint8_t blend_mask[16] = {
        255, 255, 255, 255, 255, 255, 255, 255,
        255, 255, 255, 0, 0, 0, 0, 0
    };
    const __m256i shuffleMask = _mm256_broadcastsi128_si256(*(__m128i*)shuffle_pattern);
    const __m256i blendMask = _mm256_broadcastsi128_si256(*(__m128i*)blend_mask);

    inline void GetChannels3_16x16(uint8_t* input, __m256i& rl, __m256i& gl, __m256i& bl, __m256i& rh, __m256i& gh, __m256i& bh) 
    {
        
        // Load 96 bytes of input data into three YMM registers
        __m256i ymm0 = _mm256_inserti128_si256(_mm256_loadu_si256((__m256i*) & input[0]),
                                               _mm_loadu_si128((__m128i*) & input[48]),1);

        __m256i ymm1 = _mm256_inserti128_si256(_mm256_loadu_si256((__m256i*) & input[16]),
                                               _mm_loadu_si128((__m128i*) & input[64]),1);

        __m256i ymm2 = _mm256_inserti128_si256(_mm256_loadu_si256((__m256i*) & input[32]),
                                               _mm_loadu_si128((__m128i*) & input[80]),1);


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

        // Final alignment operations
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

    const auto rmask = _mm256_setr_epi8(0, -1, -1, -1, 4, -1, -1, -1, 8, -1, -1, -1, 12, -1, -1, -1, 16, -1, -1, -1, 20, -1, -1, -1, 24, -1, -1, -1, 28, -1, -1, -1);
    const auto gmask = _mm256_setr_epi8(1, -1, -1, -1, 5, -1, -1, -1, 9, -1, -1, -1, 13, -1, -1, -1, 17, -1, -1, -1, 21, -1, -1, -1, 25, -1, -1, -1, 29, -1, -1, -1);
    const auto bmask = _mm256_setr_epi8(2, -1, -1, -1, 6, -1, -1, -1, 10, -1, -1, -1, 14, -1, -1, -1, 18, -1, -1, -1, 22, -1, -1, -1, 26, -1, -1, -1, 30, -1, -1, -1);

    inline void GetChannels4_16x16(uint8_t* src, __m256i& rl, __m256i& gl, __m256i& bl, __m256i& rh, __m256i& gh, __m256i& bh)
    {

        __m256i rgb1 = _mm256_load_si256((__m256i*)src);
        __m256i rgb2 = _mm256_load_si256((__m256i*)(src + 32));
        __m256i rgb3 = _mm256_load_si256((__m256i*)(src + 64));
        __m256i rgb4 = _mm256_load_si256((__m256i*)(src + 96));

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


}