#include "gtest/gtest.h"
#include <cstdint>

#include "definitions/macros.hpp"
#include "matrix/matrix.hpp"

TEST(MatrixConstructors, DefaultConstructor) {
    matrix m;
    EXPECT_EQ(m.Rows(), 0);
    EXPECT_EQ(m.Cols(), 0);
    EXPECT_EQ(m.Size(), 0);
}

TEST(MatrixConstructors, ParameterizedConstructor) {
    matrix m(10, 15);
    EXPECT_EQ(m.Rows(), 10);
    EXPECT_EQ(m.Cols(), 15);
    EXPECT_EQ(m.Size(), 150);
    EXPECT_TRUE((uintptr_t)m.data() % MEM_ALIGNMENT == 0);
}

TEST(MatrixConstructors, MoveConstructor) {
    matrix m(10, 10);
    float* ptr = m.data();
    EXPECT_EQ(m.Rows(), 10);
    EXPECT_EQ(m.Cols(), 10);
    EXPECT_EQ(m.Size(), 100);
    EXPECT_TRUE((uintptr_t)m.data() % MEM_ALIGNMENT == 0);

    matrix a(std::move(m));
    EXPECT_EQ(m.Rows(), 0);
    EXPECT_EQ(m.Cols(), 0);
    EXPECT_EQ(m.Size(), 0);
    EXPECT_EQ(m.data(), nullptr);

    EXPECT_EQ(a.Rows(), 10);
    EXPECT_EQ(a.Cols(), 10);
    EXPECT_EQ(a.Size(), 100);
    EXPECT_TRUE((uintptr_t)a.data() % MEM_ALIGNMENT == 0);
    EXPECT_EQ(ptr, a.data());
}

TEST(MatrixConstructors, CopyConstructor) {
    const matrix m (10, 10);
    EXPECT_EQ(m.Rows(), 10);
    EXPECT_EQ(m.Cols(), 10);
    EXPECT_EQ(m.Size(), 100);

    matrix a(m);
    EXPECT_EQ(a.Rows(), 10);
    EXPECT_EQ(a.Cols(), 10);
    EXPECT_EQ(a.Size(), 100);

    EXPECT_NE(m.Data(), a.data());
    EXPECT_TRUE((uintptr_t)m.Data() % MEM_ALIGNMENT == 0);
}


TEST(MatrixOperations, RowsAreAlignedAndIndependent) {
    matrix m(3, 9);

    for (size_t r = 0; r < m.Rows(); r++) {
        EXPECT_EQ(reinterpret_cast<uintptr_t>(m.Row(r)) % MEM_ALIGNMENT, 0);
        for (size_t c = 0; c < m.Cols(); c++) {
            m(r, c) = static_cast<float>(r * 10 + c);
        }
    }

    EXPECT_FLOAT_EQ(m(0, 8), 8.0f);
    EXPECT_FLOAT_EQ(m(1, 0), 10.0f);
    EXPECT_FLOAT_EQ(m(2, 8), 28.0f);
}

TEST(MatrixOperations, ZeroClearsAllLogicalElements) {
    matrix m(4, 11);
    for (size_t r = 0; r < m.Rows(); r++) {
        for (size_t c = 0; c < m.Cols(); c++) {
            m(r, c) = 5.0f;
        }
    }

    m.Zero();

    for (size_t r = 0; r < m.Rows(); r++) {
        for (size_t c = 0; c < m.Cols(); c++) {
            EXPECT_FLOAT_EQ(m(r, c), 0.0f);
        }
    }
}

TEST(MatrixOperations, CopyAssignmentCreatesIndependentStorage) {
    matrix source(2, 5);
    for (size_t r = 0; r < source.Rows(); r++) {
        for (size_t c = 0; c < source.Cols(); c++) {
            source(r, c) = static_cast<float>(r * 5 + c);
        }
    }

    matrix copy;
    copy = source;

    ASSERT_EQ(copy.Rows(), source.Rows());
    ASSERT_EQ(copy.Cols(), source.Cols());
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

    EXPECT_EQ(source.Rows(), 0);
    EXPECT_EQ(source.Cols(), 0);
    EXPECT_EQ(source.data(), nullptr);
    EXPECT_EQ(destination.Rows(), 2);
    EXPECT_EQ(destination.Cols(), 7);
    EXPECT_EQ(destination.data(), ptr);
}
