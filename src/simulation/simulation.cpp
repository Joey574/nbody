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
    std::default_random_engine gen(4285482);

    std::normal_distribution<float> pos(0.0f, 0.05f);
    std::normal_distribution<float> mass(0.005f, 0.05f);

    float rx = 0.5f;
    float ry = 0.5f;
    float rot = 0.0f;
    float inc = 0.075f;

    size_t ellipses = 25;
    size_t segments = 100;

    size_t per_point = data_.bodies() / (segments * ellipses);
    size_t b_idx = 0;

    for (size_t e = 0; e < ellipses; e++) {
        for (size_t s = 0; s < segments; s++) {
            float theta = TAU * s / (float)segments;

            float x = rx + cosf(theta);
            float y = ry + sinf(theta);

            float rot_x = x * cosf(rot) - y * sinf(rot);
            float rot_y = x * sinf(rot) + y * cosf(rot);

            for (size_t i = 0; i < per_point; i++) {
                data_.posx()[b_idx] = rot_x + pos(gen);
                data_.posy()[b_idx] = rot_y + pos(gen);
                data_.mass()[b_idx] = mass(gen);

                float dx = sinf(theta + rot);
                float dy = cosf(theta + rot);

                data_.velx()[b_idx] = -dx;
                data_.vely()[b_idx] = dy;

                b_idx++;
            }
        }

        rot += 0.35f;

        rx += inc * 1.2f;
        ry += inc;

        inc += 0.001f;
    }

    // TODO : sort and scale velocities
}

#undef TAU
