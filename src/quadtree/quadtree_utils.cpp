#include "quadtree.hpp"

void quadtree::debug_print_tree() const {
    for (size_t i = 0; i < size_; i++) {
        debug_print_node(i);
    }
}

void quadtree::debug_print_node(size_t i) const {
    printf("%lu: cm='%.2f', cx='%.2f', cy='%.2f', x=[%.2f, %.2f], y=[%.2f, %.2f], ne='%du', nw='%du', se='%du', sw='%du'\n",
        i, cm_[i], cx_[i], cy_[i], min_x_[i], max_x_[i], min_y_[i], max_y_[i], ne_[i], nw_[i], se_[i], sw_[i]
    );
}

void quadtree::grow(const size_t n) {
    if (!n || size_ >= n) return;

    this->size_ = n;
    free(cm_);
    free(cx_);
    free(cy_);
    free(min_x_);
    free(max_x_);
    free(min_y_);
    free(max_y_);
    free(ne_);
    free(nw_);
    free(se_);
    free(sw_);

    cm_ = (float*)aligned_alloc(MEM_ALIGNMENT, util::aligned_size(n*sizeof(float)));
    cx_ = (float*)aligned_alloc(MEM_ALIGNMENT, util::aligned_size(n*sizeof(float)));
    cy_ = (float*)aligned_alloc(MEM_ALIGNMENT, util::aligned_size(n*sizeof(float)));
    min_x_ = (float*)aligned_alloc(MEM_ALIGNMENT, util::aligned_size(n*sizeof(float)));
    max_x_ = (float*)aligned_alloc(MEM_ALIGNMENT, util::aligned_size(n*sizeof(float)));
    min_y_ = (float*)aligned_alloc(MEM_ALIGNMENT, util::aligned_size(n*sizeof(float)));
    max_y_ = (float*)aligned_alloc(MEM_ALIGNMENT, util::aligned_size(n*sizeof(float)));
    ne_ = (uint32_t*)aligned_alloc(MEM_ALIGNMENT, util::aligned_size(n*sizeof(uint32_t)));
    nw_ = (uint32_t*)aligned_alloc(MEM_ALIGNMENT, util::aligned_size(n*sizeof(uint32_t)));
    se_ = (uint32_t*)aligned_alloc(MEM_ALIGNMENT, util::aligned_size(n*sizeof(uint32_t)));
    sw_ = (uint32_t*)aligned_alloc(MEM_ALIGNMENT, util::aligned_size(n*sizeof(uint32_t)));
}

void quadtree::zero() {
    std::memset(cm_, 0, size_*sizeof(float));
    std::memset(cx_, 0, size_*sizeof(float));
    std::memset(cy_, 0, size_*sizeof(float));
    std::memset(min_x_, 0, size_*sizeof(float));
    std::memset(max_x_, 0, size_*sizeof(float));
    std::memset(min_y_, 0, size_*sizeof(float));
    std::memset(max_y_, 0, size_*sizeof(float));
    std::memset(ne_, 0, size_*sizeof(uint32_t));
    std::memset(nw_, 0, size_*sizeof(uint32_t));
    std::memset(se_, 0, size_*sizeof(uint32_t));
    std::memset(sw_, 0, size_*sizeof(uint32_t));
}

void quadtree::zero_children() {
    std::memset(ne_, 0, size_*sizeof(uint32_t));
    std::memset(nw_, 0, size_*sizeof(uint32_t));
    std::memset(se_, 0, size_*sizeof(uint32_t));
    std::memset(sw_, 0, size_*sizeof(uint32_t));
}
