struct VSIn {
    float2 pos;
    float3 color;
};

struct VSOut {
    float4 pos : SV_Position;
    float3 color;
};

[shader("vertex")]
VSOut vertMain(VSIn input) {
    VSOut output;
    output.pos = float4(input.pos, 0.0, 1.0);
    output.color = input.color;
    return output;
}

[shader("fragment")]
float4 fragMain(VSOut vertIn) : SV_TARGET {
    return float4(vertIn.color, 1.0);
}
