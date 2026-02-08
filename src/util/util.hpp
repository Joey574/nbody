#pragma once
#include <cstddef>
#include <immintrin.h>

#include "../definitions/macros.hpp"

namespace util {
    size_t aligned_size(size_t size);    

    inline float rsqrt(float x) {
        return _mm_cvtss_f32(_mm_rsqrt_ss(_mm_set_ss(x)));
    }

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

} // namespace util
