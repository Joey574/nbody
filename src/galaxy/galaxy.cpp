#include "galaxy.hpp"
#include "data/data.hpp"
#include "util/util.hpp"

#include <algorithm>
#include <cmath>
#include <stdexcept>
#include <random>
#include <assert.h>
#include <yaml-cpp/node/node.h>

#define Y_TYPE "type"
#define Y_POINTS "points"
#define Y_SEED "seed"
#define Y_X_POS "x_pos"
#define Y_Y_POS "y_pos"
#define Y_X_VEL "x_vel"
#define Y_Y_VEL "y_vel"
#define Y_CENTER_MASS "center_mass"

// uniform cluster configuration
#define Y_UN_INNER_RADIUS "inner_radius"
#define Y_UN_OUTER_RADIUS "outer_radius"
#define Y_UN_DISTRIBUTION "distribution"

// spiral clustre configuration
#define Y_SP_X_RADIUS "x_radius"
#define Y_SP_Y_RADIUS "y_radius"
#define Y_SP_ROT_START "rot_start"
#define Y_SP_INC_START "inc_start"
#define Y_SP_ELLIPSES "ellipses"
#define Y_SP_ELLIPSES_WIDTH "ellipses_width"
#define Y_SP_SEGMENTS "segments"
#define Y_SP_X_RADIUS_SCALE "x_radius_scale"
#define Y_SP_ROT_DELTA "rot_delta"
#define Y_SP_INC_DELTA "inc_delta"


data Galaxy::New(bool init_acc) const {
    switch (this->type) {
        case Type::Uniform:
            return this->NewUniform(init_acc);
        case Type::Spiral:
            return this->NewSpiral(init_acc);
        default:
            throw std::invalid_argument("invalid galaxy type");
    }
}

void Galaxy::Load(const YAML::Node yaml) {
    std::string typeName = yaml[Y_TYPE].as<std::string>();
    std::transform(typeName.begin(), typeName.end(), typeName.begin(), ::tolower);

    if (typeName == "spiral") {
        type = Type::Spiral;
        conf[Options::SP_X_RADIUS] = yaml[Y_SP_X_RADIUS].as<float>(0.0f);
        conf[Options::SP_Y_RADIUS] = yaml[Y_SP_Y_RADIUS].as<float>(0.0f);
        conf[Options::SP_ROT_START] = yaml[Y_SP_ROT_START].as<float>(0.0f);
        conf[Options::SP_INC_START] = yaml[Y_SP_INC_START].as<float>(0.0f);
        conf[Options::SP_X_RADIUS_SCALE] = yaml[Y_SP_X_RADIUS_SCALE].as<float>(0.0f);
        conf[Options::SP_ROT_DELTA] = yaml[Y_SP_ROT_DELTA].as<float>(0.0f);
        conf[Options::SP_INC_DELTA] = yaml[Y_SP_INC_DELTA].as<float>(0.0f);
        conf[Options::SP_ELLIPSES_WIDTH] = yaml[Y_SP_ELLIPSES_WIDTH].as<float>(10.0f);

        conf[Options::SP_ELLIPSES] = yaml[Y_SP_ELLIPSES].as<size_t>();
        conf[Options::SP_SEGMENTS] = yaml[Y_SP_SEGMENTS].as<size_t>();
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

    this->points = yaml[Y_POINTS].as<size_t>();

    std::default_random_engine urng(std::random_device{}());
    std::uniform_int_distribution<size_t> tmp(0, std::numeric_limits<size_t>::max());

    this->seed = yaml[Y_SEED].as<uint64_t>(tmp(urng));
    this->gen = std::default_random_engine(seed);

    this->x_pos = yaml[Y_X_POS].as<float>(0.0f);
    this->y_pos = yaml[Y_Y_POS].as<float>(0.0f);

    this->x_vel = yaml[Y_X_VEL].as<float>(0.0f);
    this->y_vel = yaml[Y_Y_VEL].as<float>(0.0f);

    this->center_mass = yaml[Y_CENTER_MASS].as<float>(-1.0f);
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
