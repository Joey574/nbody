#pragma once
#include <malloc.h>
#include <immintrin.h>
#include <stdexcept>
#include <chrono>

#include "../data/data.hpp"
#include "../cli/cli.hpp"

struct simulation {
    public:

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
    simulation(const cliargs& f) : data_(f.config.New()) {
        if (f.cpu) {
            update = &simulation::update_cpu;
        } else {
            update = &simulation::update_gpu;
            throw std::runtime_error("not implemented");
        }
    }

    // deconstructor
    //~simulation() {}

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
    data data_;

    std::chrono::nanoseconds update_cpu_bh(const float ft) noexcept;

    std::chrono::nanoseconds update_cpu(const float ft) noexcept;
    void attract_points(const float ft) noexcept;
    void move_points(const float ft) noexcept;
    void sum_acc() noexcept;

    std::chrono::nanoseconds update_gpu(const float ft) noexcept;
}; // struct simulation
