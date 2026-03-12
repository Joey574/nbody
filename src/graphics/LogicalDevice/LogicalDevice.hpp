#pragma once
#include "../../definitions/graphics.hpp" // IWYU pragma: keep
#include "../PhysicalDevice/PhysicalDevice.hpp"

struct LogicalDevice {
    public:
    void init(const PhysicalDevice& pdevice, const vk::raii::SurfaceKHR& surface);

    inline vk::raii::Device& getDevice() noexcept { return device; }
    inline vk::raii::Queue& getQueue() noexcept { return queue; }
    inline uint32_t getQueueIdx() const noexcept { return queueIdx; }

    inline const vk::raii::Device& getDevice() const noexcept { return device; }
    inline const vk::raii::Queue& getQueue() const noexcept { return queue; }

    inline operator vk::raii::Device&() { return device; }
    inline operator const vk::raii::Device&() const { return device; }

    private:
    vk::raii::Device device    = nullptr;
    vk::raii::Queue  queue     = nullptr;
    uint32_t         queueIdx  = 0;

    static constexpr const char* deviceExtensions[1] = {
        vk::KHRSwapchainExtensionName
    };
};
