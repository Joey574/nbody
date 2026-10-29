#include "galaxy.hpp"
#include <algorithm>
#include <any>
#include <stdexcept>
#include <random>
#include <yaml-cpp/node/node.h>

#define TAU 6.28318530718

#define TYPE "type"
#define POINTS "points"
#define SEED "seed"
#define X_POS "x_pos"
#define Y_POS "y_pos"
#define X_VEL "x_vel"
#define Y_VEL "y_vel"
#define CENTER_MASS "center_mass"

#define Y_UN_INNER_RADIUS "inner_radius"
#define Y_UN_OUTER_RADIUS "outer_radius"
#define Y_UN_DISTRIBUTION "distribution"

data Galaxy::New() const {
    // for now we're just gonna hard code a uniform generation
    data d(this->points, true);

    const float inner_rad = std::any_cast<float>(this->conf.at(Options::UN_INNER_RADIUS));
    const float outer_rad = std::any_cast<float>(this->conf.at(Options::UN_OUTER_RADIUS));

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

void Galaxy::Load(const YAML::Node yaml) {
    std::string typeName = yaml[TYPE].as<std::string>();
    std::transform(typeName.begin(), typeName.end(), typeName.begin(), ::tolower);

    if (typeName == "spiral") {
        type = Type::Spiral;
    } else if (typeName == "cluster") {
        type = Type::Cluster;
    } else if (typeName == "uniform") {
        type = Type::Uniform;

        conf[Options::UN_INNER_RADIUS] = yaml[Y_UN_INNER_RADIUS].as<float>();
        conf[Options::UN_OUTER_RADIUS] = yaml[Y_UN_OUTER_RADIUS].as<float>();
    } else if (typeName == "voronoi") {
        type = Type::Voronoi;
    } else {
        throw std::invalid_argument(typeName + " not recognized as an init method");
    }

    this->points = yaml[POINTS].as<size_t>();

    std::default_random_engine urng(std::random_device{}());
    std::uniform_int_distribution<size_t> tmp(0, std::numeric_limits<size_t>::max());

    this->seed = yaml[SEED].as<uint64_t>(tmp(urng));
    this->gen = std::default_random_engine(seed);

    this->x_pos = yaml[X_POS].as<float>(0.0f);
    this->y_pos = yaml[Y_POS].as<float>(0.0f);

    this->x_vel = yaml[X_VEL].as<float>(0.0f);
    this->y_vel = yaml[Y_VEL].as<float>(0.0f);

    this->center_mass = yaml[CENTER_MASS].as<float>(-1.0f);
}

uint64_t Galaxy::Seed() const noexcept {
    return this->seed;
}

size_t Galaxy::Points() const noexcept {
    return this->points;
}

Galaxy::Type Galaxy::GalaxyType() const noexcept {
    return this->type;
}

std::string Galaxy::GalaxyString() const noexcept {
    switch (this->type) {
        case Type::None:
            return "None";
        case Type::Spiral:
            return "Spiral";
        case Type::Cluster:
            return "Cluster";
        case Type::Uniform:
            return "Uniform";
        case Type::Voronoi:
            return "Voronoi";
        default:
            return "Unknown";
    }
}

#undef TAU
#undef TYPE
#undef POINTS
#undef SEED
#undef X_POS
#undef Y_POS
#undef X_VEL
#undef Y_VEL
#undef CENTER_MASS
#undef Y_UN_INNER_RADIUS
#undef Y_UN_OUTER_RADIUS
