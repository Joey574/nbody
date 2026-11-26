#pragma once
#include "graphics/PhysicalDevice/PhysicalDevice.hpp"
#include "graphics/LogicalDevice/LogicalDevice.hpp"

#include <stdexcept>

namespace vutil {
    static inline uint32_t findMemoryType(const PhysicalDevice& pd, uint32_t f, vk::MemoryPropertyFlags p) {
        auto memP = pd.Device().getMemoryProperties();

        for (uint32_t i = 0; i < memP.memoryTypeCount; i++) {
            if ((f & (1 << i)) && (memP.memoryTypes[i].propertyFlags & p) == p) {
                return i;
            }
        }

        throw std::runtime_error("failed to find a suitable memory type");
    }

    static inline void createBuffer(
        const LogicalDevice& ld,
        const PhysicalDevice& pd,
        vk::DeviceSize size,
        vk::BufferUsageFlags usage,
        vk::MemoryPropertyFlags properties,
        vk::raii::Buffer& buffer,
        vk::raii::DeviceMemory& memory)
    {
        if (size == 0) throw std::invalid_argument("cannot create zero-sized gpu buffer");

        vk::BufferCreateInfo bufferInfo {
            .size = size,
            .usage = usage,
            .sharingMode = vk::SharingMode::eExclusive,
        };

        buffer = vk::raii::Buffer(ld, bufferInfo);

        auto memRequirements = buffer.getMemoryRequirements();
        vk::MemoryAllocateInfo allocInfo {
            .allocationSize = memRequirements.size,
            .memoryTypeIndex = findMemoryType(pd, memRequirements.memoryTypeBits, properties)
        };

        memory = vk::raii::DeviceMemory(ld, allocInfo);
        buffer.bindMemory(*memory, 0);
    }

    static inline vk::raii::ShaderModule createShaderModule(
        const LogicalDevice& ld,
        const unsigned char* code,
        size_t size
    ) {
        if (size == 0 || size % sizeof(uint32_t) != 0) {
            throw std::runtime_error("invalid direct_sum.spv size");
        }

        vk::ShaderModuleCreateInfo info{
            .codeSize = size,
            .pCode = reinterpret_cast<const uint32_t*>(code)
        };
        return vk::raii::ShaderModule(ld.Device(), info);
    }

}; // vutil
