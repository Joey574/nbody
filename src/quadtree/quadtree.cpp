#include "quadtree.hpp"
#include <algorithm>
#include <cstring>
#include <assert.h>

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

        min_y_[j] = std::min(min_y_[j], py[i]);
        max_y_[j] = std::max(max_y_[j], py[i]);
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

void quadtree::compute_acc(data& d) const {
    assert(d.bodies() == this->size_);

    d.accx().zero();
    d.accy().zero();

    #pragma omp parallel
    {
        int tid = omp_get_thread_num();

        const float* __restrict ma = d.mass();
        const float* __restrict px = d.posx();
        const float* __restrict py = d.posy();
        float* __restrict ax = d.accx().row(tid);
        float* __restrict ay = d.accy().row(tid);

        #pragma omp for schedule(guided)
        for (size_t i = 0; i < size_; i++) {
            compute_acc_recurse(i, 0, ma, px, py, ax, ay);
        }
    }
}

// TODO
// this needs to be turned into a linear loop
void quadtree::compute_acc_recurse(
    size_t i,
    size_t j,
    const float* __restrict ma,
    const float* __restrict px,
    const float* __restrict py,
    float* __restrict ax,
    float* __restrict ay
) const {
    if (i == j) {
        // there still exists other nodes below this
        // that we may need to check, recurse into children
        if (ne_[i]) compute_acc_recurse(i, ne_[i], ma, px, py, ax, ay);
        if (nw_[i]) compute_acc_recurse(i, nw_[i], ma, px, py, ax, ay);
        if (se_[i]) compute_acc_recurse(i, se_[i], ma, px, py, ax, ay);
        if (sw_[i]) compute_acc_recurse(i, sw_[i], ma, px, py, ax, ay);
        return;
    };

    const bool contains_self =
        (px[i] >= min_x_[j] && px[i] <= max_x_[j]) &&
        (py[i] >= min_y_[j] && py[i] <= max_y_[j]);

    const float width  = max_x_[j] - min_x_[j];
    const float height = max_y_[j] - min_y_[j];

    const float size = std::max(width, height);
    const float sizesq = size * size;

    const float p1x = px[i];
    const float p1y = py[i];
    const float p1m = ma[i];

    const float g2x = cx_[j];
    const float g2y = cy_[j];
    const float g2m = cm_[j];

    const float dx = g2x - p1x;
    const float dy = g2y - p1y;
    const float dsq = (dx*dx) + (dy*dy);

    if (!contains_self && sizesq / dsq < this->thetasq_) {
        // this group is far enough away to be approximated
        // as a single point

        util::apply_inner_force(
            dx, dy, dsq,
            g2m, &ax[i], &ay[i]
        );
    } else {
        // this body is too close / large to compute together
        // compute its root body and recurse into children
        util::apply_force_to_one(
            p1x, p1y,
            px[j], py[j], ma[j],
            &ax[i], &ay[i]
        );

        if (nw_[j]) compute_acc_recurse(i, nw_[j], ma, px, py, ax, ay);
        if (ne_[j]) compute_acc_recurse(i, ne_[j], ma, px, py, ax, ay);
        if (sw_[j]) compute_acc_recurse(i, sw_[j], ma, px, py, ax, ay);
        if (se_[j]) compute_acc_recurse(i, se_[j], ma, px, py, ax, ay);
    }
}
