#pragma once
#include <cstddef>
#include <string>
#include "../config/config.hpp"

struct cliargs {
    public:
    cliargs() : refresh(100), cpu(false), bh(false), quiet(false), path("") {}

    void parse(int argc, char* argv[]);

    Config config;
    std::string path;
    size_t refresh = 100;
    bool cpu = false;
    bool bh = false;
    bool quiet = false;
};
