#include "FrameCapture.hpp"
#include "graphics/vutil.hpp"

#include <cstdlib>
#include <stdexcept>

void FrameCapture::init(const LogicalDevice& ld, const PhysicalDevice& pd, vk::Extent2D extent, vk::Format format, size_t slots) {
    using p = vk::MemoryPropertyFlagBits;

    cleanup();

    if (!ffmpegPixelFormat(format)) throw std::runtime_error("recording needs an 8-bit rgba/brga swapchain format, got " + vk::to_string(format));

    device_ = &ld.Device();
    extent_ = extent;

    const auto size = static_cast<vk::DeviceSize>(extent.width) * extent.height * 4;
    const auto memProps = pd.Device().getMemoryProperties();

    slots_.resize(slots);
    for (auto& s : slots_) {
        s.buffer = vk::raii::Buffer(ld, vk::BufferCreateInfo {
            .size = size,
            .usage = vk::BufferUsageFlagBits::eTransferDst,
            .sharingMode = vk::SharingMode::eExclusive
        });

        const auto req = s.buffer.getMemoryRequirements();
        uint32_t type = vutil::findMemoryType(memProps, req.memoryTypeBits, p::eHostVisible | p::eHostCached | p::eHostCoherent);
        if (type == ~0u) type = vutil::findMemoryType(memProps, req.memoryTypeBits, p::eHostVisible | p::eHostCached);
        if (type == ~0u) type = vutil::findMemoryType(memProps, req.memoryTypeBits, p::eHostVisible | p::eHostCoherent);
        if (type == ~0u) throw std::runtime_error("no host visible memory for frame readback");

        s.coherent = bool(memProps.memoryTypes[type].propertyFlags & p::eHostCoherent);
        s.memory = vk::raii::DeviceMemory(ld, vk::MemoryAllocateInfo {
            .allocationSize = req.size,
            .memoryTypeIndex = type
        });

        s.buffer.bindMemory(*s.memory, 0);
        s.mapped = s.memory.mapMemory(0, size);
        s.pending = false;
    }

    captured_ = 0;
}

void FrameCapture::cleanup() {
    for (auto& s : slots_) {
        if (s.mapped) {
            s.memory.unmapMemory();
            s.mapped = nullptr;
        }

        s.buffer = nullptr;
        s.memory = nullptr;
    }

    slots_.clear();
    device_ = nullptr;
}

void FrameCapture::record(const vk::raii::CommandBuffer& cmd, vk::Image image, size_t slot) {
    Slot& s = slots_[slot];

    const vk::BufferImageCopy region {
        .bufferOffset = 0,
        .bufferRowLength = 0,
        .bufferImageHeight = 0,
        .imageSubresource = { vk::ImageAspectFlagBits::eColor, 0, 0, 1 },
        .imageOffset = { 0, 0, 0 },
        .imageExtent = { extent_.width, extent_.height, 1 }
    };

    const vk::BufferMemoryBarrier2 toHost {
        .srcStageMask = vk::PipelineStageFlagBits2::eCopy,
        .srcAccessMask = vk::AccessFlagBits2::eTransferWrite,
        .dstStageMask = vk::PipelineStageFlagBits2::eHost,
        .dstAccessMask = vk::AccessFlagBits2::eHostRead,
        .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .buffer = *s.buffer,
        .offset = 0,
        .size = VK_WHOLE_SIZE
    };

    cmd.copyImageToBuffer(image, vk::ImageLayout::eTransferSrcOptimal, *s.buffer, region);

    const vk::DependencyInfo dependency {
        .bufferMemoryBarrierCount = 1,
        .pBufferMemoryBarriers = &toHost
    };

    cmd.pipelineBarrier2(dependency);

    s.pending = true;
    captured_++;
}

const void* FrameCapture::take(size_t slot) {
    if (slot >= slots_.size()) return nullptr;
    Slot& s = slots_[slot];

    if (!s.pending) return nullptr;
    s.pending = false;

    if (!s.coherent) {
        const vk::MappedMemoryRange range {
            .memory = *s.memory,
            .offset = 0,
            .size = VK_WHOLE_SIZE
        };

        device_->invalidateMappedMemoryRanges(range);
    }

    return s.mapped;
}

const char* FrameCapture::ffmpegPixelFormat(vk::Format format) noexcept {
    switch (format) {
        case vk::Format::eB8G8R8A8Srgb:
        case vk::Format::eB8G8R8A8Unorm:
            return "bgr0";
        case vk::Format::eR8G8B8A8Srgb:;
        case vk::Format::eR8G8B8A8Unorm:
            return "rgb0";
        default:
            return nullptr;
    }
}
