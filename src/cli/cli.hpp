#pragma once
#include "../config/config.hpp"

#include <cstddef>

struct cliargs {
    public:
        cliargs() = default;

        void Parse(int argc, char* argv[]);

        Config config;
        size_t refresh = 100;
        bool cpu = false;
        bool bh = false;
        bool quiet = false;

        bool headless = false;
        std::string record;
        uint32_t fps = 60;
        size_t frames = 0;

        bool pause = false;
        float zoom = 1.0f;
        float x = 0.0f;
        float y = 0.0f;

        size_t width = 800;
        size_t height = 800;

    private:
        void PrintUsage() const;
};
