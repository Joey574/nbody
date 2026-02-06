#include "app.hpp"

/// @brief Handles application starting and managing the main loop
/// @param f User defined cli arguments
/// @return 0 if successful
int app::run(const cliargs& f) {
    printf("\033c");
    sim = simulation(f);
    ren = renderer();
    
    if (init_window()) { return 1; };
    if (ren.init_vulkan(window, sim.bodies())) { return 1; }
    if (main_loop(f)) { return 1; }
    cleanup();

    return 0;
}

int app::init_window() {
    if (!glfwInit()) { return 1; }

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

    window = glfwCreateWindow(width_, height_, "Nbody simulation", nullptr, nullptr);
    if (!window) {
        glfwTerminate();
        return 1;
    }

    return 0;
}

int app::main_loop(const cliargs& f) {
    double sim_sum = 0.0;
    double ren_sum = 0.0;
    double tot_sum = 0.0;
    size_t count = 0;

    while (!glfwWindowShouldClose(window)) {
        count++;
        auto sim_time = std::invoke(sim.update, sim, f.fixedtime).count() * 0.000001;
        sim_sum += sim_time;

        auto ren_time = ren.render(sim).count() * 0.000001;
        ren_sum += ren_time;
        tot_sum += sim_time + ren_time;

        glfwPollEvents();

        printf("\033[H"
            "\nFrame %zu     "
            "\n\nSimulation (ms):"
            "\n\tAverage: %.2f     "
            "\n\tLast:    %.2f     "
            "\n\nRenderer (ms):"
            "\n\tAverage: %.2f     "
            "\n\tLast:    %.2f     "
            "\n\nPerformance:"
            "\n\tFPS:     %.2f     "
            "\n",
            count, 
            sim_sum / count, sim_time,
            ren_sum / count, ren_time,
            1000.00 / (tot_sum / count)
        );

        fflush(stdout);
    }

    return 0;
}

void app::cleanup() {
    ren.cleanup();

    if (window) {
        glfwDestroyWindow(window);
    }

    glfwTerminate();
}
