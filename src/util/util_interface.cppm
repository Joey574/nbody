module;
#include <cstddef>
#include <immintrin.h>

export module util;

export namespace util {

    size_t aligned_size(size_t size) {
        return (size + 31) & ~31;
    }

    float rsqrt(float x) {
    return _mm_cvtss_f32(_mm_rsqrt_ss(_mm_set_ss(x)));
    }

};