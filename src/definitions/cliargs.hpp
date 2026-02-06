#pragma once
#include <cstddef>

struct cliargs {
    public:
    cliargs() : bodies(0), fixedtime(0.0001f), cluster(true), spiral(false), cpu(false) {}
    size_t bodies;
    float fixedtime;
    bool cluster;
    bool spiral;
    bool cpu;
};
