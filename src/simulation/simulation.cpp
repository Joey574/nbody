#include "simulation/simulation.hpp"
#include <chrono>

void simulation::SumAcc() noexcept {
    // aliasing
    matrix& ax = data_.AccX();
    matrix& ay = data_.AccY();
    float* __restrict ax_top = ax.Row(0);
    float* __restrict ay_top = ay.Row(0);

    // TODO : hand roll the simd implementation
    // sum acceleration into top row
    #pragma omp parallel for simd schedule(static)
    for (size_t c = 0; c < ax.Cols(); c++) {
        float sx = ax_top[c], sy = ay_top[c];

        for (size_t r = 1; r < ax.Rows(); r++) {
            sx += ax(r, c);
            sy += ay(r, c);
        }

        ax_top[c] = sx;
        ay_top[c] = sy;
    }
}

void simulation::MovePoints(const float ft) noexcept {
    // aliasing
    const size_t n = data_.Bodies();
    float* __restrict px = data_.PosX();
    float* __restrict py = data_.PosY();
    float* __restrict vx = data_.VelX();
    float* __restrict vy = data_.VelY();
    const float* ax = data_.AccX().Row(0);
    const float* ay = data_.AccY().Row(0);

    const auto _ft = util::set1(ft);
    const size_t nv = n - (n % util::width);

    #pragma omp parallel for schedule(static)
    for (size_t i = 0; i < nv; i += util::width) {
        const auto _ax = util::load(&ax[i]);
        const auto _ay = util::load(&ay[i]);
        auto _vx = util::load(vx+i);
        auto _vy = util::load(vy+i);
        auto _px = util::load(px+i);
        auto _py = util::load(py+i);

        _vx += _ax * _ft;
        _vy += _ay * _ft;
        _px += _vx * _ft;
        _py += _vy * _ft;

        util::store(vx+i, _vx);
        util::store(vy+i, _vy);
        util::store(px+i, _px);
        util::store(py+i, _py);
    }

    size_t r = n - (n%util::width);
    for (size_t i = r; i < n; i++) {
        vx[i] += ax[i] * ft;
        vy[i] += ay[i] * ft;

        px[i] += vx[i] * ft;
        py[i] += vy[i] * ft;
    }
}

std::chrono::nanoseconds simulation::Update(float ft) {
    switch (backend_) {
        case backend::CPU:
            return UpdateCPU(ft);
        case backend::CPU_BH:
            return UpdateCpuBH(ft);
        default:
            return std::chrono::nanoseconds::zero();
    }
}
