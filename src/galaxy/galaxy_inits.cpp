#include "galaxy/galaxy.hpp"
#include "galaxy/galaxy_yaml.hpp"

#include <algorithm>
#include <cmath>
#include <limits>
#include <random>
#include <vector>
#include <yaml-cpp/node/node.h>

namespace {
    constexpr const float TAU = 6.28318530718f;
    constexpr const float PI = 3.14159265359f;

    static_assert(std::mt19937::max() == 0xFFFFFFFFu, "Rand01 assumes a 32-bit engine");
    inline float Rand01(std::mt19937& rng) {
        return (float)(rng() >> 8) * (1.0f / 16777216.0f);
    }

    // spiral
    constexpr const float DISC_SCALE_LENGTH = 0.25f;
    constexpr const float DISC_INNET_CUT = 0.02f;
    constexpr const float BULGE_SIGMA = 0.08f;
    constexpr const float ARM_FRACTION = 0.75f;

    // vornoi
    constexpr const int LLOYD_ITERATIONS = 3;
    constexpr const size_t LLOYD_SAMPLE_CELLS = 128;
    constexpr const int MAX_WALL_TRIES = 4096;

    inline void RandInDisc(std::mt19937& rng, float radius, float& x, float& y) {
        const float r2 = radius * radius;
        do {
            x = (2.0f * Rand01(rng) - 1.0f) * radius;
            y = (2.0f * Rand01(rng) - 1.0f) * radius;
        } while (x*x + y*y > r2);
    }

    struct TwoNearest {
        size_t a = 0, b = 0;
        float d1 = std::numeric_limits<float>::max();
        float d2 = std::numeric_limits<float>::max();
    };

    TwoNearest FindTwoNearest(const std::vector<float>& sx, const std::vector<float>& sy, float px, float py) {
        TwoNearest r;
        for (size_t c = 0; c < sx.size(); c++) {
            const float dx = sx[c] - px;
            const float dy = sy[c] - py;
            const float dsq = dx*dx + dy*dy;

            if (dsq < r.d1) {
                r.b = r.a;
                r.a = c;
                r.d2 = r.d1;
                r.d1 = dsq;
            } else if (dsq < r.d2) {
                r.b = c;
                r.d2 = dsq;
            }
        }

        return r;
    }
};

// uniform
shapes::Uniform shapes::Uniform::Load(const YAML::Node& n) {
    Uniform u;
    u.outer_radius = galaxy_yaml::Optional<float>(n, "outer_radius", u.outer_radius, name);
    u.inner_radius = galaxy_yaml::Optional<float>(n, "inner_radius", u.inner_radius, name);

    galaxy_yaml::Check(u.outer_radius > 0.0f, name, "'outer radius' must be > 0");
    galaxy_yaml::Check(u.inner_radius >= 0.0f && u.inner_radius < u.outer_radius, name, "'inner_radius' must be in [0, outer_radius)");
    return u;
}

void shapes::Uniform::Generate(std::span<float> x, std::span<float> y, std::mt19937& rng) const {
    const float r0 = inner_radius * inner_radius;
    const float r1 = outer_radius * outer_radius;

    for (size_t i = 0; i < x.size(); i++) {
        const float r = sqrtf(r0 + Rand01(rng) * (r1 - r0));
        const float a = Rand01(rng) * TAU;
        x[i] = r * cosf(a);
        y[i] = r * sinf(a);
    }
}

// spiral
shapes::Spiral shapes::Spiral::Load(const YAML::Node& n) {
    Spiral s;
    s.radius = galaxy_yaml::Optional<float>(n, "radius", s.radius, name);
    s.arms = galaxy_yaml::Optional<uint32_t>(n, "arms", s.arms, name);

    s.twist = galaxy_yaml::Optional<float>(n, "twist", s.twist, name);
    s.arm_width = galaxy_yaml::Optional<float>(n, "arm_width", s.arm_width, name);
    s.bulge_fraction = galaxy_yaml::Optional<float>(n, "bulge_fraction", s.bulge_fraction, name);

    galaxy_yaml::Check(s.radius > 0.0f, name, "'radius' must be > 0");
    galaxy_yaml::Check(s.arms >= 1 && s.arms <= 12, name, "'arms' must be in [1, 12]");
    galaxy_yaml::Check(s.arm_width >= 0.02f && s.arm_width <= 1.0f, name, "arm_width must be in [0.02, 1] radians");
    galaxy_yaml::Check(s.bulge_fraction >= 0.0f && s.bulge_fraction <= 0.6f, name, "'bulge_fraction' must be in [0, 0.6]");

    return s;
}

