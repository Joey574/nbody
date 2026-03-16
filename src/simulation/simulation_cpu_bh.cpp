#include "simulation.hpp"

std::chrono::nanoseconds simulation::update_cpu_bh(const float ft) noexcept {
    auto start = std::chrono::high_resolution_clock::now();
    qt.update(data_);
    return std::chrono::high_resolution_clock::now() - start;
}