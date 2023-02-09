#include "mpm_impl.h" // implement this header

#include "cu_hack.h"
#include <cuda_runtime.h>
#include <device_launch_parameters.h>
#include <curand.h>
#include <curand_kernel.h>
#include "helper_cuda.h"

/*
	// following 4 headers play NICE with NVCC+MSVC
	#include "Eigen/Core"
	#include "Eigen/LU"
	#include "Eigen/Cholesky"
	#include "Eigen/QR"
	
	// following 3 headers are NOT OK with NVCC+MSVC
	// Eigen/SVD is the VILLAIN
	#include "Eigen/SVD"
	#include "Eigen/Geometry"
	#include "Eigen/Eigenvalues"

	// so do not use <Eigen/Dense> directly
*/
#include <Eigen/core>

#include <cstdio>
#include <random>



__global__ void cu_initialize_p_kernel(float* x, float* v,
	float* C, float* F, int* material, float* Jp, float* color_device, unsigned int n_particles)
{
	const unsigned long long seed = 1;
	int idx = blockDim.x * blockIdx.x + threadIdx.x;
	// this condition is VITAL! 
	if (idx >= n_particles) return;

	int group_size = n_particles / 3;

	// this kernel will only be executed once at launch, care everything but performance
	curandState state;
	curand_init(seed, idx, 0, &state);
	float px, py;
	if (idx / group_size == 0) {
		px = 0.05f + 0 * 0.3f + curand_uniform(&state) * 0.999999f * 0.25f;
		py = 0.05f + 1 * 0.3f + curand_uniform(&state) * 0.999999f * 0.25f;
		material[idx] = 2;
		color_device[idx * 3 + 0] = 1.0f; // r
		color_device[idx * 3 + 1] = 0.976f; // g
		color_device[idx * 3 + 2] = 0.976f; // b
	}
	if (idx / group_size == 1) {
		px = 0.05f + 1 * 0.3f + curand_uniform(&state) * 0.999999f * 0.25f;
		py = 0.05f + 2 * 0.3f + curand_uniform(&state) * 0.999999f * 0.25f;
		material[idx] = 0;
		color_device[idx * 3 + 0] = 0.52f; // r
		color_device[idx * 3 + 1] = 0.80f; // g
		color_device[idx * 3 + 2] = 0.976f; // b
	}
	if (idx / group_size == 2) {
		px = 0.05f + 2 * 0.3f + curand_uniform(&state) * 0.999999f * 0.25f;
		py = 0.05f + 1 * 0.3f + curand_uniform(&state) * 0.999999f * 0.25f;
		material[idx] = 1;
		color_device[idx * 3 + 0] = 0.99f; // r
		color_device[idx * 3 + 1] = 0.7f; // g
		color_device[idx * 3 + 2] = 0.2f; // b
	}
	
	x[idx * 2 + 0] = px;
	x[idx * 2 + 1] = py;
	v[idx * 2 + 0] = v[idx * 2 + 1] = 0.0f;
	F[idx * 4 + 0] = F[idx * 4 + 3] = 1.0f;
	F[idx * 4 + 1] = F[idx * 4 + 2] = 0.0f;
	C[idx * 4 + 0] = C[idx * 4 + 1] = C[idx * 4 + 2] = C[idx * 4 + 3] = 0.0f;
	Jp[idx] = 1.0f; // fixed bug: if 0.0f here, only jelly will work. Wonder WHY?
	
}

__global__ void cu_initialize_grid_kernel(float* grid_v, float* grid_m, unsigned int n_grid)
{
	int idx = blockDim.x * blockIdx.x + threadIdx.x;

	// this condition is VITAL! 
	if (idx >= n_grid * n_grid) return;

	grid_v[idx * 2 + 0] = 0.0f;
	grid_v[idx * 2 + 1] = 0.0f;
	grid_m[idx] = 0.0f;
}

