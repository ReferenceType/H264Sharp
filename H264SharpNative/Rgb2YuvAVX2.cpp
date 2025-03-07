#include "Rgb2Yuv.h"
#ifndef ARM

#include "AVX2Common.h"


namespace H264Sharp {

    // Packs 16-bit values from ul and uh into 8-bit without saturation
    inline __m256i pack_epi16_nosat(__m256i ul, __m256i uh) {
        __m256i low_bytes = _mm256_and_si256(ul, _mm256_set1_epi16(0x00FF)); 
        __m256i high_bytes = _mm256_and_si256(uh, _mm256_set1_epi16(0x00FF));

        return _mm256_packus_epi16(low_bytes, high_bytes); // Merge into 8-bit lanes
    }

  
    /*
    const int16_t YB = 25;
    const int16_t YG = 129;
    const int16_t YR = 66;
   

    const int16_t UR = 112;
    const int16_t UG = -94;
    const int16_t UB = -18;

    const int16_t VR = -38;
    const int16_t VG = -74;
    const int16_t VB = 112;
    */
    const __m256i constv_56 =  _mm256_set1_epi16(56);
    const __m256i constv_47 =  _mm256_set1_epi16(47);
    const __m256i constv_9 =   _mm256_set1_epi16(9);

    const __m256i constv_19 =  _mm256_set1_epi16(19);
    const __m256i constv_37 =  _mm256_set1_epi16(37);

    const __m256i constv_16 = _mm256_set1_epi8(16);
    const __m256i constv_128 = _mm256_set1_epi16(128);

    const __m256i constv_66 =  _mm256_set1_epi16(66);
    const __m256i constv_129 = _mm256_set1_epi16(129);
    const __m256i constv_25 =  _mm256_set1_epi16(25);

    const __m256i maskuv = _mm256_set1_epi16(0x00FF);  // Mask to keep only lower 8 bits

