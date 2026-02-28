#pragma once
#include <cstddef>
#include <string>

struct cliargs {
    public:
    cliargs() : bodies(100), refresh(100), fixedtime(0.0001f), cluster(true), spiral(false), cpu(false), quiet(false), path("") {}

    void parse(int argc, char* argv[]);

    std::string path;
    size_t bodies;
    size_t refresh;
    float fixedtime;
    bool cluster;
    bool spiral;
    bool cpu;
    bool quiet;
};
