#include "simulation.hpp"

#define TAU 6.28318530718

std::chrono::nanoseconds simulation::update_gpu(const float dt) noexcept {
    auto s = std::chrono::high_resolution_clock::now();
    return std::chrono::high_resolution_clock::now() - s;
}

void simulation::init_cluster() noexcept {
    std::default_random_engine gen(737274);

    std::normal_distribution<float>pos(0.0f, 0.5f);
    std::uniform_real_distribution<float>vel(0, 0.5f);
    std::normal_distribution<float>mass(0.005f, 0.05f);

    // do not parallelize, creates undeterministic results
    for (size_t i = 0; i < data_.bodies(); i++) {
        data_.posx()[i] = pos(gen);
        data_.posy()[i] = pos(gen);

        float temp = vel(gen) * TAU;
        data_.velx()[i] = cosf(temp);
        data_.vely()[i] = sinf(temp) * pos(gen);

        data_.mass()[i] = 0.001f + abs(mass(gen) * 0.5f);
    }
}

void simulation::init_spiral() noexcept {
    
}

#undef TAU
