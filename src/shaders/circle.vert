#version 450

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec2 inUV;

layout(location = 0) out vec2 fragUV;
layout(location = 1) out flat int instanceID;

struct CircleData {
    vec2 pos;
    vec3 color;
};

layout(std140, binding = 0) readonly buffer CircleBuffer {
    float radius;
    CircleData circles[];
} data;

void main() {
    CircleData c = data.circles[gl_InstanceIndex];
    
    // Scale by radius, then translate by position
    gl_Position = vec4(inPosition.xy * data.radius + c.pos, 0.0, 1.0);
    
    fragUV = inUV;
    instanceID = gl_InstanceIndex;
}