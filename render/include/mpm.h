#pragma once

#include <glad/glad.h>  // must appear before cuda_gl_interop.h
#include <GLFW/glfw3.h> // must appear before cuda_gl_interop.h
#include <cuda_runtime.h>
#include <cuda_gl_interop.h>

#include "helper_cuda.h"

#include <string>

// exposed to main.cpp, renderer needs them
extern bool use_cuda_gl_interop;
 extern cudaGraphicsResource_t cu_x_handle;
extern int n_particles;
extern float* x;
extern float* color;
// we may use a struct to make it elegant

void mpm_load_config(const std::string& cfg_fpath);

void mpm_initialize();

void mpm_step();

void mpm_print_profiler();

void mpm_end();