void shapes::Spiral::Generate(std::span<float> x, std::span<float> y, std::mt19937& rng) const {
    const float h = DISC_SCALE_LENGTH * radius;
    const float r_min = DISC_INNET_CUT * radius;
    const float bulge_sigma = BULGE_SIGMA * radius;

    std::normal_distribution<float> dist(0.0f, 1.0f);

    for (size_t i = 0; i < x.size(); i++) {
        if (Rand01(rng) < bulge_fraction) {
            float bx, by;
            do {
                bx = dist(rng) * bulge_sigma;
                by = dist(rng) * bulge_sigma;
            } while (bx*bx + by*by > radius*radius);

            x[i] = bx;
            y[i] = by;
            continue;
        }

        const float q = fmaxf((1.0f - Rand01(rng)) * (1.0f - Rand01(rng)), util::epsl);
        const float r = -h * logf(q);

        float theta;
        if (Rand01(rng) < ARM_FRACTION) {
            const uint32_t arm = (uint32_t)(rng() % arms);
            theta = TAU * (float)(arm) / (float)(arms) + twist * (r / radius) + dist(rng) * arm_width;
        } else {
            theta = Rand01(rng) * TAU;
        }

        x[i] = r * cosf(theta);
        y[i] = r * sinf(theta);
    }
}

// vornoi
shapes::Voronoi shapes::Voronoi::Load(const YAML::Node &n) {
    Voronoi v;
    v.radius = galaxy_yaml::Optional<float>(n, "radius", v.radius, name);
    v.cells = galaxy_yaml::Optional<uint32_t>(n, "cells", v.cells, name);
    v.edge_width = galaxy_yaml::Optional<float>(n, "edge_width", v.edge_width, name);
    v.background = galaxy_yaml::Optional<float>(n, "background", v.background, name);

    galaxy_yaml::Check(v.radius > 0.0f, name, "'radius' must be > 0");
    galaxy_yaml::Check(v.cells >= 3 && v.cells <= 512, name, "'cells' must be in [3, 512]");
    galaxy_yaml::Check(v.edge_width >= 0.005f && v.edge_width <= 0.5f, name, "'edge_width' must be in [0.05, 0.5]");
    galaxy_yaml::Check(v.background >= 0.0f && v.background <= 1.0f, name, "'background' must be in [0, 1]");
    return v;
}

void shapes::Voronoi::Generate(std::span<float> x, std::span<float> y, std::mt19937& rng) const {
    std::vector<float> sx(cells), sy(cells);
    for (uint32_t c = 0; c < cells; c++) {
        RandInDisc(rng, radius, sx[c], sy[c]);
    }

    std::vector<double> acc_x(cells), acc_y(cells);
    std::vector<size_t> count(cells);

    for (size_t it = 0; it < LLOYD_SAMPLE_CELLS; it++) {
        std::fill(acc_x.begin(), acc_x.end(), 0.0);
        std::fill(acc_y.begin(), acc_y.end(), 0.0);
        std::fill(count.begin(), count.end(), 0);

        const size_t samples = LLOYD_SAMPLE_CELLS * cells;
        for (size_t s = 0; s < samples; s++) {
            float px, py;
            RandInDisc(rng, radius, px, py);

            const size_t c = FindTwoNearest(sx, sy, px, py).a;
            acc_x[c] += px;
            acc_y[c] += py;
            count[c]++;
        }

        for (uint32_t c = 0; c < cells; c++) {
            if (!count[c]) continue;
            sx[c] = (float)(acc_x[c] / (double)count[c]);
            sy[c] = (float)(acc_y[c] / (double)count[c]);
        }
    }

    const float spacing = radius * sqrtf(PI / (float)cells);
    const float sigma = edge_width * spacing;

    for (size_t i = 0; i < x.size(); i++) {
        float px, py;

        if (Rand01(rng) < background) {
            RandInDisc(rng, radius, px, py);
            x[i] = px;
            y[i] = py;
            continue;
        }

        for (size_t tries = 0;; tries++) {
            RandInDisc(rng, radius, px, py);

            const TwoNearest t = FindTwoNearest(sx, sy, px, py);
            const float sep = fmaxf(hypotf(sx[t.a] - sx[t.b], sy[t.a] - sy[t.b]), 1e-6f);
            const float e = (t.d2 - t.d1) / (2.0f * sep);

            if (tries >= MAX_WALL_TRIES || Rand01(rng) < expf(-0.5f * (e / sigma) * (e / sigma))) {
                break;
            }
        }

        x[i] = px;
        y[i] = py;
    }
}
