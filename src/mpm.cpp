#include "mpm.h" // implement this header
#include "mpm_impl.h" // rely on this header
#include "ticktock.h"
#include "json11.hpp"

#include <Eigen/Dense>

#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <cmath>
#include <algorithm>
#include <vector>

// ****************************************************************************
bool use_cuda_gl_interop = true;
bool use_gpu = true;

int n_particles = 75000; // exposed to main.cpp
int n_grid = 416;

// gpu resources
cudaGraphicsResource_t cu_x_handle = nullptr; // same as: struct cudaGraphicsResource* cu_x_handle
float* x_cu = nullptr;
float* v_cu = nullptr;
float* C_cu = nullptr;
float* F_cu = nullptr;
int* material_cu = nullptr;
float* Jp_cu = nullptr;
float* color; // color buffer

float* grid_v_cu = nullptr;
float* grid_m_cu = nullptr;

// cpu resources
float* x = nullptr; // exposed to main.cpp
float* v = nullptr;
float* C = nullptr;
float* F = nullptr;
int* material = nullptr;
float* Jp = nullptr;
float* color_cu; // color buffer

float* grid_v;
float* grid_m;

// physical parameters
unsigned int substep_num = 200;
float dx = 1.0f / n_grid;
float inv_dx = n_grid;
float dt = 2e-5;
float p_vol = (dx * 0.5f) * (dx * 0.5f);
float p_rho = 1.0f;
float p_mass = p_vol * p_rho;
float E = 0.1e4;
float nu = 0.2f;
float mu_0 = E / (2.0f * (1.0f + nu));
float lambda_0 = E * nu / ((1.0f + nu) * (1.0f - 2.0f * nu));
float gravity = 10.0f;

// profiler
int substep_cnt = 0;
int frame_cnt = 0;
double time_INIT = 0.0;
double time_P2G = 0.0;
double time_BDR = 0.0;
double time_G2P = 0.0;
double time_COPY = 0.0;
// ****************************************************************************


void mpm_load_config(const std::string& cfg_fpath) {
	// read config file & initialize
	auto read_file_as_str_lambda = [](const std::string& fna) -> std::string {
		std::ifstream t(fna);
		std::stringstream buffer;
		buffer << t.rdbuf();
		return buffer.str();
	};
	try {
		std::string config_string = read_file_as_str_lambda(cfg_fpath);
		std::string err;
		const auto json_cfg = json11::Json::parse(config_string, err, json11::JsonParse::COMMENTS);
		if (err != "") throw std::exception();
		n_particles = json_cfg["n_particles"].int_value();
		n_grid = json_cfg["n_grid"].int_value();
		substep_num = json_cfg["substep_num"].int_value();
		dt = static_cast<float>(json_cfg["dt"].number_value());
		p_rho = static_cast<float>(json_cfg["p_rho"].number_value());
		E = static_cast<float>(json_cfg["E"].number_value());
		nu = static_cast<float>(json_cfg["nu"].number_value());
		gravity = static_cast<float>(json_cfg["gravity"].number_value());
		use_cuda_gl_interop = json_cfg["use_cuda_gl_interop"].bool_value();
		use_gpu = json_cfg["use_gpu"].bool_value();
		dx = 1.0f / n_grid;
		inv_dx = n_grid;
		p_vol = (dx * 0.5f) * (dx * 0.5f);
		p_mass = p_vol * p_rho;
		mu_0 = E / (2.0f * (1.0f + nu));
		lambda_0 = E * nu / ((1.0f + nu) * (1.0f - 2.0f * nu));
	}
	catch (const std::exception& e) { // not a big deal~
		printf("FAILED when loading mpm_config file, using default parameters\n");
	}

	if (use_cuda_gl_interop && !use_gpu) {
		printf("ERROR: in mpm_load_config(), assert((use_cuda_gl_interop && !use_gpu) != true) failed\n");
		exit(-1);
	}
	if (!use_cuda_gl_interop) {
		x = new float[n_particles * 2]();
	}
	if (!use_gpu) {
		v = new float[n_particles * 2]();
		C = new float[n_particles * 4]();
		F = new float[n_particles * 4]();
		material = new int[n_particles]();
		Jp = new float[n_particles]();
		grid_v = new float[n_grid * n_grid * 2]();
		grid_m = new float[n_grid * n_grid]();
	}
	color = new float[n_particles * 3]();
}

void mpm_initialize() {
	if (use_gpu) {
		if (use_cuda_gl_interop) {
			checkCudaErrors(cudaGraphicsMapResources(1, &cu_x_handle));
			size_t length;
			checkCudaErrors(
				cudaGraphicsResourceGetMappedPointer((void**)&x_cu, &length, cu_x_handle));
		}
		cu_initialize(&x_cu, &v_cu, &C_cu, &F_cu, &material_cu, &Jp_cu, &color_cu, n_particles, &grid_v_cu, &grid_m_cu, n_grid);
		copy_color_from_gpu(color_cu, color, n_particles);
		if (use_cuda_gl_interop) {
			checkCudaErrors(cudaGraphicsUnmapResources(1, &cu_x_handle));
		}
	}
	else {
		initialize(x, v, F, Jp, material, color, n_particles);
	}
}



