#pragma once
#include <cstdint>
#include <cstddef>
#include <cstdlib>
#include <cstring>
#include <utility>

#include "../data/data.hpp"

/*
 * Using this tree we can quickly compute attraction values
 */

struct quadtree {
    public:
        // default constructor
        quadtree() noexcept {}

        // move constructor
        quadtree(quadtree&& other) noexcept :
            cm_(other.cm_),
            cx_(other.cx_),
            cy_(other.cy_),
            min_x_(other.min_x_),
            max_x_(other.max_x_),
            min_y_(other.min_y_),
            max_y_(other.max_y_),
            ne_(other.ne_),
            nw_(other.nw_),
            se_(other.se_),
            sw_(other.sw_),
            size_(other.size_),
            thetasq_(other.thetasq_) {
                other.cm_ = nullptr;
                other.cx_ = nullptr;
                other.cy_ = nullptr;
                other.min_x_ = nullptr;
                other.max_x_ = nullptr;
                other.min_y_ = nullptr;
                other.max_y_ = nullptr;
                other.ne_ = nullptr;
                other.nw_ = nullptr;
                other.se_ = nullptr;
                other.sw_ = nullptr;
                other.size_ = 0;
                other.thetasq_ = 0;
        }

        // copy constructor
        quadtree(const quadtree& other) noexcept : size_(0), thetasq_(other.thetasq_) {
            // size isn't set in initializer list as grow modifies it
            // and expects it to be smaller than the requested size
            this->grow(other.size_);

            std::memcpy(cm_, other.cm_, size_*sizeof(float));
            std::memcpy(cx_, other.cx_, size_*sizeof(float));
            std::memcpy(cy_, other.cy_, size_*sizeof(float));
            std::memcpy(min_x_, other.min_x_, size_*sizeof(float));
            std::memcpy(max_x_, other.max_x_, size_*sizeof(float));
            std::memcpy(min_y_, other.min_y_, size_*sizeof(float));
            std::memcpy(max_y_, other.max_y_, size_*sizeof(float));
            std::memcpy(ne_, other.ne_, size_*sizeof(uint32_t));
            std::memcpy(nw_, other.nw_, size_*sizeof(uint32_t));
            std::memcpy(se_, other.se_, size_*sizeof(uint32_t));
            std::memcpy(sw_, other.sw_, size_*sizeof(uint32_t));
        }

        // move operator
        quadtree& operator = (quadtree&& other) noexcept {
            if (this == &other) {
                return *this;
            }

            free(cm_);
            free(cx_);
            free(cy_);
            free(min_x_);
            free(max_x_);
            free(min_y_);
            free(max_y_);
            free(ne_);
            free(nw_);
            free(se_);
            free(sw_);

            cm_ = std::exchange(other.cm_, nullptr);
            cx_ = std::exchange(other.cx_, nullptr);
            cy_ = std::exchange(other.cy_, nullptr);
            min_x_ = std::exchange(other.min_x_, nullptr);
            max_x_ = std::exchange(other.max_x_, nullptr);
            min_y_ = std::exchange(other.min_y_, nullptr);
            max_y_ = std::exchange(other.max_y_, nullptr);
            ne_ = std::exchange(other.ne_, nullptr);
            nw_ = std::exchange(other.nw_, nullptr);
            se_ = std::exchange(other.se_, nullptr);
            sw_ = std::exchange(other.sw_, nullptr);

            size_ = std::exchange(other.size_, 0);
            thetasq_ = std::exchange(other.thetasq_, 0.0f);
            return *this;
        }

        // copy operator
        quadtree& operator = (const quadtree& other) noexcept {
            this->grow(other.size_);
            this->thetasq_ = other.thetasq_;

            std::memcpy(this->cm_, other.cm_, size_*sizeof(float));
            std::memcpy(this->cx_, other.cx_, size_*sizeof(float));
            std::memcpy(this->cy_, other.cy_, size_*sizeof(float));
            std::memcpy(this->min_x_, other.min_x_, size_*sizeof(float));
            std::memcpy(this->max_x_, other.max_x_, size_*sizeof(float));
            std::memcpy(this->min_y_, other.min_y_, size_*sizeof(float));
            std::memcpy(this->max_y_, other.max_y_, size_*sizeof(float));
            std::memcpy(this->ne_, other.ne_, size_*sizeof(uint32_t));
            std::memcpy(this->nw_, other.nw_, size_*sizeof(uint32_t));
            std::memcpy(this->se_, other.se_, size_*sizeof(uint32_t));
            std::memcpy(this->sw_, other.sw_, size_*sizeof(uint32_t));
            return *this;
        }


        // parameterized constructor
        quadtree(float theta) : thetasq_(theta*theta) {}

        void insert(const data& d);
        void compute_acc(data& d) const;

        void debug_print_tree() const;
        void debug_print_node(size_t i) const;

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
        float thetasq_ = 0.0f;

        void grow(const size_t n);
        void zero();
        void zero_children();

        void compute_acc_recurse(size_t i, size_t j, const float* __restrict ma, const float* __restrict px, const float* __restrict py, float* __restrict ax, float* __restrict ay) const;
};
