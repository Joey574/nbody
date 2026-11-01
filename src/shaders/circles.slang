struct ParticleState {
    float x;
    float y;
    float vx;
    float vy;
};

struct UBO {
    float4x4 view;
    float4x4 proj;
};

[[vk::binding(0, 0)]] StructuredBuffer<ParticleState> particles;
[[vk::binding(1, 0)]] StructuredBuffer<float> masses;
[[vk::binding(2, 0)]] ConstantBuffer<UBO> ubo;

struct VSOut {
    float4 position : SV_Position;
    float2 offset   : TEXCOORD0;
    float radius    : TEXCOORD1;
};

static const float2 QUAD_OFFSETS[3] = {
    float2(-1, -1),
    float2(3, -1),
    float2(-1, 3),
};

struct PushConstants {
    float4 color;
    float softness;
    float width;
    float height;
};

[[vk::push_constant]] PushConstants pc;

[shader("vertex")]
VSOut vertMain(uint vID : SV_VertexID, uint iID : SV_InstanceID) {
    const float2 offset = QUAD_OFFSETS[vID];
    const ParticleState particle = particles[iID];

    const float zoom = ubo.view[0][0];
    const float radius = clamp(log(masses[iID] + 1.0), 1.0, 10.0);
    const float4x4 mvp = mul(ubo.proj, ubo.view);

    float4 clipCenter = mul(
        mvp,
        float4(particle.x, particle.y, 0.0, 1.0)
    );
    clipCenter.x += offset.x * radius * zoom * 2.0 / pc.width;
    clipCenter.y += offset.y * radius * zoom * 2.0 / pc.height;

    VSOut result;
    result.position = clipCenter;
    result.offset = offset;
    result.radius = radius;
    return result;
}

[shader("fragment")]
float4 fragMain(VSOut fsIn) : SV_TARGET {
    const float dist = length(fsIn.offset);
    if (dist > 1.0) {
        discard;
    }

    const float alpha = 1.0 - smoothstep(1.0 - pc.softness, 1.0, dist);
    return float4(pc.color.rgb, pc.color.a * alpha);
}
