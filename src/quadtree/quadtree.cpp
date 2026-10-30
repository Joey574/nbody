#include "quadtree.hpp"
#include <cstring>

void quadtree::insert(const data& d) {
    const size_t n = d.bodies();
    if (!n) return;

    this->grow(n);
    this->zero();

    // alias
    const float* __restrict px = d.posx();
    const float* __restrict py = d.posy();
    const float* __restrict ma = d.mass();

    // add first node
    nodes_[0].cm = ma[0];
    nodes_[0].cx = px[0];
    nodes_[0].cy = py[0];

    const auto insert = [&](size_t i) {
        // new node
        nodes_[i].cm = ma[i];
        nodes_[i].cx = px[i];
        nodes_[i].cy = py[i];
    };

    const auto update_cm = [&](size_t i, size_t j) {
        // update parent cm, cx, cy
        nodes_[j].cm += ma[i];
        nodes_[j].cx += px[i]/nodes_[j].cm;
        nodes_[j].cy += py[i]/nodes_[j].cm;
    };

    for (size_t i = 1; i < n; i++) {
        for (size_t j = 0;;) {
            update_cm(i, j);

            if (px[i] > px[j] && py[i] > py[j]) { // NE
                if (!nodes_[j].ne) {
                    // node is empty, insert
                    nodes_[j].ne = i;
                    insert(i);
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
                    insert(i);
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
                    insert(i);
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
                    insert(i);
                    break;
                } else {
                    // node is populated, update k and recurse
                    j = nodes_[j].sw;
                    continue;
                }
            }
        }
    }
}

void quadtree::compute_acc(const float ft, matrix& ax, matrix& ay) {

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
    nodes_ = (Node*)aligned_alloc(MEM_ALIGNMENT, util::aligned_size(n*sizeof(Node)));
}

void quadtree::zero() {
    std::memset(this->nodes_, 0, size_*sizeof(Node));
}
