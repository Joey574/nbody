#pragma onces
#include "util/util.hpp"

#include <cstddef>
#include <malloc.h>
#include <cstring>
#include <immintrin.h>
#include <utility>

/// @brief lightweight abstraction for a matrix of floats, each row is aligned to MEM_ALIGNMENT for simd usage
struct matrix {
    public:
        matrix() = default;

        // move constructor
        matrix(matrix&& other) noexcept :
            rows_(std::exchange(other.rows_, 0)),
            cols_(std::exchange(other.cols_, 0)),
            stride_(std::exchange(other.stride_, 0)),
            data_(std::exchange(other.data_, nullptr))
            {}

        // copy constructor
        matrix(const matrix& other) :
            rows_(other.rows_),
            cols_(other.cols_),
            stride_(other.stride_) {
                const size_t bytes = rows_*stride_*sizeof(float);
                data_ = (float*)util::mem_aligned_alloc(bytes);
                memcpy(data_, other.data_, bytes);
            }

        // parameterized constructor
        matrix(size_t r, size_t c) :
            rows_(r),
            cols_(c),
            stride_(util::aligned_size(c*sizeof(float))/sizeof(float)),
            data_((float*)util::mem_aligned_alloc(r*stride_*sizeof(float)))
            {}

        // move operator
        matrix& operator = (matrix&& other) noexcept {
            if (this == &other) return *this;
            clear();

            rows_ = std::exchange(other.rows_, 0);
            cols_ = std::exchange(other.cols_, 0);
            stride_ = std::exchange(other.stride_, 0);
            data_ = std::exchange(other.data_, nullptr);
            return *this;
        }

        // copy operator
        matrix& operator = (const matrix& other) {
            if (this == &other) return *this;
            clear();

            rows_ = other.rows_;
            cols_ = other.cols_;
            stride_ = other.stride_;

            const size_t bytes = rows_*stride_*sizeof(float);

            data_ = (float*)util::mem_aligned_alloc(bytes);
            memcpy(data_, other.data_, bytes);
            return *this;
        }

        // deconstructor
        ~matrix() {
            clear();
        }

        inline void clear() noexcept {
            rows_ = 0;
            cols_ = 0;
            stride_ = 0;
            free(data_);

            data_ = nullptr;
        }

        void zero() noexcept {
            const size_t bytes = rows_*stride_*sizeof(float);
            std::memset(data_, 0, bytes);
        }

        const float& operator () (size_t r, size_t c) const noexcept {
            return data_[r*stride_+c];
        }
        float& operator () (size_t r, size_t c) noexcept {
            return data_[r*stride_+c];
        }

        const inline float* row(size_t r) const noexcept {
            return &data_[r*stride_];
        }
        inline float* row(size_t r) noexcept {
            return &data_[r*stride_];
        }

        const inline float* data() const { return data_; }
        inline float* data() { return data_; }

        constexpr size_t rows() const noexcept { return rows_; }
        constexpr size_t cols() const noexcept { return cols_; }
        constexpr size_t size() const noexcept { return rows_ * cols_; }

    private:
        // stride_ must appear before data_ due to its use in initializer list
        // and godforsaken cpp semantics (see parameterized constructor)
        size_t stride_ = 0;
        size_t rows_ = 0;
        size_t cols_ = 0;
        float* __restrict data_ = nullptr;
};
