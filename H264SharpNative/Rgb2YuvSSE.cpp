#include "Rgb2Yuv.h"
#ifndef __arm__
namespace H264Sharp {

    inline void GetChannels3_16x8_SSE(const uint8_t* ptr, __m128i& al, __m128i& bl, __m128i& cl, __m128i& ah, __m128i& bh, __m128i& ch)
    {
        const __m128i m0 = _mm_setr_epi8(0, 0, -1, 0, 0, -1, 0, 0, -1, 0, 0, -1, 0, 0, -1, 0);
        const __m128i m1 = _mm_setr_epi8(0, -1, 0, 0, -1, 0, 0, -1, 0, 0, -1, 0, 0, -1, 0, 0);
        __m128i s0 = _mm_loadu_si128((const __m128i*)ptr);
        __m128i s1 = _mm_loadu_si128((const __m128i*)(ptr + 16));
        __m128i s2 = _mm_loadu_si128((const __m128i*)(ptr + 32));
        __m128i a0 = _mm_blendv_epi8(_mm_blendv_epi8(s0, s1, m0), s2, m1);
        __m128i b0 = _mm_blendv_epi8(_mm_blendv_epi8(s1, s2, m0), s0, m1);
        __m128i c0 = _mm_blendv_epi8(_mm_blendv_epi8(s2, s0, m0), s1, m1);
        const __m128i sh_b = _mm_setr_epi8(0, 3, 6, 9, 12, 15, 2, 5, 8, 11, 14, 1, 4, 7, 10, 13);
        const __m128i sh_g = _mm_setr_epi8(1, 4, 7, 10, 13, 0, 3, 6, 9, 12, 15, 2, 5, 8, 11, 14);
        const __m128i sh_r = _mm_setr_epi8(2, 5, 8, 11, 14, 1, 4, 7, 10, 13, 0, 3, 6, 9, 12, 15);
        __m128i a = _mm_shuffle_epi8(a0, sh_b);
        __m128i b = _mm_shuffle_epi8(b0, sh_g);
        __m128i c = _mm_shuffle_epi8(c0, sh_r);

        al = _mm_unpacklo_epi8(a, _mm_setzero_si128());
        ah = _mm_unpackhi_epi8(a, _mm_setzero_si128());

        bl = _mm_unpacklo_epi8(b, _mm_setzero_si128());
        bh = _mm_unpackhi_epi8(b, _mm_setzero_si128());

        cl = _mm_unpacklo_epi8(c, _mm_setzero_si128());
        ch = _mm_unpackhi_epi8(c, _mm_setzero_si128());
    }

    inline void GetChannels4_16x8_SSE(const uint8_t* ptr, __m128i& al, __m128i& bl, __m128i& cl, __m128i& ah, __m128i& bh, __m128i& ch)
    {
        __m128i u0 = _mm_loadu_si128((const __m128i*)ptr); // a0 b0 c0 d0 a1 b1 c1 d1 ...
        __m128i u1 = _mm_loadu_si128((const __m128i*)(ptr + 16)); // a4 b4 c4 d4 ...
        __m128i u2 = _mm_loadu_si128((const __m128i*)(ptr + 32)); // a8 b8 c8 d8 ...
        __m128i u3 = _mm_loadu_si128((const __m128i*)(ptr + 48)); // a12 b12 c12 d12 ...

        __m128i v0 = _mm_unpacklo_epi8(u0, u2); // a0 a8 b0 b8 ...
        __m128i v1 = _mm_unpackhi_epi8(u0, u2); // a2 a10 b2 b10 ...
        __m128i v2 = _mm_unpacklo_epi8(u1, u3); // a4 a12 b4 b12 ...
        __m128i v3 = _mm_unpackhi_epi8(u1, u3); // a6 a14 b6 b14 ...

        u0 = _mm_unpacklo_epi8(v0, v2); // a0 a4 a8 a12 ...
        u1 = _mm_unpacklo_epi8(v1, v3); // a2 a6 a10 a14 ...
        u2 = _mm_unpackhi_epi8(v0, v2); // a1 a5 a9 a13 ...
        u3 = _mm_unpackhi_epi8(v1, v3); // a3 a7 a11 a15 ...

        v0 = _mm_unpacklo_epi8(u0, u1); // a0 a2 a4 a6 ...
        v1 = _mm_unpacklo_epi8(u2, u3); // a1 a3 a5 a7 ...
        v2 = _mm_unpackhi_epi8(u0, u1); // c0 c2 c4 c6 ...
        v3 = _mm_unpackhi_epi8(u2, u3); // c1 c3 c5 c7 ...

        __m128i a = _mm_unpacklo_epi8(v0, v1);
        __m128i b = _mm_unpackhi_epi8(v0, v1);
        __m128i c = _mm_unpacklo_epi8(v2, v3);
        //__m128i d = _mm_unpackhi_epi8(v2, v3);

        al = _mm_unpacklo_epi8(a, _mm_setzero_si128());
        ah = _mm_unpackhi_epi8(a, _mm_setzero_si128());

        bl = _mm_unpacklo_epi8(b, _mm_setzero_si128());
        bh = _mm_unpackhi_epi8(b, _mm_setzero_si128());

        cl = _mm_unpacklo_epi8(c, _mm_setzero_si128());
        ch = _mm_unpackhi_epi8(c, _mm_setzero_si128());

    }

