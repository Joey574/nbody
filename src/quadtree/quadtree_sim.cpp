#include "quadtree.hpp"
#include "util/util.hpp"

#include <cfloat>
#include <cmath>
#include <assert.h>

void quadtree::acc(data& d) const {
    const size_t n = d.bodies();
    d.accx().zero();
    d.accy().zero();

    #pragma omp parallel
    {
        const int tid = omp_get_thread_num();

        const float* __restrict px = d.posx();
        const float* __restrict py = d.posy();
        float* __restrict ax = d.accx().row(tid);
        float* __restrict ay = d.accy().row(tid);

        #pragma omp for schedule(guided)
        for (size_t i = 0; i < n; i++) {
            compute_acc_point(px[i], py[i], ax[i], ay[i]);
        }
    }
}

void quadtree::compute_acc_point(
    float px,
    float py,
    float& __restrict ax,
    float& __restrict ay
) const {
    size_t node = ROOT;

    const float* __restrict nx = nodes_.X();
    const float* __restrict ny = nodes_.Y();
    const float* __restrict nm = nodes_.M();

    const size_t* __restrict nn = nodes_.N();
    const size_t* __restrict nc = nodes_.C();

    const float* __restrict qs = quads_.S();

    while(1) {
        const float x = nx[node];
        const float y = ny[node];
        const float m = nm[node];

        const float s = qs[node];

        const float dx = x - px;
        const float dy = y - py;

        const float dsq = (dx*dx) + (dy*dy);

        if (nodes_.is_leaf(node) || s*s < dsq * theta_*theta_) {
            const float denom = (dsq + util::epsl) * sqrtf(dsq);

            const float f = fminf(m/denom, FLT_MAX);

            ax += dx * f;
            ay += dy * f;

            if (nn[node] == 0) {
                break;
            }

            node = nn[node];
        } else {
            node = nc[node];
        }
    }
}
