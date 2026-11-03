#include "gtest/gtest.h"

#include <cstdint>
#include <utility>

#include "data/data.hpp"

TEST(DataConstructors, DefaultConstructor) {
    data d;

    EXPECT_EQ(d.bodies(), 0);
    EXPECT_EQ(d.posx(), nullptr);
    EXPECT_EQ(d.posy(), nullptr);
    EXPECT_EQ(d.velx(), nullptr);
    EXPECT_EQ(d.vely(), nullptr);
    EXPECT_EQ(d.mass(), nullptr);
    EXPECT_EQ(d.accx().rows(), 0);
    EXPECT_EQ(d.accy().rows(), 0);
}

TEST(DataConstructors, ParameterizedConstructor) {
    data d(17, true);

    EXPECT_EQ(d.bodies(), 17);
    EXPECT_NE(d.posx(), nullptr);
    EXPECT_NE(d.posy(), nullptr);
    EXPECT_NE(d.velx(), nullptr);
    EXPECT_NE(d.vely(), nullptr);
    EXPECT_NE(d.mass(), nullptr);
    EXPECT_EQ(reinterpret_cast<uintptr_t>(d.posx()) % MEM_ALIGNMENT, 0);
    EXPECT_EQ(reinterpret_cast<uintptr_t>(d.posy()) % MEM_ALIGNMENT, 0);
    EXPECT_EQ(d.accx().cols(), 17);
    EXPECT_EQ(d.accy().cols(), 17);
}

TEST(DataConstructors, CopyConstructorCopiesParticleArrays) {
    data source(3, true);
    for (size_t i = 0; i < source.bodies(); i++) {
        source.posx()[i] = static_cast<float>(i) + 0.5f;
        source.posy()[i] = static_cast<float>(i) + 1.5f;
        source.velx()[i] = static_cast<float>(i) + 2.5f;
        source.vely()[i] = static_cast<float>(i) + 3.5f;
        source.mass()[i] = static_cast<float>(i) + 4.5f;
    }

    data copy(source);

    EXPECT_EQ(copy.bodies(), source.bodies());
    EXPECT_NE(copy.posx(), source.posx());
    EXPECT_NE(copy.mass(), source.mass());
    for (size_t i = 0; i < source.bodies(); i++) {
        EXPECT_FLOAT_EQ(copy.posx()[i], source.posx()[i]);
        EXPECT_FLOAT_EQ(copy.posy()[i], source.posy()[i]);
        EXPECT_FLOAT_EQ(copy.velx()[i], source.velx()[i]);
        EXPECT_FLOAT_EQ(copy.vely()[i], source.vely()[i]);
        EXPECT_FLOAT_EQ(copy.mass()[i], source.mass()[i]);
    }

    copy.posx()[0] = -100.0f;
    EXPECT_FLOAT_EQ(source.posx()[0], 0.5f);
}

TEST(DataConstructors, MoveConstructorTransfersParticleArrays) {
    data source(4, true);
    float* posx = source.posx();
    float* mass = source.mass();

    data moved(std::move(source));

    EXPECT_EQ(source.bodies(), 0);
    EXPECT_EQ(source.posx(), nullptr);
    EXPECT_EQ(source.mass(), nullptr);
    EXPECT_EQ(moved.bodies(), 4);
    EXPECT_EQ(moved.posx(), posx);
    EXPECT_EQ(moved.mass(), mass);
}

TEST(DataOperations, AppendPreservesExistingAndNewParticles) {
    data first(2, true);
    data second(1, false);

    first.posx()[0] = 1.0f;
    first.posx()[1] = 2.0f;
    first.mass()[0] = 3.0f;
    first.mass()[1] = 4.0f;

    second.posx()[0] = 5.0f;
    second.mass()[0] = 6.0f;

    first.append(second, true);

    ASSERT_EQ(first.bodies(), 3);
    EXPECT_FLOAT_EQ(first.posx()[0], 1.0f);
    EXPECT_FLOAT_EQ(first.posx()[1], 2.0f);
    EXPECT_FLOAT_EQ(first.posx()[2], 5.0f);
    EXPECT_FLOAT_EQ(first.mass()[0], 3.0f);
    EXPECT_FLOAT_EQ(first.mass()[1], 4.0f);
    EXPECT_FLOAT_EQ(first.mass()[2], 6.0f);
    EXPECT_EQ(first.accx().cols(), 3);
    EXPECT_EQ(first.accy().cols(), 3);
    EXPECT_EQ(second.bodies(), 1);
    EXPECT_FLOAT_EQ(second.posx()[0], 5.0f);
}

TEST(DataOperations, SortKeepsParticlePropertiesTogether) {
    data d(3, false);

    d.posx()[0] = 2.0f; d.posy()[0] = 0.0f;
    d.posx()[1] = 0.0f; d.posy()[1] = 1.0f;
    d.posx()[2] = 0.5f; d.posy()[2] = 0.0f;

    for (size_t i = 0; i < d.bodies(); i++) {
        d.velx()[i] = static_cast<float>(10 + i);
        d.vely()[i] = static_cast<float>(20 + i);
        d.mass()[i] = static_cast<float>(30 + i);
    }

    d.sort();

    EXPECT_FLOAT_EQ(d.posx()[0], 0.5f);
    EXPECT_FLOAT_EQ(d.mass()[0], 32.0f);
    EXPECT_FLOAT_EQ(d.velx()[0], 12.0f);
    EXPECT_FLOAT_EQ(d.posx()[1], 0.0f);
    EXPECT_FLOAT_EQ(d.posy()[1], 1.0f);
    EXPECT_FLOAT_EQ(d.mass()[1], 31.0f);
    EXPECT_FLOAT_EQ(d.vely()[1], 21.0f);
    EXPECT_FLOAT_EQ(d.posx()[2], 2.0f);
    EXPECT_FLOAT_EQ(d.mass()[2], 30.0f);
    EXPECT_FLOAT_EQ(d.velx()[2], 10.0f);
}

TEST(DataOperations, ZeroAccelerationClearsEveryThreadRow) {
    data d(5, true);

    for (size_t r = 0; r < d.accx().rows(); r++) {
        for (size_t c = 0; c < d.accx().cols(); c++) {
            d.accx()(r, c) = 1.0f;
            d.accy()(r, c) = -1.0f;
        }
    }

    d.zero_acc();

    for (size_t r = 0; r < d.accx().rows(); r++) {
        for (size_t c = 0; c < d.accx().cols(); c++) {
            EXPECT_FLOAT_EQ(d.accx()(r, c), 0.0f);
            EXPECT_FLOAT_EQ(d.accy()(r, c), 0.0f);
        }
    }
}

TEST(DataOperations, ClearReleasesStorageAndCanBeRepeated) {
    data d(4, true);
    d.clear();

    EXPECT_EQ(d.bodies(), 0);
    EXPECT_EQ(d.posx(), nullptr);
    EXPECT_EQ(d.posy(), nullptr);
    EXPECT_EQ(d.velx(), nullptr);
    EXPECT_EQ(d.vely(), nullptr);
    EXPECT_EQ(d.mass(), nullptr);
    EXPECT_EQ(d.accx().rows(), 0);
    EXPECT_EQ(d.accy().rows(), 0);

    EXPECT_NO_THROW(d.clear());
}
