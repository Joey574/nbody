#include "gtest/gtest.h"

#include "galaxy/galaxy.hpp"


TEST(GalaxyConstructor, DefaultConstructor) {
    Galaxy g;
    EXPECT_EQ(g.GalaxyType(), Galaxy::Type::None);
    EXPECT_EQ(g.GalaxyString(), "None");
    EXPECT_EQ(g.Points(), 0);
    EXPECT_EQ(g.Seed(), 0);

    EXPECT_ANY_THROW(g.New(true));
}
