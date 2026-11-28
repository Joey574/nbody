#include "gtest/gtest.h"

#include <cmath>

#include <cmath>

#include "simulation/simulation.hpp"
#include "test_config.hpp"

TEST(SimulationBackends, SelectsCpuDirectSumBackend) {
    temporary_config file(TWO_BODY_CONFIG);

    cliargs args;
    args.cpu = true;
    args.bh = false;
    args.config.Load(file.path().string());

    simulation sim(args);

    EXPECT_EQ(sim.GetBackend(), simulation::backend::CPU);
    EXPECT_FALSE(sim.IsGPU());
    EXPECT_EQ(sim.Points(), 2);
}

TEST(SimulationBackends, SelectsCpuBarnesHutBackend) {
    temporary_config file(TWO_BODY_CONFIG);

    cliargs args;
    args.cpu = true;
    args.bh = true;
    args.config.Load(file.path().string());

    simulation sim(args);

    EXPECT_EQ(sim.GetBackend(), simulation::backend::CPU_BH);
    EXPECT_FALSE(sim.IsGPU());
    EXPECT_EQ(sim.Points(), 2);
}

TEST(SimulationUpdates, CpuDirectSumComputesFinitePairwiseAcceleration) {
    temporary_config file(TWO_BODY_CONFIG);

    cliargs args;
    args.cpu = true;
    args.bh = false;
    args.config.Load(file.path().string());

    simulation sim(args);
    const float originalX = sim.GetData().PosX()[0];
    const float originalY = sim.GetData().PosY()[0];

    sim.Update(0.0f);

    const float ax = sim.GetData().AccX()(0, 0);
    const float ay = sim.GetData().AccY()(0, 0);
    EXPECT_TRUE(std::isfinite(ax));
    EXPECT_TRUE(std::isfinite(ay));
    EXPECT_GT(std::sqrt(ax * ax + ay * ay), 0.0f);
    EXPECT_FLOAT_EQ(sim.GetData().PosX()[0], originalX);
    EXPECT_FLOAT_EQ(sim.GetData().PosY()[0], originalY);
}

TEST(SimulationUpdates, CpuBarnesHutComputesFinitePairwiseAcceleration) {
    temporary_config file(TWO_BODY_CONFIG);

    cliargs args;
    args.cpu = true;
    args.bh = true;
    args.config.Load(file.path().string());

    simulation sim(args);
    sim.Update(0.0f);

    const float ax = sim.GetData().AccX()(0, 0);
    const float ay = sim.GetData().AccY()(0, 0);
    EXPECT_TRUE(std::isfinite(ax));
    EXPECT_TRUE(std::isfinite(ay));
    EXPECT_GT(std::sqrt(ax * ax + ay * ay), 0.0f);
}

TEST(SimulationUpdates, ZeroThetaBarnesHutMatchesDirectSum) {
    temporary_config file(TWO_BODY_CONFIG);

    cliargs directArgs;
    directArgs.cpu = true;
    directArgs.bh = false;
    directArgs.config.Load(file.path().string());

    cliargs barnesHutArgs;
    barnesHutArgs.cpu = true;
    barnesHutArgs.bh = true;
    barnesHutArgs.config.Load(file.path().string());

    simulation direct(directArgs);
    simulation barnesHut(barnesHutArgs);

    direct.Update(0.0f);
    barnesHut.Update(0.0f);

    ASSERT_EQ(direct.Points(), barnesHut.Points());
    for (size_t i = 0; i < direct.Points(); i++) {
        EXPECT_NEAR(direct.GetData().AccX()(0, i), barnesHut.GetData().AccX()(0, i), 0.025f);
        EXPECT_NEAR(direct.GetData().AccY()(0, i), barnesHut.GetData().AccY()(0, i), 0.025f);
    }
}
