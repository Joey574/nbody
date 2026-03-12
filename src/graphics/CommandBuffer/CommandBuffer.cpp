#include "CommandBuffer.hpp"

void CommandBuffer::init(const LogicalDevice& ldevice, uint32_t frames) { 
    vk::CommandPoolCreateInfo poolInfo {
        .flags = vk::CommandPoolCreateFlagBits::eResetCommandBuffer,
        .queueFamilyIndex = ldevice.QueueIdx()
    };

    commandPool = vk::raii::CommandPool(ldevice, poolInfo);

    commandBuffers.clear();
    vk::CommandBufferAllocateInfo allocInfo {
        .commandPool = commandPool,
        .level = vk::CommandBufferLevel::ePrimary,
        .commandBufferCount = frames
    };

    commandBuffers = vk::raii::CommandBuffers(ldevice, allocInfo);
}
