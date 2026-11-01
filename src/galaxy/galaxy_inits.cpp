#include "galaxy.hpp"
#include <random>
#define TAU 6.28318530718

data Galaxy::NewUniform(bool init_acc) const {
    // for now we're just gonna hard code a uniform generation
    data d(this->points, init_acc);

    const float inner_rad = std::any_cast<float>(conf.at(Options::UN_INNER_RADIUS));
    const float outer_rad = std::any_cast<float>(conf.at(Options::UN_OUTER_RADIUS));

    // TODO
    if (this->conf.contains(Options::UN_DISTRIBUTION)) {}
    //const std::string distribution = std::any_cast<std::string>(this->conf.at(Options::UN_DISTRIBUTION));

    std::uniform_real_distribution<float> angle(0.0f, 1.0f);
    std::normal_distribution<float> pos(0.0f, 0.5f);

    size_t i = 0;
    if (this->center_mass > 0.0f) {
        d.posx()[0] = this->x_pos;
        d.posy()[0] = this->y_pos;
        d.velx()[0] = this->x_vel;
        d.vely()[0] = this->y_vel;
        d.mass()[0] = this->center_mass;
        i++;
    }

    const float t = inner_rad / outer_rad;
    const float t2 = t*t;

    for (; i < this->points; i++) {
        const float a = angle(gen) * TAU;

        const float r = abs(pos(gen)) * (1.0f - t2) + t2;
        const float scale = outer_rad * sqrtf(r);

        const float sin_a = sinf(a);
        const float cos_a = cosf(a);

        const float vel = this->center_mass > 0.0f ?
            (sqrtf(this->center_mass / scale)) :
            1.0f;

        d.posx()[i] = (cos_a * scale) + this->x_pos;
        d.posy()[i] = (sin_a * scale) + this->y_pos;
        d.velx()[i] = (sin_a * vel)   + this->x_vel;
        d.vely()[i] = (-cos_a * vel)  + this->y_vel;
        d.mass()[i] = 1.0f;
    }

    return d;
}

data Galaxy::NewSpiral(bool init_acc) const {
    data d(this->points, init_acc);

    std::normal_distribution<float> pos(0.0f, 1.0f);

    float rx = conf.contains(Options::SP_X_RADIUS) ? std::any_cast<float>(conf.at(Options::SP_X_RADIUS)) : 0.0f;
    float ry = conf.contains(Options::SP_Y_RADIUS) ? std::any_cast<float>(conf.at(Options::SP_Y_RADIUS)) : 0.0f;
    float rot = conf.contains(Options::SP_ROT_START) ? std::any_cast<float>(conf.at(Options::SP_ROT_START)) : 0.0f;
    float inc = conf.contains(Options::SP_INC_START) ? std::any_cast<float>(conf.at(Options::SP_INC_START)) : 0.0f;

    float rx_scale = conf.contains(Options::SP_X_RADIUS_SCALE) ? std::any_cast<float>(conf.at(Options::SP_X_RADIUS_SCALE)) : 0.0f;
    float rot_delta = conf.contains(Options::SP_ROT_DELTA) ? std::any_cast<float>(conf.at(Options::SP_ROT_DELTA)) : 0.0f;
    float inc_delta = conf.contains(Options::SP_INC_DELTA) ? std::any_cast<float>(conf.at(Options::SP_INC_DELTA)) : 0.0f;

    size_t ellipses = conf.contains(Options::SP_ELLIPSES) ? std::any_cast<size_t>(conf.at(Options::SP_ELLIPSES)) : 0;
    size_t segments = conf.contains(Options::SP_ELLIPSES) ? std::any_cast<size_t>(conf.at(Options::SP_ELLIPSES)) : 0;
    size_t per_point = std::max<size_t>(this->points / (segments*ellipses) + 1, 1);

    d.posx()[0] = this->x_pos;
    d.posy()[0] = this->y_pos;
    d.velx()[0] = this->x_vel;
    d.vely()[0] = this->y_vel;
    d.mass()[0] = this->center_mass;

    size_t b_idx = 1;
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

                float dx = sinf(theta + rot);
                float dy = cosf(theta + rot);

                d.velx()[b_idx] = -dx;
                d.vely()[b_idx] = dy;
            }
        }

        rx += inc * rx_scale;
        ry += inc;

        rot += rot_delta;
        inc += inc_delta;
    }

    return d;
}
