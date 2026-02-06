#pragma once

#if defined(__AVX512__)
#define MEM_ALIGNMENT 64
#elif defined(__AVX2__)
#define MEM_ALIGNMENT 32
#else
#define MEM_ALIGNMENT 32
#endif