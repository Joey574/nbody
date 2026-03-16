#pragma once
#include <cstddef>
#include <string>
#include "../config/config.hpp"

struct cliargs {
    public:
    cliargs() : refresh(100), cpu(false), quiet(false), path("") {}

    void parse(int argc, char* argv[]);

    Config config;
    std::string path;
    size_t refresh;
    bool cpu;
    bool quiet;
};