void cu_initialize(float** x_device, float** v_device,
	float** C_device, float** F_device, int** material_device, float** Jp_device, float** color_device,
	unsigned int n_particles,
	float** grid_v_device, float** grid_m_device, unsigned int n_grid) 
{
	const unsigned int dim = 2;
	int n_threads = 128;

	// particles
	int n_blocks = (n_particles + n_threads - 1) / n_threads;
	if (*x_device == nullptr) {
		checkCudaErrors(cudaMalloc(x_device, sizeof(float) * n_particles * dim));
	} // else we use GL_CUDA interop
	checkCudaErrors(cudaMalloc(v_device, sizeof(float) * n_particles * dim));
	checkCudaErrors(cudaMalloc(C_device, sizeof(float) * n_particles * dim * dim));
	checkCudaErrors(cudaMalloc(F_device, sizeof(float) * n_particles * dim * dim));
	checkCudaErrors(cudaMalloc(material_device, sizeof(int) * n_particles));
	checkCudaErrors(cudaMalloc(Jp_device, sizeof(float) * n_particles));
	checkCudaErrors(cudaMalloc(color_device, sizeof(float) * n_particles * 3));
	cu_initialize_p_kernel KERNEL_ARGS2(n_blocks, n_threads)
		(*x_device, *v_device, *C_device, *F_device, *material_device, *Jp_device, *color_device, n_particles);

	// grid
	n_blocks = (n_grid * n_grid + n_threads - 1) / n_threads;
	checkCudaErrors(cudaMalloc(grid_v_device, sizeof(float) * n_grid * n_grid * dim));
	checkCudaErrors(cudaMalloc(grid_m_device, sizeof(float) * n_grid * n_grid));
	cu_initialize_grid_kernel KERNEL_ARGS2(n_blocks, n_threads) (*grid_v_device, *grid_m_device, n_grid);
}




void cu_inigrid_substep(float* grid_v_device, float* grid_m_device, unsigned int n_grid) {
	const unsigned int dim = 2;
	int n_threads = 128;

	int n_blocks = (n_grid * n_grid + n_threads - 1) / n_threads;
	cu_initialize_grid_kernel KERNEL_ARGS2(n_blocks, n_threads) (grid_v_device, grid_m_device, n_grid);
	// checkCudaErrors(cudaDeviceSynchronize());
}



__device__ void cu_svd22_raw(const float* a, float* u, float* s, float* v) {
	// const float a[4], float u[4], float s[2], float v[4]
	s[0] = (sqrtf(powf(a[0] - a[3], 2) + powf(a[1] + a[2], 2)) + 
		sqrtf(powf(a[0] + a[3], 2) + powf(a[1] - a[2], 2))) / 2.0f;
	s[1] = fabsf(s[0] - sqrtf(powf(a[0] - a[3], 2) + powf(a[1] + a[2], 2)));
	v[2] = (s[0] > s[1]) ? 
		sinf((atan2f(2 * (a[0] * a[1] + a[2] * a[3]), 
			a[0] * a[0] - a[1] * a[1] + a[2] * a[2] - a[3] * a[3])) / 2.0f) : 0.0f;
	v[0] = sqrtf(1 - v[2] * v[2]);
	v[1] = -v[2];
	v[3] = v[0];
	u[0] = (s[0] != 0) ? (a[0] * v[0] + a[1] * v[2]) / s[0] : 1.0f;
	u[2] = (s[0] != 0) ? (a[2] * v[0] + a[3] * v[2]) / s[0] : 0.0f;
	u[1] = (s[1] != 0) ? (a[0] * v[1] + a[1] * v[3]) / s[1] : -u[2];
	u[3] = (s[1] != 0) ? (a[2] * v[1] + a[3] * v[3]) / s[1] : u[0];
}




