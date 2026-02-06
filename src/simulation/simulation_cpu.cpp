#include "simulation.hpp"
/*

Switches update_cpu implementation based on compile time definitions
supports both AVX512 and AVX2 directly, all others will fall back
on omp and compiler for vectorization

*/

#if defined(__AVX512F__)
std::chrono::nanoseconds simulation::update_cpu(const float ft) noexcept {
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

    // constants
    const __m512 _opf = _mm512_set1_ps(1.5f);
    const __m512 _pf = _mm512_set1_ps(0.5f);
    const __m512 _epsl = _mm512_set1_ps(1e-12f);

    // gravitational constant is set to 1 for purposes of this simulation
    #pragma omp parallel
    {
        int tid = omp_get_thread_num();

        #pragma omp for schedule(static)
        for (size_t i = 0; i < n; i++) {
            const float p1x = px[i];
            const float p1y = py[i];
            const float p1m = ma[i];

            const __m512 _p1x = _mm512_set1_ps(p1x);
            const __m512 _p1y = _mm512_set1_ps(p1y);
            const __m512 _p1m = _mm512_set1_ps(p1m);

            __m512 _a1x_sum = _mm512_setzero_ps();
            __m512 _a1y_sum = _mm512_setzero_ps();

            size_t j = i+1;
            for (; j+15 < n; j += 16) {
                const __m512 _p2x = _mm512_loadu_ps(&px[j]);
                const __m512 _p2y = _mm512_loadu_ps(&py[j]);
                const __m512 _p2m = _mm512_loadu_ps(&ma[j]);

                // compute distance squared
                const __m512 _dx = _p2x - _p1x;
                const __m512 _dy = _p2y - _p1y;
                const __m512 _dsq = _epsl + (_dx*_dx) + (_dy*_dy);

                // fast inv sqrt with newton step
                __m512 _inv = _mm512_rsqrt14_ps(_dsq);
                _inv = _inv * (_opf - _pf * _dsq * _inv * _inv);
                const __m512 _inv3 = _inv * _inv * _inv;

                // compute intermediate values
                const __m512 _ivx = _dx * _inv3;
                const __m512 _ivy = _dy * _inv3;

                // compute acceleration
                const __m512 _a1x = _ivx * _p2m;
                const __m512 _a1y = _ivy * _p2m;
                const __m512 _a2x = _ivx * _p1m;
                const __m512 _a2y = _ivy * _p1m;

                _mm512_storeu_ps(&ax(tid, j), _a2x + _mm512_loadu_ps(&ax(tid, j)));
                _mm512_storeu_ps(&ay(tid, j), _a2y + _mm512_loadu_ps(&ay(tid, j)));
                _a1x_sum += _a1x;
                _a1y_sum += _a1y;
            }

            float a1x_final = util::sum512(_a1x_sum);
            float a1y_final = util::sum512(_a1y_sum);

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

        #pragma omp barrier

        // sum acceleration into top row
        for (size_t r = 1; r < ax.rows(); r++) {

            #pragma omp for simd schedule(static)
            for (size_t c = 0; c < ax.cols(); c++) {
                ax(0, c) += ax(r, c);
                ay(0, c) += ay(r, c);
            }
        }

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

#elif defined(__AVX2__)
std::chrono::nanoseconds simulation::update_cpu(const float ft) noexcept {
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

    // constants
    const __m256 _opf = _mm256_set1_ps(1.5f);
    const __m256 _pf = _mm256_set1_ps(0.5f);
    const __m256 _epsl = _mm256_set1_ps(1e-12f);

    // gravitational constant is set to 1 for purposes of this simulation
    #pragma omp parallel
    {
        int tid = omp_get_thread_num();

        #pragma omp for schedule(static)
        for (size_t i = 0; i < n; i++) {
            const float p1x = px[i];
            const float p1y = py[i];
            const float p1m = ma[i];

            const __m256 _p1x = _mm256_set1_ps(p1x);
            const __m256 _p1y = _mm256_set1_ps(p1y);
            const __m256 _p1m = _mm256_set1_ps(p1m);

            __m256 _a1x_sum = _mm256_setzero_ps();
            __m256 _a1y_sum = _mm256_setzero_ps();

            size_t j = i+1;
            for (; j+7 < n; j += 8) {
                const __m256 _p2x = _mm256_loadu_ps(&px[j]);
                const __m256 _p2y = _mm256_loadu_ps(&py[j]);
                const __m256 _p2m = _mm256_loadu_ps(&ma[j]);

                // compute distance squared
                const __m256 _dx = _p2x - _p1x;
                const __m256 _dy = _p2y - _p1y;
                const __m256 _dsq = _epsl + (_dx*_dx) + (_dy*_dy);

                // fast inv sqrt with newton step
                __m256 _inv = _mm256_rsqrt_ps(_dsq);
                _inv = _inv * (_opf - _pf * _dsq * _inv * _inv);
                const __m256 _inv3 = _inv * _inv * _inv;

                // compute intermediate values
                const __m256 _ivx = _dx * _inv3;
                const __m256 _ivy = _dy * _inv3;

                // compute acceleration
                const __m256 _a1x = _ivx * _p2m;
                const __m256 _a1y = _ivy * _p2m;
                const __m256 _a2x = _ivx * _p1m;
                const __m256 _a2y = _ivy * _p1m;

                _mm256_storeu_ps(&ax(tid, j), _a2x + _mm256_loadu_ps(&ax(tid, j)));
                _mm256_storeu_ps(&ay(tid, j), _a2y + _mm256_loadu_ps(&ay(tid, j)));
                _a1x_sum += _a1x;
                _a1y_sum += _a1y;
            }

            float a1x_final = util::sum256(_a1x_sum);
            float a1y_final = util::sum256(_a1y_sum);

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

        #pragma omp barrier

        // sum acceleration into top row
        for (size_t r = 1; r < ax.rows(); r++) {

            #pragma omp for simd schedule(static)
            for (size_t c = 0; c < ax.cols(); c++) {
                ax(0, c) += ax(r, c);
                ay(0, c) += ay(r, c);
            }
        }

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
#else
std::chrono::nanoseconds simulation::update_cpu(const float ft) noexcept {
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

    // constants
    const __m256 _opf = _mm256_set1_ps(1.5f);
    const __m256 _pf = _mm256_set1_ps(0.5f);
    const __m256 _epsl = _mm256_set1_ps(1e-12f);

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

        #pragma omp barrier

        // sum acceleration into top row
        for (size_t r = 1; r < ax.rows(); r++) {

            #pragma omp for simd schedule(static)
            for (size_t c = 0; c < ax.cols(); c++) {
                ax(0, c) += ax(r, c);
                ay(0, c) += ay(r, c);
            }
        }

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
#endif