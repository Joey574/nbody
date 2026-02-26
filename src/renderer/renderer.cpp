module;
#include <chrono>
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <ranges>
#include <algorithm>

#include "../dependencies/dependencies.hpp"
#include "../definitions/definitions.hpp"
module renderer;
import simulation;

std::chrono::nanoseconds renderer::render(const simulation& sim) {
    auto s = std::chrono::high_resolution_clock::now();
    return std::chrono::high_resolution_clock::now() - s;
}