// be careful of using std::functions
__global__ void cu_p2g_substep_kernel(
	float* x, float* v, float* F, float* C, int* material, float* Jp, unsigned int n_particles,
	float* grid_v, float* grid_m, unsigned int n_grid,
	float dx, float inv_dx, float dt, float mu_0, float lambda_0, float p_vol, float p_mass )
{
	int p = blockDim.x * blockIdx.x + threadIdx.x;

	// this condition is VITAL! 
	if (p >= n_particles) return;

	Eigen::Vector2i base{ int(x[p * 2 + 0] * inv_dx - 0.5),
							  int(x[p * 2 + 1] * inv_dx - 0.5) };
	Eigen::Vector2f fx =
		Eigen::Vector2f(x[p * 2 + 0], x[p * 2 + 1]) * inv_dx - base.cast<float>();

	Eigen::Vector2f w[3]{
		0.5f * (1.5f - fx.array()) * (1.5f - fx.array()),
		0.75f - (fx.array() - 1.0f) * (fx.array() - 1.0f),
		0.5f * (fx.array() - 0.5f) * (fx.array() - 0.5f)
	};

	Eigen::Matrix2f temp_F;
	temp_F << F[p * 4 + 0], F[p * 4 + 1],
		F[p * 4 + 2], F[p * 4 + 3];

	Eigen::Matrix2f identity;
	identity.setIdentity();
	Eigen::Matrix2f temp_C;
	temp_C << C[p * 4 + 0], C[p * 4 + 1],
		C[p * 4 + 2], C[p * 4 + 3];

	temp_F = (identity + dt * temp_C) * (temp_F);

	float h;
	h = expf(10.0f * (1.0f - Jp[p]));
	if (material[p] == 1) {
		h = 0.3f;
	}

	float mu = mu_0 * h;
	float la = lambda_0 * h;
	if (material[p] == 0) {
		mu = 0.0f;
	}

	// SVD-related begin
	// we use analytical solution for 2x2 SVD instead of Eigen, test it on CPU first
	const float A[4]{ temp_F(0, 0), temp_F(0, 1) , temp_F(1, 0) , temp_F(1, 1) };
	float U_data[4], S_data[2], V_data[4];
	cu_svd22_raw(A, U_data, S_data, V_data);
	Eigen::Matrix2f U, V;
	U << U_data[0], U_data[1], U_data[2], U_data[3];
	V << V_data[0], V_data[1], V_data[2], V_data[3];

	float J = 1.0f;
	// shall we expand this for-loop?
	for (int i = 0; i < 2; i++) {
		float new_sig = S_data[i];
		if (material[p] == 2)
			new_sig = min(max(S_data[i], 1.0f - 2.5e-2f), 1.0f + 4.5e-3f);
		Jp[p] *= S_data[i] / new_sig;
		S_data[i] = new_sig;
		J *= new_sig;
	}
	// if (J < 0.01) { printf("%f", J); }
	
	if (material[p] == 0) {
		temp_F = identity * sqrtf(J);
	}
	else if (material[p] == 2) {
		Eigen::Matrix2f sig22;
		sig22 << S_data[0], 0, 0, S_data[1];
		temp_F = U * sig22 * V.transpose();
	}

	Eigen::Matrix2f stress, affine;
	stress = 2 * mu * (temp_F - U * V.transpose()) * temp_F.transpose() + identity * la * J * (J - 1);
	stress = (-dt * p_vol * 4 * inv_dx * inv_dx) * stress;
	affine = stress + p_mass * temp_C;
	// SVD-related end

	// shall we expand this for-loop?
	for (int i = 0; i < 3; i++) {
		for (int j = 0; j < 3; j++) {
			Eigen::Vector2i offset;
			offset << i, j;
			Eigen::Vector2f dpos;
			dpos = (offset.cast<float>() - fx) * dx;
			float weight = w[i](0) * w[j](1);
			Eigen::Vector2i temp_index;
			temp_index = base + offset;
			Eigen::Vector2f temp_v, temp_gv;
			temp_v(0) = v[p * 2 + 0];
			temp_v(1) = v[p * 2 + 1];
			temp_gv = weight * (p_mass * temp_v + affine * dpos);
			unsigned int index = temp_index(0) * n_grid + temp_index(1);
			
			// atomic add!
			atomicAdd(&grid_v[index * 2 + 0], temp_gv(0));
			atomicAdd(&grid_v[index * 2 + 1], temp_gv(1));
			atomicAdd(&grid_m[index],  weight * p_mass);
			// grid_v[index * 2 + 0] += temp_gv(0);
			// grid_v[index * 2 + 1] += temp_gv(1);
			// grid_m[index] += weight * p_mass;
		}
	}

	F[p * 4 + 0] = temp_F(0, 0);
	F[p * 4 + 1] = temp_F(0, 1);
	F[p * 4 + 2] = temp_F(1, 0);
	F[p * 4 + 3] = temp_F(1, 1);
}


void cu_p2g_substep(
	float* x_device, float* v_device, float* F_device, float* C_device, int* material_device, float* Jp_device, unsigned int n_particles,
	float* grid_v_device, float* grid_m_device, unsigned int n_grid,
	float dx, float inv_dx, float dt, float mu_0, float lambda_0, float p_vol, float p_mass) {
	const unsigned int dim = 2;
	int n_threads = 128;

	// particles
	int n_blocks = (n_particles + n_threads - 1) / n_threads;

	// FIXME: error in fluid/snow material
	cu_p2g_substep_kernel KERNEL_ARGS2(n_blocks, n_threads)
		(x_device, v_device, F_device, C_device, material_device, Jp_device, n_particles, 
			grid_v_device, grid_m_device, n_grid, 
			dx, inv_dx, dt, mu_0, lambda_0, p_vol, p_mass);
	// checkCudaErrors(cudaDeviceSynchronize());
}



