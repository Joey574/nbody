#pragma once
#include "../../definitions/graphics.hpp" // IWYU pragma: keep
#include "../physicaldevice/physicaldevice.hpp"

struct logicaldevice {
    public:
    void init(const physicaldevice& pdevice, const vk::raii::SurfaceKHR& surface);

    vk::raii::Device& getDevice() noexcept { return device; }
    vk::raii::Queue& getQueue() noexcept { return queue; }
    uint32_t getQueueIdx() const noexcept { return queueIdx; }

    const vk::raii::Device& getDevice() const noexcept { return device; }
    const vk::raii::Queue& getQueue() const noexcept { return queue; }

    private:
    vk::raii::Device device    = nullptr;
    vk::raii::Queue  queue     = nullptr;
    uint32_t         queueIdx  = 0;

    static constexpr const char* deviceExtensions[1] = {
        vk::KHRSwapchainExtensionName
    };
};
