#include "gtest/gtest.h"

#include <cmath>
#include <utility>
#include <vector>

#include "quadtree/quadtree.hpp"

namespace {
    std::vector<float> SumAccelerationRows(const matrix& acceleration) {
        std::vector<float> result(acceleration.Cols(), 0.0f);
        for (size_t r = 0; r < acceleration.Rows(); r++) {
            for (size_t c = 0; c < acceleration.Cols(); c++) {
                result[c] += acceleration(r, c);
            }
        }
        return result;
    }

    std::vector<float> DirectAcceleration(
        const data& particles,
        const bool xComponent
    ) {
        std::vector<float> result(particles.Bodies(), 0.0f);
        for (size_t i = 0; i < particles.Bodies(); i++) {
            for (size_t j = 0; j < particles.Bodies(); j++) {
                if (i == j) continue;

                const float dx = particles.PosX()[j] - particles.PosX()[i];
                const float dy = particles.PosY()[j] - particles.PosY()[i];
                const float distanceSquared = dx * dx + dy * dy;
                const float inverseDistanceCubed =
                    1.0f / (distanceSquared * std::sqrt(distanceSquared));

                result[i] += (xComponent ? dx : dy) * particles.Mass()[j] * inverseDistanceCubed;
            }
        }
        return result;
    }
}

TEST(QuadtreeConstruction, ThetaIsStoredAsSquaredValue) {
    quadtree tree(0.5f);
    EXPECT_FLOAT_EQ(tree.Theta(), 0.5f);
}

TEST(QuadtreeForces, ZeroThetaMatchesDirectPairwiseForces) {
    data particles(4, true);

    particles.PosX()[0] = -1.0f; particles.PosY()[0] = 0.5f;
    particles.PosX()[1] = 1.5f;  particles.PosY()[1] = -0.5f;
    particles.PosX()[2] = 0.25f; particles.PosY()[2] = 2.0f;
    particles.PosX()[3] = -2.0f; particles.PosY()[3] = -1.5f;

    particles.Mass()[0] = 2.0f;
    particles.Mass()[1] = 3.0f;
    particles.Mass()[2] = 0.5f;
    particles.Mass()[3] = 1.25f;

    const auto expectedX = DirectAcceleration(particles, true);
    const auto expectedY = DirectAcceleration(particles, false);

    quadtree tree(0.0f);
    tree.Insert(particles);
    tree.Acc(particles);

    const auto actualX = SumAccelerationRows(particles.AccX());
    const auto actualY = SumAccelerationRows(particles.AccY());

    ASSERT_EQ(actualX.size(), particles.Bodies());
    ASSERT_EQ(actualY.size(), particles.Bodies());
    for (size_t i = 0; i < particles.Bodies(); i++) {
        EXPECT_NEAR(actualX[i], expectedX[i], 2e-4f);
        EXPECT_NEAR(actualY[i], expectedY[i], 2e-4f);
    }
}

TEST(QuadtreeForces, SingleBodyHasZeroAcceleration) {
    data particles(1, true);
    particles.PosX()[0] = 4.0f;
    particles.PosY()[0] = -3.0f;
    particles.Mass()[0] = 10.0f;

    quadtree tree(0.5f);
    tree.Insert(particles);
    tree.Acc(particles);

    const auto actualX = SumAccelerationRows(particles.AccX());
    const auto actualY = SumAccelerationRows(particles.AccY());
    EXPECT_FLOAT_EQ(actualX[0], 0.0f);
    EXPECT_FLOAT_EQ(actualY[0], 0.0f);
}
