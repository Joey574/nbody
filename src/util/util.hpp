#pragma once
#include "definitions/macros.hpp"

#include <cstddef>
#include <cstdlib>
#include <immintrin.h>
#include <climits>
#include <string>
#include <cstdint>
#include <vector>

namespace util {
    static inline size_t aligned_size(size_t size) {
        return (size + 31) & ~31;
    }

    // automatically handles aligning bytes to MEM_ALIGNMENT
    static inline void* mem_aligned_alloc(size_t bytes) {
        return aligned_alloc(MEM_ALIGNMENT, aligned_size(bytes));
    }

    static inline float frsqrt(float x) {
        return _mm_cvtss_f32(_mm_rsqrt_ss(_mm_set_ss(x)));
    }

    static inline float inv_sqrt(float x) {
        float inv = util::frsqrt(x);
        return inv * (1.5f - 0.5f * x * inv * inv);
    }

    static bool insertion_sort_capped(std::vector<uint64_t>& k, size_t max_shifts) {
        size_t shifts = 0;
        for (size_t i = 1; i < k.size(); i++) {
            const uint64_t v = k[i];
            size_t j = i;
            while (j > 0 && k[j-1] > v) { k[j] = k[j-1]; --j; ++shifts; }
            k[j] = v;
            if (shifts > max_shifts) return false;
        }

        return true;
    }

    inline size_t parse_string(const std::string& s) {
        if (s.empty()) { return 0; }

        size_t pos = 0;
        size_t base = std::stoul(s, &pos);

        if (pos < s.length()) {
            char modifier = std::toupper(s[pos]);
            switch (modifier) {
                case 'K': base *= 1000;       break;
                case 'M': base *= 1000000;    break;
                case 'B': base *= 1000000000; break;
            }
        }

        return base;
    }

    static inline bool is_near(float a, float b, float epsl = 1e-12f) {
        return std::abs(a-b) < epsl;
    }

    static inline uint32_t part1By1(uint32_t v) {
        v &= 0x0000FFFF;
        v = (v | (v << 8)) & 0x00FF00FF;
        v = (v | (v << 4)) & 0x0F0F0F0F;
        v = (v | (v << 2)) & 0x33333333;
        v = (v | (v << 1)) & 0x55555555;
        return v;
    }

    // https://stackoverflow.com/questions/26856268/morton-index-from-2d-point-with-floats
    static_assert(
        (sizeof(int) == sizeof(float)) &&
        (sizeof(int)*CHAR_BIT == 32) &&
        (sizeof(long long)*CHAR_BIT == 64),
        "We need 32-bit ints and floats, and 64-bit long longs!"
    );
    static inline long long morton_code(float x, float y) {
        auto ix = reinterpret_cast<const unsigned&>(x);
        auto iy = reinterpret_cast<const unsigned&>(y);

        const auto ixs = static_cast<int>(ix) >> 31;
        const auto iys = static_cast<int>(iy) >> 31;

        ix = (((ix & 0x7FFFFFFFL) ^ ixs) - ixs) + 0x7FFFFFFFL;
        iy = (((iy & 0x7FFFFFFFL) ^ iys) - iys) + 0x7FFFFFFFL;

        long long xx = ix;
        long long yy = iy;

        xx = (xx | (xx << 16)) & 0x0000FFFF0000FFFFLL;
        yy = (yy | (yy << 16)) & 0x0000FFFF0000FFFFLL;

        xx = (xx | (xx << 8 )) & 0x00FF00FF00FF00FFLL;
        yy = (yy | (yy << 8 )) & 0x00FF00FF00FF00FFLL;

        xx = (xx | (xx << 4 )) & 0x0F0F0F0F0F0F0F0FLL;
        yy = (yy | (yy << 4 )) & 0x0F0F0F0F0F0F0F0FLL;

        xx = (xx | (xx << 2 )) & 0x3333333333333333LL;
        yy = (yy | (yy << 2 )) & 0x3333333333333333LL;

        xx = (xx | (xx << 1 )) & 0x5555555555555555LL;
        yy = (yy | (yy << 1 )) & 0x5555555555555555LL;

        return xx | (yy << 1);
    }

    static inline constexpr float epsl = 1e-12f;

    static inline void apply_inner_force(
        float dx,
        float dy,
        float dsq,
        float p2m,
        float* __restrict a1x,
        float* __restrict a1y
    ) {
        float inv = util::frsqrt(dsq);
        inv = inv * (1.5f - 0.5f * dsq * inv * inv);

        const float inv3 = inv*inv*inv;

        const float ivx = dx * inv3;
        const float ivy = dy * inv3;

        a1x[0] += ivx * p2m;
        a1y[0] += ivy * p2m;
    }

