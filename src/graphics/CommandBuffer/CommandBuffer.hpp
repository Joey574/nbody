#pragma once
#include "../../definitions/graphics.hpp" // IWYU pragma: keep
#include "../LogicalDevice/LogicalDevice.hpp"

struct CommandBuffer {
    private:
    std::vector<vk::raii::CommandBuffer> commandBuffers;

    vk::raii::CommandPool commandPool = nullptr;
    public:

    void init(const LogicalDevice& ldevice, uint32_t frames);
};