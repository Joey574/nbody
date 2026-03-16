#pragma once
#include "../data/data.hpp"

struct quadtree {
    private:
    std::vector<std::pair<uint64_t, size_t>> zcurve;

    void sort(data& data) noexcept {
        const size_t n = data.bodies();
        if (zcurve.size() == 0) { zcurve = std::vector<std::pair<uint64_t, size_t>>(n); }

        auto [minx, maxx] = std::minmax_element(data.posx(), data.posx()+n);
        auto [miny, maxy] = std::minmax_element(data.posy(), data.posy()+n);

        float rx = (*maxx > *minx) ? (*maxx - *minx) : 1.0f;
        float ry = (*maxy > *miny) ? (*maxy - *miny) : 1.0f;

        #pragma omp parallel for simd schedule(static)
        for (size_t i = 0; i < n; i++) {
            // get normalized x, y values
            uint32_t x = (uint32_t)std::min((double)(data.posx()[i] - *minx) / rx * 0x100000000, (double)0xFFFFFFFFu);
            uint32_t y = (uint32_t)std::min((double)(data.posy()[i] - *miny) / ry * 0x100000000, (double)0xFFFFFFFFu);
            uint64_t code = _pdep_u64((uint64_t)x, 0x5555555555555555ULL) | _pdep_u64((uint64_t)y, 0xAAAAAAAAAAAAAAAAULL);

            zcurve[i] = { code, i };
        }

        std::sort(zcurve.begin(), zcurve.end());

        for (size_t i = 0; i < n-1; i++) {
            size_t cur = i;
            size_t next = zcurve[cur].second;

            while (next != i) {
                std::swap(data.posx()[cur], data.posx()[next]);
                std::swap(data.posy()[cur], data.posy()[next]);
                std::swap(data.velx()[cur], data.velx()[next]);
                std::swap(data.vely()[cur], data.vely()[next]);
                std::swap(data.mass()[cur], data.mass()[next]);
                zcurve[cur].second = cur;
                cur = next;
                next = zcurve[cur].second;
            }
            zcurve[cur].second = cur;
        }
    }

    public:
    void update(data& data) noexcept {
        // sort based on zcurve
        sort(data);
    }
        
};
