#include "quad.hpp"
#include "util/util.hpp"

#include <cstring>

void quads::push(float x, float y, float s) {
    if (size_+1 > capacity_) reserve(capacity_ ? capacity_*2 : 64);


    quad_x_[size_] = x;
    quad_y_[size_] = y;
    quad_s_[size_] = s;
    size_++;
}

void quads::push_quadrant(size_t quad, size_t quadrant) {
    const float size = quad_s_[quad] * 0.5f;
    const float x = quad_x_[quad] + (float)((quadrant & 1) - 0.5f) * size;
    const float y = quad_y_[quad] + (float)((quadrant >> 1) - 0.5f) * size;
    push(x, y, size);
}

void quads::subdivide_and_push(size_t quad, size_t nexts[4], nodes& n) {
    for (size_t i = 0; i < 4; i++) {
        push_quadrant(quad, i);
        n.push_empty(nexts[i]);
    }
}

void quads::reserve(size_t n) {
    if (capacity_ >= n) return;

    const size_t bytes = n * sizeof(float);
    const size_t used = size_ * sizeof(float);

    float* __restrict xs = (float*)util::mem_aligned_alloc(bytes);
    float* __restrict ys = (float*)util::mem_aligned_alloc(bytes);
    float* __restrict ss = (float*)util::mem_aligned_alloc(bytes);

    if (size_) {
        std::memcpy(xs, quad_x_, used);
        std::memcpy(ys, quad_y_, used);
        std::memcpy(ss, quad_s_, used);
    }

    free(quad_s_); quad_s_ = ss;
    free(quad_x_); quad_x_ = xs;
    free(quad_y_); quad_y_ = ys;

    capacity_ = n;
}

void quads::clear() {
    size_ = 0;
}

void quads::zero() {
    const size_t bytes = size_ * sizeof(float);

    std::memset(quad_x_, 0, bytes);
    std::memset(quad_y_, 0, bytes);
    std::memset(quad_s_, 0, bytes);
}

void quads::cleanup() {
    free(quad_x_); quad_x_ = nullptr;
    free(quad_y_); quad_y_ = nullptr;
    free(quad_s_); quad_s_ = nullptr;

    size_ = 0;
    capacity_ = 0;
}
