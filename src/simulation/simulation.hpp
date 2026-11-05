#pragma once
#include "data/data.hpp"
#include "cli/cli.hpp"
#include "quadtree/quadtree.hpp"

#include "graphics/GPUSimulation/GPUSimulation.hpp"

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

        void initGPU(const LogicalDevice& ld, const PhysicalDevice& pd);
        void recordGPU(const vk::raii::CommandBuffer& cmd, float dt);

        std::chrono::nanoseconds update(float dt);

        bool isGPU() const noexcept { return backend_ == backend::GPU || backend_ == backend::GPU_BH; }

        backend get_backend() const noexcept { return backend_; }

        data& get_data() { return data_; }
        const data& get_data() const { return data_; }

        size_t points() const noexcept { return isGPU() ? gpu_.bodies() : data_.bodies(); }

        vk::Buffer gpuState() const { return gpu_.currentState(); }
        vk::Buffer gpuMasses() const { return gpu_.massBuffer(); }

        void cleanup();

    private:
        data data_;
        backend backend_ = backend::CPU;

        quadtree tree_;
        GPUSimulation gpu_;

        std::chrono::nanoseconds update_cpu(const float ft) noexcept;
        std::chrono::nanoseconds update_cpu_bh(const float ft) noexcept;

        void compute_acc() noexcept;
        void move_points(const float ft) noexcept;
        void sum_acc() noexcept;
}; // struct simulation
