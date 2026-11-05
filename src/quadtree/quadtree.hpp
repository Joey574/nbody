#pragma once

#include <cstdint>
#include <cstddef>
#include <cstdlib>
#include <cstring>
#include <utility>

#include "../data/data.hpp"

// TODO
// should try to implement this is parrot and see what the perf is

struct quadtree {
public:
    // default constructor
    quadtree() noexcept = default;

    // move constructor
    quadtree(quadtree&& other) noexcept :
        cm_(std::exchange(other.cm_, nullptr)),
        cx_(std::exchange(other.cx_, nullptr)),
        cy_(std::exchange(other.cy_, nullptr)),
        gx_(std::exchange(other.gx_, nullptr)),
        gy_(std::exchange(other.gy_, nullptr)),
        w_(std::exchange(other.w_, nullptr)),
        ne_(std::exchange(other.ne_, nullptr)),
        nw_(std::exchange(other.nw_, nullptr)),
        se_(std::exchange(other.se_, nullptr)),
        sw_(std::exchange(other.sw_, nullptr)),
        next_(std::exchange(other.next_, nullptr)),
        size_(std::exchange(other.size_, 0)),
        capacity_(std::exchange(other.capacity_, 0)),
        theta_(std::exchange(other.theta_, 0.0f)) {}

    // copy constructor
    quadtree(const quadtree& other) :
        theta_(other.theta_) {
        this->copy_from(other);
    }

    // move operator
    quadtree& operator = (quadtree&& other) noexcept {
        if (this == &other) {
            return *this;
        }

        this->release();

        cm_ = std::exchange(other.cm_, nullptr);
        cx_ = std::exchange(other.cx_, nullptr);
        cy_ = std::exchange(other.cy_, nullptr);
        gx_ = std::exchange(other.gx_, nullptr);
        gy_ = std::exchange(other.gy_, nullptr);
        w_ = std::exchange(other.w_, nullptr);

        ne_ = std::exchange(other.ne_, nullptr);
        nw_ = std::exchange(other.nw_, nullptr);
        se_ = std::exchange(other.se_, nullptr);
        sw_ = std::exchange(other.sw_, nullptr);
        next_ = std::exchange(other.next_, nullptr);

        size_ = std::exchange(other.size_, 0);
        capacity_ = std::exchange(other.capacity_, 0);
        theta_ = std::exchange(other.theta_, 0.0f);

        return *this;
    }

    // copy operator
    quadtree& operator = (const quadtree& other) {
        if (this == &other) {
            return *this;
        }

        this->copy_from(other);
        return *this;
    }

    // parameterized constructor
    explicit quadtree(float theta) :
        theta_(theta) {}

    void insert(const data& d);
    void compute_acc(data& d) const;

    void debug_print_tree() const;
    void debug_print_node(size_t i) const;

    float theta() const noexcept { return theta_; }

    ~quadtree() {
        this->release();
    }

private:
    float* __restrict gx_ = nullptr;
    float* __restrict gy_ = nullptr;
    float* __restrict cm_ = nullptr;
    float* __restrict cx_ = nullptr;
    float* __restrict cy_ = nullptr;
    float* __restrict w_ = nullptr;
    uint32_t* __restrict ne_ = nullptr;
    uint32_t* __restrict nw_ = nullptr;
    uint32_t* __restrict se_ = nullptr;
    uint32_t* __restrict sw_ = nullptr;
    uint32_t* __restrict next_ = nullptr;

    size_t size_ = 0;
    size_t capacity_ = 0;
    float theta_ = 1.0f;

    void grow(size_t n);
    void zero();
    void zero_children();

    void release() noexcept;
    void copy_from(const quadtree& other);

    void compute_acc_iterative(
        size_t leaf,
        const float* __restrict ma,
        const float* __restrict px,
        const float* __restrict py,
        float* __restrict ax,
        float* __restrict ay
    ) const;
};