    // Packs 16-bit values from ul and uh into 8-bit without saturation
    inline __m128i pack_epi16_nosat(__m128i ul, __m128i uh) {
        __m128i low_bytes = _mm_and_si128(ul, _mm_set1_epi16(0x00FF));
        __m128i high_bytes = _mm_and_si128(uh, _mm_set1_epi16(0x00FF));

        return _mm_packus_epi16(low_bytes, high_bytes); // Merge into 8-bit lanes
    }

    const __m128i constv_56 = _mm_set1_epi16(56);
    const __m128i constv_47 = _mm_set1_epi16(47);
    const __m128i constv_9 = _mm_set1_epi16(9);

    const __m128i constv_19 = _mm_set1_epi16(19);
    const __m128i constv_37 = _mm_set1_epi16(37);

    const __m128i constv_16 = _mm_set1_epi8(16);
    const __m128i constv_128 = _mm_set1_epi16(128);

    const __m128i constv_66 = _mm_set1_epi16(66);
    const __m128i constv_129 = _mm_set1_epi16(129);
    const __m128i constv_25 = _mm_set1_epi16(25);

    const __m128i maskuv = _mm_set1_epi16(0x00FF);


    template <int NUM_CH, bool IS_RGB>
    inline void RGBToI420_SSE_Body(const uint8_t* RESTRICT src, uint8_t* RESTRICT y_plane, int32_t  width, int32_t  height, int32_t  stride, int32_t  begin, int32_t  end) {
        const int chroma_width = width / 2;
        const int src_stride = stride;
        const int y_stride = width;
        const int chroma_stride = chroma_width;
        uint8_t* RESTRICT u_plane = y_plane + (y_stride * height);
        uint8_t* RESTRICT v_plane = u_plane + (y_stride * height) / 4;

        for (int y = begin; y < end; y += 2)
        {
            for (int x = 0; x < width; x += 16)
            {

                __m128i r0l, g0l, b0l, r0h, g0h, b0h; //first row
                __m128i r1l, g1l, b1l, r1h, g1h, b1h; //second row
                uint8_t* RESTRICT src_row1 = (uint8_t*)src + y * src_stride + (x * NUM_CH);
                uint8_t* RESTRICT src_row2 = (uint8_t*)src + ((y + 1) * src_stride) + (x * NUM_CH);

                if constexpr (NUM_CH > 3)
                {
                    if constexpr (IS_RGB)
                        GetChannels4_16x8_SSE(src_row1, r0l, g0l, b0l, r0h, g0h, b0h);
                    else
                        GetChannels4_16x8_SSE(src_row1, b0l, g0l, r0l, b0h, g0h, r0h);

                    if constexpr (IS_RGB)
                        GetChannels4_16x8_SSE(src_row2, r1l, g1l, b1l, r1h, g1h, b1h);
                    else
                        GetChannels4_16x8_SSE(src_row2, b1l, g1l, r1l, b1h, g1h, r1h);
                }
                else
                {
                    if constexpr (IS_RGB)
                        GetChannels3_16x8_SSE(src_row1, r0l, g0l, b0l, r0h, g0h, b0h);
                    else
                        GetChannels3_16x8_SSE(src_row1, b0l, g0l, r0l, b0h, g0h, r0h);

                    if constexpr (IS_RGB)
                        GetChannels3_16x8_SSE(src_row2, r1l, g1l, b1l, r1h, g1h, b1h);
                    else
                        GetChannels3_16x8_SSE(src_row2, b1l, g1l, r1l, b1h, g1h, r1h);

                }
                /*
                *   buffer[yIndex++] = ((YB * b + YG * g + YR * r) >> Shift) + YOffset;
                    buffer[yIndex++] = ((YB * b1 + YG * g1 + YR * r1) >> Shift) + YOffset;


                    buffer[uIndex++] = ((UR * r + UG * g + UB * b) >> Shift) + UVOffset;
                    buffer[vIndex++] = ((VR * r + VG * g + VB * b) >> Shift) + UVOffset;
                */

                auto ry0l = _mm_mullo_epi16(r0l, constv_66);
                auto gy0l = _mm_mullo_epi16(g0l, constv_129);
                auto by0l = _mm_mullo_epi16(b0l, constv_25);

                auto ry0h = _mm_mullo_epi16(r0h, constv_66);
                auto gy0h = _mm_mullo_epi16(g0h, constv_129);
                auto by0h = _mm_mullo_epi16(b0h, constv_25);

                auto ry1l = _mm_mullo_epi16(r1l, constv_66);
                auto gy1l = _mm_mullo_epi16(g1l, constv_129);
                auto by1l = _mm_mullo_epi16(b1l, constv_25);

                auto ry1h = _mm_mullo_epi16(r1h, constv_66);
                auto gy1h = _mm_mullo_epi16(g1h, constv_129);
                auto by1h = _mm_mullo_epi16(b1h, constv_25);


                __m128i y0l = _mm_srli_epi16(_mm_add_epi16(_mm_add_epi16(ry0l, gy0l), by0l), 8);
                __m128i y0h = _mm_srli_epi16(_mm_add_epi16(_mm_add_epi16(ry0h, gy0h), by0h), 8);

                __m128i y1l = _mm_srli_epi16(_mm_add_epi16(_mm_add_epi16(ry1l, gy1l), by1l), 8);
                __m128i y1h = _mm_srli_epi16(_mm_add_epi16(_mm_add_epi16(ry1h, gy1h), by1h), 8);

                __m128i y0 = _mm_packus_epi16(y0l, y0h);
                __m128i y1 = _mm_packus_epi16(y1l, y1h);
                
                y0 = _mm_add_epi8(y0, constv_16);
                y1 = _mm_add_epi8(y1, constv_16);

                _mm_storeu_si128((__m128i*)(y_plane + y * y_stride + x), y0);
                _mm_storeu_si128((__m128i*)(y_plane + (y + 1) * y_stride + x), y1);

                __m128i rul = (_mm_mullo_epi16(r0l, constv_56));
                __m128i gul = (_mm_mullo_epi16(g0l, constv_47));
                __m128i bul = (_mm_mullo_epi16(b0l, constv_9));

                __m128i ruh = (_mm_mullo_epi16(r0h, constv_56));
                __m128i guh = (_mm_mullo_epi16(g0h, constv_47));
                __m128i buh = (_mm_mullo_epi16(b0h, constv_9));

                __m128i rvl = (_mm_mullo_epi16(r0l, constv_19));
                __m128i gvl = (_mm_mullo_epi16(g0l, constv_37));
                __m128i bvl = (_mm_mullo_epi16(b0l, constv_56));

                __m128i rvh = (_mm_mullo_epi16(r0h, constv_19));
                __m128i gvh = (_mm_mullo_epi16(g0h, constv_37));
                __m128i bvh = (_mm_mullo_epi16(b0h, constv_56));

                __m128i ul = _mm_srli_epi16(_mm_sub_epi16(rul, _mm_add_epi16(bul, gul)), 7);
                __m128i uh = _mm_srli_epi16(_mm_sub_epi16(ruh, _mm_add_epi16(buh, guh)), 7);

                __m128i vl = _mm_srli_epi16(_mm_sub_epi16(bvl, _mm_add_epi16(rvl, gvl)), 7);
                __m128i vh = _mm_srli_epi16(_mm_sub_epi16(bvh, _mm_add_epi16(rvh, gvh)), 7);


                ul = _mm_add_epi16(ul, constv_128);
                uh = _mm_add_epi16(uh, constv_128);

                vl = _mm_add_epi16(vl, constv_128);
                vh = _mm_add_epi16(vh, constv_128);

                __m128i U = pack_epi16_nosat(ul, uh);
                __m128i V = pack_epi16_nosat(vl, vh);

                U = _mm_and_si128(U, maskuv);
                U = _mm_packus_epi16(U, U);
                //auto us = _mm_cvtsi128_si64(U);


                V = _mm_and_si128(V, maskuv);
                V = _mm_packus_epi16(V, V);
                //auto vs = _mm_cvtsi128_si64(V);


                _mm_storel_epi64((__m128i*)(v_plane + (y / 2) * chroma_stride + x / 2), U);
                _mm_storel_epi64((__m128i*)(u_plane + (y / 2) * chroma_stride + x / 2), V);
            }
        }
    }

