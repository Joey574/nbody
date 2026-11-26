#pragma once
#include "definitions/graphics.hpp" // IWYU pragma: keep

#include "graphics/PhysicalDevice/PhysicalDevice.hpp"
#include "graphics/LogicalDevice/LogicalDevice.hpp"

struct FrameCapture {
    public:
        void init(const LogicalDevice& ld, const PhysicalDevice& pd, vk::Extent2D extent, vk::Format format, size_t slots);
        void cleanup();

        void record(const vk::raii::CommandBuffer& cmd, vk::Image image, size_t slot);

        const void* take(size_t slot);

        bool matches(vk::Extent2D e) const noexcept { return e == extent_; }
        uint64_t captured() const noexcept { return captured_; }

        static const char* ffmpegPixelFormat(vk::Format format) noexcept;

    private:
        struct Slot {
            vk::raii::Buffer buffer = nullptr;
            vk::raii::DeviceMemory memory = nullptr;
            void* mapped = nullptr;
            bool coherent = true;
            bool pending = false;
        };

        const vk::raii::Device* device_ = nullptr;
        std::vector<Slot> slots_;
        vk::Extent2D extent_{};
        uint64_t captured_ = 0;
};