void _substep() {
	if (use_gpu) {
		// init
		TICK(INIT);
		cu_inigrid_substep(grid_v_cu, grid_m_cu, n_grid);
		time_INIT += TOCK_VAL(INIT);

		//P2G
		TICK(P2G);
		cu_p2g_substep(x_cu, v_cu, F_cu, C_cu, material_cu, Jp_cu, n_particles,
			grid_v_cu, grid_m_cu, n_grid,
			dx, inv_dx, dt, mu_0, lambda_0, p_vol, p_mass);

		time_P2G += TOCK_VAL(P2G);

		// boundary condition
		TICK(BOUNDARY);
		cu_boundary_substep(grid_v_cu, grid_m_cu, n_grid, dt, gravity);
		time_BDR += TOCK_VAL(BOUNDARY);

		// G2P
		TICK(G2P);
		cu_g2p_substep(x_cu, v_cu, C_cu, n_particles, grid_v_cu, grid_m_cu, n_grid, dt, inv_dx);
		time_G2P += TOCK_VAL(G2P);
	}
	else {
		TICK(INIT);
		inigrid_substep(grid_v, grid_m, n_grid);
		time_INIT += TOCK_VAL(INIT);

		//P2G
		TICK(P2G);
		p2g_substep(x, v, F, C, material, Jp, n_particles,
			grid_v, grid_m, n_grid,
			dx, inv_dx, dt, mu_0, lambda_0, p_vol, p_mass);
		time_P2G += TOCK_VAL(P2G);

		// boundary condition
		TICK(BOUNDARY);
		boundary_substep(grid_v, grid_m, n_grid, dt, gravity);
		time_BDR += TOCK_VAL(BOUNDARY);

		// G2P
		TICK(G2P);
		g2p_substep(x, v, C, n_particles, grid_v, grid_m, n_grid, dt, inv_dx);
		time_G2P += TOCK_VAL(G2P);
	}
	
	substep_cnt++;
}



void mpm_step() {
	if (use_gpu) {
		if (use_cuda_gl_interop) {
			TICK(COPY);
			checkCudaErrors(cudaGraphicsMapResources(1, &cu_x_handle));
			size_t length = n_particles * 8;
			checkCudaErrors(
				cudaGraphicsResourceGetMappedPointer((void**)&x_cu, &length, cu_x_handle));
			time_COPY += TOCK_VAL(COPY);
		}
		for (int i = 0; i < substep_num; i++) {
			_substep();
		}
		if (use_cuda_gl_interop) {
			TICK(COPY);
			checkCudaErrors(cudaGraphicsUnmapResources(1, &cu_x_handle));
			time_COPY += TOCK_VAL(COPY);
		}
		else {
			TICK(COPY);
			copy_x_from_gpu(x_cu, x, n_particles);
			time_COPY += TOCK_VAL(COPY);
		}
	}
	else {
		for (int i = 0; i < substep_num; i++) {
			_substep();
		}
	}

	frame_cnt += 1;
}


void mpm_print_profiler() {
	if (substep_cnt == 0) {
		time_INIT = -1.0;
 		time_P2G = -1.0;
		time_BDR = -1.0;
		time_G2P = -1.0;
		time_COPY = -1.0;
	}
	else {
		time_INIT = time_INIT / (double)substep_cnt * substep_num;
		time_P2G = time_P2G / (double)substep_cnt * substep_num;
		time_BDR = time_BDR / (double)substep_cnt * substep_num;
		time_G2P = time_G2P / (double)substep_cnt * substep_num;
		time_COPY = time_COPY / (double)frame_cnt;
	}

	printf("--MPM per-frame profiler(average)--\n");
	printf("-init:       %.6f\n", time_INIT);
	printf("-p2g:        %.6f\n", time_P2G);
	printf("-boundary    %.6f\n", time_BDR);
	printf("-g2p:        %.6f\n", time_G2P);
	printf("total time:  %.6f\n", std::max(time_INIT + time_P2G + time_BDR + time_G2P, -1.0));
	printf("copy:        %.6f\n", time_COPY);
	printf("-----------------------------------\n");
}

void mpm_end() {
	if (!use_cuda_gl_interop) {
		delete[] x;
	}
	if (!use_gpu) {
		delete[] v;
		delete[] C;
		delete[] F;
		delete[] material;
		delete[] Jp;
		delete[] grid_v;
		delete[] grid_m;
	}
	delete[] color;
}