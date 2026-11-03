#include "galaxy.hpp"
#include "util/util.hpp"

#include <random>
#include <assert.h>
#define TAU 6.28318530718

void Galaxy::SetInitialVelocity(data& d) const noexcept {
    assert(this->points == d.bodies());

    // alias
    const float* __restrict px = d.posx();
    const float* __restrict py = d.posy();
    float* __restrict vx = d.velx();
    float* __restrict vy = d.vely();

    if (this->center_mass > util::epsl) {
        vx[0] = x_vel;
        vy[0] = y_vel;
    }

    #pragma omp parallel for simd schedule(static)
    for (size_t i = 0; i < this->points; i++) {
        if (this->center_mass > util::epsl && i == 0) continue; // skip center mass

        const float x = px[i] - this->x_pos;
        const float y = py[i] - this->y_pos;

        const float angle = atan2f(y, x);
        const float dist = sqrtf(x*x + y*y);

        const float sin = sinf(angle);
        const float cos = cosf(angle);

        const float v = this->center_mass > util::epsl ? (sqrtf(this->center_mass / dist)) : 1.0f;

        vx[i] = sin*v + this->x_vel;
        vy[i] = -cos*v + this->y_vel;
    }
}

data Galaxy::NewUniform(bool init_acc) const {
    data d(this->points, init_acc);

    const float inner_rad = std::any_cast<float>(conf.at(Options::UN_INNER_RADIUS));
    const float outer_rad = std::any_cast<float>(conf.at(Options::UN_OUTER_RADIUS));

    float* __restrict px = d.posx();
    float* __restrict py = d.posy();
    float* __restrict ma = d.mass();

    // TODO
    if (this->conf.contains(Options::UN_DISTRIBUTION)) {}
    //const std::string distribution = std::any_cast<std::string>(this->conf.at(Options::UN_DISTRIBUTION));

    std::uniform_real_distribution<float> angle(0.0f, 1.0f);
    std::normal_distribution<float> pos(0.0f, 0.5f);

    size_t i = 0;
    if (this->center_mass > util::epsl) {
        px[0] = this->x_pos;
        py[0] = this->y_pos;
        ma[0] = this->center_mass;
        i++;
    }

    const float t = inner_rad / outer_rad;
    const float t2 = t*t;

    for (; i < this->points; i++) {
        const float a = angle(gen) * TAU;

        const float unit_dist = sqrtf(abs(pos(gen)) * (1.0f - t2) + t2);
        const float scale = outer_rad * unit_dist;

        const float sin_a = sinf(a);
        const float cos_a = cosf(a);

        const float vel = this->center_mass > 0.0f ?
            (sqrtf(this->center_mass / scale)) :
            1.0f;

        px[i] = (cos_a * scale) + this->x_pos;
        py[i] = (sin_a * scale) + this->y_pos;
        ma[i] = 1.0f;
    }

    this->SetInitialVelocity(d);
    return d;
}

data Galaxy::NewSpiral(bool init_acc) const {
    data d(this->points, init_acc);

    float rx = conf.contains(Options::SP_X_RADIUS) ? std::any_cast<float>(conf.at(Options::SP_X_RADIUS)) : 0.0f;
    float ry = conf.contains(Options::SP_Y_RADIUS) ? std::any_cast<float>(conf.at(Options::SP_Y_RADIUS)) : 0.0f;
    float rot = conf.contains(Options::SP_ROT_START) ? std::any_cast<float>(conf.at(Options::SP_ROT_START)) : 0.0f;
    float inc = conf.contains(Options::SP_INC_START) ? std::any_cast<float>(conf.at(Options::SP_INC_START)) : 0.0f;

    float rx_scale = conf.contains(Options::SP_X_RADIUS_SCALE) ? std::any_cast<float>(conf.at(Options::SP_X_RADIUS_SCALE)) : 0.0f;
    float rot_delta = conf.contains(Options::SP_ROT_DELTA) ? std::any_cast<float>(conf.at(Options::SP_ROT_DELTA)) : 0.0f;
    float inc_delta = conf.contains(Options::SP_INC_DELTA) ? std::any_cast<float>(conf.at(Options::SP_INC_DELTA)) : 0.0f;

    float ellipses_width = conf.contains(Options::SP_ELLIPSES_WIDTH) ? std::any_cast<float>(conf.at(Options::SP_ELLIPSES_WIDTH)) : 0;

    size_t ellipses = conf.contains(Options::SP_ELLIPSES) ? std::any_cast<size_t>(conf.at(Options::SP_ELLIPSES)) : 0;
    size_t segments = conf.contains(Options::SP_SEGMENTS) ? std::any_cast<size_t>(conf.at(Options::SP_SEGMENTS)) : 0;
    size_t per_point = std::max<size_t>(this->points / (segments*ellipses) + 1, 1);

    std::normal_distribution<float> pos(0.0f, ellipses_width);

    size_t b_idx = 0;
    if (center_mass > util::epsl) {
        d.posx()[0] = this->x_pos;
        d.posy()[0] = this->y_pos;
        d.velx()[0] = this->x_vel;
        d.vely()[0] = this->y_vel;
        d.mass()[0] = this->center_mass;
        b_idx++;
    }

    for (size_t e = 0; e < ellipses; e++) {
        for (size_t s = 0; s < segments; s++) {
            float theta = TAU * s / (float)segments;

            float x = rx * cosf(theta);
            float y = ry * sinf(theta);

            float rotx = x * cosf(rot) - y * sinf(rot);
            float roty = y * sinf(rot) + y * cosf(rot);

            for (size_t i = 0; i < per_point && b_idx < this->points; i++, b_idx++) {
                d.posx()[b_idx] = rotx + pos(gen);
                d.posy()[b_idx] = roty + pos(gen);
                d.mass()[b_idx] = b_idx == 1;
            }
        }

        rx += inc * rx_scale;
        ry += inc;

        rot += rot_delta;
        inc += inc_delta;
    }

    this->SetInitialVelocity(d);
    return d;
}
