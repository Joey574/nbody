#include "simulation.hpp"

std::chrono::nanoseconds simulation::UpdateCPUBH(const float ft) noexcept {
    auto start = std::chrono::high_resolution_clock::now();

    tree_.Insert(data_);
    tree_.Acc(data_);
    SumAcc();
    MovePoints(ft);

    return std::chrono::high_resolution_clock::now() - start;
}
