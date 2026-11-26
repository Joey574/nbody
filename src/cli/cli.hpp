#pragma once
#include "../config/config.hpp"

#include <cstddef>

struct cliargs {
    public:
        cliargs() = default;

        void parse(int argc, char* argv[]);

        Config config;
        size_t refresh = 100;
        bool cpu = false;
        bool bh = false;
        bool quiet = false;

        std::string record;
        uint32_t fps = 60;
        size_t frames = 0;

        bool pause = false;
        float zoom = 1.0f;
        float x = 0.0f;
        float y = 0.0f;

    private:
        void printUsage() const;
};
