module;
#include <functional>
#include <iostream>
#include <chrono>
export module app;
import cli;
import simulation;
import renderer;

export struct app {
    public:

    int run(const cliargs& f);

    private:
    simulation sim = simulation();
    renderer ren = renderer();

    int main_loop(const cliargs& f);
    void cleanup();    
};