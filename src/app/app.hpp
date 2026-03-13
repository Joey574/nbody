#pragma once
#include "../simulation/simulation.hpp"
#include "../renderer/renderer.hpp"
#include "../cli/cli.hpp"

struct app {
    private:
    simulation sim = simulation();
    renderer   ren = renderer();
    float lastFrameTime = 0.0f;

    int main_loop(const cliargs& f);
    void cleanup();

    public:
    int run(const cliargs& f);
};