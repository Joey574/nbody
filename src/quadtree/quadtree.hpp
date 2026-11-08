#pragma once
#include "data/data.hpp"
#include "quadtree/node.hpp"
#include "quadtree/quad.hpp"

#include <utility>

// TODO
// should try to implement this is parrot and see what the perf is

struct quadtree {
    public:
        // default constructor
        quadtree() noexcept = default;

        // copies
        quadtree(const quadtree& other) noexcept = delete;
        quadtree& operator=(const quadtree& other) noexcept = delete;

        // move constructor
        quadtree(quadtree&& other) noexcept :
            quads_(std::move(other.quads_)),
            nodes_(std::move(other.nodes_)),
            theta_(std::exchange(other.theta_, 0.0f))
        {}

        // move operator
        quadtree& operator=(quadtree&& other) noexcept {
            if (this == &other) return *this;
            quads_ = std::move(other.quads_);
            nodes_ = std::move(other.nodes_);
            theta_ = std::exchange(other.theta_, 0);

            return *this;
        }

        // parameterized constructor
        quadtree(float theta) : theta_(theta) {}

        void insert(const data& d);
        void acc(data& d) const;

        float theta() const noexcept { return theta_; }

    private:
        static constexpr size_t ROOT = 0;

        float theta_ = 1.0f;

        quads quads_ = {};
        nodes nodes_ = {};
        std::vector<size_t> parents_;

        void insert_point(float x, float y, float m);

        size_t subdivide(size_t node);
        void propagate();

        void compute_acc_point(
            float px,
            float py,
            float& __restrict ax,
            float& __restrict ay
        ) const;
};
