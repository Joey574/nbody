#pragma once
#include "data/data.hpp"
#include "quadtree/node.hpp"
#include "quadtree/quad.hpp"

#include <cmath>
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
            thetasq_(std::exchange(other.thetasq_, 0.0f))
        {}

        // move operator
        quadtree& operator=(quadtree&& other) noexcept {
            if (this == &other) return *this;
            quads_ = std::move(other.quads_);
            nodes_ = std::move(other.nodes_);
            thetasq_ = std::exchange(other.thetasq_, 0);

            return *this;
        }

        // parameterized constructor
        quadtree(float theta) : thetasq_(theta*theta) {}

        template <typename Func>
        requires std::invocable<Func, size_t, size_t, const nodes&>
        void TraverseSeq(Func&& visit) const {
            const size_t n = nodes_.Size();
            for (size_t i = 0; i < n; i++) {
                visit(i, nodes_);
            }
        }

        template <typename Func>
        requires std::invocable<Func, size_t, size_t, const nodes&>
        void TraversePar(Func&& visit) const {
            const size_t n = nodes_.Size();

            #pragma omp parallel
            {
                const int tid = omp_get_thread_num();

                #pragma omp for schedule(guided, 16)
                for (size_t i = 0; i < n; i++) {
                    visit(i, tid, nodes_);
                }
            }
        }

        void Insert(const data& d);
        void Acc(data& d) const;

        float Theta() const noexcept { return std::sqrt(thetasq_); }

    private:
        static constexpr size_t ROOT = 0;

        float thetasq_ = 1.0f;

        quads quads_ = {};
        nodes nodes_ = {};
        std::vector<size_t> parents_;
        std::vector<uint64_t> keys_;

        void InsertPoints(float x, float y, float m);

        size_t Subdivide(size_t node);
        void Propogate();

        void ComputeAccPoint(
            float px,
            float py,
            float& __restrict ax,
            float& __restrict ay
        ) const;
};
