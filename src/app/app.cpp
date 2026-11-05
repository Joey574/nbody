#include "app.hpp"

#include <chrono>

/// @brief Handles application starting and managing the main loop
/// @param f User defined cli arguments
/// @return 0 if successful
int app::run(const cliargs& f) {
    if (!f.quiet) { printf("\033c"); }
    sim = simulation(f);
    ren.initVulkan();

    if (sim.isGPU()) {
        sim.initGPU(ren.logicalDevice(), ren.physicalDevice());
    }

    ren.initRendering(sim);
    main_loop(f);

    ren.waitIdle();
    ren.releaseSimulationReferences();
    sim.cleanup();
    ren.cleanup();
    return 0;
}

int app::main_loop(const cliargs& f) {
    using namespace std::chrono;
    double sim_sum = 0.0;
    double ren_sum = 0.0;
    double tot_sum = 0.0;
    size_t count = 0;

    auto frame_start = high_resolution_clock::now();
    auto last_print = high_resolution_clock::now();
    const auto ft = f.config.FixedTime();

    while (!ren.should_close()) {
        count++;
        auto time = high_resolution_clock::now();
        float dt = duration<float>(high_resolution_clock::now() - frame_start).count();
        frame_start = time;

        auto sim_time = sim.update(ft).count() * 0.000001;
        sim_sum += sim_time;

        auto ren_time = ren.render(sim, dt, ft).count() * 0.000001;
        ren_sum += ren_time;
        tot_sum += sim_time + ren_time;

        // lock debugging output to 100ms refresh
        if (!f.quiet && high_resolution_clock::now() - last_print >= milliseconds(f.refresh)) {
            if (sim.isGPU()) {
                printf(
                    "\033[H"
                    "Frame %zu"
                    "\n\nRenderer (ms):"
                    "\n\tAverage: %.2f     "
                    "\n\tLast:    %.2f     "
                    "\n\nPerformance:"
                    "\n\tFPS:     %.2f     "
                    "\n",
                    count,
                    ren_sum / count, ren_time,
                    1000.00 / (tot_sum / count)
                );
            } else {
                printf(
                    "\033[H"
                    "Frame %zu"
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
            }

            last_print = high_resolution_clock::now();
        }
    }

    return 0;
}
