#pragma once
#include "../../definitions/graphics.hpp" // IWYU pragma: keep
#include "../PhysicalDevice/PhysicalDevice.hpp"
#include "../LogicalDevice/LogicalDevice.hpp"

struct UBO {
    glm::mat4 view;
    glm::mat4 proj;
};

struct UBOBuffer {
    private:
    vk::raii::Buffer       buffer = nullptr;
    vk::raii::DeviceMemory memory = nullptr;
    UBO*                   mapped = nullptr;

    static uint32_t findMemType(const PhysicalDevice& pd, uint32_t f, vk::MemoryPropertyFlags p);
    static void createBuffer(const LogicalDevice& ld, const PhysicalDevice& pd, vk::DeviceSize size, vk::BufferUsageFlags usage, vk::MemoryPropertyFlags properties, vk::raii::Buffer& buffer, vk::raii::DeviceMemory& bufferMemory);

    public:
    void init(const LogicalDevice& ld, const PhysicalDevice& pd);
    void update(const UBO& ubo) { memcpy(mapped, &ubo, sizeof(UBO)); }

    vk::DescriptorBufferInfo Info() const { return { *buffer, 0, sizeof(UBO) }; }
};
