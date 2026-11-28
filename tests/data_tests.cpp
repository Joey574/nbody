#include "gtest/gtest.h"

#include <cstdint>
#include <utility>

#include "data/data.hpp"

TEST(DataConstructors, DefaultConstructor) {
    data d;

    EXPECT_EQ(d.Bodies(), 0);
    EXPECT_EQ(d.PosX(), nullptr);
    EXPECT_EQ(d.PosY(), nullptr);
    EXPECT_EQ(d.VelX(), nullptr);
    EXPECT_EQ(d.VelY(), nullptr);
    EXPECT_EQ(d.Mass(), nullptr);
    EXPECT_EQ(d.AccX().Rows(), 0);
    EXPECT_EQ(d.AccY().Rows(), 0);
}

TEST(DataConstructors, ParameterizedConstructor) {
    data d(17, true);

    EXPECT_EQ(d.Bodies(), 17);
    EXPECT_NE(d.PosX(), nullptr);
    EXPECT_NE(d.PosY(), nullptr);
    EXPECT_NE(d.VelX(), nullptr);
    EXPECT_NE(d.VelY(), nullptr);
    EXPECT_NE(d.Mass(), nullptr);
    EXPECT_EQ(reinterpret_cast<uintptr_t>(d.PosX()) % MEM_ALIGNMENT, 0);
    EXPECT_EQ(reinterpret_cast<uintptr_t>(d.PosY()) % MEM_ALIGNMENT, 0);
    EXPECT_EQ(d.AccX().Cols(), 17);
    EXPECT_EQ(d.AccY().Cols(), 17);
}

TEST(DataConstructors, CopyConstructorCopiesParticleArrays) {
    data source(3, true);
    for (size_t i = 0; i < source.Bodies(); i++) {
        source.PosX()[i] = static_cast<float>(i) + 0.5f;
        source.PosY()[i] = static_cast<float>(i) + 1.5f;
        source.VelX()[i] = static_cast<float>(i) + 2.5f;
        source.VelY()[i] = static_cast<float>(i) + 3.5f;
        source.Mass()[i] = static_cast<float>(i) + 4.5f;
    }

    data copy(source);

    EXPECT_EQ(copy.Bodies(), source.Bodies());
    EXPECT_NE(copy.PosX(), source.PosX());
    EXPECT_NE(copy.Mass(), source.Mass());
    for (size_t i = 0; i < source.Bodies(); i++) {
        EXPECT_FLOAT_EQ(copy.PosX()[i], source.PosX()[i]);
        EXPECT_FLOAT_EQ(copy.PosY()[i], source.PosY()[i]);
        EXPECT_FLOAT_EQ(copy.VelX()[i], source.VelX()[i]);
        EXPECT_FLOAT_EQ(copy.VelY()[i], source.VelY()[i]);
        EXPECT_FLOAT_EQ(copy.Mass()[i], source.Mass()[i]);
    }

    copy.PosX()[0] = -100.0f;
    EXPECT_FLOAT_EQ(source.PosX()[0], 0.5f);
}

TEST(DataConstructors, MoveConstructorTransfersParticleArrays) {
    data source(4, true);
    float* posx = source.PosX();
    float* mass = source.Mass();

    data moved(std::move(source));

    EXPECT_EQ(source.Bodies(), 0);
    EXPECT_EQ(source.PosX(), nullptr);
    EXPECT_EQ(source.Mass(), nullptr);
    EXPECT_EQ(moved.Bodies(), 4);
    EXPECT_EQ(moved.PosX(), posx);
    EXPECT_EQ(moved.Mass(), mass);
}

TEST(DataOperations, AppendPreservesExistingAndNewParticles) {
    data first(2, true);
    data second(1, false);

    first.PosX()[0] = 1.0f;
    first.PosX()[1] = 2.0f;
    first.Mass()[0] = 3.0f;
    first.Mass()[1] = 4.0f;

    second.PosX()[0] = 5.0f;
    second.Mass()[0] = 6.0f;

    first.Append(second, true);

    ASSERT_EQ(first.Bodies(), 3);
    EXPECT_FLOAT_EQ(first.PosX()[0], 1.0f);
    EXPECT_FLOAT_EQ(first.PosX()[1], 2.0f);
    EXPECT_FLOAT_EQ(first.PosX()[2], 5.0f);
    EXPECT_FLOAT_EQ(first.Mass()[0], 3.0f);
    EXPECT_FLOAT_EQ(first.Mass()[1], 4.0f);
    EXPECT_FLOAT_EQ(first.Mass()[2], 6.0f);
    EXPECT_EQ(first.AccX().Cols(), 3);
    EXPECT_EQ(first.AccY().Cols(), 3);
    EXPECT_EQ(second.Bodies(), 1);
    EXPECT_FLOAT_EQ(second.PosX()[0], 5.0f);
}

TEST(DataOperations, SortKeepsParticlePropertiesTogether) {
    data d(3, false);

    d.PosX()[0] = 2.0f; d.PosY()[0] = 0.0f;
    d.PosX()[1] = 0.0f; d.PosY()[1] = 1.0f;
    d.PosX()[2] = 0.5f; d.PosY()[2] = 0.0f;

    for (size_t i = 0; i < d.Bodies(); i++) {
        d.VelX()[i] = static_cast<float>(10 + i);
        d.VelY()[i] = static_cast<float>(20 + i);
        d.Mass()[i] = static_cast<float>(30 + i);
    }

    d.Sort();

    EXPECT_FLOAT_EQ(d.PosX()[0], 0.5f);
    EXPECT_FLOAT_EQ(d.Mass()[0], 32.0f);
    EXPECT_FLOAT_EQ(d.VelX()[0], 12.0f);
    EXPECT_FLOAT_EQ(d.PosX()[1], 0.0f);
    EXPECT_FLOAT_EQ(d.PosY()[1], 1.0f);
    EXPECT_FLOAT_EQ(d.Mass()[1], 31.0f);
    EXPECT_FLOAT_EQ(d.VelY()[1], 21.0f);
    EXPECT_FLOAT_EQ(d.PosX()[2], 2.0f);
    EXPECT_FLOAT_EQ(d.Mass()[2], 30.0f);
    EXPECT_FLOAT_EQ(d.VelX()[2], 10.0f);
}

TEST(DataOperations, ZeroAccelerationClearsEveryThreadRow) {
    data d(5, true);

    for (size_t r = 0; r < d.AccX().Rows(); r++) {
        for (size_t c = 0; c < d.AccX().Cols(); c++) {
            d.AccX()(r, c) = 1.0f;
            d.AccY()(r, c) = -1.0f;
        }
    }

    d.ZeroAcc();

    for (size_t r = 0; r < d.AccX().Rows(); r++) {
        for (size_t c = 0; c < d.AccX().Cols(); c++) {
            EXPECT_FLOAT_EQ(d.AccX()(r, c), 0.0f);
            EXPECT_FLOAT_EQ(d.AccY()(r, c), 0.0f);
        }
    }
}

TEST(DataOperations, ClearReleasesStorageAndCanBeRepeated) {
    data d(4, true);
    d.Clear();

    EXPECT_EQ(d.Bodies(), 0);
    EXPECT_EQ(d.PosX(), nullptr);
    EXPECT_EQ(d.PosY(), nullptr);
    EXPECT_EQ(d.VelX(), nullptr);
    EXPECT_EQ(d.VelY(), nullptr);
    EXPECT_EQ(d.Mass(), nullptr);
    EXPECT_EQ(d.AccX().Rows(), 0);
    EXPECT_EQ(d.AccY().Rows(), 0);

    EXPECT_NO_THROW(d.Clear());
}
