#pragma once

/*

Defines helper macros and definitions to be used for compilation

MEM_ALIGNMENT : dictates the parameter used for aligned_alloc
*/

#ifdef __AVX512F__
#define MEM_ALIGNMENT 64
#elif defined(__AVX2__)
#define MEM_ALIGNMENT 32
#else
#define MEM_ALIGNMENT 32
#endif
