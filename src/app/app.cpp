#include "app.hpp"

#include <GLFW/glfw3.h>
#include <chrono>

bool app::paused = false;

/// @brief Handles application starting and managing the main loop
/// @param f User defined cli arguments
/// @return 0 if successful
int app::Run(const cliargs& f) {
    app::paused = f.pause;

    if (!f.quiet) { printf("\033c"); }
    sim = simulation(f);
    if (!f.record.empty()) {
        ren.EnableRecording(f.record, f.fps, f.frames);
    }

    ren.RegisterKey(GLFW_KEY_PAUSE, std::function(PauseCallback));
    ren.RegisterKey(GLFW_KEY_SPACE, std::function(PauseCallback));
    ren.InitVulkan(f);

    if (sim.isGPU()) {
        sim.initGPU(ren.GetLogicalDevice(), ren.GetPhysicalDevice());
    }

    ren.initRendering(sim);
    MainLoop(f);

    ren.WaitIdle();
    ren.ReleaseSimulationReferences();
    sim.cleanup();
    ren.Cleanup();
    return 0;
}

int app::MainLoop(const cliargs& f) {
    using namespace std::chrono;
    double sim_sum = 0.0;
    double ren_sum = 0.0;
    double tot_sum = 0.0;
    size_t count = 0;

    auto frame_start = high_resolution_clock::now();
    auto last_print = high_resolution_clock::now();
    const auto ft = f.config.FixedTime();

    while (!ren.ShouldClose()) {
        auto time = high_resolution_clock::now();
        float dt = duration<float>(high_resolution_clock::now() - frame_start).count();
        frame_start = time;

        double sim_time = 0.0;
        if (!app::paused) {
            sim_time = sim.update(ft).count() * 0.000001;
            sim_sum += sim_time;
            count++;
        }

        auto ren_time = ren.Render(sim, dt, ft, app::paused).count() * 0.000001;
        if (!app::paused) {
            ren_sum += ren_time;
            tot_sum += sim_time + ren_time;
        }

        // lock debugging output to refresh
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
