#pragma once
#include <glm/glm.hpp>

struct CircleData {
    glm::vec2 pos;
    alignas(16) glm::vec3 color; 
};

struct CircleSSBO {
    float radius;
    CircleData circles[]; 
};
