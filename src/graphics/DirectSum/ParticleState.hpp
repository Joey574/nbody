#pragma once
#include <cstddef>

struct alignas(16) ParticleState {
    float x;
    float y;
    float vx;
    float vy;
};

static_assert(sizeof(ParticleState) == 16);
static_assert(alignof(ParticleState) == 16);
static_assert(offsetof(ParticleState, x) == 0);
static_assert(offsetof(ParticleState, y) == 4);
static_assert(offsetof(ParticleState, vx) == 8);
static_assert(offsetof(ParticleState, vy) == 12);
