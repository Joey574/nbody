#include "galaxy.hpp"
#include <algorithm>
#include <any>
#include <stdexcept>
#include <random>
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
