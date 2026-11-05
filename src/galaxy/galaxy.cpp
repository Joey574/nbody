#include "galaxy.hpp"
#include "data/data.hpp"
#include "galaxy/galaxy_yaml.hpp"
#include "util/util.hpp"

#include <stdexcept>
#include <random>
#include <utility>
#include <string>
#include <vector>
#include <yaml-cpp/node/node.h>

namespace {
    template<size_t... Is>
    std::string KnownTypesImpl(std::index_sequence<Is...>) {
        std::string s;
        const auto add = [&s](const char* name) {
            if (!s.empty()) s += ", ";
            s += name;
        };
        (add(std::variant_alternative_t<Is, Galaxy::Shape>::name), ...);
        return s;
    }

    std::string KnownTypes() {
        return KnownTypesImpl(std::make_index_sequence<std::variant_size_v<Galaxy::Shape>>{});
    }

    template <size_t I = 0>
    Galaxy::Shape LoadShape(const std::string& type, const YAML::Node& n) {
        if constexpr (I == std::variant_size_v<Galaxy::Shape>) {
            throw std::invalid_argument("unknown galaxy type '" + type + "' (known: " + KnownTypes() +  ")");
        } else {
            using S = std::variant_alternative_t<I, Galaxy::Shape>;
            if (type == S::name) return S::Load(n);
            return LoadShape<I+1>(type, n);
        }
    }

    uint64_t RandomSeed() {
        std::random_device rd;
        return (static_cast<uint64_t>(rd()) << 32) | rd();
    }
}; // namespace

void Galaxy::Load(const YAML::Node& n) {
    std::string type = galaxy_yaml::Require<std::string>(n, "type", "galaxy");
    std::transform(type.begin(), type.end(), type.begin(), ::tolower);
    const char* ctx = type.c_str();

    points = galaxy_yaml::Require<size_t>(n, "points", ctx);
    galaxy_yaml::Check(points >= 1, ctx, "'points' must be at least 1");

    seed = galaxy_yaml::Optional<uint64_t>(n, "seed", RandomSeed(), ctx);

    x_pos = galaxy_yaml::Optional<float>(n, "x_pos", x_pos, ctx);
    y_pos = galaxy_yaml::Optional<float>(n, "y_pos", y_pos, ctx);
    x_vel = galaxy_yaml::Optional<float>(n, "x_vel", x_vel, ctx);
    y_vel = galaxy_yaml::Optional<float>(n, "y_vel", y_vel, ctx);

    center_mass = galaxy_yaml::Optional<float>(n, "center_mass", center_mass, ctx);
    spin = galaxy_yaml::Optional<float>(n, "spin", spin, ctx);
    softening = galaxy_yaml::Optional<float>(n, "softening", softening, ctx);
    galaxy_yaml::Check(softening >= 0.0f, ctx, "'softening' must be >= 0");

    shape = LoadShape(type, n);
}

data Galaxy::New(bool init_acc) const {
    data d(points, init_acc);
    std::mt19937 rng(seed);

    float* __restrict px = d.posx();
    float* __restrict py = d.posy();
    float* __restrict ma = d.mass();

    size_t first = 0;
    if (HasCenter()) {
        px[0] = x_pos;
        py[0] = y_pos;
        ma[0] = center_mass;
        first = 1;
    }

    const size_t n = points-first;
    std::visit([&](const auto& s) {
        s.Generate(std::span<float>(px+first, n), std::span<float>(py+first, n), rng);
    }, shape);

    #pragma omp parallel for schedule(static)
    for (size_t i = first; i < points; i++) {
        px[i] += x_pos;
        py[i] += y_pos;
        ma[i] = 1.0f;
    }

    SetInitialVelocity(d, first);
    return d;
}

void Galaxy::SetInitialVelocity(data& d, size_t first) const {

    // alias
    const size_t total = d.bodies();
    const float* __restrict px = d.posx();
    const float* __restrict py = d.posy();
    const float* __restrict ma = d.mass();
    float* __restrict vx = d.velx();
    float* __restrict vy = d.vely();

    if (HasCenter()) {
        vx[0] = x_vel;
        vy[0] = y_vel;
    }

    const size_t n = total  - first;
    if (!n) return;

    std::vector<float> r2(n);
    #pragma omp parallel for simd schedule(static)
    for (size_t i = 0; i < n; i++) {
        const float dx = px[first + i] - x_pos;
        const float dy = py[first + i] - y_pos;
        r2[i] = dx*dx + dy*dy;
    }

    std::vector<size_t> order(n);
    std::iota(order.begin(), order.end(), 0);
    std::sort(order.begin(), order.end(), [&](size_t a, size_t b){ return r2[a] < r2[b]; });

    const float softsq = softening * softening;
    double enclosed = HasCenter() ? center_mass : 0.0f;

    for (const size_t i : order) {
        const size_t k = first + i;
        const double m = ma[k];
        const double m_enc = enclosed + 0.5 * m;
        enclosed += m;

        if (r2[i] < util::epsl) {
            vx[k] = x_vel;
            vy[k] = y_vel;
            continue;
        }

        const float r = sqrtf(r2[i]);
        const float v2 = (float)m_enc * r2[i] / powf(r2[i] + softsq, 1.5f);
        const float v = spin * sqrtf(v2);

        const float dx = px[k] - x_pos;
        const float dy = py[k] - y_pos;
        vx[k] = -dy / r * v + x_vel;
        vy[k] = dx / r * v + y_vel;
    }
}

uint64_t Galaxy::Seed() const noexcept {
    return this->seed;
}

size_t Galaxy::Points() const noexcept {
    return this->points;
}

std::string Galaxy::GalaxyString() const noexcept {
    return std::visit([](const auto& s) {
        return std::string(std::decay_t<decltype(s)>::name);
    }, shape);
}

const Galaxy::Shape& Galaxy::GetShape() const noexcept { return shape; }
