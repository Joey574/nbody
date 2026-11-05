#pragma once
#include "../data/data.hpp"
#include "util/util.hpp"

#include <cstddef>
#include <cstdint>
#include <random>
#include <span>
#include <variant>
#include <string>
#include <yaml-cpp/node/node.h>
#include <yaml-cpp/yaml.h>

namespace shapes {
    struct Uniform {
        static constexpr const char* name = "uniform";

        float inner_radius = 0.0f;
        float outer_radius = 100.0f;

        static Uniform Load(const YAML::Node& n);
        void Generate(std::span<float> x, std::span<float> y, std::mt19937& rng) const;
    };

    struct Spiral {
        static constexpr const char* name = "spiral";

        float radius = 100.0f;
        uint32_t arms = 2;
        float twist = 4.0f;
        float arm_width = 0.25f;
        float bulge_fraction = 0.1f;

        static Spiral Load(const YAML::Node& n);
        void Generate(std::span<float> x, std::span<float> y, std::mt19937& rng) const;
    };

    struct Voronoi {
        static constexpr const char* name = "voronoi";

        float radius = 100.0f;
        uint32_t cells = 24;
        float edge_width = 0.08f;
        float background = 0.1f;

        static Voronoi Load(const YAML::Node& n);
        void Generate(std::span<float> x, std::span<float> y, std::mt19937& rng) const;
    };
} // namespace shapes

class Galaxy {
    public:
        using Shape = std::variant<shapes::Uniform, shapes::Spiral, shapes::Voronoi>;

        void Load(const YAML::Node& n);

        data New(bool init_acc) const;

        uint64_t Seed() const noexcept;
        size_t Points() const noexcept;
        std::string GalaxyString() const noexcept;
        const Shape& GetShape() const noexcept;

    private:
        bool HasCenter() const noexcept { return center_mass > util::epsl; }

        data NewUniform(bool init_acc) const;
        data NewSpiral(bool init_acc) const;

        void SetInitialVelocity(data& d, size_t first) const;

        size_t points = 0;
        uint64_t seed = 0;

        float x_pos = 0.0f;
        float y_pos = 0.0f;

        float x_vel = 0.0f;
        float y_vel = 0.0f;

        float center_mass = 0.0f;
        float softening = 0.0f;
        float spin = 1.0f;

        Shape shape;
};
