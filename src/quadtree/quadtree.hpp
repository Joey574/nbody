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
        void compute_acc(data& d);

        void debug_print_tree();

    private:
        // TODO
        // this can be made an SoA instead of an AoS
        // this would match the memory order of data
        // allowing easy copy first first element ma
        // py and py. And is generally more performant
        struct Node {
            float cm = 0.0f;
            float cx = 0.0f;
            float cy = 0.0f;

            float min_x;
            float max_x;

            uint32_t ne = 0;
            uint32_t nw = 0;
            uint32_t se = 0;
            uint32_t sw = 0;
        };

        Node* nodes_;
        size_t size_;

        void grow(const size_t n);
        void zero();

        void compute_acc_recurse(data& d, size_t i, size_t j, int tid);

        void insert_assertions(const data& d);
};
