#pragma once
#include "data/data.hpp"
#include "cli/cli.hpp"
#include "quadtree/quadtree.hpp"

#include "graphics/DirectSum/DirectSum.hpp"

#include <chrono>

struct simulation {
    public:
        enum class backend {
          CPU, CPU_BH,
          GPU, GPU_BH
        };

        simulation() = default;

        // parameterized constructor
        simulation(const cliargs& f) :
            backend_(f.cpu ? (f.bh ? backend::CPU_BH : backend::CPU) : (f.bh ? backend::GPU_BH : backend::GPU)),
            data_(f.config.New(f.cpu)),
            tree_(f.config.Theta())
            {}

        void InitGPU(const LogicalDevice& ld, const PhysicalDevice& pd);
        void RecordGPU(const vk::raii::CommandBuffer& cmd, float dt);

        std::chrono::nanoseconds Update(float ft);

        bool IsGPU() const noexcept { return backend_ == backend::GPU || backend_ == backend::GPU_BH; }

        backend GetBackend() const noexcept { return backend_; }

        data& GetData() { return data_; }
        const data& GetData() const { return data_; }

        size_t Points() const noexcept { return IsGPU() ? dsgpu_.bodies() : data_.Bodies(); }

        vk::Buffer GPUState() const { return dsgpu_.currentState(); }
        vk::Buffer GPUMasses() const { return dsgpu_.massBuffer(); }

        void Cleanup();

    private:
        data data_;
        backend backend_ = backend::CPU;

        quadtree tree_;
        DirectSum dsgpu_;

        std::chrono::nanoseconds UpdateCPU(const float ft) noexcept;
        std::chrono::nanoseconds UpdateCPUBH(const float ft) noexcept;

        void ComputeAcc() noexcept;
        void MovePoints(const float ft) noexcept;
        void SumAcc() noexcept;
}; // struct simulation
