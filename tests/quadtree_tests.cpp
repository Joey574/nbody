#include "gtest/gtest.h"

#include <cmath>
#include <utility>
#include <vector>

#include "quadtree/quadtree.hpp"

namespace {
    std::vector<float> SumAccelerationRows(const matrix& acceleration) {
        std::vector<float> result(acceleration.cols(), 0.0f);
        for (size_t r = 0; r < acceleration.rows(); r++) {
            for (size_t c = 0; c < acceleration.cols(); c++) {
                result[c] += acceleration(r, c);
            }
        }
        return result;
    }

    std::vector<float> DirectAcceleration(
        const data& particles,
        const bool xComponent
    ) {
        std::vector<float> result(particles.bodies(), 0.0f);
        for (size_t i = 0; i < particles.bodies(); i++) {
            for (size_t j = 0; j < particles.bodies(); j++) {
                if (i == j) continue;

                const float dx = particles.posx()[j] - particles.posx()[i];
                const float dy = particles.posy()[j] - particles.posy()[i];
                const float distanceSquared = dx * dx + dy * dy;
                const float inverseDistanceCubed =
                    1.0f / (distanceSquared * std::sqrt(distanceSquared));

                result[i] += (xComponent ? dx : dy) * particles.mass()[j] * inverseDistanceCubed;
            }
        }
        return result;
    }
}

TEST(QuadtreeConstruction, ThetaIsStoredAsSquaredValue) {
    quadtree tree(0.5f);
    EXPECT_FLOAT_EQ(tree.theta(), 0.5f);
}

TEST(QuadtreeForces, ZeroThetaMatchesDirectPairwiseForces) {
    data particles(4, true);

    particles.posx()[0] = -1.0f; particles.posy()[0] = 0.5f;
    particles.posx()[1] = 1.5f;  particles.posy()[1] = -0.5f;
    particles.posx()[2] = 0.25f; particles.posy()[2] = 2.0f;
    particles.posx()[3] = -2.0f; particles.posy()[3] = -1.5f;

    particles.mass()[0] = 2.0f;
    particles.mass()[1] = 3.0f;
    particles.mass()[2] = 0.5f;
    particles.mass()[3] = 1.25f;

    const auto expectedX = DirectAcceleration(particles, true);
    const auto expectedY = DirectAcceleration(particles, false);

    quadtree tree(0.0f);
    tree.insert(particles);
    tree.compute_acc(particles);

    const auto actualX = SumAccelerationRows(particles.accx());
    const auto actualY = SumAccelerationRows(particles.accy());

    ASSERT_EQ(actualX.size(), particles.bodies());
    ASSERT_EQ(actualY.size(), particles.bodies());
    for (size_t i = 0; i < particles.bodies(); i++) {
        EXPECT_NEAR(actualX[i], expectedX[i], 2e-4f);
        EXPECT_NEAR(actualY[i], expectedY[i], 2e-4f);
    }
}

TEST(QuadtreeForces, SingleBodyHasZeroAcceleration) {
    data particles(1, true);
    particles.posx()[0] = 4.0f;
    particles.posy()[0] = -3.0f;
    particles.mass()[0] = 10.0f;

    quadtree tree(0.5f);
    tree.insert(particles);
    tree.compute_acc(particles);

    const auto actualX = SumAccelerationRows(particles.accx());
    const auto actualY = SumAccelerationRows(particles.accy());
    EXPECT_FLOAT_EQ(actualX[0], 0.0f);
    EXPECT_FLOAT_EQ(actualY[0], 0.0f);
}

TEST(QuadtreeLifecycle, CopyAndMoveRetainTheBuiltTree) {
    data particles(3, true);
    particles.posx()[0] = -1.0f; particles.posy()[0] = 0.0f;
    particles.posx()[1] = 1.0f;  particles.posy()[1] = 0.0f;
    particles.posx()[2] = 0.0f;  particles.posy()[2] = 2.0f;
    particles.mass()[0] = 1.0f;
    particles.mass()[1] = 2.0f;
    particles.mass()[2] = 3.0f;

    quadtree original(0.0f);
    original.insert(particles);

    quadtree copied(original);
    EXPECT_FLOAT_EQ(copied.theta(), original.theta());
    copied.compute_acc(particles);
    const auto expectedX = SumAccelerationRows(particles.accx());
    const auto expectedY = SumAccelerationRows(particles.accy());

    quadtree moved(std::move(copied));
    EXPECT_FLOAT_EQ(moved.theta(), original.theta());
    moved.compute_acc(particles);

    const auto actualX = SumAccelerationRows(particles.accx());
    const auto actualY = SumAccelerationRows(particles.accy());
    for (size_t i = 0; i < particles.bodies(); i++) {
        EXPECT_NEAR(actualX[i], expectedX[i], 1e-5f);
        EXPECT_NEAR(actualY[i], expectedY[i], 1e-5f);
    }
}
