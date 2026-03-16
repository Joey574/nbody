/* 
Author: Joey Soroka
Updated: 2/23/26
Purpose: Implements basic member functions for the simulation class
Comments: Currently contains the update_gpu method implementation, which doesn't do anything
*/

#include "simulation.hpp"
#include <random>

#define TAU 6.28318530718

void simulation::init_cluster(const ClusterConfig& conf, size_t seed) noexcept {
    std::default_random_engine urng(737274);

    std::normal_distribution<float>pos(0.0f, 15.0f);
    std::uniform_real_distribution<float>vel(0.2f, 0.5f);
    std::normal_distribution<float>mass(0.05f, 0.005f);

    // do not parallelize, creates undeterministic results
    for (size_t i = 0; i < data_.bodies(); i++) {
        data_.posx()[i] = pos(urng);
        data_.posy()[i] = pos(urng);

        float temp = vel(urng) * TAU;
        data_.velx()[i] = cosf(temp);
        data_.vely()[i] = sinf(temp) * pos(urng);

        data_.mass()[i] = 0.0025f + abs(mass(urng) * 0.5f);
    }
}

void simulation::init_spiral(const SpiralConfig& conf, size_t seed) noexcept { 
    std::default_random_engine gen(seed);

    std::normal_distribution<float> pos(conf.pos_mean, conf.pos_std);
    std::normal_distribution<float> mass(conf.mass_mean, conf.mass_std);

    float rx = conf.rx;
    float ry = conf.ry;
    float rot = conf.rot_start;
    float inc = conf.inc_start;

    size_t ellipses = conf.ellipses;
    size_t segments = conf.segments;
    size_t per_point = std::max<size_t>((data_.bodies() / (segments * ellipses))+1, 1);

    size_t b_idx = 0;
    for (size_t e = 0; e < ellipses && b_idx < data_.bodies(); e++) {
        for (size_t s = 0; s < segments && b_idx < data_.bodies(); s++) {
            float theta = TAU * s / (float)segments;

            float x = rx + cosf(theta);
            float y = ry + sinf(theta);

            float rot_x = x * cosf(rot) - y * sinf(rot);
            float rot_y = x * sinf(rot) + y * cosf(rot);

            for (size_t i = 0; i < per_point && b_idx < data_.bodies(); i++) {
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

        rx += inc * conf.rx_scale;
        ry += inc;

        rot += conf.rot_delta;
        inc += conf.inc_delta;
    }

    // sort based on distance
    data_.sort();

    // scale velocites
    for (size_t i = 0; i < data_.bodies(); i++) {
        float x = data_.posx()[i];
        float y = data_.posy()[i];
        float v = std::sqrtf(float(i) / std::sqrt(x*x+y*y));

        data_.velx()[i] *= v;
        data_.vely()[i] *= v;
    }
}

#undef TAU