    //----
    template <int NUM_CH, bool IS_RGB>
    void Rgb2Yuv::RGBToI420_SSE(const uint8_t* RESTRICT src, uint8_t* RESTRICT y_plane, int32_t  width, int32_t  height, int32_t  stride, int32_t  numThreads)
    {

        if (numThreads > 1)
        {
            ThreadPool::ForRange(width, height, [&](int begin, int end)
                {
                    RGBToI420_SSE_Body<NUM_CH, IS_RGB>(src, y_plane, width, height, stride, begin, end);
                }, numThreads);
        }
        else
            RGBToI420_SSE_Body<NUM_CH, IS_RGB>(src, y_plane, width, height, stride, 0, height);


    }

    template void Rgb2Yuv::RGBToI420_SSE<3, false>(const uint8_t* RESTRICT src, uint8_t* RESTRICT y_plane, int32_t  width, int32_t  height, int32_t  stride, int32_t  numThreads);
    template void Rgb2Yuv::RGBToI420_SSE<3, true>(const uint8_t* RESTRICT src, uint8_t* RESTRICT y_plane, int32_t  width, int32_t  height, int32_t  stride, int32_t  numThreads);
    template void Rgb2Yuv::RGBToI420_SSE<4, false>(const uint8_t* RESTRICT src, uint8_t* RESTRICT y_plane, int32_t  width, int32_t  height, int32_t  stride, int32_t  numThreads);
    template void Rgb2Yuv::RGBToI420_SSE<4, true>(const uint8_t* RESTRICT src, uint8_t* RESTRICT y_plane, int32_t  width, int32_t  height, int32_t  stride, int32_t  numThreads);




}
#endif