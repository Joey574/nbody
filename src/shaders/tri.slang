struct VSIn {
    float2 inPos;
};

struct VSOut {
    float4 pos : SV_Position;
};

[shader("vertex")]
VSOut vertMain(VSIn input) {
    VSOut output;
    output.pos = float4(input.inPos, 0.0, 1.0);
    return output;
}

[shader("fragment")]
float4 fragMain(VSOut inVert) : SV_Target {
    //float2 pos = inVert.ndcPos;
    //clip(radius - distance(center, pos));

    return float4(1.0, 1.0f, 1.0f, 1.0f);
}
