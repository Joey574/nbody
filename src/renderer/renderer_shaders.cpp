#include "renderer.hpp"

constexpr const unsigned char renderer::shaderBytes[] = {
    #embed "../shaders/compiled/circles.spv"
};
constexpr const size_t renderer::shaderSize = sizeof(shaderBytes);
