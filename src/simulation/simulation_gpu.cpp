#include "simulation.hpp"

void simulation::initGPU(const LogicalDevice& ld, const PhysicalDevice& pd) {
    if (!isGPU()) return;

    dsgpu_.init(data_, ld, pd);
    data_.clear();
}

void simulation::recordGPU(const vk::raii::CommandBuffer& cmd, float dt) {
    if (!isGPU()) return;
    dsgpu_.record(cmd, dt);
}

void simulation::cleanup() {
    dsgpu_.cleanup();
}
