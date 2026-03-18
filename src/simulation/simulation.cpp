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

void simulation::init_uniform(const UniformConfig& conf, size_t seed) noexcept {
    std::default_random_engine gen(seed);
    std::uniform_real_distribution<float> angle(0.0f, 1.0f);

    const float irad = 25.0f;
    const float orad = sqrtf((float)data_.bodies()) * 5.0f;

    // centeral body
    data_.posx()[0] = 0.0f;
    data_.posy()[0] = 0.0f;
    data_.velx()[0] = 0.0f;
    data_.vely()[0] = 0.0f;
    data_.mass()[0] = 1e6f;

    for (size_t i = 1; i < data_.bodies(); i++) {
        float a = angle(gen) * TAU;
        float sina = sinf(a);
        float cosa = cosf(a);

        float t = irad / orad;
        float r = angle(gen) * (1.0f - t*t) + t*t;
        float scale = orad * sqrtf(r);

        data_.posx()[i] = cosa*scale;
        data_.posy()[i] = sina*scale;
        data_.velx()[i] = sina;
        data_.vely()[i] = -cosa;
        data_.mass()[i] = 1.0f;
    }

    // sort based on distance
    data_.sort();

    // scale velocites
    #pragma omp parallel for simd schedule(static)
    for (size_t i = 0; i < data_.bodies(); i++) {
        float x = data_.posx()[i];
        float y = data_.posy()[i];
        float v = sqrtf(float(i) / sqrtf(1e-12f+x*x+y*y));

        data_.velx()[i] *= v;
        data_.vely()[i] *= v;
    }
}

void simulation::init_voronoi(const VoronoiConfig& conf, size_t seed) noexcept {
    size_t clusters = 100;
    float rad = 20.0f;

    std::default_random_engine urng(seed);
    std::uniform_real_distribution<float> cluster_rad(0.3f, rad);
    std::uniform_real_distribution<float> cluster_angle(0.0f, 1.0f);
    std::uniform_real_distribution<float> cluster_weight(0.05f, 1.1f);
    std::uniform_real_distribution<float> cluster_spread(1.0f, 5.0f);

    std::normal_distribution<float> pos_x;
    std::normal_distribution<float> pos_y;

    std::vector<float> seedx(clusters);
    std::vector<float> seedy(clusters);
    std::vector<float> seedw(clusters);
    std::vector<float> seedp(clusters);

    seedx[0] = 0;
    seedy[0] = 0;
    seedw[0] = 4.0f;
    seedp[0] = rad*rad;

    float sumw = seedw[0];

    for (size_t c = 1; c < clusters; c++) {
        float r = cluster_rad(urng) * rad;
        float angle = cluster_angle(urng) * TAU;
        
        seedx[c] = r * cosf(angle);
        seedy[c] = r * sinf(angle);
        seedw[c] = cluster_weight(urng);
        seedp[c] = cluster_spread(urng) * rad;
        sumw += seedw[c];
    }

    auto weighted_pick = [&](){
        std::uniform_real_distribution<float> dist(0.0f, sumw);
        float v = dist(urng);

        for (size_t i = 0; i < seedw.size(); i++) {
            v -= seedw[i];
            if (v <= 0.0f) {
                return i;
            }
        }

        return seedw.size()-1;
    };

    data_.posx()[0] = 0.0f;
    data_.posy()[0] = 0.0f;
    data_.velx()[0] = 0.0f;
    data_.vely()[0] = 0.0f;
    data_.mass()[0] = 5000.0f;

    // initialize positions and velocities
    for (size_t i = 1; i < data_.bodies(); i++) {
        size_t sidx = weighted_pick();

        pos_x.param(std::normal_distribution<float>::param_type(0.0f, seedp[sidx]));
        pos_y.param(std::normal_distribution<float>::param_type(0.0f, seedp[sidx]));

        float x = seedx[sidx] + pos_x(urng);
        float y = seedy[sidx] + pos_y(urng);
        float d = 1e-12f + sqrtf(x*x+y*y);

        float sina = y/d;
        float cosa = x/d;
        data_.posx()[i] = x;
        data_.posy()[i] = y;
        data_.velx()[i] = sina;
        data_.vely()[i] = -cosa;
        data_.mass()[i] = 1.0f;
    }

    // sort based on distance
    data_.sort();

    // scale velocites
    for (size_t i = 0; i < data_.bodies(); i++) {
        float x = data_.posx()[i];
        float y = data_.posy()[i];
        float v = std::sqrtf(float(i) / (1e-12f + std::sqrt(x*x+y*y)));

        data_.velx()[i] *= v;
        data_.vely()[i] *= v;
    }
}

#undef TAU
