#include "simulation.hpp"
#include <vulkan/vulkan_raii.hpp>

void simulation::initGPU(const LogicalDevice& ld, const PhysicalDevice& pd) {
    if (!isGPU()) return;

    gpu_.init(data_, ld, pd);
    data_.clear();
}

void simulation::recordGPU(const vk::raii::CommandBuffer& cmd, float dt) {
    if (!isGPU()) return;
    gpu_.record(cmd, dt);
}

void simulation::cleanup() {
    gpu_.cleanup();
}
