#pragma once
#include <functional>
#include <iostream>
#include <chrono>

//#include "../simulation/simulation.hpp"
import simulation;
#include "../renderer/renderer.hpp"

struct app {
    public:

    int run(const cliargs& f);

    private:
    simulation sim = simulation();
    renderer ren = renderer();

    int main_loop(const cliargs& f);
    void cleanup();    
};
