#pragma once
#include <cstddef>
#include <immintrin.h>

namespace util {
    size_t aligned_size(size_t size);    

    inline float rsqrt(float x) {
        return _mm_cvtss_f32(_mm_rsqrt_ss(_mm_set_ss(x)));
    }

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

} // namespace util
