#pragma once
#include <functional>
#include <iostream>
#include <chrono>

#include "../simulation/simulation.hpp"
#include "../renderer/renderer.hpp"

struct app {
    public:

    ~app() { cleanup(); }

    int run(const cliargs& f);

    private:
    GLFWwindow* window = nullptr;
    simulation sim = simulation();
    renderer ren = renderer();

    int init_window();
    int main_loop(const cliargs& f);
    void cleanup();    

    size_t width_ = 720;
    size_t height_ = 720;
};
