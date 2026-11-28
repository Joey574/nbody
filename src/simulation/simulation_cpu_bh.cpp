#include "simulation.hpp"

std::chrono::nanoseconds simulation::UpdateCpuBH(const float ft) noexcept {
    auto start = std::chrono::high_resolution_clock::now();

    tree_.insert(data_);
    tree_.acc(data_);
    SumAcc();
    MovePoints(ft);

    return std::chrono::high_resolution_clock::now() - start;
}
