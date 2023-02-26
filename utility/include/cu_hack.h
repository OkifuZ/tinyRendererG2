#pragma once

// what we need for CUDA

#include <cuda_runtime.h>
#include "helper_cuda.h"

// Visual Studio Intellisense has conflict with CUDA kernel calls of format <<<x,y,z,w>>>
// a macro may help getting rid of annoying red lines
// may refer: https://stackoverflow.com/questions/6061565/setting-up-visual-studio-intellisense-for-cuda-kernel-calls

// Also, some intrisic functions of cuda(eg, math API, atomicAdd() ...) is not supported by Intellisense
// another hack is needed
// may refer: https://stackoverflow.com/questions/46289253/cuda-intrinsic-functions-are-undefined-according-to-visual-studios-intellisense

// Sure you could not use these ugly hacks, no matter with building 

#ifdef __INTELLISENSE__ // we fool VS intellisence here
#include "intellisense_cuda_intrinsics.h"
#define KERNEL_ARGS2(grid, block)
#define KERNEL_ARGS3(grid, block, sh_mem)
#define KERNEL_ARGS4(grid, block, sh_mem, stream)
#else // but we are honest to nvcc
#define KERNEL_ARGS2(grid, block) <<< grid, block >>>
#define KERNEL_ARGS3(grid, block, sh_mem) <<< grid, block, sh_mem >>>
#define KERNEL_ARGS4(grid, block, sh_mem, stream) <<< grid, block, sh_mem, stream >>>
#endif

