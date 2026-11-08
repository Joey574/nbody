static const float MIN_SUBPIXEL_ALPHA = 1.0 / 384.0;
static const float AA_WIDTH_PX = 1.0;
static const float SUBPIXEL_GAIN = 0.85;

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
    float4 position  : SV_Position;
    float2 offsetPx  : TEXCOORD0;
    float radiusPx   : TEXCOORD1;
    float alphaScale : TEXCOORD2;
};

static const float2 QUAD_OFFSETS[3] = {
    float2(-1, -1),
    float2(3, -1),
    float2(-1, 3),
};

struct PushConstants {
    float4 color;
    float minRadiusPx;
    float width;
    float height;
};

[[vk::push_constant]] PushConstants pc;

[shader("vertex")]
VSOut vertMain(uint vID : SV_VertexID, uint iID : SV_InstanceID) {
    const float2 offset = QUAD_OFFSETS[vID];
    const ParticleState particle = particles[iID];

    const float zoom = abs(ubo.view[0][0]);
    const float worldRadius = clamp(log(masses[iID] + 1.0), 1.0, 10.0);

    const float screenRadiusPx = worldRadius * zoom;
    const float shapeRadiusPx = max(screenRadiusPx, pc.minRadiusPx);

    const float footprintPx = shapeRadiusPx + AA_WIDTH_PX;

    const float subpixelRatio = saturate(screenRadiusPx / shapeRadiusPx);
    const float alphaScale = max(subpixelRatio * lerp(SUBPIXEL_GAIN, 1.0, subpixelRatio), MIN_SUBPIXEL_ALPHA);

    const float4x4 mvp = mul(ubo.proj, ubo.view);
    float4 clipCenter = mul(
        mvp,
        float4(particle.x, particle.y, 0.0, 1.0)
    );

    clipCenter.x += offset.x * footprintPx * 2.0 / pc.width;
    clipCenter.y += offset.y * footprintPx * 2.0 / pc.height;

    VSOut result;
    result.position = clipCenter;
    result.offsetPx = offset * footprintPx;
    result.radiusPx = shapeRadiusPx;
    result.alphaScale = alphaScale;
    return result;
}

[shader("fragment")]
float4 fragMain(VSOut fsIn) : SV_TARGET {
    const float signedDistancePx = fsIn.radiusPx - length(fsIn.offsetPx);
    if (signedDistancePx < -AA_WIDTH_PX) {
        discard;
    }

    const float edgeCoverage = smoothstep(-AA_WIDTH_PX, AA_WIDTH_PX, signedDistancePx);
    return float4(pc.color.rgb, pc.color.a * fsIn.alphaScale * edgeCoverage);
}
