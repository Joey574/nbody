#version 450

layout(location = 0) in vec2 fragUV;
layout(location = 1) in flat int instanceID;

layout(location = 0) out vec4 outColor;

struct CircleData {
    vec2 pos;
    vec3 color;
};

layout(std140, binding = 0) readonly buffer CircleBuffer {
    float radius;
    CircleData circles[];
} data;

void main() {
    if (dot(fragUV - vec2(0.5)) > 0.25) {
        discard;
    }

    outColor = vec4(data.circles[instanceID].color, 1.0);
}
