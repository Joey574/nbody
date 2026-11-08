#include "quadtree/quadtree.hpp"

#include <algorithm>
#include <cassert>
#include <cstdio>
#include <omp.h>

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

    nodes_.reserve(n*4);
    quads_.reserve(n*4);

    nodes_.clear();
    quads_.clear();
    parents_.clear();

    const float side = std::max(rangeX, rangeY);
    quads_.push(minX + rangeX * 0.5f, minY + rangeY * 0.5f, side);
    nodes_.push_empty(0);

    for (size_t body = 0; body < n; body++) {
        const float x = px[body];
        const float y = py[body];
        const float m = ma[body];

        insert_point(x, y, m);
    }

    propagate();
}

void quadtree::insert_point(float x, float y, float m) {
    size_t node = ROOT;

    while (nodes_.is_branch(node)) {
        size_t quadrant = quads_.find_quadrant(node, x, y);
        node = nodes_.C()[node] + quadrant;
    }

    if (nodes_.is_empty(node)) {
        nodes_.set(node, x, y, m);
        return;
    }

    float nx = nodes_.X()[node];
    float ny = nodes_.Y()[node];
    float nm = nodes_.M()[node];

    // handle edge case of same position
    if (nx == x && ny == y) {
        nodes_.M()[node] += m;
        return;
    }

    while(1) {
        size_t children = subdivide(node);

        size_t q1 = quads_.find_quadrant(node, nx, ny);
        size_t q2 = quads_.find_quadrant(node, x, y);

        if (q1 == q2) {
            node = children + q1;
        } else {
            size_t n1 = children + q1;
            size_t n2 = children + q2;

            nodes_.set(n1, nx, ny, nm);
            nodes_.set(n2, x, y, m);
            return;
        }
    }
}

size_t quadtree::subdivide(size_t node) {
    parents_.push_back(node);
    size_t children = nodes_.size();
    nodes_.C()[node] = children;

    size_t nexts[4] = {
        children + 1,
        children + 2,
        children + 3,
        nodes_.N()[node]
    };

    quads_.subdivide_and_push(node, nexts, nodes_);
    return children;
}

void quadtree::propagate() {
    float* __restrict nx = nodes_.X();
    float* __restrict ny = nodes_.Y();
    float* __restrict nm = nodes_.M();
    const size_t* __restrict nc = nodes_.C();

    for (size_t k = parents_.size(); k-- > 0;) {
        const size_t node = parents_[k];
        const size_t i = nc[node];

        const float m = nm[i] + nm[i+1] + nm[i+2] + nm[i+3];
        const float x = nx[i]*nm[i] + nx[i+1]*nm[i+1] + nx[i+2]*nm[i+2] + nx[i+3]*nm[i+3];
        const float y = ny[i]*nm[i] + ny[i+1]*nm[i+1] + ny[i+2]*nm[i+2] + ny[i+3]*nm[i+3];

        nm[node] = m;
        nx[node] = m > 0.0f ? x / m : 0.0f;
        ny[node] = m > 0.0f ? y / m : 0.0f;
    }
}