__global__ void cu_boundary_substep_kernel(float* grid_v, float* grid_m, unsigned int n_grid,
	float dt, float gravity)
{
	int idx = blockDim.x * blockIdx.x + threadIdx.x;

	// this condition is VITAL! 
	if (idx >= n_grid * n_grid) return;

	// idx = i * n_grid + j
	int i = idx / n_grid;
	int j = idx % n_grid;

	// the condition here might be a big overhead, we may optimize it
	
	if (grid_m[idx] > 0) {
		grid_v[idx * 2 + 0] = (1 / grid_m[idx]) * grid_v[idx * 2 + 0];
		grid_v[idx * 2 + 1] = (1 / grid_m[idx]) * grid_v[idx * 2 + 1];
		grid_v[idx * 2 + 1] -= dt * gravity * 1.0f;
		if (i < 3 && grid_v[idx * 2] < 0) {
			grid_v[idx * 2] = 0;
		}
		if (i > n_grid - 3 && grid_v[idx * 2] > 0) {
			grid_v[idx * 2] = 0;
		}
		if (j < 3 && grid_v[idx * 2 + 1] < 0) {
			grid_v[idx * 2 + 1] = 0;
		}
		if (j > n_grid - 3 && grid_v[idx * 2 + 1] > 0) {
			grid_v[idx * 2 + 1] = 0;
		}
	}
}

void cu_boundary_substep(float* grid_v_device, float* grid_m_device, unsigned int n_grid,
	float dt, float gravity)
{
	const unsigned int dim = 2;
	int n_threads = 128;

	int n_blocks = (n_grid * n_grid + n_threads - 1) / n_threads;
	cu_boundary_substep_kernel KERNEL_ARGS2(n_blocks, n_threads) 
		(grid_v_device, grid_m_device, n_grid, dt, gravity);
	// checkCudaErrors(cudaDeviceSynchronize());
}

__global__ void cu_g2p_substep_kernel(
	float* x, float* v, float* C, unsigned int n_particles,
	float* grid_v, float* grid_m, unsigned int n_grid,
	float dt, float inv_dx)
{
	const unsigned int dim = 2;
	int p = blockDim.x * blockIdx.x + threadIdx.x;
	

	// this condition is VITAL! 
	if (p >= n_particles) return;

	Eigen::Vector2i base{ int(x[p * 2 + 0] * inv_dx - 0.5f),
							  int(x[p * 2 + 1] * inv_dx - 0.5f) };

	Eigen::Vector2f fx =
		Eigen::Vector2f(x[p * 2 + 0], x[p * 2 + 1]) * inv_dx - base.cast<float>();

	Eigen::Vector2f w[3] {
		0.5f * (1.5f - fx.array()) * (1.5f - fx.array()),
		0.75f - (fx.array() - 1.0f) * (fx.array() - 1.0f),
		0.5f * (fx.array() - 0.5f) * (fx.array() - 0.5f)
	};

	Eigen::Vector2f new_v; new_v.setZero();
	Eigen::Matrix2f new_c; new_c.setZero();
	// we may expand this for-loop
	// I got a feel that CUDA has little love with inner for-loop
	for (int i = 0; i < 3; i++) {
		for (int j = 0; j < 3; j++) {
			Eigen::Vector2f dpos, vij;
			vij << float(i), float(j);
			dpos = vij - fx;
			unsigned int index = (base(0) + i) * n_grid + base(1) + j;
			
			Eigen::Vector2f g_v{ grid_v[index * 2 + 0], grid_v[index * 2 + 1] };
			float weight;
			weight = w[i](0) * w[j](1);
			new_v += weight * g_v;
			new_c += 4 * inv_dx * weight * g_v * dpos.transpose();
		}
	}
	v[p * 2 + 0] = new_v(0);
	v[p * 2 + 1] = new_v(1);
	C[p * 4 + 0] = new_c(0);
	C[p * 4 + 1] = new_c(1);
	C[p * 4 + 2] = new_c(2);
	C[p * 4 + 3] = new_c(3);
	x[p * 2 + 0] += dt * v[p * 2 + 0];
	x[p * 2 + 1] += dt * v[p * 2 + 1];
}

