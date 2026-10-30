#pragma once
#include <concepts>
#include <cstdint>
#include <cstddef>
#include <cstdlib>

/*
 * The main loop should go something like this
 *
 * Insert all bodies into a quadtree, each leaf has at most 1 body
 * CoM should likely be computed on the way down?
 *
 * Using this tree we can quickly compute attraction values
 */

struct quadtree {
    public:

    private:
        struct Node {
            uint32_t nw;
            uint32_t ne;
            uint32_t sw;
            uint32_t se;
        };

        Node* data;
        size_t size;
        size_t capacity;
};
