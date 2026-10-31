/*
Author: Joey Soroka
Updated: 2/26/26
Purpose: Implements nbody simulation for the cpu
Comments: Switches update_cpu implementation based on compile time definitions.
Supports both AVX512 and AVX2 explicitly, all others will fall back on omp and
compiler for vectorization
*/

#include "simulation.hpp"
#include <algorithm>

std::chrono::nanoseconds simulation::update_cpu(const float ft) noexcept {
    auto start = std::chrono::high_resolution_clock::now();

    compute_acc();
    sum_acc();
    move_points(ft);

    return std::chrono::high_resolution_clock::now() - start;
}

/// @brief Generic custom simd update function
/// @return Nanoseconds it took to run update
void simulation::compute_acc() noexcept {
    data_.zero_acc();

    // aliasing
    const size_t n = data_.bodies();
    const float* __restrict ma = data_.mass();
    const float* __restrict px = data_.posx();
    const float* __restrict py = data_.posy();
    matrix& __restrict ax = data_.accx();
    matrix& __restrict ay = data_.accy();

    // TODO
    // test out blocked implementation to decrease pressure on ax and ay

    // gravitational constant is set to 1 for purposes of this simulation
    #pragma omp parallel
    {
        int tid = omp_get_thread_num();
        float* __restrict ax_row = ax.row(tid);
        float* __restrict ay_row = ay.row(tid);

        #pragma omp for schedule(static)
        for (size_t i = 0; i < n; i++) {
            const float p1x = px[i];
            const float p1y = py[i];
            const float p1m = ma[i];

            const auto _p1x = util::set1(p1x);
            const auto _p1y = util::set1(p1y);
            const auto _p1m = util::set1(p1m);

            auto _a1x_sum = util::zero();
            auto _a1y_sum = util::zero();

            size_t j = i+1;
            for (; j+util::last < n; j += util::width) {
                const auto _p2x = util::loadu(&px[j]);
                const auto _p2y = util::loadu(&py[j]);
                const auto _p2m = util::loadu(&ma[j]);

                // compute distance squared
                const auto _dx = _p2x - _p1x;
                const auto _dy = _p2y - _p1y;
                const auto _dsq = util::_epsl + (_dx*_dx) + (_dy*_dy);

                // fast inv sqrt with newton step
                const auto _inv = util::rsqrt(_dsq);
                const auto _inv3 = _inv * _inv * _inv;

                // compute intermediate values
                const auto _ivx = _dx * _inv3;
                const auto _ivy = _dy * _inv3;

                // compute and store accelerations
                util::storeu(&ax_row[j], util::loadu(&ax_row[j]) - (_ivx * _p1m));
                util::storeu(&ay_row[j], util::loadu(&ay_row[j]) - (_ivy * _p1m));
                _a1x_sum += _ivx * _p2m;
                _a1y_sum += _ivy * _p2m;
            }

            float a1x_final = util::hsum(_a1x_sum);
            float a1y_final = util::hsum(_a1y_sum);

            // remainder handling
            for(; j < n; j++) {
                util::apply_force_to_two(
                    p1x, p1y, p1m,
                    px[j], py[j], ma[j],
                    &a1x_final, &a1y_final,
                    &ax_row[j], &ay_row[j]
                );
            }

            ax_row[i] += a1x_final;
            ay_row[i] += a1y_final;
        }
    }
}
