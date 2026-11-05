#include "quadtree/quadtree.hpp"

#include <algorithm>
#include <cassert>
#include <cmath>
#include <cstdio>
#include <limits>
#include <omp.h>

namespace {
    // Maximum number of subdivision levels below the virtual root.
    //
    // Bodies that would exceed this depth are stored in a linked
    // overflow list belonging to the terminal cell.
    constexpr size_t MAX_TREE_DEPTH = 128;
    constexpr size_t MAX_STACK_SIZE = 4 * MAX_TREE_DEPTH + 8;
}

void insert(const data& d) {

}

void quadtree::insert(const data& d) {
    const size_t n = d.bodies();
    if (!n) return;

    // alias
    const float* __restrict px = d.posx();
    const float* __restrict py = d.posy();
    const float* __restrict ma = d.mass();

    auto [minXptr, maxXptr] = std::minmax_element(&px[0], &px[n]);
    auto [minYptr, maxYptr] = std::minmax_element(&py[0], &py[n]);
    float minX = *minXptr;
    float maxX = *maxXptr;
    float minY = *minYptr;
    float maxY = *maxYptr;

    const float rangeX = (maxX) - (minX);
    const float rangeY = (maxY) - (minY);

    this->grow(n);

    size_ = n;
    this->zero();

    gx_[0] = minX + rangeX * 0.5f;
    gy_[0] = minY + rangeY * 0.5f;
    float side = std::max(rangeX, rangeY);

    w_[0] = side;

    for (size_t body = 0; body < n; body++) {
        const float x = px[body];
        const float y = py[body];
        const float mass = ma[body];

        const uint32_t bodyNode = body+1;

        uint32_t node = 0;
        size_t depth = 0;

        for (;;) {
            const float oldMass = cm_[node];
            const float newMass = oldMass + mass;

            if (newMass != 0.0f) [[likely]] {
                if (oldMass == 0.0f) [[unlikely]] {
                    cx_[node] = x;
                    cy_[node] = y;
                } else {
                    const float fraction = mass / newMass;

                    cx_[node] += (x - cx_[node]) * fraction;
                    cy_[node] += (y - cy_[node]) * fraction;
                }
            }

            cm_[node] = newMass;

            if (depth >= MAX_TREE_DEPTH) {
                next_[bodyNode] = next_[node];
                next_[node] = bodyNode;

                gx_[bodyNode] = gx_[node];
                gy_[bodyNode] = gy_[node];
                w_[bodyNode] = w_[node];

                cm_[bodyNode] = mass;
                cx_[bodyNode] = x;
                cy_[bodyNode] = y;

                break;
            }

            const bool isRight = x > gx_[node];
            const bool isUp = y > gy_[node];

            uint32_t* quad;

            if (isRight) {
                quad = isUp ? &ne_[node] : &se_[node];
            } else {
                quad = isUp ? &nw_[node] : &sw_[node];
            }

            const uint32_t child = *quad;

            if (!child) {
                // -------------------------------------------------------------
                // Empty quadrant: create a new physical body node.
                // -------------------------------------------------------------

                *quad = bodyNode;

                const float xOffset = w_[node] * 0.25f;
                const float yOffset = w_[node] * 0.25f;

                gx_[bodyNode] = gx_[node] + (isRight ? xOffset : -xOffset);
                gy_[bodyNode] = gy_[node] + (isUp ? yOffset : -yOffset);

                w_[bodyNode] = w_[node] * 0.5f;

                // A newly created node initially contains only its own body.
                cm_[bodyNode] = mass;
                cx_[bodyNode] = x;
                cy_[bodyNode] = y;

                break;
            }

            // -------------------------------------------------------------
            // Occupied quadrant: descend into the existing node.
            // -------------------------------------------------------------

            node = child;
            depth++;
        }
    }
}

void quadtree::compute_acc(data& d) const {
    assert(d.bodies() == size_);

    d.accx().zero();
    d.accy().zero();

    if (!size_) {
        return;
    }

    #pragma omp parallel
    {
        const int tid = omp_get_thread_num();

        const float* __restrict ma = d.mass();
        const float* __restrict px = d.posx();
        const float* __restrict py = d.posy();

        float* __restrict ax = d.accx().row(tid);
        float* __restrict ay = d.accy().row(tid);

        #pragma omp for schedule(guided)
        for (size_t i = 0; i < size_; i++) {
            compute_acc_iterative(i, ma, px, py, ax, ay);
        }
    }
}

void quadtree::compute_acc_iterative(
    size_t leaf,
    const float* __restrict ma,
    const float* __restrict px,
    const float* __restrict py,
    float* __restrict ax,
    float* __restrict ay
) const {
    uint32_t stack[MAX_STACK_SIZE];
    size_t top = 0;

    stack[top++] = 0;

    const float p1x = px[leaf];
    const float p1y = py[leaf];

    const uint32_t targetNode = leaf+1;

    while (top > 0) {
        const uint32_t root = stack[--top];
        const float dx = cx_[root] - p1x;
        const float dy = cy_[root] - p1y;

        const float dsq = dx * dx + dy * dy;

        bool containsSelf = (root == 0);

        if (root != 0) {
            const float halfW = w_[root] * 0.5f;

            const float epsX =
                4.0f * std::numeric_limits<float>::epsilon() *
                std::max({
                    1.0f,
                    std::fabs(p1x),
                    std::fabs(gx_[root])
                });

            const float epsY =
                4.0f * std::numeric_limits<float>::epsilon() *
                std::max({
                    1.0f,
                    std::fabs(p1y),
                    std::fabs(gy_[root])
                });

            containsSelf =
                p1x >= gx_[root] - halfW - epsX &&
                p1x <= gx_[root] + halfW + epsX &&
                p1y >= gy_[root] - halfW - epsY &&
                p1y <= gy_[root] + halfW + epsY;
        }

        const float cellSize = w_[root];
        const float cellSizeSq = cellSize * cellSize;

        const bool approximate =
            root != 0 &&
            !containsSelf &&
            dsq > 0.0f &&
            cellSizeSq < theta_ * theta_ * dsq;

        if (approximate) {
            if (cm_[root] != 0.0f) {
                util::apply_inner_force(
                    dx,
                    dy,
                    dsq,
                    cm_[root],
                    &ax[leaf],
                    &ay[leaf]
                );
            }

            continue;
        }

        if (root != 0) {
            const size_t body = static_cast<size_t>(root - 1);

            if (root != targetNode) {
                util::apply_force_to_one(
                    p1x,
                    p1y,
                    px[body],
                    py[body],
                    ma[body],
                    &ax[leaf],
                    &ay[leaf]
                );
            }

            for (uint32_t other = next_[root];
                 other != 0;
                 other = next_[other]) {

                const size_t otherBody = static_cast<size_t>(other - 1);

                if (other != targetNode) {
                    util::apply_force_to_one(
                        p1x,
                        p1y,
                        px[otherBody],
                        py[otherBody],
                        ma[otherBody],
                        &ax[leaf],
                        &ay[leaf]
                    );
                }
            }
        }

        assert(top + 4 <= MAX_STACK_SIZE);
        if (sw_[root]) stack[top++] = sw_[root];
        if (se_[root]) stack[top++] = se_[root];
        if (nw_[root]) stack[top++] = nw_[root];
        if (ne_[root]) stack[top++] = ne_[root];
    }
}
