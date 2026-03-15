#include "simulation.hpp"

std::chrono::nanoseconds simulation::update_gpu(const float dt) noexcept {
    auto s = std::chrono::high_resolution_clock::now();
    return std::chrono::high_resolution_clock::now() - s;
}