    static inline void apply_force_to_one(
        float p1x,
        float p1y,
        float p2x,
        float p2y,
        float p2m,
        float* __restrict a1x,
        float* __restrict a1y
    ) {
        const float dx = p2x - p1x;
        const float dy = p2y - p1y;
        const float dsq = epsl + (dx*dx) + (dy*dy);

        apply_inner_force(
            dx, dy, dsq,
            p2m, a1x, a1y
        );
    }

    static inline void apply_force_to_two(
        float p1x,
        float p1y,
        float p1m,
        float p2x,
        float p2y,
        float p2m,
        float* __restrict a1x,
        float* __restrict a1y,
        float* __restrict a2x,
        float* __restrict a2y
    ) {
        const float dx = p2x - p1x;
        const float dy = p2y - p1y;
        const float dsq = epsl + (dx*dx) + (dy*dy);

        float inv = util::frsqrt(dsq);
        inv = inv * (1.5f - 0.5f * dsq * inv * inv);

        const float inv3 = inv*inv*inv;

        const float ivx = dx * inv3;
        const float ivy = dy * inv3;

        a1x[0] += ivx * p2m;
        a1y[0] += ivy * p2m;
        a2x[0] -= ivx * p1m;
        a2y[0] -= ivy * p1m;
    }

    #ifdef __AVX512F__
        using reg = __m512;
        static inline constexpr size_t width = 16;
        static inline constexpr size_t last = 15;
        static inline const __m512 _epsl = _mm512_set1_ps(epsl);
        static inline const __m512 _opf = _mm512_set1_ps(1.5f);
        static inline const __m512 _pf = _mm512_set1_ps(0.5f);

        static inline reg load(const float* p) { return _mm512_load_ps(p); }
        static inline void store(float* p, reg r) { _mm512_store_ps(p, r); }

        static inline reg loadu(const float* p) { return _mm512_loadu_ps(p); }
        static inline void storeu(float* p, reg r) { _mm512_storeu_ps(p, r); }

        static inline reg set1(float f) { return _mm512_set1_ps(f); }
        static inline reg zero() { return _mm512_setzero_ps(); }

        static inline reg min(reg a, reg b) { return _mm512_min_ps(a, b); }
        static inline reg max(reg a, reg b) { return _mm512_max_ps(a, b); }

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

            __m128 shuf64  = _mm_movehdup_ps(res128);
            __m128 res64   = _mm_add_ps(res128, shuf64);

            __m128 final   = _mm_add_ss(res64, _mm_movehl_ps(res64, res64));

            return _mm_cvtss_f32(final);
        }
    #elif defined(__AVX2__)
        using reg = __m256;
        static inline constexpr size_t width = 8;
        static inline constexpr size_t last = 7;
        static inline const __m256 _epsl = _mm256_set1_ps(epsl);
        static inline const __m256 _opf = _mm256_set1_ps(1.5f);
        static inline const __m256 _pf = _mm256_set1_ps(0.5f);

        static inline reg load(const float* p) { return _mm256_load_ps(p); }
        static inline void store(float* p, reg r) { _mm256_store_ps(p, r); }

        static inline reg loadu(const float* p) { return _mm256_loadu_ps(p); }
        static inline void storeu(float* p, reg r) { _mm256_storeu_ps(p, r); }

        static inline reg set1(float f) { return _mm256_set1_ps(f); }
        static inline reg zero() { return _mm256_setzero_ps(); }

        static inline reg min(reg a, reg b) { return _mm256_min_ps(a, b); }
        static inline reg max(reg a, reg b) { return _mm256_max_ps(a, b); }

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
    #else
        using reg = float;
        static inline constexpr size_t width = 1;
        static inline constexpr size_t last = 0;
        static inline const float _epsl = epsl;
        static inline const float _opf = 1.5f;
        static inline const float _pf = 0.5f;

        static inline reg load(const float* p) { return *p; }
        static inline void store(float* p, reg r) { *p = r; }

        static inline reg loadu(const float* p) { return load(p); }
        static inline void storeu(float* p, reg r) { store(p, r); }

        static inline reg set1(float f) { return f; }
        static inline reg zero() { return 0.0f; }

        static inline reg min(reg a, reg b) { return std::min(a, b); }
        static inline reg max(reg a, reg b) { return std::max(a, b); }

        static inline reg rsqrt(reg d2) {
            return frsqrt(d2);
        }
        static inline float hsum(reg r) {
           return r;
        }
    #endif
};
