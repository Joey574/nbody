/*
Author: Joey Soroka
Purpose: Implements nbody direct sum algorithm for the cpu
Comments: Switches update_cpu implementation based on compile time definitions.
Supports both AVX512 and AVX2 explicitly, all others will fall back on omp and
compiler for vectorization
*/

#include "simulation/simulation.hpp"
#include "util/util.hpp"

std::chrono::nanoseconds simulation::UpdateCPU(const float ft) noexcept {
    auto start = std::chrono::high_resolution_clock::now();

    ComputeAcc();
    SumAcc();
    MovePoints(ft);

    return std::chrono::high_resolution_clock::now() - start;
}

/// @brief Generic custom simd update function
/// @return Nanoseconds it took to run update
void simulation::ComputeAcc() noexcept {
    data_.ZeroAcc();

    // aliasing
    const size_t n = data_.Bodies();
    const float* __restrict ma = data_.Mass();
    const float* __restrict px = data_.PosX();
    const float* __restrict py = data_.PosY();
    matrix& __restrict ax = data_.AccX();
    matrix& __restrict ay = data_.AccY();

    #pragma omp parallel
    {
        int tid = omp_get_thread_num();
        float* __restrict ax_row = ax.Row(tid);
        float* __restrict ay_row = ay.Row(tid);

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
            const bool j_aligned = j % util::width == 0;

            for (; j+util::last < n; j += util::width) {
                const auto _p2x = j_aligned ? util::load(&px[j]) : util::loadu(&px[j]);
                const auto _p2y = j_aligned ? util::load(&py[j]) : util::loadu(&py[j]);
                const auto _p2m = j_aligned ? util::load(&ma[j]) : util::loadu(&ma[j]);

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
                // only 1 in util::width iterations are aligned
                if (j_aligned) [[unlikely]] {
                    util::store(&ax_row[j], util::load(&ax_row[j]) - (_ivx * _p1m));
                    util::store(&ay_row[j], util::load(&ay_row[j]) - (_ivy * _p1m));
                } else {
                    util::storeu(&ax_row[j], util::loadu(&ax_row[j]) - (_ivx * _p1m));
                    util::storeu(&ay_row[j], util::loadu(&ay_row[j]) - (_ivy * _p1m));
                }
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
