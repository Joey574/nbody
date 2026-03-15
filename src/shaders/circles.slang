struct UBO {
    float4x4 view;
    float4x4 proj;
};

[[vk::binding(0, 0)]] StructuredBuffer<float> x;
[[vk::binding(1, 0)]] StructuredBuffer<float> y;
[[vk::binding(2, 0)]] StructuredBuffer<float> r;
[[vk::binding(3, 0)]] ConstantBuffer<UBO> ubo;

struct VSOut {
    float4 position : SV_Position;
    float2 offset   : TEXCOORD0;
    float radius    : TEXCOORD1;
};

static const float2 QUAD_OFFSETS[6] = {
    float2(-1, -1),
    float2(1, -1),
    float2(-1, 1),
    float2(-1, 1),
    float2(1, -1),
    float2(1, 1),
};

struct PushConstants {
    float4 color;
    float softness;
}

[[vk::push_constant]] PushConstants pc;

[shader("vertex")]
VSOut vertMain(uint vID: SV_VertexID, uint iID: SV_InstanceID) {
    float2 offset = QUAD_OFFSETS[vID];

    float cx = x[iID];
    float cy = x[iID];
    // float r = r[iID];
    float r = 100.0f;

    float2 wPos = float2(cx, cy) + offset * r;
    float4x4 mvp = mul(ubo.proj, ubo.view);

    VSOut out;
    out.position = mul(mvp, float4(wPos, 0.0, 1.0));
    out.offset = offset;
    out.radius = r;
    return out;
}

[shader("fragment")]
float4 fragMain(VSOut in) : SV_TARGET {
    float dist = length(in.offset);

    if (dist > 1.0) {
        discard;
    }

    float alpha = 1.0 - smoothstep(1.0 - pc.softness, 1.0, dist);
    //return float4(pc.color.rgb, pc.color.a * alpha);
    return float4(1.0f, 1.0f, 1.0f, 1.0f);
}
