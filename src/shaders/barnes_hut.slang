static const uint BLOCK_SIZE = 128;
static const float epsl = 1e-12f;

struct ParticleState {
    float x;
    float y;
    float vx;
    float vy;
};

struct SimulationParams {
    uint samples;
    float dt;
};

[[vk::binding(0, 0)]]
StructuredBuffer<ParticleState> inputState;

[[vk::binding(1, 0)]]
RWStructuredBuffer<ParticleState> outputState;

[[vk::binding(2, 0)]]
StructuredBuffer<float> masses;

[[vk::push_constant]]
SimulationParams params;

groupshared float2 sharedPositions[BLOCK_SIZE];
groupshared float sharedMasses[BLOCK_SIZE];
groupshared uint sharedMorton[BLOCK_SIZE];

uint expandBits2D(uint v) {
    v = (v | (v << 8)) & 0x00FF00FFu;
    v = (v | (v << 4)) & 0x0F0F0F0Fu;
    v = (v | (v << 2)) & 0x33333333u;
    v = (v | (v << 1)) & 0x55555555u;
    return v;
}

uint morton2D(float x, float y) {
    x = min(max(x * 65536.0f, 0.0f), 65535.0f);
    y = min(max(y * 65536.0f, 0.0f), 65535.0f);
    uint xx = expandBits2D((uint)x);
    uint yy = expandBits2D((uint)y);
    return (xx << 1) | yy;
}

[shader("compute")]
[numthreads(BLOCK_SIZE, 1, 1)]
void bhMain(
    uint3 dispatchID : SV_DispatchThreadID,
    uint3 groupThreadID : SV_GroupThreadID
) {
    const uint i = dispatchID.x;
    const uint tid = groupThreadID.x;

    const bool active = i < params.samples;

    for (uint base = 0; base < params.samples; base += BLOCK_SIZE) {
        const uint j = base+tid;

        if (j < params.samples) {
            sharedPositions[tid] = float2(inputState[j].x, inputState[j].y);
            sharedMorton[tid] = morton2D(inputState[j].x, inputState[j].y);
            sharedMasses[tid] = masses[j];
        } else {
            sharedPositions[tid] = float2(0.0, 0.0);
            sharedMorton[tid] = 0;
            sharedMasses[tid] = 0.0;
        }

        // sync load
        GroupMemoryBarrierWithGroupSync();
    }
}
