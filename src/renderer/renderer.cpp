module;
#include <chrono>
#include <vector>
#include <fstream>

module renderer;
import simulation;

std::chrono::nanoseconds renderer::render(const simulation& sim) {
    auto s = std::chrono::high_resolution_clock::now();
    return std::chrono::high_resolution_clock::now() - s;
}

std::vector<char> renderer::readFile(const std::string& path) {
    std::ifstream file(path, std::ios::ate | std::ios::binary);
    if (!file.is_open()) {
        std::__throw_runtime_error(("failed to open file: " + path).c_str());
    }

    std::vector<char> buffer(file.tellg());
    file.seekg(0, std::ios::beg);
    file.read(buffer.data(), static_cast<std::streamsize>(buffer.size()));
    file.close();

    return buffer;
}
