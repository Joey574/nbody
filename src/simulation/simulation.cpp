#include "simulation.hpp"


void simulation::sum_acc() noexcept {
    // aliasing
    matrix& ax = data_.accx();
    matrix& ay = data_.accy();
    float* __restrict ax_top = ax.row(0);
    float* __restrict ay_top = ay.row(0);

    // TODO : hand roll the simd implementation
    // sum acceleration into top row
    #pragma omp parallel for simd schedule(static)
    for (size_t c = 0; c < ax.cols(); c++) {
        float sx = ax_top[c], sy = ay_top[c];

        for (size_t r = 1; r < ax.rows(); r++) {
            sx += ax(r, c);
            sy += ay(r, c);
        }

        ax_top[c] = sx;
        ay_top[c] = sy;
    }
}

void simulation::move_points(const float ft) noexcept {
    // aliasing
    const size_t n = data_.bodies();
    float* __restrict px = data_.posx();
    float* __restrict py = data_.posy();
    float* __restrict vx = data_.velx();
    float* __restrict vy = data_.vely();
    const float* ax = data_.accx().row(0);
    const float* ay = data_.accy().row(0);

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
