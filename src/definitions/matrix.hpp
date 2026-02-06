#pragma once
#include <cstddef>
#include <malloc.h>
#include <cstring>

#include "../util/util.hpp"

struct matrix {
    public:

    // default constructor
    matrix() noexcept : 
        rows_(0),
        cols_(0), 
        data_(nullptr) {}

    // move constructor
    matrix(matrix&& other) noexcept : 
        rows_(other.rows_),
        cols_(other.cols_),
        data_(other.data_) {
            other.rows_ = 0;
            other.cols_ = 0;
            other.data_ = nullptr;
        }

    // copy constructor
    matrix(const matrix& other) :
        rows_(other.rows_),
        cols_(other.cols_) {
            data_ = (float*)aligned_alloc(32, util::aligned_size(rows_*cols_*sizeof(float)));
            std::memcpy(data_, other.data_, rows_*cols_*sizeof(float));
        }

    // custom constructor
    matrix(size_t r, size_t c) : 
    rows_(r), 
    cols_(c), 
    data_((float*)aligned_alloc(32, util::aligned_size(r*c*sizeof(float)))) {}

    // move operator
    matrix& operator = (matrix&& other) noexcept {
        rows_ = other.rows_;
        cols_ = other.cols_;
        data_ = other.data_;
        
        other.rows_ = 0;
        other.cols_ = 0;
        other.data_ = nullptr;
        return *this;
    }

    // copy operator
    matrix& operator = (const matrix& other) {
        rows_ = other.rows_;
        cols_ = other.cols_;
        data_ = (float*)aligned_alloc(32, util::aligned_size(rows_*cols_*sizeof(float)));
        std::memcpy(data_, other.data_, rows_*cols_*sizeof(float));
        return *this;
    }

    // deconstructor
    ~matrix() {
        rows_ = 0;
        cols_ = 0;
        if (data_) { free(data_); }
    }

    void zero() noexcept {
        #pragma omp parallel for
        for (size_t r = 0; r < rows_; r++) {
            std::memset(&data_[r*cols_], 0, cols_*sizeof(float));
        }
    }

    const float& operator () (size_t r, size_t c) const noexcept {
        return data_[r*cols_+c];
    }
    float& operator () (size_t r, size_t c) noexcept {
        return data_[r*cols_+c];
    }

    constexpr size_t rows() const noexcept { return rows_; }
    constexpr size_t cols() const noexcept { return cols_; }

    private:
    float* __restrict data_;
    size_t rows_;
    size_t cols_;
};
