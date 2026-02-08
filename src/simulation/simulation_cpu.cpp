#include "simulation.hpp"
/*

Switches update_cpu implementation based on compile time definitions.
Supports both AVX512 and AVX2 explicitly, all others will fall back
on omp and compiler for vectorization

*/

std::chrono::nanoseconds simulation::update_cpu(const float ft) noexcept {
    #ifdef USE_AVX512
        return update_cpu_simd<simd_policy::AVX512>(ft);
    #elifdef USE_AVX2
        return update_cpu_simd<simd_policy::AVX2>(ft);
    #else
        return update_cpu_fallback(ft);
    #endif
}

/// @brief Generic custom simd update function
/// @tparam p simd_policy to be used in the update
/// @param ft Fixed time used for update
/// @return Nanoseconds it took to run update
template <typename p>
std::chrono::nanoseconds simulation::update_cpu_simd(const float ft) noexcept {
    auto s = std::chrono::high_resolution_clock::now();
    data_.zero_acc();

    // aliasing
    const size_t n = data_.bodies();
    const float* __restrict ma = data_.mass();
    float* __restrict px = data_.posx();
    float* __restrict py = data_.posy();
    float* __restrict vx = data_.velx();
    float* __restrict vy = data_.vely();
    matrix& ax = data_.accx();
    matrix& ay = data_.accy();

    // constants
    const auto _epsl = p::set1(1e-12f);

    // gravitational constant is set to 1 for purposes of this simulation
    #pragma omp parallel
    {
        int tid = omp_get_thread_num();

        #pragma omp for schedule(static)
        for (size_t i = 0; i < n; i++) {
            const float p1x = px[i];
            const float p1y = py[i];
            const float p1m = ma[i];

            const auto _p1x = p::set1(p1x);
            const auto _p1y = p::set1(p1y);
            const auto _p1m = p::set1(p1m);

            auto _a1x_sum = p::zero();
            auto _a1y_sum = p::zero();

            size_t j = i+1;
            for (; j+(p::width-1) < n; j += p::width) {
                const auto _p2x = p::load(&px[j]);
                const auto _p2y = p::load(&py[j]);
                const auto _p2m = p::load(&ma[j]);

                // compute distance squared
                const auto _dx = _p2x - _p1x;
                const auto _dy = _p2y - _p1y;
                const auto _dsq = _epsl + (_dx*_dx) + (_dy*_dy);

                // fast inv sqrt with newton step
                const auto _inv = p::rsqrt(_dsq);
                const auto _inv3 = _inv * _inv * _inv;

                // compute intermediate values
                const auto _ivx = _dx * _inv3;
                const auto _ivy = _dy * _inv3;

                // compute and store accelerations
                p::store(&ax(tid, j), (_ivx * _p1m) + p::load(&ax(tid, j)));
                p::store(&ay(tid, j), (_ivy * _p1m) + p::load(&ay(tid, j)));
                _a1x_sum += _ivx * _p2m;;
                _a1y_sum += _ivy * _p2m;;
            }

            float a1x_final = p::hsum(_a1x_sum);
            float a1y_final = p::hsum(_a1y_sum);

            // remainder handling
            for(; j < n; j++) {
                const float p2x = px[j];
                const float p2y = py[j];
                const float p2m = ma[j];

                const float dx = p2x - p1x;
                const float dy = p2y - p1y;
                const float dsq = 1e-12f + (dx*dx) + (dy*dy);

                // fast rsqrt with netwon step
                float inv = util::rsqrt(dsq);
                inv = inv * (1.5f - 0.5f * dsq * inv * inv);

                // compute intermediate values
                float inv_dis3 = inv * inv * inv;
                float ivx = dx * inv_dis3;
                float ivy = dy * inv_dis3;

                // update acceleration values
                a1x_final += ivx * p2m;
                a1y_final += ivy * p2m;
                ax(tid, j) += ivx * p1m;
                ax(tid, j) += ivy * p1m;
            }

            ax(tid, i) = a1x_final;
            ay(tid, i) = a1y_final;
        }

        // explicit barrier
        #pragma omp barrier

        // sum acceleration into top row
        for (size_t r = 1; r < ax.rows(); r++) {

            #pragma omp for simd schedule(static)
            for (size_t c = 0; c < ax.cols(); c++) {
                ax(0, c) += ax(r, c);
                ay(0, c) += ay(r, c);
            }
        }

        // explicit barrier
        #pragma omp barrier

        // update body positions and velocities
        #pragma omp for simd schedule(static)
        for (size_t i = 0; i < n; i++) {
            vx[i] += ax(0, i) * ft;
            vy[i] += ay(0, i) * ft;

            px[i] += vx[i] * ft;
            py[i] += vy[i] * ft;
        }
    }

    return std::chrono::high_resolution_clock::now() - s;
}

/// @brief Fall back funciton for cpu updates, uses compiler for vectorization
/// @param ft Fixed time used for update
/// @return Nanoseconds it took to run update
std::chrono::nanoseconds simulation::update_cpu_fallback(const float ft) noexcept {
    auto s = std::chrono::high_resolution_clock::now();
    data_.zero_acc();

    // aliasing
    const size_t n = data_.bodies();
    float* __restrict px = data_.posx();
    float* __restrict py = data_.posy();
    float* __restrict vx = data_.velx();
    float* __restrict vy = data_.vely();
    float* __restrict ma = data_.mass();
    matrix& ax = data_.accx();
    matrix& ay = data_.accy();

    // gravitational constant is set to 1 for purposes of this simulation
    #pragma omp parallel
    {
        int tid = omp_get_thread_num();

        #pragma omp for schedule(static)
        for (size_t i = 0; i < n; i++) {
            const float p1x = px[i];
            const float p1y = py[i];
            const float p1m = ma[i];

            float a1x_final = 0.0f;
            float a1y_final = 0.0f;

            #pragma omp simd
            for(size_t j = i+1; j < n; j++) {
                const float p2x = px[j];
                const float p2y = py[j];
                const float p2m = ma[j];

                const float dx = p2x - p1x;
                const float dy = p2y - p1y;
                const float dsq = 1e-12f + (dx*dx) + (dy*dy);

                // fast rsqrt with netwon step
                float inv = util::rsqrt(dsq);
                inv = inv * (1.5f - 0.5f * dsq * inv * inv);

                // compute intermediate values
                float inv_dis3 = inv * inv * inv;
                float ivx = dx * inv_dis3;
                float ivy = dy * inv_dis3;

                // update acceleration values
                a1x_final += ivx * p2m;
                a1y_final += ivy * p2m;
                ax(tid, j) += ivx * p1m;
                ax(tid, j) += ivy * p1m;
            }

            ax(tid, i) = a1x_final;
            ay(tid, i) = a1y_final;
        }

        // explicit barrier
        #pragma omp barrier

        // sum acceleration into top row
        for (size_t r = 1; r < ax.rows(); r++) {

            #pragma omp for simd schedule(static)
            for (size_t c = 0; c < ax.cols(); c++) {
                ax(0, c) += ax(r, c);
                ay(0, c) += ay(r, c);
            }
        }

        // explicit barrier
        #pragma omp barrier

        // update body positions and velocities
        #pragma omp for simd schedule(static)
        for (size_t i = 0; i < n; i++) {
            vx[i] += ax(0, i) * ft;
            vy[i] += ay(0, i) * ft;

            px[i] += vx[i] * ft;
            py[i] += vy[i] * ft;
        }
    }

    return std::chrono::high_resolution_clock::now() - s;
}
