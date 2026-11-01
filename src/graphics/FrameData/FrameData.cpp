#include "FrameData.hpp"
#include <stdexcept>

void FrameData::init(const data& d, const LogicalDevice& ld, const PhysicalDevice& pd) {
    cleanup();
    count = d.bodies();
    if (!count) throw std::invalid_argument("cannot initialize 0 particle buffer");

    const vk::DeviceSize stateBytes = count * sizeof(ParticleState);
    const vk::DeviceSize massBytes = count * sizeof(float);

    const auto limits = pd.Device().getProperties().limits;
    if (stateBytes > limits.maxStorageBufferRange ||
        massBytes > limits.maxStorageBufferRange) {
        throw std::length_error("CPU particle buffers exceed maxStorageBufferRange");
    }

    constexpr vk::BufferUsageFlags usage = vk::BufferUsageFlagBits::eStorageBuffer;
    constexpr vk::MemoryPropertyFlags properties =
        vk::MemoryPropertyFlagBits::eHostVisible |
        vk::MemoryPropertyFlagBits::eHostCoherent;

    createBuffer(ld, pd, stateBytes, usage, properties, stateBuffer, stateMemory);
    createBuffer(ld, pd, massBytes, usage, properties, massBuffer, massMemory);

    mappedState = static_cast<ParticleState*>(stateMemory.mapMemory(0, stateBytes));
    mappedMasses = static_cast<float*>(massMemory.mapMemory(0, massBytes));
    update(d);
}

void FrameData::update(const data& d) {
    if (d.bodies() != count) {
        throw std::invalid_argument("particle count changed after renderer buffer initialization");
    }

    for (size_t i = 0; i < count; ++i) {
        mappedState[i] = {
            d.posx()[i],
            d.posy()[i],
            d.velx()[i],
            d.vely()[i]
        };
        mappedMasses[i] = d.mass()[i];
    }
}

void FrameData::cleanup() {
    if (mappedState) {
        stateMemory.unmapMemory();
        mappedState = nullptr;
    }

    if (mappedMasses) {
        massMemory.unmapMemory();
        mappedMasses = nullptr;
    }

    stateBuffer = nullptr;
    massBuffer = nullptr;
    stateMemory = nullptr;
    massMemory = nullptr;
    count = 0;
}

vk::DescriptorBufferInfo FrameData::stateInfo() const {
    return {*stateBuffer, 0, count * sizeof(ParticleState)};
}

vk::DescriptorBufferInfo FrameData::massInfo() const {
    return {*massBuffer, 0, count * sizeof(float)};
}

uint32_t FrameData::findMemType(
    const PhysicalDevice& pd,
    uint32_t memoryTypeBits,
    vk::MemoryPropertyFlags properties
) {
    const auto memoryProperties = pd.Device().getMemoryProperties();
    for (uint32_t i = 0; i < memoryProperties.memoryTypeCount; ++i) {
        const bool supported = (memoryTypeBits & (1u << i)) != 0;
        const bool hasProperties =
            (memoryProperties.memoryTypes[i].propertyFlags & properties) == properties;
        if (supported && hasProperties) {
            return i;
        }
    }
    throw std::runtime_error("failed to find a suitable Vulkan memory type");
}

void FrameData::createBuffer(
    const LogicalDevice& ld,
    const PhysicalDevice& pd,
    vk::DeviceSize size,
    vk::BufferUsageFlags usage,
    vk::MemoryPropertyFlags properties,
    vk::raii::Buffer& buffer,
    vk::raii::DeviceMemory& bufferMemory
) {
    const vk::BufferCreateInfo bufferInfo{
        .size = size,
        .usage = usage,
        .sharingMode = vk::SharingMode::eExclusive
    };
    buffer = vk::raii::Buffer(ld.Device(), bufferInfo);

    const auto requirements = buffer.getMemoryRequirements();
    const vk::MemoryAllocateInfo allocation{
        .allocationSize = requirements.size,
        .memoryTypeIndex = findMemType(pd, requirements.memoryTypeBits, properties)
    };
    bufferMemory = vk::raii::DeviceMemory(ld.Device(), allocation);
    buffer.bindMemory(*bufferMemory, 0);
}
