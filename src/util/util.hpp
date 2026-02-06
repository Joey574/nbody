#pragma once
#include <cstddef>
#include <immintrin.h>

namespace util {
    size_t aligned_size(size_t size);    

    inline float rsqrt(float x) {
        return _mm_cvtss_f32(_mm_rsqrt_ss(_mm_set_ss(x)));
    }

    #if defined(__AVX2__)
    inline float sum256(const __m256 v) {
        __m128 vlow  = _mm256_castps256_ps128(v);
        __m128 vhigh = _mm256_extractf128_ps(v, 1);
        __m128 v128  = _mm_add_ps(vlow, vhigh);

        __m128 shuf  = _mm_movehl_ps(v128, v128);
        __m128 v64   = _mm_add_ps(v128, shuf);
        
        __m128 swiz  = _mm_shuffle_ps(v64, v64, _MM_SHUFFLE(1, 1, 1, 1)); 
        __m128 v32   = _mm_add_ss(v64, swiz);

        return _mm_cvtss_f32(v32);
    }
    #endif

    #if defined(__AVX512F__)
    inline float sum512(const __m512 v) {
        __m256 low256  = _mm512_castps512_ps256(v);
        __m256 high256 = _mm512_extractf32x8_ps(v, 1);
        __m256 res256  = _mm256_add_ps(low256, high256);

        __m128 low128  = _mm256_castps256_ps128(res256);
        __m128 high128 = _mm256_extractf128_ps(res256, 1);
        __m128 res128  = _mm_add_ps(low128, high128);

        __m128 shuf64  = _mm_movehdup_ps(res128);        // Broadcast odds/evens or shuffle
        __m128 res64   = _mm_add_ps(res128, _mm_unpackhi_ps(res128, res128));

        __m128 final   = _mm_add_ss(res64, _mm_movehl_ps(res64, res64));

        return _mm_cvtss_f32(final);
    }
    #endif

} // namespace util
