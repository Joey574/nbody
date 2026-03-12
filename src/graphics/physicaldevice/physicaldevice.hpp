#pragma once
#include "../../definitions/graphics.hpp" // IWYU pragma: keep

struct physicaldevice {
    public:
    void init(const vk::raii::Instance& instance);

    vk::raii::PhysicalDevice& get() noexcept { return device; }
    const vk::raii::PhysicalDevice& get() const noexcept { return device; }

    private:
    vk::raii::PhysicalDevice device = nullptr;
};
