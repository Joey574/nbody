#include "SoABuffers.hpp"

void SoABuffers::init(const data& d, const LogicalDevice& ld, const PhysicalDevice& pd) {
    count = d.bodies();
    vk::DeviceSize size = d.bodies() * sizeof(float);

    constexpr const vk::BufferUsageFlags usage = vk::BufferUsageFlagBits::eStorageBuffer;
    constexpr const vk::MemoryPropertyFlags properties = vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent;

    createBuffer(ld, pd, size, usage, properties, xBuf, xMem);
    createBuffer(ld, pd, size, usage, properties, yBuf, yMem);
    createBuffer(ld, pd, size, usage, properties, rBuf, rMem);

    x = static_cast<float*>(xMem.mapMemory(0, size));
    y = static_cast<float*>(yMem.mapMemory(0, size));
    r = static_cast<float*>(rMem.mapMemory(0, size));
}

void SoABuffers::update(const data& d) {
    const size_t bytes = d.bodies() * sizeof(float);
    memcpy(x, d.posx(), bytes);
    memcpy(y, d.posy(), bytes);
    memcpy(r, d.mass(), bytes);
}

vk::DescriptorBufferInfo SoABuffers::xInfo() const {
    return { *xBuf, 0, count * sizeof(float)};
}

vk::DescriptorBufferInfo SoABuffers::yInfo() const {
    return { *yBuf, 0, count * sizeof(float)};
}

vk::DescriptorBufferInfo SoABuffers::rInfo() const {
    return { *rBuf, 0, count * sizeof(float)};
}

uint32_t SoABuffers::findMemType(const PhysicalDevice& pd, uint32_t f, vk::MemoryPropertyFlags p) {
    auto memP = pd.Device().getMemoryProperties();
    
    for (uint32_t i = 0; i < memP.memoryTypeCount; i++) {
        if ((f & (1 << i)) && (memP.memoryTypes[i].propertyFlags & p) == p) {
            return i;
        }
    }

    throw std::runtime_error("failed to find a suitable memory type");
}

void SoABuffers::createBuffer(const LogicalDevice& ld, const PhysicalDevice& pd, vk::DeviceSize size, vk::BufferUsageFlags usage, vk::MemoryPropertyFlags properties, vk::raii::Buffer& buffer, vk::raii::DeviceMemory& bufferMemory) {
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
