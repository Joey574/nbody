#include "renderer.hpp"

constexpr const unsigned char renderer::shader_bytes[] = {
    #embed "../shaders/compiled/circles.spv"
};
constexpr const size_t renderer::shader_size = sizeof(shader_bytes);
