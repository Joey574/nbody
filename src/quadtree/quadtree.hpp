#pragma once
#include "../data/data.hpp"

struct zcurve {
    private:
    std::vector<std::pair<uint64_t, size_t>> idxs;

    public:
    inline void sort(data& data) noexcept {
        const size_t n = data.bodies();
        if (idxs.size() == 0) { idxs = std::vector<std::pair<uint64_t, size_t>>(n); }

        auto [pminx, pmaxx] = std::minmax_element(data.posx(), data.posx()+n);
        auto [pminy, pmaxy] = std::minmax_element(data.posy(), data.posy()+n);

        auto minx = *pminx;
        auto maxx = *pmaxx;
        auto miny = *pminy;
        auto maxy = *pmaxy;

        float rx = (maxx > minx) ? (maxx - minx) : 1.0f;
        float ry = (maxy > miny) ? (maxy - miny) : 1.0f;

        #pragma omp parallel for simd schedule(static)
        for (size_t i = 0; i < n; i++) {
            // get normalized x, y values
            uint32_t x = (uint32_t)std::min((double)(data.posx()[i] - minx) / rx * 0x100000000, (double)0xFFFFFFFFu);
            uint32_t y = (uint32_t)std::min((double)(data.posy()[i] - miny) / ry * 0x100000000, (double)0xFFFFFFFFu);
            uint64_t code = _pdep_u64((uint64_t)x, 0x5555555555555555ULL) | _pdep_u64((uint64_t)y, 0xAAAAAAAAAAAAAAAAULL);

            idxs[i] = { code, i };
        }

        std::sort(idxs.begin(), idxs.end());

        // cycle chasing swap to order data values according to the zcurve
        for (size_t i = 0; i < n-1; i++) {
            size_t cur = i;
            size_t next = idxs[cur].second;

            while (next != i) {
                std::swap(data.posx()[cur], data.posx()[next]);
                std::swap(data.posy()[cur], data.posy()[next]);
                std::swap(data.velx()[cur], data.velx()[next]);
                std::swap(data.vely()[cur], data.vely()[next]);
                std::swap(data.mass()[cur], data.mass()[next]);
                idxs[cur].second = cur;
                cur = next;
                next = idxs[cur].second;
            }
            idxs[cur].second = cur;
        }
    }
};
