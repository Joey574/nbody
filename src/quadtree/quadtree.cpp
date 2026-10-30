#include "quadtree.hpp"
#include <algorithm>
#include <cstring>
#include <assert.h>
#include <yaml-cpp/node/detail/node.h>

void quadtree::insert(const data& d) {
    const size_t n = d.bodies();
    if (!n) return;

    this->grow(n);
    this->zero_children();

    // alias
    const float* __restrict px = d.posx();
    const float* __restrict py = d.posy();
    const float* __restrict ma = d.mass();

    // insert node data
    std::memcpy(cm_, ma, n*sizeof(float));
    std::memcpy(cx_, px, n*sizeof(float));
    std::memcpy(cy_, py, n*sizeof(float));
    std::memcpy(min_x_, px, n*sizeof(float));
    std::memcpy(max_x_, px, n*sizeof(float));
    std::memcpy(min_y_, py, n*sizeof(float));
    std::memcpy(max_y_, py, n*sizeof(float));

    const auto update_root = [&](size_t i, size_t j) {
        // update parent cm, cx, cy
        const float old_mass = cm_[j];
        cm_[j] += ma[i];

        cx_[j] = ((cx_[j] * old_mass) + (ma[i] * px[i])) / (cm_[j]);
        cy_[j] = ((cy_[j] * old_mass) + (ma[i] * py[i])) / (cm_[j]);

        min_x_[j] = std::min(min_x_[j], px[i]);
        max_x_[j] = std::max(max_x_[j], px[i]);
    };

    for (size_t i = 1; i < n; i++) {
        for (size_t j = 0;;) {
            update_root(i, j);

            uint32_t* v =
                (px[i] > px[j] && py[i] > py[j]) ? &ne_[j] :
                (px[i] < px[j] && py[i] > py[j]) ? &nw_[j] :
                (px[i] > px[j] && py[i] < py[j]) ? &se_[j] :
                &sw_[j];
            uint32_t k = *v;

            if (!k) {
                v[0] = i;
                break;
            }

            j = k;
        }
    }
}

void quadtree::compute_acc(data& d) {
    assert(d.bodies() == this->size_);

    d.accx().zero();
    d.accy().zero();

    #pragma omp parallel
    {
        int tid = omp_get_thread_num();

        #pragma omp for schedule(dynamic)
        for (size_t i = 0; i < size_; i++) {
            compute_acc_recurse(d, i, 0, tid);
        }
    }
}

void quadtree::debug_print_tree() {
    for (size_t i = 0; i < size_; i++) {
        printf("%lu: cm='%.2f', cx='%.2f', cy='%.2f', ne='%du', nw='%du', se='%du', sw='%du'\n",
            i, cm_[i], cx_[i], cy_[i], ne_[i], nw_[i], se_[i], sw_[i]
        );
    }
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

// TODO
// this needs to be turned into a linear loop
void quadtree::compute_acc_recurse(data& d, size_t i, size_t j, int tid) {
    if (i == j) {
        // there still exists other nodes below this
        // that we may need to check, recurse into children
        if (ne_[i]) compute_acc_recurse(d, i, ne_[i], tid);
        if (nw_[i]) compute_acc_recurse(d, i, nw_[i], tid);
        if (se_[i]) compute_acc_recurse(d, i, se_[i], tid);
        if (sw_[i]) compute_acc_recurse(d, i, sw_[i], tid);
        return;
    };

    const size_t n = d.bodies();

    const float* __restrict ma = d.mass();
    const float* __restrict px = d.posx();
    const float* __restrict py = d.posy();
    float* __restrict ax_row = d.accx().row(tid);
    float* __restrict ay_row = d.accy().row(tid);

    const float size = ((max_x_[j] - min_x_[j]) + (max_y_[j] - min_y_[j])) * 0.5f;
    const float sizesq = size*size;

    const float p1x = px[i];
    const float p1y = py[i];
    const float p1m = ma[i];

    const float g2x = cx_[j];
    const float g2y = cy_[j];
    const float g2m = cm_[j];

    const float dx = g2x - p1x;
    const float dy = g2y - p1y;
    const float dsq = util::epsl + (dx*dx) + (dy*dy);

    if (sizesq / dsq < this->thetasq_) {
        // this group is far enough away to be approximated
        // as a single group

        util::apply_inner_force(
            dx, dy, dsq,
            g2m, &ax_row[i], &ay_row[i]
        );
    } else {
        // this body is too close / large to compute together
        // compute its root body and recurse into children
        util::apply_force_to_one(
            p1x, p1y,
            px[j], py[j], ma[j],
            &ax_row[i], &ay_row[i]
        );

        if (nw_[j]) compute_acc_recurse(d, i, nw_[j], tid);
        if (ne_[j]) compute_acc_recurse(d, i, ne_[j], tid);
        if (sw_[j]) compute_acc_recurse(d, i, sw_[j], tid);
        if (se_[j]) compute_acc_recurse(d, i, se_[j], tid);
    }
}
