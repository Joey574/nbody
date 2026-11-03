#include "gtest/gtest.h"

#include <stdexcept>

#include "config/config.hpp"
#include "test_config.hpp"

TEST(ConfigLoading, LoadsTimingAndBarnesHutParameters) {
    temporary_config file(R"yaml(
fixed_time: 0.02
theta: 0.75
galaxies:
  - type: uniform
    points: 2
    seed: 10
    inner_radius: 0.5
    outer_radius: 1.0
)yaml");

    Config config;
    config.Load(file.path().string());

    EXPECT_FLOAT_EQ(config.FixedTime(), 0.02f);
    EXPECT_FLOAT_EQ(config.Theta(), 0.75f);
}

TEST(ConfigLoading, UsesDefaultTimingAndThetaWhenOmitted) {
    temporary_config file(R"yaml(
galaxies:
  - type: uniform
    points: 1
    seed: 10
    inner_radius: 0.0
    outer_radius: 1.0
)yaml");

    Config config;
    config.Load(file.path().string());

    EXPECT_FLOAT_EQ(config.FixedTime(), 0.1f);
    EXPECT_FLOAT_EQ(config.Theta(), 1.0f);
}

TEST(ConfigInitialization, CombinesAllConfiguredGalaxies) {
    temporary_config file(R"yaml(
fixed_time: 0.01
theta: 0.0
galaxies:
  - type: uniform
    points: 3
    seed: 101
    inner_radius: 0.5
    outer_radius: 2.0
    x_pos: 1.0
    y_pos: 2.0
    center_mass: 5.0
  - type: uniform
    points: 2
    seed: 202
    inner_radius: 0.25
    outer_radius: 1.0
    x_pos: -3.0
    y_pos: -4.0
    center_mass: 7.0
)yaml");

    Config config;
    config.Load(file.path().string());
    data generated = config.New(true);

    ASSERT_EQ(generated.bodies(), 5);
    EXPECT_FLOAT_EQ(generated.posx()[0], 1.0f);
    EXPECT_FLOAT_EQ(generated.posy()[0], 2.0f);
    EXPECT_FLOAT_EQ(generated.mass()[0], 5.0f);
    EXPECT_FLOAT_EQ(generated.posx()[3], -3.0f);
    EXPECT_FLOAT_EQ(generated.posy()[3], -4.0f);
    EXPECT_FLOAT_EQ(generated.mass()[3], 7.0f);
    EXPECT_EQ(generated.accx().cols(), 5);
    EXPECT_EQ(generated.accy().cols(), 5);
}

TEST(ConfigInitialization, RejectsMissingOrEmptyGalaxySequence) {
    temporary_config file("fixed_time: 0.01\ngalaxies: []\n");

    Config config;
    config.Load(file.path().string());
    EXPECT_THROW(config.New(false), std::invalid_argument);
}

TEST(ConfigInitialization, OmitsAccelerationStorageWhenNotRequested) {
    temporary_config file(TWO_BODY_CONFIG);

    Config config;
    config.Load(file.path().string());
    data generated = config.New(false);

    ASSERT_EQ(generated.bodies(), 2);
    EXPECT_EQ(generated.accx().rows(), 0);
    EXPECT_EQ(generated.accx().cols(), 0);
    EXPECT_EQ(generated.accy().rows(), 0);
    EXPECT_EQ(generated.accy().cols(), 0);
}
