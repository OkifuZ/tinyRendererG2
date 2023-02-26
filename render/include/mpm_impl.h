#pragma once

// GPU related
// ----------------------------------------------------------------------------

void cu_initialize(float** x_device, float** v_device,
	float** C_device, float** F_device, int** material_device, float** Jp_device, float** color_device,
	unsigned int n_particles,
	float** grid_v_device, float** grid_m_device, unsigned int n_grid);

void cu_inigrid_substep(float* grid_v, float* grid_m, unsigned int n_grid);

void cu_p2g_substep(
	float* x_device, float* v_device, float* F_device, float* C_device, int* material_device, float* Jp_device, unsigned int n_particles,
	float* grid_v_device, float* grid_m_device, unsigned int n_grid,
	float dx, float inv_dx, float dt, float mu_0, float lambda_0, float p_vol, float p_mass);

void cu_boundary_substep(float* grid_v_device, float* grid_m_device, unsigned int n_grid,
	float dt, float gravity);

void cu_g2p_substep(float* x, float* v, float* C, unsigned int n_particles,
	float* grid_v, float* grid_m, unsigned int n_grid,
	float dt, float inv_dx);

// copy color buffer from device to host
void copy_color_from_gpu(float* color_device, float* color_host, unsigned int n_particles);


// CPU related
// ----------------------------------------------------------------------------

void initialize(float* x, float* v, float* F, float* Jp, int* material, float* color, unsigned int n_particles);

void inigrid_substep(float* grid_v, float* grid_m, unsigned int n_grid);

void p2g_substep(
	float* x, float* v, float* F, float* C, int* material, float* Jp, unsigned int n_particles,
	float* grid_v, float* grid_m, unsigned int n_grid,
	float dx, float inv_dx, float dt, float mu_0, float lambda_0, float p_vol, float p_mass);

void boundary_substep(float* grid_v, float* grid_m, unsigned int n_grid,
	float dt, float gravity);

void g2p_substep(float* x, float* v, float* C, unsigned int n_particles,
	float* grid_v, float* grid_m, unsigned int n_grid,
	float dt, float inv_dx);


// Auxiliary (debug)
// ----------------------------------------------------------------------------

void arrcopy_gpu_to_cpu(float* x_device, float* v_device, float* C_device, float* F_device, int* material_device, float* Jp_device,
	float* x_host, float* v_host, float* C_host, float* F_host, int* material_host, float* Jp_host, unsigned int n_particles,
	float* grid_v_device, float* grid_m_device, float* grid_v_host, float* grid_m_host, unsigned int n_grid);

void arrcopy_cpu_to_gpu(float* x_device, float* v_device, float* C_device, float* F_device, int* material_device, float* Jp_device,
	float* x_host, float* v_host, float* C_host, float* F_host, int* material_host, float* Jp_host, unsigned int n_particles,
	float* grid_v_device, float* grid_m_device, float* grid_v_host, float* grid_m_host, unsigned int n_grid);

void copy_x_from_gpu(float* x_device, float* x_host, unsigned int n_particles);


void sync_all();
