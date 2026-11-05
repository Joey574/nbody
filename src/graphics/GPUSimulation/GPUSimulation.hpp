#pragma once
#include "data/data.hpp"

#include "graphics/LogicalDevice/LogicalDevice.hpp"
#include "graphics/PhysicalDevice/PhysicalDevice.hpp"

#include <cstdint>

struct GPUSimulation {
    public:
        static constexpr uint32_t BLOCK_SIZE = 128;

        struct Params {
            uint32_t samples;
            float dt;
        };

        GPUSimulation() = default;
        GPUSimulation(const GPUSimulation&) = delete;
        GPUSimulation& operator=(const GPUSimulation&) = delete;

        GPUSimulation(GPUSimulation&& other) noexcept {
            *this = std::move(other);
        }
        GPUSimulation& operator=(GPUSimulation&& other) noexcept {
            if (this == &other) return *this;
            cleanup();

            count = std::exchange(other.count, 0);
            currentIndex = std::exchange(other.currentIndex, 0);

            stateBuffers = std::move(other.stateBuffers);
            stateMemories = std::move(other.stateMemories);
            massesBuffer = std::move(other.massesBuffer);
            massesMemory = std::move(other.massesMemory);
            descriptorLayout = std::move(other.descriptorLayout);
            descriptorPool = std::move(other.descriptorPool);
            descriptorSets = std::move(other.descriptorSets);
            pipelineLayout = std::move(other.pipelineLayout);
            pipeline = std::move(other.pipeline);
            return *this;
        }

        ~GPUSimulation() { cleanup(); }

        void init(const data& d, const LogicalDevice& ld, const PhysicalDevice& pd);
        void record(const vk::raii::CommandBuffer& cmd, float dt);
        void cleanup();

        vk::Buffer currentState() const;
        vk::Buffer massBuffer() const;
        uint32_t bodies() const noexcept { return count; };

    private:
        uint32_t count = 0;
        uint32_t currentIndex = 0;

        std::array<vk::raii::Buffer, 2> stateBuffers{nullptr, nullptr};
        std::array<vk::raii::DeviceMemory, 2> stateMemories{nullptr, nullptr};
        vk::raii::Buffer massesBuffer = nullptr;
        vk::raii::DeviceMemory massesMemory = nullptr;

        vk::raii::DescriptorSetLayout descriptorLayout = nullptr;
        vk::raii::DescriptorPool descriptorPool = nullptr;
        vk::raii::DescriptorSets descriptorSets = nullptr;

        vk::raii::PipelineLayout pipelineLayout = nullptr;
        vk::raii::Pipeline pipeline = nullptr;

        static uint32_t findMemoryType(
            const PhysicalDevice& pd,
            uint32_t memoryTypeBits,
            vk::MemoryPropertyFlags properties
        );

        static void createBuffer(
            const LogicalDevice& ld,
            const PhysicalDevice& pd,
            vk::DeviceSize size,
            vk::BufferUsageFlags usage,
            vk::MemoryPropertyFlags properties,
            vk::raii::Buffer& buffer,
            vk::raii::DeviceMemory& memory
        );
};
