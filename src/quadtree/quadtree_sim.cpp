#include "quadtree.hpp"
#include "util/util.hpp"

#include <assert.h>

void quadtree::Acc(data& d) const {
    const size_t n = d.Bodies();
    d.AccX().Zero();
    d.AccY().Zero();

    const float* __restrict px = d.PosX();
    const float* __restrict py = d.PosY();

    #pragma omp parallel
    {
        const int tid = omp_get_thread_num();
        float* __restrict ax = d.AccX().Row(tid);
        float* __restrict ay = d.AccY().Row(tid);

        #pragma omp for schedule(guided, 16)
        for (size_t i = 0; i < n; i++) {
            ComputeAccPoint(px[i], py[i], ax[i], ay[i]);
        }
    }
}

void quadtree::ComputeAccPoint(
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
        const float dx = nx[node] - px;
        const float dy = ny[node] - py;
        const float dsq = dx*dx + dy*dy;

        if (nodes_.IsLeaf(node) || qs[node]*qs[node] < dsq * thetasq_) {
            const float m = nm[node];

            const float soft_dsq = dsq + util::epsl;

            const   float inv = util::inv_sqrt(soft_dsq);
            const float inv3 = inv * inv * inv;

            ax += dx * inv3 * m;
            ay += dy * inv3 * m;

            const size_t next = nn[node];
            if (next == 0) {
                break;
            }

            node = next;
        } else {
            node = nc[node];
        }
    }
}
