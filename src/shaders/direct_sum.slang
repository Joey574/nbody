static const uint BLOCK_SIZE = 128;

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

[shader("compute")]
[numthreads(BLOCK_SIZE, 1, 1)]
void computeMain(
    uint3 dispatchID : SV_DispatchThreadID,
    uint3 groupThreadID : SV_GroupThreadID
) {
    const uint i = dispatchID.x;
    const uint tid = groupThreadID.x;

    const bool active = i < params.samples;

    float2 pos = float2(0.0, 0.0);
    float2 vel = float2(0.0, 0.0);

    if (active) {
        ParticleState p = inputState[i];
        pos = float2(p.x, p.y);
        vel = float2(p.vx, p.vy);
    }

    float2 acc = float2(0.0, 0.0);

    for (uint base = 0; base < params.samples; base += BLOCK_SIZE) {
        const uint j = base+tid;

        if (j < params.samples) {
            sharedPositions[tid] = float2(inputState[j].x, inputState[j].y);
            sharedMasses[tid] = masses[j];
        } else {
            sharedPositions[tid] = float2(0.0, 0.0);
            sharedMasses[tid] = 0.0;
        }

        // sync load
        GroupMemoryBarrierWithGroupSync();

        if (active) {
            const uint tileCount = min(BLOCK_SIZE, params.samples - base);

            for (uint k = 0; k < tileCount; k++) {
                const uint other = base + k;
                if (other == i) {
                    continue;
                }

                const float2 delta = sharedPositions[k] - pos;
                const float dsq = dot(delta, delta);

                const float inv = rsqrt(dsq);
                const float inv3 = inv*inv*inv;

                acc += delta * sharedMasses[k] * inv3;
            }
        }
    }

    // sync acc compute
    GroupMemoryBarrierWithGroupSync();

    if (!active) {
        return;
    }

    vel += acc * params.dt;
    pos += vel * params.dt;

    ParticleState r;
    r.x = pos.x;
    r.y = pos.y;
    r.vx = vel.x;
    r.vy = vel.y;
    outputState[i] = r;
}
