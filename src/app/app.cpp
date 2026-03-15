#include "app.hpp"

#include <functional>
#include <chrono>


/// @brief Handles application starting and managing the main loop
/// @param f User defined cli arguments
/// @return 0 if successful
int app::run(const cliargs& f) {
    if (!f.quiet) { printf("\033c"); }
    sim = simulation(f);
    ren = renderer();
    
    ren.init(sim.get_data(), f.path);
    if (main_loop(f)) { return 1; }
    cleanup();

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

    while (!ren.should_close()) {
        count++;
        auto time = high_resolution_clock::now();
        float dt = duration<float>(high_resolution_clock::now() - frame_start).count();
        frame_start = time;

        auto sim_time = std::invoke(sim.update, sim, f.fixedtime).count() * 0.000001;
        sim_sum += sim_time;

        auto ren_time = ren.render(sim.get_data(), dt).count() * 0.000001;
        ren_sum += ren_time;
        tot_sum += sim_time + ren_time;

        // lock debugging output to 100ms refresh
        if (!f.quiet && high_resolution_clock::now() - last_print >= milliseconds(f.refresh)) {
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

            last_print = high_resolution_clock::now();
        }
    }

    return 0;
}

void app::cleanup() {
    ren.cleanup();
}