void cu_g2p_substep(
	float* x_device, float* v_device, float* C_device, unsigned int n_particles,
	float* grid_v_device, float* grid_m_device, unsigned int n_grid,
	float dt, float inv_dx) {
	const unsigned int dim = 2;
	int n_threads = 128;

	// particles
	int n_blocks = (n_particles + n_threads - 1) / n_threads;

	cu_g2p_substep_kernel KERNEL_ARGS2(n_blocks, n_threads)
		(x_device, v_device, C_device, n_particles, grid_v_device, grid_m_device, n_grid, dt, inv_dx);
	// checkCudaErrors(cudaDeviceSynchronize());
}


// auxiliary functions

void arrcopy_gpu_to_cpu(
	float* x_device, float* v_device, float* C_device, float* F_device, int* material_device, float* Jp_device,
	float* x_host, float* v_host, float* C_host, float* F_host, int* material_host, float* Jp_host, unsigned int n_particles,
	float* grid_v_device, float* grid_m_device, float* grid_v_host, float* grid_m_host, unsigned int n_grid) {
	const unsigned int dim = 2;
	checkCudaErrors(cudaMemcpy(x_host, x_device, sizeof(float) * n_particles * dim, cudaMemcpyDeviceToHost));
	checkCudaErrors(cudaMemcpy(v_host, v_device, sizeof(float) * n_particles * dim, cudaMemcpyDeviceToHost));
	checkCudaErrors(cudaMemcpy(C_host, C_device, sizeof(float) * n_particles * dim * dim, cudaMemcpyDeviceToHost));
	checkCudaErrors(cudaMemcpy(F_host, F_device, sizeof(float) * n_particles * dim * dim, cudaMemcpyDeviceToHost));
	checkCudaErrors(cudaMemcpy(material_host, material_device, sizeof(int) * n_particles, cudaMemcpyDeviceToHost));
	checkCudaErrors(cudaMemcpy(Jp_host, Jp_device, sizeof(float) * n_particles, cudaMemcpyDeviceToHost));
	checkCudaErrors(cudaMemcpy(grid_v_host, grid_v_device, sizeof(float) * n_grid * n_grid * dim, cudaMemcpyDeviceToHost));
	checkCudaErrors(cudaMemcpy(grid_m_host, grid_m_device, sizeof(float) * n_grid * n_grid, cudaMemcpyDeviceToHost));
	
}

void arrcopy_cpu_to_gpu(
	float* x_device, float* v_device, float* C_device, float* F_device, int* material_device, float* Jp_device,
	float* x_host, float* v_host, float* C_host, float* F_host, int* material_host, float* Jp_host, unsigned int n_particles,
	float* grid_v_device, float* grid_m_device, float* grid_v_host, float* grid_m_host, unsigned int n_grid) {
	const unsigned int dim = 2;
	checkCudaErrors(cudaMemcpy(x_device, x_host, sizeof(float) * n_particles * dim, cudaMemcpyHostToDevice));
	checkCudaErrors(cudaMemcpy(v_device, v_host, sizeof(float) * n_particles * dim, cudaMemcpyHostToDevice));
	checkCudaErrors(cudaMemcpy(C_device, C_host, sizeof(float) * n_particles * dim * dim, cudaMemcpyHostToDevice));
	checkCudaErrors(cudaMemcpy(F_device, F_host, sizeof(float) * n_particles * dim * dim, cudaMemcpyHostToDevice));
	checkCudaErrors(cudaMemcpy(material_device, material_host, sizeof(int) * n_particles, cudaMemcpyHostToDevice));
	checkCudaErrors(cudaMemcpy(Jp_device, Jp_host, sizeof(float) * n_particles, cudaMemcpyHostToDevice));
	checkCudaErrors(cudaMemcpy(grid_v_device, grid_v_host, sizeof(float) * n_grid * n_grid * dim, cudaMemcpyHostToDevice));
	checkCudaErrors(cudaMemcpy(grid_m_device, grid_m_host, sizeof(float) * n_grid * n_grid, cudaMemcpyHostToDevice));
}


void copy_x_from_gpu(float* x_device, float* x_host, unsigned int n_particles) {
	const unsigned int dim = 2;
	checkCudaErrors(cudaMemcpy(x_host, x_device, sizeof(float) * n_particles * dim, cudaMemcpyDeviceToHost));
}

void copy_color_from_gpu(float* color_device, float* color_host, unsigned int n_particles) {
	checkCudaErrors(cudaMemcpy(color_host, color_device, sizeof(float) * n_particles * 3, cudaMemcpyDeviceToHost));
}

void sync_all() {
	checkCudaErrors(cudaDeviceSynchronize());
}