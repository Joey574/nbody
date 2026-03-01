static float2 positions[3] = float2[](
    float2(0.0, -0.5),
    float2(0.5, 0.5),
    float2(-0.5, 0.5)
);

static float3 colors[3] = float3[](
    float3(1.0, 0.0, 0.0),
    float3(0.0, 1.0, 0.0),
    float3(0.0, 0.0, 1.0)
);

static const float2 center = (positions[0] + positions[1] + positions[2]) / 3.0f;
static const float radius = 0.25f;

struct vertexOutput {
    float3 color;
    float4 sv_position : SV_Position;
    float2 ndcPos      : TEXCOORD0;
};

[shader("vertex")]
vertexOutput vertMain(uint vid : SV_VertexID) {
    vertexOutput output;
    output.sv_position = float4(positions[vid], 0.0, 1.0);
    output.ndcPos = positions[vid];
    output.color = colors[vid];
    return output;
}

[shader("fragment")]
float4 fragMain(vertexOutput inVert) : SV_Target {
    float2 pos = inVert.ndcPos;
    clip(radius - distance(center, pos));

    return float4(inVert.color, 1.0f);
}
