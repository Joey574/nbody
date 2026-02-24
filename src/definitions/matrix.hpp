#pragma once
#include <cstddef>
#include <malloc.h>
#include <cstring>

#include "macros.hpp"
#include "../util/util.hpp"

/// @brief lightweight abstraction for a matrix of floats, each row is aligned to MEM_ALIGNMENT for simd usage
struct matrix {
    public:

    // default constructor
    matrix() noexcept :
        rows_(0),
        cols_(0),
        stride_(0),
        data_(nullptr) {}

    // move constructor
    matrix(matrix&& other) noexcept : 
        rows_(other.rows_),
        cols_(other.cols_),
        stride_(other.stride_),
        data_(other.data_) {
            other.rows_ = 0;
            other.cols_ = 0;
            other.stride_ = 0;
            other.data_ = nullptr;
        }

    // copy constructor
    matrix(const matrix& other) :
        rows_(other.rows_),
        cols_(other.cols_),
        stride_(other.stride_) {
            auto bytes = rows_*stride_*sizeof(float);
            data_ = (float*)aligned_alloc(MEM_ALIGNMENT, bytes);
            std::memcpy(data_, other.data_, bytes);
        }

    // custom constructor
    matrix(size_t r, size_t c) : 
    rows_(r),
    cols_(c),
    stride_(util::aligned_size(c*sizeof(float))/sizeof(float)),
    data_((float*)aligned_alloc(MEM_ALIGNMENT, r*util::aligned_size(c*sizeof(float)))) {}

    // move operator
    matrix& operator = (matrix&& other) noexcept {
        if (this == &other) return *this;
        
        rows_ = other.rows_;
        cols_ = other.cols_;
        stride_ = other.stride_;

        if (data_) { free(data_); }
        data_ = other.data_;
        
        other.rows_ = 0;
        other.cols_ = 0;
        other.stride_ = 0;
        other.data_ = nullptr;
        return *this;
    }

    // copy operator
    matrix& operator = (const matrix& other) {
        if (this == &other) return *this;

        if (data_) { free(data_); }
        rows_ = other.rows_;
        cols_ = other.cols_;
        stride_ = other.stride_;
        auto bytes = rows_*stride_*sizeof(float);

        data_ = (float*)aligned_alloc(MEM_ALIGNMENT, bytes);
        std::memcpy(data_, other.data_, bytes);
        return *this;
    }

    // deconstructor
    ~matrix() {
        rows_ = 0;
        cols_ = 0;
        stride_ = 0;
        if (data_) { free(data_); data_ = nullptr; }
    }

    void zero() noexcept {
        auto bytes = rows_*stride_*sizeof(float);
        std::memset(data_, 0, bytes);
    }

    const float& operator () (size_t r, size_t c) const noexcept {
        return data_[r*stride_+c];
    }
    float& operator () (size_t r, size_t c) noexcept {
        return data_[r*stride_+c];
    }
    inline float* row(size_t r) noexcept {
        return &data_[r*stride_];
    }

    constexpr size_t rows() const noexcept { return rows_; }
    constexpr size_t cols() const noexcept { return cols_; }

    private:
    float* __restrict data_;
    size_t stride_;
    size_t rows_;
    size_t cols_;
};
