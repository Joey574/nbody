#pragma once
#include <cstddef>
#include <immintrin.h>

#include "../definitions/macros.hpp"

namespace util {
    size_t aligned_size(size_t size);    

    inline float rsqrt(float x) {
        return _mm_cvtss_f32(_mm_rsqrt_ss(_mm_set_ss(x)));
    }
} // namespace util
