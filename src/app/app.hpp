#pragma once
#include "../simulation/simulation.hpp"
#include "../renderer/renderer.hpp"
#include "../cli/cli.hpp"

struct app {
    public:
    int run(const cliargs& f);

    private:
    simulation sim;
    renderer   ren;

    int main_loop(const cliargs& f);
};
