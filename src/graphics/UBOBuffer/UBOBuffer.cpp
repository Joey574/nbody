#include "UBOBuffer.hpp"

void UBOBuffer::init(const LogicalDevice& ld, const PhysicalDevice& pd) {
    vk::DeviceSize size = sizeof(UBO);

    constexpr const vk::BufferUsageFlags usage = vk::BufferUsageFlagBits::eUniformBuffer;
    constexpr const vk::MemoryPropertyFlags properties = vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent;

    createBuffer(ld, pd, size, usage, properties, buffer, memory);

    mapped = static_cast<UBO*>(memory.mapMemory(0, size));
}

uint32_t UBOBuffer::findMemType(const PhysicalDevice& pd, uint32_t f, vk::MemoryPropertyFlags p) {
    auto memP = pd.Device().getMemoryProperties();
    
    for (uint32_t i = 0; i < memP.memoryTypeCount; i++) {
        if ((f & (1 << i)) && (memP.memoryTypes[i].propertyFlags & p) == p) {
            return i;
        }
    }

    throw std::runtime_error("failed to find a suitable memory type");
}

void UBOBuffer::createBuffer(const LogicalDevice& ld, const PhysicalDevice& pd, vk::DeviceSize size, vk::BufferUsageFlags usage, vk::MemoryPropertyFlags properties, vk::raii::Buffer& buffer, vk::raii::DeviceMemory& bufferMemory) {
    vk::BufferCreateInfo bufferInfo {
        .size = size,
        .usage = usage,
        .sharingMode = vk::SharingMode::eExclusive,
    };

    buffer = vk::raii::Buffer(ld, bufferInfo);
    
    auto memRequirements = buffer.getMemoryRequirements();
    vk::MemoryAllocateInfo allocInfo {
        .allocationSize = memRequirements.size,
        .memoryTypeIndex = findMemType(pd, memRequirements.memoryTypeBits, properties)
    };

    bufferMemory = vk::raii::DeviceMemory(ld, allocInfo);
    buffer.bindMemory(*bufferMemory, 0);
}
