#include "quadtree.hpp"
#include <algorithm>
#include <cstring>
#include <assert.h>

void quadtree::insert(const data& d) {
    const size_t n = d.bodies();
    if (!n) return;

    this->grow(n);
    this->zero();

    // alias
    const float* __restrict px = d.posx();
    const float* __restrict py = d.posy();
    const float* __restrict ma = d.mass();

    // insert node data
    #pragma omp parallel for simd schedule(static)
    for (size_t i = 0; i < n; i++) {
        nodes_[i].cm = ma[i];
        nodes_[i].cx = px[i];
        nodes_[i].cy = py[i];
        nodes_[i].min_x = px[i];
        nodes_[i].max_x = px[i];
    }

    const auto update_root = [&](size_t i, size_t j) {
        // update parent cm, cx, cy
        nodes_[j].cm += ma[i];
        nodes_[j].cx += (ma[i] * px[i]) / nodes_[j].cm;
        nodes_[j].cy += (ma[i] * py[i]) / nodes_[j].cm;
        nodes_[j].min_x = std::min(nodes_[j].min_x, px[i]);
        nodes_[j].max_x = std::max(nodes_[j].max_x, px[i]);
    };

    for (size_t i = 1; i < n; i++) {
        for (size_t j = 0;;) {
            if (i == j) break;

            update_root(i, j);

            if (px[i] > px[j] && py[i] > py[j]) { // NE
                if (!nodes_[j].ne) {
                    // node is empty, insert
                    nodes_[j].ne = i;
                    break;
                } else {
                    // node is populated, update k and recurse
                    j = nodes_[j].ne;
                    continue;
                }

            } else if (px[i] < px[j] && py[i] > py[j]) { // NW
                if (!nodes_[j].nw) {
                    // node is empty, insert
                    nodes_[j].nw = i;
                    break;
                } else {
                    // node is populated, update k and recurse
                    j = nodes_[j].nw;
                    continue;
                }

            } else if (px[i] > px[j] && py[i] < py[j]) { // SE
                if (!nodes_[j].se) {
                    // node is empty, insert
                    nodes_[j].se = i;
                    break;
                } else {
                    // node is populated, update k and recurse
                    j = nodes_[j].se;
                    continue;
                }

            } else { // SW
                if (!nodes_[j].sw) {
                    // node is empty, insert
                    nodes_[j].sw = i;
                    break;
                } else {
                    // node is populated, update k and recurse
                    j = nodes_[j].sw;
                    continue;
                }
            }
        }
    }

    #ifdef DEBUG
    this->insert_assertions(d);
    #endif
}

void quadtree::compute_acc(data& d) {
    assert(d.bodies() == this->size_);

    const size_t ne = nodes_[0].ne;
    const size_t nw = nodes_[0].nw;
    const size_t se = nodes_[0].se;
    const size_t sw = nodes_[0].sw;

    for (size_t i = 0; i < size_; i++) {
        // for each node, check childen
        compute_acc_recurse(d, i, ne);
        compute_acc_recurse(d, i, nw);
        compute_acc_recurse(d, i, se);
        compute_acc_recurse(d, i, sw);
    }
}

void quadtree::debug_print_tree() {
    for (size_t i = 0; i < size_; i++) {
        const auto n = &nodes_[i];
        printf("%lu: cm='%.2f', cx='%.2f', cy='%.2f', ne='%du', nw='%du', se='%du', sw='%du'\n",
            i, n->cm, n->cx, n->cy, n->ne, n->nw, n->se, n->sw
        );
    }
}

void quadtree::grow(const size_t n) {
    if (!n || size_ >= n) return;

    this->size_ = n;
    free(nodes_);
    nodes_ = (Node*)aligned_alloc(MEM_ALIGNMENT, util::aligned_size(n*sizeof(Node)));
}

void quadtree::zero() {
    std::memset(this->nodes_, 0, size_*sizeof(Node));
}

void quadtree::compute_acc_recurse(data& d, size_t i, size_t j) {
    if (!j) return; // 0 means no child node, first call already ommits 0th j

    if (i == j) {
        // there still exists other nodes below this
        // that we may need to check, recurse into children
        compute_acc_recurse(d, i, nodes_[i].ne);
        compute_acc_recurse(d, i, nodes_[i].nw);
        compute_acc_recurse(d, i, nodes_[i].se);
        compute_acc_recurse(d, i, nodes_[i].sw);
        return;
    };

    const float theta = 0.1f;
    const size_t n = d.bodies();
    const float* __restrict ma = d.mass();
    const float* __restrict px = d.posx();
    const float* __restrict py = d.posy();
    float* __restrict ax_row = d.accx().row(0);
    float* __restrict ay_row = d.accy().row(0);

    const float size = nodes_[j].max_x - nodes_[j].min_x;

    const float dx = px[i] - nodes_[j].cx;
    const float dy = py[i] - nodes_[j].cy;
    const float dsq = util::epsl + (dx*dx) + (dy*dy);

    float inv = util::frsqrt(dsq);
    inv = inv * (1.5f - 0.5f * dsq * inv * inv);

    if (size*inv < theta) {
        const float inv3 = inv*inv*inv;
        const float p2m = nodes_[j].cm;

        const float ivx = dx * inv3;
        const float ivy = dy * inv3;

        ax_row[i] += ivx * p2m;
        ay_row[i] += ivy * p2m;

    } else {
        compute_acc_recurse(d, i, nodes_[j].nw);
        compute_acc_recurse(d, i, nodes_[j].ne);
        compute_acc_recurse(d, i, nodes_[j].sw);
        compute_acc_recurse(d, i, nodes_[j].se);
    }
}

void quadtree::insert_assertions(const data& d) {
    for (size_t i = 0; i < this->size_; i++) {
        printf("%lu: root.cm='%.2f', cm='%.2f', ne.cm='%.2f', nw.cm='%.2f', se.cm='%.2f', sw.cm='%.2f'\n",
            i, nodes_[i].cm, nodes_[nodes_[i].ne].cm +
            nodes_[nodes_[i].nw].cm +
            nodes_[nodes_[i].se].cm +
            nodes_[nodes_[i].sw].cm + d.mass()[i], nodes_[nodes_[i].ne].cm, nodes_[nodes_[i].nw].cm, nodes_[nodes_[i].se].cm, nodes_[nodes_[i].sw].cm
        );

        assert(nodes_[i].cm ==
            nodes_[nodes_[i].ne].cm +
            nodes_[nodes_[i].nw].cm +
            nodes_[nodes_[i].se].cm +
            nodes_[nodes_[i].sw].cm +
            d.mass()[i]
        );

        //assert(nodes_[i].cx ==
        //    (nodes_[nodes_[i].ne].cm*nodes_[nodes_[i].ne].cx) / nodes_[i].cm +
        //    (nodes_[nodes_[i].nw].cm*nodes_[nodes_[i].nw].cx) / nodes_[i].cm +
        //    (nodes_[nodes_[i].se].cm*nodes_[nodes_[i].se].cx) / nodes_[i].cm +
        //    (nodes_[nodes_[i].sw].cm*nodes_[nodes_[i].sw].cx) / nodes_[i].cm +
        //    (d.mass()[i] * d.posx()[i]) / nodes_[i].cm
        //);
    }
}
