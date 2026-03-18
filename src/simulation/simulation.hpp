#pragma once
#include <malloc.h>
#include <immintrin.h>
#include <omp.h>
#include <chrono>

#include "../quadtree/quadtree.hpp"
#include "../data/data.hpp"
#include "../cli/cli.hpp"

struct simulation {
    public:
    enum init { CLUSTER, SPIRAL };

    // default constructor
    simulation() :
        data_(data()),
        update(nullptr) {}

    // move constructor
    simulation(simulation&& other) noexcept : 
        data_(std::move(other.data_)),
        update(other.update) {}

    // copy constructor
    simulation(const simulation& other) noexcept :
        data_(other.data_),
        update(other.update) {}

    // custom constructor
    simulation(const cliargs& f) :
        data_(f.config.Points(), f.cpu) {
            if (f.config.Type() == "cluster") {
                init_cluster(f.config.Cluster(), f.config.Seed());
            } else if (f.config.Type() == "spiral") {
                init_spiral(f.config.Spiral(), f.config.Seed());
            } else if (f.config.Type() == "uniform") {
                init_uniform(f.config.Uniform(), f.config.Seed());
            } else if (f.config.Type() == "voronoi") {
                init_voronoi(f.config.Voronoi(), f.config.Seed());
            } else {
                throw std::runtime_error("invalid initilization");
            }

            if (f.cpu) {
                update = &simulation::update_cpu;
            } else {
                update = &simulation::update_gpu;
                // TODO : Copy initial values into gpu buffers
            }
        }

    // deconstructor
    ~simulation() {}

    // move operator
    simulation& operator = (simulation&& other) noexcept {
        update = other.update;
        data_ = std::move(other.data_);

        other.update = nullptr;
        return *this;
    }

    // copy operator
    simulation& operator = (const simulation& other) noexcept {
        update = other.update;
        data_ = other.data_;
        return *this;
    }
    
    std::chrono::nanoseconds (simulation::*update)(const float) noexcept;
    
    data& get_data() { return data_; }
    const float* posx() const noexcept { return data_.posx(); }
    const float* posy() const noexcept { return data_.posy(); }

    size_t bodies() const noexcept { return data_.bodies(); }

    private:
    quadtree qt;
    data data_;

    std::chrono::nanoseconds update_cpu_bh(const float ft) noexcept;

    std::chrono::nanoseconds update_cpu(const float ft) noexcept;
    std::chrono::nanoseconds update_cpu_simd(const float ft) noexcept;
    std::chrono::nanoseconds update_cpu_fallback(const float ft) noexcept;

    std::chrono::nanoseconds update_gpu(const float ft) noexcept;

    void init_cluster(const ClusterConfig& conf, size_t seed) noexcept;
    void init_spiral(const SpiralConfig& conf, size_t seed) noexcept;
    void init_uniform(const UniformConfig& conf, size_t seed) noexcept;
    void init_voronoi(const VoronoiConfig& conf, size_t seed) noexcept;
}; // struct simulation