    template <int NUM_CH, bool IS_RGB>
    inline void RGBToI420_AVX2_(const uint8_t* RESTRICT src, uint8_t* RESTRICT y_plane, int32_t  width, int32_t  height, int32_t  stride, int32_t  begin, int32_t  end) {
        const int chroma_width = width / 2;
        const int src_stride = stride;
        const int y_stride = width;
        const int chroma_stride = chroma_width;
        uint8_t* RESTRICT u_plane = y_plane + (y_stride * height);
        uint8_t* RESTRICT v_plane = u_plane + (y_stride * height) / 4;

        for (int y = begin; y < end; y += 2)
        {
            for (int x = 0; x < width; x += 32) 
            {

                __m256i r0l, g0l, b0l, r0h, g0h, b0h; //first row
                __m256i r1l, g1l, b1l, r1h, g1h, b1h; //second row
                uint8_t* RESTRICT src_row1 = (uint8_t*)src + y * src_stride + (x * NUM_CH);
                uint8_t* RESTRICT src_row2 = (uint8_t*)src + ((y + 1) * src_stride) + (x * NUM_CH);

                if constexpr(NUM_CH > 3) 
                {
                    if constexpr(IS_RGB)
                        GetChannels4_16x16(src_row1, r0l, g0l, b0l, r0h, g0h, b0h);
                    else
                        GetChannels4_16x16(src_row1, b0l, g0l, r0l, b0h, g0h, r0h);

                    if constexpr(IS_RGB)
                        GetChannels4_16x16(src_row2, r1l, g1l, b1l, r1h, g1h, b1h);
                    else
                        GetChannels4_16x16(src_row2, b1l, g1l, r1l, b1h, g1h, r1h);
                }
                else 
                {
                    if constexpr (IS_RGB)
                        GetChannels3_16x16(src_row1, r0l, g0l, b0l, r0h, g0h, b0h);
                    else
                        GetChannels3_16x16(src_row1, b0l, g0l, r0l, b0h, g0h, r0h);

                    if constexpr (IS_RGB)
                        GetChannels3_16x16(src_row2, r1l, g1l, b1l, r1h, g1h, b1h);
                    else
                        GetChannels3_16x16(src_row2, b1l, g1l, r1l, b1h, g1h, r1h);
                    
                }
                /*
                *   buffer[yIndex++] = ((YB * b + YG * g + YR * r) >> Shift) + YOffset;
                    buffer[yIndex++] = ((YB * b1 + YG * g1 + YR * r1) >> Shift) + YOffset;


                    buffer[uIndex++] = ((UR * r + UG * g + UB * b) >> Shift) + UVOffset;
                    buffer[vIndex++] = ((VR * r + VG * g + VB * b) >> Shift) + UVOffset;
                */

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

                __m256i packed = _mm256_packus_epi16(y0l, y0h);
                __m256i y0 = _mm256_permute4x64_epi64(packed, _MM_SHUFFLE(3, 1, 2, 0));

                packed = _mm256_packus_epi16(y1l, y1h);
                __m256i y1 = _mm256_permute4x64_epi64(packed, _MM_SHUFFLE(3, 1, 2, 0));

                y0 = _mm256_add_epi8(y0, constv_16);
                y1 = _mm256_add_epi8(y1, constv_16);

                _mm256_storeu_si256((__m256i*)(y_plane + y * y_stride + x), y0);
                _mm256_storeu_si256((__m256i*)(y_plane + (y + 1) * y_stride + x), y1);

                __m256i rul = ( _mm256_mullo_epi16(r0l, constv_56));
                __m256i gul = (_mm256_mullo_epi16(g0l, constv_47));
                __m256i bul = (_mm256_mullo_epi16(b0l, constv_9));

                __m256i ruh = (_mm256_mullo_epi16(r0h, constv_56));
                __m256i guh = (_mm256_mullo_epi16(g0h, constv_47));
                __m256i buh = (_mm256_mullo_epi16(b0h, constv_9));

                __m256i rvl = (_mm256_mullo_epi16(r0l, constv_19));
                __m256i gvl = (_mm256_mullo_epi16(g0l, constv_37));
                __m256i bvl = (_mm256_mullo_epi16(b0l, constv_56));

                __m256i rvh = (_mm256_mullo_epi16(r0h, constv_19));
                __m256i gvh = (_mm256_mullo_epi16(g0h, constv_37));
                __m256i bvh = (_mm256_mullo_epi16(b0h, constv_56));

                __m256i ul = _mm256_srli_epi16(_mm256_sub_epi16(rul,_mm256_add_epi16(bul, gul)),7);
                __m256i uh = _mm256_srli_epi16(_mm256_sub_epi16(ruh,_mm256_add_epi16(buh, guh)),7);

                __m256i vl = _mm256_srli_epi16(_mm256_sub_epi16(bvl, _mm256_add_epi16(rvl, gvl)),7);
                __m256i vh = _mm256_srli_epi16(_mm256_sub_epi16(bvh, _mm256_add_epi16(rvh, gvh)), 7);

               
                ul = _mm256_add_epi16(ul, constv_128);
                uh = _mm256_add_epi16(uh, constv_128);

                vl = _mm256_add_epi16(vl, constv_128);
                vh = _mm256_add_epi16(vh, constv_128);

                packed = pack_epi16_nosat(ul, uh);
                __m256i U = _mm256_permute4x64_epi64(packed, _MM_SHUFFLE(3, 1, 2, 0));
                
                packed = pack_epi16_nosat(vl, vh);
                __m256i V = _mm256_permute4x64_epi64(packed, _MM_SHUFFLE(3, 1, 2, 0));

               
                U = _mm256_and_si256(U, maskuv);

                U = _mm256_packus_epi16(U, U);
                U = _mm256_permute4x64_epi64(U, _MM_SHUFFLE(3, 1, 2, 0));
                __m128i us = _mm256_castsi256_si128(U);

               
                V = _mm256_and_si256(V, maskuv);

                V = _mm256_packus_epi16(V, V);
                V = _mm256_permute4x64_epi64(V, _MM_SHUFFLE(3, 1, 2, 0));
                __m128i vs = _mm256_castsi256_si128(V);
               
              
                _mm_storeu_si128((__m128i*)(v_plane + (y / 2) * chroma_stride + x / 2), us);
                _mm_storeu_si128((__m128i*)(u_plane + (y / 2) * chroma_stride + x / 2), vs);
            }
        }
    }

    template <int NUM_CH, bool IS_RGB>
    void Rgb2Yuv::RGBXToI420_AVX2(const uint8_t* RESTRICT src, uint8_t* RESTRICT y_plane, int32_t  width, int32_t  height, int32_t  stride, int32_t  numThreads)
    {

        if (numThreads > 1)
        {
            ThreadPool::ForRange(width, height, [&](int begin, int end)
                {
                    RGBToI420_AVX2_<NUM_CH, IS_RGB>(src, y_plane, width, height, stride, begin, end);
                }, numThreads);
        }
        else
            RGBToI420_AVX2_<NUM_CH, IS_RGB>(src, y_plane, width, height, stride, 0, height);

       
    }

    template void Rgb2Yuv::RGBXToI420_AVX2<3, false>(const uint8_t* RESTRICT src,  uint8_t* RESTRICT y_plane, int32_t  width, int32_t  height, int32_t  stride, int32_t  numThreads);
    template void Rgb2Yuv::RGBXToI420_AVX2<3, true>(const uint8_t* RESTRICT src,  uint8_t* RESTRICT y_plane, int32_t  width, int32_t  height, int32_t  stride, int32_t  numThreads);
    template void Rgb2Yuv::RGBXToI420_AVX2<4, false>(const uint8_t* RESTRICT src,  uint8_t* RESTRICT y_plane, int32_t  width, int32_t  height, int32_t  stride, int32_t  numThreads);
    template void Rgb2Yuv::RGBXToI420_AVX2<4, true>(const uint8_t* RESTRICT src,  uint8_t* RESTRICT y_plane, int32_t  width, int32_t  height, int32_t  stride, int32_t  numThreads);




}
#endif