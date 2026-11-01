#pragma once
#include "../../definitions/graphics.hpp"

#include "../../data/data.hpp"
#include "../PhysicalDevice/PhysicalDevice.hpp"
#include "../LogicalDevice/LogicalDevice.hpp"
#include "../GPUSimulation/ParticleState.hpp"

struct FrameData {
    public:
        FrameData() = default;
        FrameData(const FrameData&) = delete;
        FrameData& operator=(const FrameData&) = delete;
        ~FrameData() { cleanup(); }

        void init(const data& d, const LogicalDevice& ld, const PhysicalDevice& pd);
        void update(const data& d);
        void cleanup();

        vk::DescriptorBufferInfo stateInfo() const;
        vk::DescriptorBufferInfo massInfo() const;

    private:
        size_t count;
        vk::raii::Buffer stateBuffer = nullptr;
        vk::raii::DeviceMemory stateMemory = nullptr;
        vk::raii::Buffer massBuffer = nullptr;
        vk::raii::DeviceMemory massMemory = nullptr;
        ParticleState* mappedState = nullptr;
        float* mappedMasses = nullptr;


        vk::raii::Buffer       xBuf = nullptr, yBuf = nullptr, rBuf = nullptr;
        vk::raii::DeviceMemory xMem = nullptr, yMem = nullptr, rMem = nullptr;
        float                  *x   = nullptr, *y   = nullptr, *r   = nullptr;

        static uint32_t findMemType(const PhysicalDevice& pd, uint32_t f, vk::MemoryPropertyFlags p);
        static void createBuffer(const LogicalDevice& ld, const PhysicalDevice& pd, vk::DeviceSize size, vk::BufferUsageFlags usage, vk::MemoryPropertyFlags properties, vk::raii::Buffer& buffer, vk::raii::DeviceMemory& bufferMemory);
};
