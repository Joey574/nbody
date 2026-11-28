#include "simulation.hpp"

void simulation::InitGPU(const LogicalDevice& ld, const PhysicalDevice& pd) {
    if (!IsGPU()) return;

    dsgpu_.init(data_, ld, pd);
    data_.Clear();
}

void simulation::recordGPU(const vk::raii::CommandBuffer& cmd, float dt) {
    if (!IsGPU()) return;
    dsgpu_.record(cmd, dt);
}

void simulation::Cleanup() {
    dsgpu_.cleanup();
}
