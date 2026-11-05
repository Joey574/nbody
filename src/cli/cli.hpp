#pragma once
#include "../config/config.hpp"

#include <cstddef>

struct cliargs {
    public:
        cliargs() : refresh(100), cpu(false), bh(false), quiet(false) {}

        void parse(int argc, char* argv[]);

        Config config;
        size_t refresh = 100;
        bool cpu = false;
        bool bh = false;
        bool quiet = false;
};
