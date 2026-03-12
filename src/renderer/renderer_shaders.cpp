#include "renderer.hpp"

const unsigned char renderer::shader_bytes[] = { 
    #embed "../shaders/tri.spv" 
};

const size_t renderer::shader_size = sizeof(renderer::shader_bytes);