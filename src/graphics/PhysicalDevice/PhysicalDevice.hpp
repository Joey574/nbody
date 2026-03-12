#pragma once
#include "../../definitions/graphics.hpp" // IWYU pragma: keep

struct PhysicalDevice {
    public:
    void init(const vk::raii::Instance& instance);

    vk::raii::PhysicalDevice& get() noexcept { return device; }
    const vk::raii::PhysicalDevice& get() const noexcept { return device; }

    operator vk::raii::PhysicalDevice&() { return device; }
    operator const vk::raii::PhysicalDevice&() const { return device; }

    private:
    vk::raii::PhysicalDevice device = nullptr;
};
