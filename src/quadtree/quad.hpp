#pragma once
#include "node.hpp"

#include <utility>
#include <cstddef>

struct quads {
    public:
        // default constructor
        quads() = default;

        // copies
        quads(const quads& other) noexcept = delete;
        quads& operator=(const quads& other) noexcept = delete;

        // move constructor
        quads(quads&& other) noexcept :
            size_(std::exchange(other.size_, 0)),
            capacity_(std::exchange(other.capacity_, 0)),
            quad_x_(std::exchange(other.quad_x_, nullptr)),
            quad_y_(std::exchange(other.quad_y_, nullptr)),
            quad_s_(std::exchange(other.quad_s_, nullptr))
        {}

        // move operator
        quads& operator=(quads&& other) noexcept {
            if (this == &other) return *this;
            cleanup();

            size_ = std::exchange(other.size_, 0);
            capacity_ = std::exchange(other.capacity_, 0);
            quad_x_ = std::exchange(other.quad_x_, nullptr);
            quad_y_ = std::exchange(other.quad_y_, nullptr);
            quad_s_ = std::exchange(other.quad_s_, nullptr);

            return *this;
        }

        // parameterized constructor
        quads(size_t n) { reserve(n); }

        void reserve(size_t n);

        void clear();
        void zero();

        void push(float x, float y, float s);

        void push_quadrant(size_t quad, size_t quadrant);
        void subdivide_and_push(size_t quad, size_t nexts[4], nodes& n);

        inline size_t find_quadrant(size_t i, float x, float y) const {
            return (size_t)((y > quad_y_[i])) << 1 | (size_t)(x > quad_x_[i]);
        }

        inline size_t size() const { return size_; }
        inline const float* __restrict X() const { return quad_x_; }
        inline const float* __restrict Y() const { return quad_y_; }
        inline const float* __restrict S() const { return quad_s_; }

        inline float* __restrict X() { return quad_x_; }
        inline float* __restrict Y() { return quad_y_; }
        inline float* __restrict S() { return quad_s_; }

        ~quads() { cleanup(); }
    private:
        size_t size_ = 0;
        size_t capacity_ = 0;
        float* __restrict quad_x_ = nullptr;
        float* __restrict quad_y_ = nullptr;
        float* __restrict quad_s_ = nullptr;

        void cleanup();
};
