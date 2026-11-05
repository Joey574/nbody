#include "definitions/macros.hpp"
#include "quadtree.hpp"
#include "util/util.hpp"

#include <assert.h>
#include <cstdint>
#include <cstdlib>

void quadtree::debug_print_tree() const {
    for (size_t i = 0; i <= size_; i++) {
        debug_print_node(i);
    }
}

void quadtree::debug_print_node(size_t i) const {
    assert(i <= size_);

    printf(
        "%zu: "
        "mass='%.3f', "
        "com=(%.3f, %.3f), "
        "geom=(%.3f, %.3f), "
        "w='%.3f'"
        "ne='%u', nw='%u', se='%u', sw='%u', "
        "next='%u'\n",
        i,
        cm_[i],
        cx_[i],
        cy_[i],
        gx_[i],
        gy_[i],
        w_[i],
        ne_[i],
        nw_[i],
        se_[i],
        sw_[i],
        next_[i]
    );
}

void quadtree::zero() {
    if (!capacity_) return;

    const size_t count = capacity_ + 1;
    const size_t bytes = count*sizeof(float);

    std::memset(cm_, 0, bytes);
    std::memset(cx_, 0, bytes);
    std::memset(cy_, 0, bytes);
    std::memset(gx_, 0, bytes);
    std::memset(gy_, 0, bytes);
    std::memset(w_, 0, bytes);

    this->zero_children();
}

void quadtree::zero_children() {
    if (!capacity_) return;

    const size_t count = capacity_ + 1;
    const size_t bytes = count*sizeof(uint32_t);

    std::memset(ne_, 0, bytes);
    std::memset(nw_, 0, bytes);
    std::memset(se_, 0, bytes);
    std::memset(sw_, 0, bytes);
    std::memset(next_, 0, bytes);
}

void quadtree::copy_from(const quadtree& other) {
    this->grow(other.size_);
    theta_ = other.theta_;
    size_ = other.size_;

    assert(capacity_ >= other.capacity_);
    assert(size_ >= other.size_);

    if (!size_) return;

    const size_t count = size_ + 1;
    const size_t floatBytes = count*sizeof(float);
    const size_t indexBytes = count*sizeof(uint32_t);

    std::memcpy(cm_, other.cm_, floatBytes);
    std::memcpy(cx_, other.cx_, floatBytes);
    std::memcpy(cy_, other.cy_, floatBytes);
    std::memcpy(gx_, other.gx_, floatBytes);
    std::memcpy(gy_, other.gy_, floatBytes);
    std::memcpy(w_, other.w_, floatBytes);

    std::memcpy(ne_, other.ne_, indexBytes);
    std::memcpy(nw_, other.nw_, indexBytes);
    std::memcpy(se_, other.se_, indexBytes);
    std::memcpy(sw_, other.sw_, indexBytes);
    std::memcpy(next_, other.next_, indexBytes);
}

void quadtree::grow(const size_t n) {
    if (n <= capacity_) return;

    this->release();
    capacity_ = n;

    const size_t count = n + 1;
    const size_t floatBytes = util::aligned_size(count * sizeof(float));
    const size_t indexBytes = util::aligned_size(count * sizeof(uint32_t));
    const size_t alignment = util::aligned_size(MEM_ALIGNMENT);

    cm_ = (float*)aligned_alloc(alignment, floatBytes);
    cx_ = (float*)aligned_alloc(alignment, floatBytes);
    cy_ = (float*)aligned_alloc(alignment, floatBytes);
    gx_ = (float*)aligned_alloc(alignment, floatBytes);
    gy_ = (float*)aligned_alloc(alignment, floatBytes);
    w_ = (float*)aligned_alloc(alignment, floatBytes);

    ne_ = (uint32_t*)aligned_alloc(alignment, indexBytes);
    nw_ = (uint32_t*)aligned_alloc(alignment, indexBytes);
    se_ = (uint32_t*)aligned_alloc(alignment, indexBytes);
    sw_ = (uint32_t*)aligned_alloc(alignment, indexBytes);
    next_ = (uint32_t*)aligned_alloc(alignment, indexBytes);
}

void quadtree::release() noexcept {
    free(cm_); cm_ = nullptr;
    free(cx_); cx_ = nullptr;
    free(cy_); cy_ = nullptr;
    free(gx_); gx_ = nullptr;
    free(gy_); gy_ = nullptr;
    free(w_); w_ = nullptr;

    free(ne_); ne_ = nullptr;
    free(nw_); nw_ = nullptr;
    free(se_); se_ = nullptr;
    free(sw_); sw_ = nullptr;
    free(next_); next_ = nullptr;

    capacity_ = 0;
    size_ = 0;
}
