#include "node.hpp"
#include "util/util.hpp"

#include <cstring>
#include <assert.h>

void nodes::push(float x, float y, float m) {
    if (size_+1 > capacity_) reserve(capacity_ ? capacity_*2 : 64);
    assert(capacity_ > size_);

    node_x_[size_] = x;
    node_y_[size_] = y;
    node_m_[size_] = m;
    node_next_[size_] = 0;
    node_children_[size_] = 0;
    size_++;
}

void nodes::push_empty(size_t next) {
    if (size_+1 > capacity_) reserve(capacity_ ? capacity_*2 : 64);
    assert(capacity_ > size_);

    node_x_[size_] = 0.0f;
    node_y_[size_] = 0.0f;
    node_m_[size_] = 0.0f;
    node_children_[size_] = 0;

    node_next_[size_] = next;
    size_++;

}

void nodes::set(size_t i, float x, float y, float m) {
    assert(i < size_);

    node_x_[i] = x;
    node_y_[i] = y;
    node_m_[i] = m;
}

void nodes::reserve(size_t n) {
    if (capacity_ >= n) return;

    const size_t floatBytes = n * sizeof(float);
    const size_t sizeBytes = n * sizeof(size_t);
    const size_t floatUsed = size_ * sizeof(float);
    const size_t sizeUsed = size_ * sizeof(size_t);

    float* __restrict xs = (float*)util::mem_aligned_alloc(floatBytes);
    float* __restrict ys = (float*)util::mem_aligned_alloc(floatBytes);
    float* __restrict ms = (float*)util::mem_aligned_alloc(floatBytes);
    size_t* __restrict ns = (size_t*)util::mem_aligned_alloc(sizeBytes);
    size_t* __restrict cs = (size_t*)util::mem_aligned_alloc(sizeBytes);
    size_t* __restrict ps = (size_t*)util::mem_aligned_alloc(sizeBytes);

    if (size_) {
        std::memcpy(xs, node_x_, floatUsed);
        std::memcpy(ys, node_y_, floatUsed);
        std::memcpy(ms, node_m_, floatUsed);
        std::memcpy(ns, node_next_, sizeUsed);
        std::memcpy(cs, node_children_, sizeUsed);
    }

    free(node_x_);
    free(node_y_);
    free(node_m_);
    free(node_next_);
    free(node_children_);

    node_x_ = xs;
    node_y_ = ys;
    node_m_ = ms;
    node_next_ = ns;
    node_children_ = cs;
    capacity_ = n;
}

void nodes::clear() {
    size_ = 0;
}

void nodes::zero() {
    const size_t floatBytes = size_ * sizeof(float);
    const size_t sizeBytes = size_ * sizeof(size_t);

    std::memset(node_x_, 0, floatBytes);
    std::memset(node_y_, 0, floatBytes);
    std::memset(node_m_, 0, floatBytes);
    std::memset(node_next_, 0, sizeBytes);
    std::memset(node_children_, 0, sizeBytes);
}

void nodes::cleanup() {
    free(node_x_); node_x_ = nullptr;
    free(node_y_); node_y_ = nullptr;
    free(node_m_); node_m_ = nullptr;
    free(node_next_); node_next_ = nullptr;
    free(node_children_); node_children_ = nullptr;

    size_ = 0;
    capacity_ = 0;
}
