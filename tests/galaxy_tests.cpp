#include "gtest/gtest.h"

#include <cmath>
#include <stdexcept>

#include "galaxy/galaxy.hpp"

TEST(GalaxyConstructor, DefaultConstructor) {
    Galaxy g;
    EXPECT_EQ(g.GalaxyType(), Galaxy::Type::None);
    EXPECT_EQ(g.GalaxyString(), "None");
    EXPECT_EQ(g.Points(), 0);
    EXPECT_EQ(g.Seed(), 0);

    EXPECT_ANY_THROW(g.New(true));
}

TEST(GalaxyConstructor, LoadsUniformConfiguration) {
    const YAML::Node config = YAML::Load(R"yaml(
type: uniform
points: 8
seed: 77
inner_radius: 0.5
outer_radius: 2.0
)yaml");

    Galaxy galaxy(config);

    EXPECT_EQ(galaxy.GalaxyType(), Galaxy::Type::Uniform);
    EXPECT_EQ(galaxy.GalaxyString(), "Uniform");
    EXPECT_EQ(galaxy.Points(), 8);
    EXPECT_EQ(galaxy.Seed(), 77);
}

TEST(GalaxyInitialization, UniformInitializesConfiguredCenterAndBodies) {
    const YAML::Node config = YAML::Load(R"yaml(
type: uniform
points: 12
seed: 91
inner_radius: 0.5
outer_radius: 3.0
x_pos: 2.0
y_pos: -4.0
x_vel: 0.25
y_vel: -0.5
center_mass: 20.0
)yaml");

    Galaxy galaxy(config);
    data generated = galaxy.New(true);

    ASSERT_EQ(generated.bodies(), 12);
    EXPECT_FLOAT_EQ(generated.posx()[0], 2.0f);
    EXPECT_FLOAT_EQ(generated.posy()[0], -4.0f);
    EXPECT_FLOAT_EQ(generated.velx()[0], 0.25f);
    EXPECT_FLOAT_EQ(generated.vely()[0], -0.5f);
    EXPECT_FLOAT_EQ(generated.mass()[0], 20.0f);
    EXPECT_EQ(generated.accx().cols(), 12);
    EXPECT_EQ(generated.accy().cols(), 12);

    for (size_t i = 0; i < generated.bodies(); i++) {
        EXPECT_TRUE(std::isfinite(generated.posx()[i]));
        EXPECT_TRUE(std::isfinite(generated.posy()[i]));
        EXPECT_TRUE(std::isfinite(generated.velx()[i]));
        EXPECT_TRUE(std::isfinite(generated.vely()[i]));
        EXPECT_GT(generated.mass()[i], 0.0f);
    }
}

TEST(GalaxyInitialization, UniformSeedProducesRepeatableInitialConditions) {
    const YAML::Node config = YAML::Load(R"yaml(
type: uniform
points: 16
seed: 12345
inner_radius: 0.25
outer_radius: 2.5
center_mass: 10.0
)yaml");

    Galaxy firstGalaxy(config);
    Galaxy secondGalaxy(config);
    data first = firstGalaxy.New(false);
    data second = secondGalaxy.New(false);

    ASSERT_EQ(first.bodies(), second.bodies());
    for (size_t i = 0; i < first.bodies(); i++) {
        EXPECT_FLOAT_EQ(first.posx()[i], second.posx()[i]);
        EXPECT_FLOAT_EQ(first.posy()[i], second.posy()[i]);
        EXPECT_FLOAT_EQ(first.velx()[i], second.velx()[i]);
        EXPECT_FLOAT_EQ(first.vely()[i], second.vely()[i]);
        EXPECT_FLOAT_EQ(first.mass()[i], second.mass()[i]);
    }
}

TEST(GalaxyConstructor, UnknownTypeIsRejected) {
    const YAML::Node config = YAML::Load(R"yaml(
type: unknown
points: 10
)yaml");

    EXPECT_THROW((void)Galaxy(config), std::invalid_argument);
}

TEST(GalaxyInitialization, RecognizedButUnimplementedTypesCannotGenerateData) {
    const YAML::Node config = YAML::Load(R"yaml(
type: cluster
points: 10
seed: 1
)yaml");

    Galaxy galaxy(config);
    EXPECT_EQ(galaxy.GalaxyType(), Galaxy::Type::Cluster);
    EXPECT_THROW(galaxy.New(false), std::invalid_argument);
}
