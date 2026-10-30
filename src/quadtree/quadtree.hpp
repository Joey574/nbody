#pragma once
#include <cstdint>
#include <cstddef>
#include <cstdlib>

#include "../data/data.hpp"

/*
 * Using this tree we can quickly compute attraction values
 */

// TODO
// thetasq_ currently cannot be set from config, default constructor is called from somewhere?

struct quadtree {
    public:
        quadtree() {}

        quadtree(float theta) : thetasq_(theta*theta) {}

        void insert(const data& d);
        void compute_acc(data& d);

        void debug_print_tree();

        const inline float thetasq() const { return this->thetasq_; }

        ~quadtree() {
            free(cm_); cm_ = nullptr;
            free(cx_); cx_ = nullptr;
            free(cy_); cy_ = nullptr;
            free(min_x_); min_x_ = nullptr;
            free(max_x_); max_x_ = nullptr;
            free(min_y_); min_y_ = nullptr;
            free(max_y_); max_y_ = nullptr;
            free(ne_); ne_ = nullptr;
            free(nw_); nw_ = nullptr;
            free(se_); se_ = nullptr;
            free(sw_); sw_ = nullptr;
        }

    private:
        float* __restrict cm_ = nullptr;
        float* __restrict cx_ = nullptr;
        float* __restrict cy_ = nullptr;
        float* __restrict min_x_ = nullptr;
        float* __restrict max_x_ = nullptr;
        float* __restrict min_y_ = nullptr;
        float* __restrict max_y_ = nullptr;
        uint32_t* __restrict ne_ = nullptr;
        uint32_t* __restrict nw_ = nullptr;
        uint32_t* __restrict se_ = nullptr;
        uint32_t* __restrict sw_ = nullptr;

        size_t size_ = 0;
        float thetasq_ = 1.0f;

        void grow(const size_t n);
        void zero();
        void zero_children();

        void compute_acc_recurse(data& d, size_t i, size_t j, int tid);
};
