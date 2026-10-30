#pragma once
#include <cstdint>
#include <cstddef>
#include <cstdlib>

#include "../data/data.hpp"

/*
 * Using this tree we can quickly compute attraction values
 */

struct quadtree {
    public:

        void insert(const data& d);
        void compute_acc(const float ft, matrix& ax, matrix& ay);

        void debug_print_tree();

    private:
        struct Node {
            float cm = 0.0f;
            float cx = 0.0f;
            float cy = 0.0f;

            uint32_t ne = 0;
            uint32_t nw = 0;
            uint32_t se = 0;
            uint32_t sw = 0;
        };

        Node* nodes_;
        size_t size_;

        void grow(const size_t n);
        void zero();
};
