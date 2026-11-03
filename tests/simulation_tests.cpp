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

    EXPECT_EQ(sim.get_backend(), simulation::backend::CPU);
    EXPECT_FALSE(sim.isGPU());
    EXPECT_EQ(sim.points(), 2);
}

TEST(SimulationBackends, SelectsCpuBarnesHutBackend) {
    temporary_config file(TWO_BODY_CONFIG);

    cliargs args;
    args.cpu = true;
    args.bh = true;
    args.config.Load(file.path().string());

    simulation sim(args);

    EXPECT_EQ(sim.get_backend(), simulation::backend::CPU_BH);
    EXPECT_FALSE(sim.isGPU());
    EXPECT_EQ(sim.points(), 2);
}

TEST(SimulationUpdates, CpuDirectSumComputesFinitePairwiseAcceleration) {
    temporary_config file(TWO_BODY_CONFIG);

    cliargs args;
    args.cpu = true;
    args.bh = false;
    args.config.Load(file.path().string());

    simulation sim(args);
    const float originalX = sim.get_data().posx()[0];
    const float originalY = sim.get_data().posy()[0];

    sim.update(0.0f);

    const float ax = sim.get_data().accx()(0, 0);
    const float ay = sim.get_data().accy()(0, 0);
    EXPECT_TRUE(std::isfinite(ax));
    EXPECT_TRUE(std::isfinite(ay));
    EXPECT_GT(std::sqrt(ax * ax + ay * ay), 0.0f);
    EXPECT_FLOAT_EQ(sim.get_data().posx()[0], originalX);
    EXPECT_FLOAT_EQ(sim.get_data().posy()[0], originalY);
}

TEST(SimulationUpdates, CpuBarnesHutComputesFinitePairwiseAcceleration) {
    temporary_config file(TWO_BODY_CONFIG);

    cliargs args;
    args.cpu = true;
    args.bh = true;
    args.config.Load(file.path().string());

    simulation sim(args);
    sim.update(0.0f);

    const float ax = sim.get_data().accx()(0, 0);
    const float ay = sim.get_data().accy()(0, 0);
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

    direct.update(0.0f);
    barnesHut.update(0.0f);

    ASSERT_EQ(direct.points(), barnesHut.points());
    for (size_t i = 0; i < direct.points(); i++) {
        EXPECT_NEAR(direct.get_data().accx()(0, i), barnesHut.get_data().accx()(0, i), 2e-2f);
        EXPECT_NEAR(direct.get_data().accy()(0, i), barnesHut.get_data().accy()(0, i), 2e-2f);
    }
}
