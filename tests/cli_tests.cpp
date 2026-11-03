#include "gtest/gtest.h"

#include <stdexcept>
#include <string>

#include "cli/cli.hpp"
#include "test_config.hpp"

TEST(CliParsing, ParsesBackendAndOutputOptions) {
    temporary_config file(TWO_BODY_CONFIG);
    std::string configPath = file.path().string();

    char program[] = "nbody";
    char cpu[] = "--cpu";
    char bh[] = "--bh";
    char quiet[] = "-q";
    char refresh[] = "--refresh";
    char refreshValue[] = "25";
    char config[] = "-f";
    char* argv[] = {
        program, cpu, bh, quiet, refresh, refreshValue,
        config, configPath.data()
    };

    cliargs args;
    args.parse(8, argv);

    EXPECT_TRUE(args.cpu);
    EXPECT_TRUE(args.bh);
    EXPECT_TRUE(args.quiet);
    EXPECT_EQ(args.refresh, 25);
    EXPECT_EQ(args.config.New(false).bodies(), 2);
}

TEST(CliParsing, DefaultsToGpuDirectSumAndStandardRefreshRate) {
    cliargs args;
    EXPECT_FALSE(args.cpu);
    EXPECT_FALSE(args.bh);
    EXPECT_FALSE(args.quiet);
    EXPECT_EQ(args.refresh, 100);
}

TEST(CliParsing, RejectsUnknownArguments) {
    char program[] = "nbody";
    char invalid[] = "--not-an-option";
    char* argv[] = { program, invalid };

    cliargs args;
    EXPECT_ANY_THROW(args.parse(2, argv));
}
