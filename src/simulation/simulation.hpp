#pragma once
#include <malloc.h>
#include <cmath>
#include <immintrin.h>
#include <chrono>
#include <random>
#include <omp.h>

#include "../util/util.hpp"
#include "../definitions/definitions.hpp"

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
        data_(f.bodies, f.cpu) {
            if (f.cluster) {
                init_cluster();
            } else if (f.spiral) {
                init_spiral();
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
    void renderable(CircleSSBO& ssbo) const noexcept;

    private:
    data data_;

    std::chrono::nanoseconds update_cpu(const float ft) noexcept;
    std::chrono::nanoseconds update_cpu_fallback(const float ft) noexcept;
    template <typename simd_policy> std::chrono::nanoseconds update_cpu_simd(const float ft) noexcept;

    std::chrono::nanoseconds update_gpu(const float ft) noexcept;

    void init_cluster() noexcept;
    void init_spiral() noexcept;
}; // struct simulation
