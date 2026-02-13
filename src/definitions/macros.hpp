#pragma once

/*

Defines helper macros and definitions to be used for compilation

USE_AVX512 : used in other parts of the project to enable custom avx512 simd
USE_AVX2 : used in other parts of the project to enable custom avx2 simd
MEM_ALIGNMENT : dictates the parameter used for aligned_alloc

*/

#ifdef __AVX512__
#define USE_AVX512
#endif

#ifdef __AVX2__
#define USE_AVX2
#endif

#ifdef USE_AVX512
#define MEM_ALIGNMENT 64
#elifdef USE_AVX2
#define MEM_ALIGNMENT 32
#else
#define MEM_ALIGNMENT 16
#endif
