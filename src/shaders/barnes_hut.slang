/* Resources:
 * https://edwardsjohnmartin.github.io/publications/papers/morrical2017parallel.pdf
 *  Priority: HIGH
 *  Subject: Parallel quadtree construction on collections of objects
 *  Year: 2017
 *  Comment: First article to read, it is explicitly building the kind of tree we are also aiming for
 *      make sure to look at references 10, 2, 11, paper describes them as 'point quad-trees'

 * https://ajdillhoff.github.io/notes/parallel_sorting_algorithms/
 *  Priority: LOW
 *  Subject: Parallel Sorting Algorithms
 *  Year: 2024
 *  Comment: Simple blog post on parallel sorting, will be useful later down the road for sorting morton codes, not needed elsewise

 * https://github.com/b0nes164/GPUSorting
 *  Priority: MED
 *  Subject: GPUSorting
 *  Year: 2024
 *  Comment: Implements 'OneSweep' a fast one pass gpu sorting algorithm in hlsl for unity

 * https://arxiv.org/pdf/2206.01784
 *  Priority: MED
 *  Subject: OneSweep
 *  Year: 2022
 *  Comment: Accompanying paper, provides academic reasoning behind the OneSweep algorithm

 * https://dl.acm.org/doi/10.1145/3079079.3079105
 *  Priority: LOW
 *  Subject: Fast segmented sort on GPUs
 *  Year: 2017

 * https://link.springer.com/chapter/10.1007/978-3-031-39698-4_45
 *  Priority: MED
 *  Subject: Faster Segmented Sort on GPUs
 *  Year: 2023

 * https://dl.acm.org/doi/10.1145/2304576.2304621
 *  Priority: LOW
 *  Subject: GPU merge path: a GPU merging algorithm
 *  Year: 2012

 * https://www.sciencedirect.com/science/article/abs/pii/S0167819121001289?via=ihub
 *  Priority: MED
 *  Subject: An evaluation of fast segmented sorting implementations on GPUs
 *  Year: 2021

 * https://moderngpu.github.io/segsort.html
 *  Priority: LOW
 *  Subject: Segmented Sort and Locality Sort
 *  Year: 2013

 * https://dl.acm.org/doi/10.1145/3016078.2851169
 *  Priority: LOW
 *  Subject: GPU multisplit
 *  Year: 2016

 * https://xi.zulipchat.com/#narrow/stream/197075-gpu/topic/A.20Better.20Sort.20for.20Sparse.20Strip.20Rendering
 *  Priority: LOW
 *  Subject: A better Sort for Sparce Strip Rendering
 *  Year: 2024
 *  Comment: A forum page discussing fast gpu algorithms

 * https://developer.nvidia.com/blog/thinking-parallel-part-i-collision-detection-gpu/
 *  Priority: COMPLETE
 *  Subject: Thinking Parallel
 *  Year: 2012
 *  Comment: Part 3 is the most directly relatable to this algorithm as it discusses parallel BVH creation
*/

static const uint BLOCK_SIZE = 256;
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

[[vk::binding(3, 0)]]
RWStructuredBuffer<float> mortonCodes;

[[vk::push_constant]]
SimulationParams params;

groupshared float2 sharedPositions[BLOCK_SIZE];
groupshared float sharedMasses[BLOCK_SIZE];

// Interleaves bits of input
uint expandBits2D(uint v) {
    v = (v | (v << 8)) & 0x00FF00FFu;
    v = (v | (v << 4)) & 0x0F0F0F0Fu;
    v = (v | (v << 2)) & 0x33333333u;
    v = (v | (v << 1)) & 0x55555555u;
    return v;
}

// Computes the 2d morton code for the given coords
uint morton2D(float2 p) {
    float x = min(max(p.x * 65536.0f, 0.0f), 65535.0f);
    float y = min(max(p.y * 65536.0f, 0.0f), 65535.0f);
    uint xx = expandBits2D((uint)x);
    uint yy = expandBits2D((uint)y);
    return (xx << 1) | yy;
}

[shader("compute")]
[numthreads(BLOCK_SIZE, 1, 1)]
void broadMain(
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

    // TODO
    // currently unsure about how morton codes should be stored
    // we will need a vk binding as we must hold information about
    // an entire sorted list of morton codes
    // however, it may be preferred to have a shared morton segment
    // for the workgroup to read / write from, i believe it largely
    // depends on what we end up having to do with the morton codes

    for (uint base = 0; base < params.samples; base += BLOCK_SIZE) {
        const uint j = base+tid;

        if (j < params.samples) {
            sharedPositions[tid] = float2(inputState[j].x, inputState[j].y);
            mortonCodes[j] = morton2D(pos);
            sharedMasses[tid] = masses[j];
        } else {
            sharedPositions[tid] = float2(0.0, 0.0);
            mortonCodes[j] = 0.0;
            sharedMasses[tid] = 0.0;
        }

        // sync load
        GroupMemoryBarrierWithGroupSync();
    }
}
