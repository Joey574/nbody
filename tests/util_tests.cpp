#include "gtest/gtest.h"

#include "definitions/macros.hpp"
#include "util/util.hpp"

TEST(UtilParsing, AlignedSizeRoundsUpToMemoryAlignment) {
    EXPECT_EQ(util::aligned_size(0), 0);
    EXPECT_EQ(util::aligned_size(1), MEM_ALIGNMENT);
    EXPECT_EQ(util::aligned_size(MEM_ALIGNMENT), MEM_ALIGNMENT);
    EXPECT_EQ(util::aligned_size(MEM_ALIGNMENT-1), MEM_ALIGNMENT);
    EXPECT_EQ(util::aligned_size(MEM_ALIGNMENT+1), 2 * MEM_ALIGNMENT);
}

TEST(UtilParsing, ParseStringSupportsDecimalSizeSuffixes) {
    EXPECT_EQ(util::parse_string(""), 0);
    EXPECT_EQ(util::parse_string("42"), 42);
    EXPECT_EQ(util::parse_string("2k"), 2000);
    EXPECT_EQ(util::parse_string("3M"), 3000000);
    EXPECT_EQ(util::parse_string("4b"), 4000000000ULL);
}

TEST(UtilParsing, ParseStringRejectsNonNumericInput) {
    EXPECT_ANY_THROW(util::parse_string("abc"));
}

TEST(UtilForces, ApplyForceToOneUsesOtherMass) {
    float ax = 0.0f;
    float ay = 0.0f;

    util::apply_force_to_one(0.0f, 0.0f, 2.0f, 0.0f, 5.0f, &ax, &ay);

    EXPECT_NEAR(ax, 1.25f, 1e-4f);
    EXPECT_NEAR(ay, 0.0f, 1e-6f);
}

TEST(UtilForces, ApplyForceToTwoConservesMassWeightedAcceleration) {
    float a1x = 0.0f;
    float a1y = 0.0f;
    float a2x = 0.0f;
    float a2y = 0.0f;

    util::apply_force_to_two(
        0.0f, 0.0f, 3.0f,
        2.0f, 0.0f, 5.0f,
        &a1x, &a1y, &a2x, &a2y
    );

    EXPECT_NEAR(a1x, 1.25f, 1e-4f);
    EXPECT_NEAR(a2x, -0.75f, 1e-4f);
    EXPECT_NEAR(a1y, 0.0f, 1e-6f);
    EXPECT_NEAR(a2y, 0.0f, 1e-6f);
    EXPECT_NEAR(3.0f * a1x + 5.0f * a2x, 0.0f, 1e-4f);
}
