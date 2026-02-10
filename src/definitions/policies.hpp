#pragma once
#include <immintrin.h>

#include "macros.hpp"
#include "../util/util.hpp"

namespace simd_policy {
    #ifdef USE_AVX512
    struct AVX512 {
        using reg = __m512;
        static inline constexpr size_t width = 16;
        static inline constexpr size_t last = 15;
        
        static inline reg load(const float* p) { return _mm512_load_ps(p); }
        static inline void store(float* p, reg r) { _mm512_store_ps(p, r); }
        
        static inline reg loadu(const float* p) { return _mm512_loadu_ps(p); }
        static inline void storeu(float* p, reg r) { _mm512_storeu_ps(p, r); }

        static inline reg set1(float f) { return _mm512_set1_ps(f); }
        static inline reg zero() { return _mm512_setzero_ps(); }
        
        static inline reg rsqrt(reg d2) {
            reg r = _mm512_rsqrt14_ps(d2);
            return r * (_opf - _pf * d2 * r * r);
        }
        static inline float hsum(reg r) { 
            __m256 low256  = _mm512_castps512_ps256(r);
            __m256 high256 = _mm512_extractf32x8_ps(r, 1);
            __m256 res256  = _mm256_add_ps(low256, high256);

            __m128 low128  = _mm256_castps256_ps128(res256);
            __m128 high128 = _mm256_extractf128_ps(res256, 1);
            __m128 res128  = _mm_add_ps(low128, high128);

            __m128 shuf64  = _mm_movehdup_ps(res128);        // Broadcast odds/evens or shuffle
            __m128 res64   = _mm_add_ps(res128, _mm_unpackhi_ps(res128, res128));

            __m128 final   = _mm_add_ss(res64, _mm_movehl_ps(res64, res64));

            return _mm_cvtss_f32(final);
        }

        static inline const __m512 _opf = _mm512_set1_ps(1.5f);
        static inline const __m512 _pf = _mm512_set1_ps(0.5f);
    };
    #endif

    #ifdef USE_AVX2
    struct AVX2 {
        using reg = __m256;
        static inline constexpr size_t width = 8;
        static inline constexpr size_t last = 7;
        
        static inline reg load(const float* p) { return _mm256_load_ps(p); }
        static inline void store(float* p, reg r) { _mm256_store_ps(p, r); }

        static inline reg loadu(const float* p) { return _mm256_loadu_ps(p); }
        static inline void storeu(float* p, reg r) { _mm256_storeu_ps(p, r); }
        
        static inline reg set1(float f) { return _mm256_set1_ps(f); }
        static inline reg zero() { return _mm256_setzero_ps(); }
        
        static inline reg rsqrt(reg d2) {
            reg r = _mm256_rsqrt_ps(d2);
            return r * (_opf - _pf * d2 * r * r);
        }
        static inline float hsum(reg r) { 
            __m128 vlow  = _mm256_castps256_ps128(r);
            __m128 vhigh = _mm256_extractf128_ps(r, 1);
            __m128 v128  = _mm_add_ps(vlow, vhigh);

            __m128 shuf  = _mm_movehl_ps(v128, v128);
            __m128 v64   = _mm_add_ps(v128, shuf);
            
            __m128 swiz  = _mm_shuffle_ps(v64, v64, _MM_SHUFFLE(1, 1, 1, 1)); 
            __m128 v32   = _mm_add_ss(v64, swiz);

            return _mm_cvtss_f32(v32);
        }

        static inline const __m256 _opf = _mm256_set1_ps(1.5f);
        static inline const __m256 _pf = _mm256_set1_ps(0.5f);
    };
    #endif
}