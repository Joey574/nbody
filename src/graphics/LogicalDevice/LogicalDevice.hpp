#pragma once
#include "../../definitions/graphics.hpp" // IWYU pragma: keep
#include "../PhysicalDevice/PhysicalDevice.hpp"

struct LogicalDevice {
    public:
    void init(const PhysicalDevice& pdevice, const vk::raii::SurfaceKHR& surface);

    inline auto& Device() noexcept { return device; }
    inline auto& Queue() noexcept { return queue; }

    inline const auto& Device() const noexcept { return device; }
    inline const auto& Queue() const noexcept { return queue; }
    inline uint32_t QueueIdx() const noexcept { return queueIdx; }

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
