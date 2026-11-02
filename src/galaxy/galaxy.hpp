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

        Galaxy() noexcept {}
        Galaxy(const YAML::Node conf) {
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

        Type type = Type::None;
        size_t points = 0;
        uint64_t seed = 0;

        float x_pos = 0.0f;
        float y_pos = 0.0f;

        float x_vel = 0.0f;
        float y_vel = 0.0f;

        float center_mass = 0.0f;

        std::unordered_map<Options, std::any> conf;
        mutable std::default_random_engine gen;
};
