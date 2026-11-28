#pragma once
#include "simulation/simulation.hpp"
#include "renderer/renderer.hpp"
#include "cli/cli.hpp"

struct app {
    public:
    int Run(const cliargs& f);

    private:
    simulation sim;
    renderer   ren;

    int MainLoop(const cliargs& f);

    static bool paused;
    static void PauseCallback(GLFWwindow* w, int scancode, int action, int mods);
};
