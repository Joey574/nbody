#include "gtest/gtest.h"
#include <cstdint>
#include <gtest/gtest.h>

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
    EXPECT_TRUE((uintptr_t)m.data() % MEM_ALIGNMENT == 0);
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
