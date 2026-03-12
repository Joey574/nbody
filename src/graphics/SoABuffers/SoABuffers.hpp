#pragma once
#include "../../definitions/graphics.hpp" // IWYU pragma: keep

#include "../../data/data.hpp"
#include "../PhysicalDevice/PhysicalDevice.hpp"
#include "../LogicalDevice/LogicalDevice.hpp"

struct SoABuffers {
    private:
    size_t count;
    vk::raii::Buffer       xBuf = nullptr, yBuf = nullptr, rBuf = nullptr;
    vk::raii::DeviceMemory xMem = nullptr, yMem = nullptr, rMem = nullptr;
    float                  *x   = nullptr, *y   = nullptr, *r   = nullptr;

    static uint32_t findMemType(const PhysicalDevice& pd, uint32_t f, vk::MemoryPropertyFlags p);
    static void createBuffer(const LogicalDevice& ld, const PhysicalDevice& pd, vk::DeviceSize size, vk::BufferUsageFlags usage, vk::MemoryPropertyFlags properties, vk::raii::Buffer& buffer, vk::raii::DeviceMemory& bufferMemory);

    public:
    void init(const data& d, const LogicalDevice& ld, const PhysicalDevice& pd);
    void update(const data& d);

    vk::DescriptorBufferInfo xInfo() const;
    vk::DescriptorBufferInfo yInfo() const;
    vk::DescriptorBufferInfo rInfo() const;
};
