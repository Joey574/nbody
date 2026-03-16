struct UBO {
    float4x4 view;
    float4x4 proj;
};

[[vk::binding(0, 0)]] StructuredBuffer<float> xb;
[[vk::binding(1, 0)]] StructuredBuffer<float> yb;
[[vk::binding(2, 0)]] StructuredBuffer<float> rb;
[[vk::binding(3, 0)]] ConstantBuffer<UBO> ubo;

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
VSOut vertMain(uint vID: SV_VertexID, uint iID: SV_InstanceID) {
    float2 offset = QUAD_OFFSETS[vID];

    float zoom = ubo.view[0][0];
    float cx = xb[iID];
    float cy = yb[iID];
    float r = log(rb[iID]+1);

    float4x4 mvp = mul(ubo.proj, ubo.view);

    float4 clipCenter = mul(mvp, float4(cx, cy, 0.0, 1.0));
    clipCenter.x += offset.x * r * zoom * 2.0 / pc.width;
    clipCenter.y += offset.y * r * zoom * 2.0 / pc.height;

    VSOut vsOut;
    vsOut.position = clipCenter;
    vsOut.offset = offset;
    vsOut.radius = r;
    return vsOut;
}

[shader("fragment")]
float4 fragMain(VSOut fsIn) : SV_TARGET {
    float dist = length(fsIn.offset);
    if (dist > 1.0) { discard; }

    float alpha = 1.0 - smoothstep(1.0 - pc.softness, 1.0, dist);
    return float4(pc.color.rgb, pc.color.a * alpha);
}
