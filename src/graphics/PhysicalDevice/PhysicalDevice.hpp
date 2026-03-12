#pragma once
#include "../../definitions/graphics.hpp" // IWYU pragma: keep

struct PhysicalDevice {
    public:
    void init(const vk::raii::Instance& instance);

    inline auto& Device() noexcept { return device; }
    inline const auto& Device() const noexcept { return device; }

    operator vk::raii::PhysicalDevice&() { return device; }
    operator const vk::raii::PhysicalDevice&() const { return device; }

    private:
    vk::raii::PhysicalDevice device = nullptr;
};
