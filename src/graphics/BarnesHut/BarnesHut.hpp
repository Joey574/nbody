#pragma once

#include "data/data.hpp"
#include "graphics/LogicalDevice/LogicalDevice.hpp"
#include "graphics/PhysicalDevice/PhysicalDevice.hpp"

#include <cstdint>

struct BarnesHut {
    public:
        static constexpr uint32_t BLOCK_SIZE = 128;

        struct Params {
            uint32_t samples;
            float ft;
        };

        BarnesHut() = default;
        BarnesHut(const BarnesHut&) = delete;
        BarnesHut& operator=(const BarnesHut&) = delete;


        ~BarnesHut() { cleanup(); }

        void init(const data& d, const LogicalDevice& ld, const PhysicalDevice& pd);
        void record(const vk::raii::CommandBuffer& cmd, float ft);
        void cleanup();

    private:
        uint32_t count = 0;
        uint32_t currentIndex = 0;
};
