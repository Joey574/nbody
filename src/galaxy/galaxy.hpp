#pragma once
#include <cstddef>
#include <cstdint>
#include <unordered_map>
#include <any>
#include <random>
#include <yaml-cpp/yaml.h>

#include "../data/data.hpp"

struct Galaxy {
    public:
        enum class Type {
            None, Spiral, Cluster, Uniform, Voronoi
        };

        Galaxy() noexcept  : type(Type::None), points(0) {}
        Galaxy(const YAML::Node conf) : type(Type::None), points(0) {
            this->Load(conf);
        }

        /// @brief Creates a new galaxy based on the previously loaded config, this is a factory function and each call is independent of the last, (internal rng state updates between calls)
        data New(bool init_acc) const;

        void Load(const YAML::Node conf);

        uint64_t Seed() const noexcept;
        size_t Points() const noexcept;

        Type GalaxyType() const noexcept;
        std::string GalaxyString() const noexcept;

    private:
        data NewUniform(bool init_acc) const;
        data NewSpiral(bool init_acc) const;

        enum class Options {
            UN_INNER_RADIUS,
            UN_OUTER_RADIUS,
            UN_DISTRIBUTION,

            SP_X_RADIUS,
            SP_Y_RADIUS,
            SP_ROT_START,
            SP_INC_START,
            SP_ELLIPSES,
            SP_SEGMENTS,
            SP_X_RADIUS_SCALE,
            SP_ROT_DELTA,
            SP_INC_DELTA
        };

        Type type;
        size_t points;
        uint64_t seed;

        float x_pos;
        float y_pos;

        float x_vel;
        float y_vel;

        float center_mass;

        std::unordered_map<Options, std::any> conf;
        mutable std::default_random_engine gen;
};
