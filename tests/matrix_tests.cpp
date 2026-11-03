#include "gtest/gtest.h"
#include <cstdint>

#include "definitions/macros.hpp"
#include "matrix/matrix.hpp"

TEST(MatrixConstructors, DefaultConstructor) {
    matrix m;
    EXPECT_EQ(m.rows(), 0);
    EXPECT_EQ(m.cols(), 0);
    EXPECT_EQ(m.size(), 0);
}

TEST(MatrixConstructors, ParameterizedConstructor) {
    matrix m(10, 15);
    EXPECT_EQ(m.rows(), 10);
    EXPECT_EQ(m.cols(), 15);
    EXPECT_EQ(m.size(), 150);
    EXPECT_TRUE((uintptr_t)m.data() % MEM_ALIGNMENT == 0);
}

TEST(MatrixConstructors, MoveConstructor) {
    matrix m(10, 10);
    float* ptr = m.data();
    EXPECT_EQ(m.rows(), 10);
    EXPECT_EQ(m.cols(), 10);
    EXPECT_EQ(m.size(), 100);
    EXPECT_TRUE((uintptr_t)m.data() % MEM_ALIGNMENT == 0);

    matrix a(std::move(m));
    EXPECT_EQ(m.rows(), 0);
    EXPECT_EQ(m.cols(), 0);
    EXPECT_EQ(m.size(), 0);
    EXPECT_EQ(m.data(), nullptr);

    EXPECT_EQ(a.rows(), 10);
    EXPECT_EQ(a.cols(), 10);
    EXPECT_EQ(a.size(), 100);
    EXPECT_TRUE((uintptr_t)a.data() % MEM_ALIGNMENT == 0);
    EXPECT_EQ(ptr, a.data());
}

TEST(MatrixConstructors, CopyConstructor) {
    const matrix m (10, 10);
    EXPECT_EQ(m.rows(), 10);
    EXPECT_EQ(m.cols(), 10);
    EXPECT_EQ(m.size(), 100);

    matrix a(m);
    EXPECT_EQ(a.rows(), 10);
    EXPECT_EQ(a.cols(), 10);
    EXPECT_EQ(a.size(), 100);

    EXPECT_NE(m.data(), a.data());
    EXPECT_TRUE((uintptr_t)m.data() % MEM_ALIGNMENT == 0);
}


TEST(MatrixOperations, RowsAreAlignedAndIndependent) {
    matrix m(3, 9);

    for (size_t r = 0; r < m.rows(); r++) {
        EXPECT_EQ(reinterpret_cast<uintptr_t>(m.row(r)) % MEM_ALIGNMENT, 0);
        for (size_t c = 0; c < m.cols(); c++) {
            m(r, c) = static_cast<float>(r * 10 + c);
        }
    }

    EXPECT_FLOAT_EQ(m(0, 8), 8.0f);
    EXPECT_FLOAT_EQ(m(1, 0), 10.0f);
    EXPECT_FLOAT_EQ(m(2, 8), 28.0f);
}

TEST(MatrixOperations, ZeroClearsAllLogicalElements) {
    matrix m(4, 11);
    for (size_t r = 0; r < m.rows(); r++) {
        for (size_t c = 0; c < m.cols(); c++) {
            m(r, c) = 5.0f;
        }
    }

    m.zero();

    for (size_t r = 0; r < m.rows(); r++) {
        for (size_t c = 0; c < m.cols(); c++) {
            EXPECT_FLOAT_EQ(m(r, c), 0.0f);
        }
    }
}

TEST(MatrixOperations, CopyAssignmentCreatesIndependentStorage) {
    matrix source(2, 5);
    for (size_t r = 0; r < source.rows(); r++) {
        for (size_t c = 0; c < source.cols(); c++) {
            source(r, c) = static_cast<float>(r * 5 + c);
        }
    }

    matrix copy;
    copy = source;

    ASSERT_EQ(copy.rows(), source.rows());
    ASSERT_EQ(copy.cols(), source.cols());
    EXPECT_NE(copy.data(), source.data());
    EXPECT_FLOAT_EQ(copy(1, 4), source(1, 4));

    copy(1, 4) = -1.0f;
    EXPECT_FLOAT_EQ(source(1, 4), 9.0f);
}

TEST(MatrixOperations, MoveAssignmentTransfersStorage) {
    matrix source(2, 7);
    float* ptr = source.data();

    matrix destination(1, 1);
    destination = std::move(source);

    EXPECT_EQ(source.rows(), 0);
    EXPECT_EQ(source.cols(), 0);
    EXPECT_EQ(source.data(), nullptr);
    EXPECT_EQ(destination.rows(), 2);
    EXPECT_EQ(destination.cols(), 7);
    EXPECT_EQ(destination.data(), ptr);
}
