#include "simulation.hpp"

std::chrono::nanoseconds simulation::update_cpu_bh(const float ft) noexcept {
    auto start = std::chrono::high_resolution_clock::now();
    
    constexpr float thetasq = 0.1f;

    // data is now sorted on a zcurve
    zcurve_.sort(data_);

    for (size_t i = 0; i < data_.bodies(); i++) {

        float tmass = 0.0f;
        float tx = 0.0f;
        float ty = 0.0f;

        float minx, miny = std::numeric_limits<float>::max();
        float maxx, maxy = std::numeric_limits<float>::min();

        // iterate until error rate is too large
        for (size_t j = 0; j < data_.bodies(); j++) {
            if (i == j) continue;

            minx = data_.posx()[j] < minx ? data_.posx()[j] : minx;
            maxx = data_.posx()[j] > maxx ? data_.posx()[j] : maxx;
            miny = data_.posy()[j] < miny ? data_.posy()[j] : miny;
            maxy = data_.posy()[j] > maxy ? data_.posy()[j] : maxy;

            tx += data_.posx()[j] * data_.mass()[j];
            ty += data_.posy()[j] * data_.mass()[j];
            tmass += data_.mass()[j];

            float cx = tx / tmass;
            float cy = ty / tmass;

            float dx = cx-data_.posx()[j];
            float dy = cy-data_.posy()[j];
            float dsq = dx*dx+dy*dy;
            float s = std::max<float>(maxx-minx, maxy-miny);
            float ssq = s*s;

            if (ssq / dsq > thetasq) {
                // need to backstep one, and treat cx, cy, and tmass as values of a single point
            } else {
                continue;
            }
        }

    }

    return std::chrono::high_resolution_clock::now() - start;
}