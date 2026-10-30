#include "quadtree.hpp"
#include <algorithm>
#include <cstring>
#include <assert.h>
#include <yaml-cpp/node/detail/node.h>

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
        const float old_mass = nodes_[j].cm;
        nodes_[j].cm += ma[i];

        nodes_[j].cx = ((nodes_[j].cx * old_mass) + (ma[i] * px[i])) / (nodes_[j].cm);
        nodes_[j].cy = ((nodes_[j].cy * old_mass) + (ma[i] * py[i])) / (nodes_[j].cm);

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
                    // node is populated, update j and recurse
                    j = nodes_[j].se;
                    continue;
                }

            } else { // SW
                if (!nodes_[j].sw) {
                    // node is empty, insert
                    nodes_[j].sw = i;
                    break;
                } else {
                    // node is populated, update j and recurse
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

// TODO
// this needs to be turned into a linear loop
// this needs to make use of multithreading, should take a tid input
void quadtree::compute_acc_recurse(data& d, size_t i, size_t j, int tid) {
    if (i == j) {
        // there still exists other nodes below this
        // that we may need to check, recurse into children
        if (nodes_[i].ne) compute_acc_recurse(d, i, nodes_[i].ne, tid);
        if (nodes_[i].nw) compute_acc_recurse(d, i, nodes_[i].nw, tid);
        if (nodes_[i].se) compute_acc_recurse(d, i, nodes_[i].se, tid);
        if (nodes_[i].sw) compute_acc_recurse(d, i, nodes_[i].sw, tid);
        return;
    };

    const float theta = 1.0f;
    const float thetasq = theta*theta;
    const size_t n = d.bodies();
    const float* __restrict ma = d.mass();
    const float* __restrict px = d.posx();
    const float* __restrict py = d.posy();
    float* __restrict ax_row = d.accx().row(tid);
    float* __restrict ay_row = d.accy().row(tid);

    const float size = nodes_[j].max_x - nodes_[j].min_x;
    const float sizesq = size*size;

    const float p1x = px[i];
    const float p1y = py[i];
    const float p1m = ma[i];

    const float p2x = nodes_[j].cx;
    const float p2y = nodes_[j].cy;
    const float p2m = nodes_[j].cm;

    const float dx = p2x - p1x;
    const float dy = p2y - p2y;
    const float dsq = util::epsl + (dx*dx) + (dy*dy);

    if (sizesq / dsq < thetasq) {
        float inv = util::frsqrt(dsq);
        inv = inv * (1.5f - 0.5f * dsq * inv * inv);

        const float inv3 = inv*inv*inv;
        const float p2m = nodes_[j].cm;

        const float ivx = dx * inv3;
        const float ivy = dy * inv3;

        ax_row[i] += ivx * p2m;
        ay_row[i] += ivy * p2m;

    } else {
        if (nodes_[j].nw) compute_acc_recurse(d, i, nodes_[j].nw, tid);
        if (nodes_[j].ne) compute_acc_recurse(d, i, nodes_[j].ne, tid);
        if (nodes_[j].sw) compute_acc_recurse(d, i, nodes_[j].sw, tid);
        if (nodes_[j].se) compute_acc_recurse(d, i, nodes_[j].se, tid);
    }
}

void quadtree::insert_assertions(const data& d) {
    debug_print_tree();

    for (size_t i = 0; i < this->size_; i++) {
        const Node* n = &nodes_[i];
        const Node* ne = n->ne != 0 ? &nodes_[n->ne] : nullptr;
        const Node* nw = n->nw != 0 ? &nodes_[n->nw] : nullptr;
        const Node* se = n->se != 0 ? &nodes_[n->se] : nullptr;
        const Node* sw = n->sw != 0 ? &nodes_[n->sw] : nullptr;

        printf("%lu\n", i);
        printf("\tmass: root.cm='%.2f', cm='%.2f', ne.cm='%.2f', nw.cm='%.2f', se.cm='%.2f', sw.cm='%.2f'\n",
            n->cm,
            (ne ? ne->cm : 0) +
            (nw ? nw->cm : 0) +
            (se ? se->cm : 0) +
            (sw ? sw->cm : 0) +
            d.mass()[i],
            (ne ? ne->cm : 0),
            (nw ? nw->cm : 0),
            (se ? se->cm : 0),
            (sw ? sw->cm : 0)
        );

        assert(util::is_near(n->cm,
            (ne ? ne->cm : 0) +
            (nw ? nw->cm : 0) +
            (se ? se->cm : 0) +
            (sw ? sw->cm : 0) +
            d.mass()[i])
        );

        printf("\tcx: root.cx='%.2f', cx='%.2f', ne.cx='%.2f', nw.cx='%.2f', se.cx='%.2f', sw.cx='%.2f'\n",
            n->cx, (
            (ne ? ne->cm * ne->cx : 0.0f) +
            (nw ? nw->cm * nw->cx : 0.0f) +
            (se ? se->cm * se->cx : 0.0f) +
            (sw ? sw->cm * sw->cx : 0.0f) +
            (d.mass()[i] * d.posx()[i]))
            / n->cm,
            ne->cx,
            nw->cx,
            se->cx,
            sw->cx
        );

        assert(util::is_near(n->cx, (
           (ne ? ne->cm * ne->cx : 0.0f) +
           (nw ? nw->cm * nw->cx : 0.0f) +
           (se ? se->cm * se->cx : 0.0f) +
           (sw ? sw->cm * sw->cx : 0.0f) +
           (d.mass()[i] * d.posx()[i]))
            / n->cm)
        );

        printf("\tcy: root.cy='%.2f', cy='%.2f', ne.cy='%.2f', nw.cy='%.2f', se.cy='%.2f', sw.cy='%.2f'\n",
            n->cy, (
            (ne ? ne->cm * ne->cy : 0.0f) +
            (nw ? nw->cm * nw->cy : 0.0f) +
            (se ? se->cm * se->cy : 0.0f) +
            (sw ? sw->cm * sw->cy : 0.0f) +
            (d.mass()[i] * d.posx()[i]))
            / n->cm,
            ne->cy,
            nw->cy,
            se->cy,
            sw->cy
        );

        assert(util::is_near(n->cy, (
           (ne ? ne->cm * ne->cy : 0.0f) +
           (nw ? nw->cm * nw->cy : 0.0f) +
           (se ? se->cm * se->cy : 0.0f) +
           (sw ? sw->cm * sw->cy : 0.0f) +
           (d.mass()[i] * d.posy()[i]))
            / n->cm)
        );
    }

    exit(0);
}
