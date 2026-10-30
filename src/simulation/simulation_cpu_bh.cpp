#include "simulation.hpp"

std::chrono::nanoseconds simulation::update_cpu_bh(const float ft) noexcept {
    auto start = std::chrono::high_resolution_clock::now();

    tree_.insert(data_);
    tree_.compute_acc(ft, data_.accx(), data_.accy());
    sum_acc();
    move_points(ft);

    return std::chrono::high_resolution_clock::now() - start;
}
