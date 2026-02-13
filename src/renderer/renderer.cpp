#include "renderer.hpp"

std::chrono::nanoseconds renderer::render(const simulation& sim) {
    auto s = std::chrono::high_resolution_clock::now();
    return std::chrono::high_resolution_clock::now() - s;
}
