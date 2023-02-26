#include<glad/glad.h>
#include<GLFW/glfw3.h>
#include<glm/glm.hpp>
#include<glm/gtc/matrix_transform.hpp>
#include<glm/gtc/type_ptr.hpp>
#include"camera.h"
#include"shader_m.h"
#include<iostream>
#include<vector>
#include<map>
#include<cuda_runtime.h>
#include<device_launch_parameters.h>
#include"helper_cuda.h"
#include<curand.h>
#include<curand_kernel.h>
#include<Eigen/Core>
#include<cmath>
#include"svd3_cuda.h"
#include<cstdio>
#include<ticktock.h>
#include"render.h"
#include<fstream>


const int dim = 3;
const int n_particles = 50000;
const int n_grid = 64;
const int n_total_grid = n_grid * n_grid * n_grid;
int step = 80;
float dt = 1e-4;
float dx = 1.0f / n_grid;
float inv_dx = n_grid;
float p_rho = 1.0f;
float p_vol = (dx * 0.5f) * (dx * 0.5f);
float p_mass = p_vol * p_rho;
float gravity[] = { 0,-3.8,0 };
float E = 1000.0f;
float nu = 0.2f;
float mu_0 = E / (2.0f * (1.0f + nu));
float lambda_0 = E * nu / ((1.0f + nu) * (1.0f - 2.0f * nu));
int bound = 3;
const int WATER = 0;
const int JELLY = 1;
const int SNOW = 2;

const unsigned int sphere_row = 15;
const unsigned int sphere_col = 20;
float sphere_vertices[6 * sphere_row * sphere_col];  
unsigned int sphere_indices[6 * (sphere_row - 1) * sphere_col];

const unsigned int tool_row = 3;
const unsigned int tool_col = 20;
const int n_rigid_particles = 2 * (tool_row - 1) * tool_col;
const int n_tool_vertices = tool_row * tool_col;

//float tool_vertices_host_temp[6 * tool_row * tool_col];
float tool_vertices_host[6 * tool_row * tool_col];
unsigned int tool_indices_host[6 * (tool_row - 1) * tool_col];
float tool_center_host[dim] = { 0.6f, 0.68f, 0.60f };
float tool_center_init[dim] = { 0.6f, 0.68f, 0.60f };
float rigid_x_host[dim * n_rigid_particles];

float F_x_host[dim * n_particles];
float F_v_host[dim * n_particles];
float F_C_host[dim * dim * n_particles];
float F_dg_host[dim * dim * n_particles];
float F_Jp_host[n_particles];
float F_colors_host[3 * n_particles];
int F_materials_host[n_particles];
float F_grid_v_host[dim * n_total_grid];
float F_grid_m_host[n_total_grid];
int F_used_host[n_particles];

float C_grid_d_host[n_total_grid];
int C_grid_A_host[n_total_grid];
int C_grid_T_host[n_total_grid];
int C_grid_surface_host[n_total_grid];
float C_p_d_host[n_particles];
int C_p_A_host[n_particles];
int C_p_T_host[n_particles];
float C_p_n_host[dim * n_particles];

char* MCTable_path = "MC_Table.txt";
const int n_MC_grid = 64;
const int n_MC_total_grid = n_MC_grid * n_MC_grid * n_MC_grid;
float MC_dx = 1.0f / n_MC_grid;
float MC_inv_dx = n_MC_grid;
float MC_SDF_host[n_MC_total_grid];
int* n_MC_triangle_host;
int MC_et_host[256 * 4 * 3];
float MC_vertices_host[dim * 15 * n_MC_total_grid];
float MC_vertice_color_host[3 * 15 * n_MC_total_grid];
int MC_triangle[15 * n_MC_total_grid];
float MC_node_pos[dim * n_MC_total_grid];
float MC_node_color[3 * n_MC_total_grid];

unsigned int n_threads = 128;
unsigned int n_particle_blocks = (n_particles + n_threads - 1) / n_threads;
unsigned int n_grid_blocks = (n_total_grid + n_threads - 1) / n_threads;
unsigned int n_tool_vertices_blocks = (n_tool_vertices + n_threads - 1) / n_threads;
unsigned int n_rigid_blocks = (n_rigid_particles + n_threads - 1) / n_threads;
unsigned int n_MC_grid_blocks = (n_MC_total_grid + n_threads - 1) / n_threads;

void init_chuiti() {
    std::ifstream file;
    file.open("particles.ply");

    for (int i = 0; i < n_particles; i++) {
        file >> F_x_host[i * 3 + 0];
        F_x_host[i * 3 + 0] = F_x_host[i * 3 + 0] / 30.0 + 0.45;
        file >> F_x_host[i * 3 + 1];
        F_x_host[i * 3 + 1] = F_x_host[i * 3 + 1] / 30.0 - 4.65;
        file >> F_x_host[i * 3 + 2];
        F_x_host[i * 3 + 2] = F_x_host[i * 3 + 2] / 30.0;
    }
    file.close();
}

__global__ void init_particle(
    int n_particles,
    float* F_x_device,
    float* F_v_device,
    float* F_C_device,
    float* F_dg_device,
    float* F_Jp_device,
    float* F_colors_device,
    int* F_materials_device,
    int* F_used_device
) {
    int tid = blockDim.x * blockIdx.x + threadIdx.x;
    if (tid > n_particles)
        return;
    curandState state;
    curand_init(1, tid, 0, &state);
    int group_size = n_particles;
    //F_x_device[tid * 3 + 0] = curand_uniform(&state) * 0.3 + 0.3 + 0.1 * (tid / group_size);
    //F_x_device[tid * 3 + 1] = curand_uniform(&state) * 0.3 + 0.0 + 0.32 * (tid / group_size);
    //F_x_device[tid * 3 + 2] = curand_uniform(&state) * 0.3 + 0.3 + 0.1 * (tid / group_size);
    F_v_device[tid * 3 + 0] = 0.0f;
    F_v_device[tid * 3 + 1] = 0.0f;
    F_v_device[tid * 3 + 2] = 0.0f;
    F_C_device[tid * 9 + 0] = 0.0f;
    F_C_device[tid * 9 + 1] = 0.0f;
    F_C_device[tid * 9 + 2] = 0.0f;
    F_C_device[tid * 9 + 3] = 0.0f;
    F_C_device[tid * 9 + 4] = 0.0f;
    F_C_device[tid * 9 + 5] = 0.0f;
    F_C_device[tid * 9 + 6] = 0.0f;
    F_C_device[tid * 9 + 7] = 0.0f;
    F_C_device[tid * 9 + 8] = 0.0f;
    F_dg_device[tid * 9 + 0] = 1.0f;
    F_dg_device[tid * 9 + 1] = 0.0f;
    F_dg_device[tid * 9 + 2] = 0.0f;
    F_dg_device[tid * 9 + 3] = 0.0f;
    F_dg_device[tid * 9 + 4] = 1.0f;
    F_dg_device[tid * 9 + 5] = 0.0f;
    F_dg_device[tid * 9 + 6] = 0.0f;
    F_dg_device[tid * 9 + 7] = 0.0f;
    F_dg_device[tid * 9 + 8] = 1.0f;
    F_Jp_device[tid] = 1.0f;
    F_materials_device[tid] = 1;
    F_colors_device[tid * 3 + 0] = 0.93f;
    F_colors_device[tid * 3 + 1] = 0.33f;
    F_colors_device[tid * 3 + 2] = 0.13f;
    F_used_device[tid] = 1;
}

__global__ void init_rigid_particles(
    int n_rigid_particles,
    float* tool_vertices_device,
    unsigned int* tool_indices_device,
    float* tool_center_device,
    float* rigid_x_device
) {
    int tid = blockDim.x * blockIdx.x + threadIdx.x;
    if (tid > n_rigid_particles)
        return;
    int vid1 = tool_indices_device[tid * 3 + 0];
    int vid2 = tool_indices_device[tid * 3 + 1];
    int vid3 = tool_indices_device[tid * 3 + 2];
    rigid_x_device[tid * 3 + 0] = (tool_vertices_device[vid1 * 6 + 0] + tool_vertices_device[vid2 * 6 + 0] + tool_vertices_device[vid3 * 6 + 0]) / 3.0f;
    rigid_x_device[tid * 3 + 1] = (tool_vertices_device[vid1 * 6 + 1] + tool_vertices_device[vid2 * 6 + 1] + tool_vertices_device[vid3 * 6 + 1]) / 3.0f;
    rigid_x_device[tid * 3 + 2] = (tool_vertices_device[vid1 * 6 + 2] + tool_vertices_device[vid2 * 6 + 2] + tool_vertices_device[vid3 * 6 + 2]) / 3.0f;
}

__global__ void rigid_move(
    int n_rigid_particles,
    float* rigid_x_device,
    float dt
) {
    int tid = blockDim.x * blockIdx.x + threadIdx.x;
    if (tid > n_rigid_particles)
        return;
    rigid_x_device[tid * 3 + 1] -= dt;

}

__global__ void tool_vertices_move(
    int n_tool_vertices,
    float* tool_vertices_device,
    float dt
) {
    int tid = blockDim.x * blockIdx.x + threadIdx.x;
    if (tid > n_tool_vertices)
        return;
    tool_vertices_device[tid * 6 + 1] -= dt;
}

__device__ Eigen::Vector3f cross(Eigen::Vector3f a, Eigen::Vector3f b) {
    return Eigen::Vector3f(a.y() * b.z() - b.y() * a.z(), -(a.x() * b.z() - b.x() * a.z()), a.x() * b.y() - b.x() * a.y());
}

__global__ void grid_clear(
    int n_total_grid,
    float* F_grid_v_device,
    float* F_grid_m_device,
    float* C_grid_d_device,
    int* C_grid_A_device,
    int* C_grid_T_device,
    int* C_grid_surface_device
) {
    int tid = blockDim.x * blockIdx.x + threadIdx.x;
    if (tid > n_total_grid)
        return;
    F_grid_v_device[tid * 3 + 0] = 0.0f;
    F_grid_v_device[tid * 3 + 1] = 0.0f;
    F_grid_v_device[tid * 3 + 2] = 0.0f;
    F_grid_m_device[tid] = 0.0f;

    C_grid_d_device[tid] = 0.0f;
    C_grid_A_device[tid] = 0;
    C_grid_T_device[tid] = 0;
    C_grid_surface_device[tid] = -1;
}

__global__ void boundary_condition(
    int n_total_grid,
    float* F_grid_v_device,
    float* F_grid_m_device,
    float dt,
    int n_grid,
    int bound,
    float inv_dx
) {
    int tid = blockDim.x * blockIdx.x + threadIdx.x;
    if (tid > n_total_grid)
        return;
    if (F_grid_m_device[tid] > 0) {
        F_grid_v_device[tid * 3 + 0] /= F_grid_m_device[tid];
        F_grid_v_device[tid * 3 + 1] /= F_grid_m_device[tid];
        F_grid_v_device[tid * 3 + 2] /= F_grid_m_device[tid];
    }
    F_grid_v_device[tid * 3 + 1] -= dt * 3.8f;
    int g_x, g_y, g_z;
    g_x = tid / (n_grid * n_grid);
    g_y = (tid - g_x * n_grid * n_grid) / n_grid;
    g_z = tid % n_grid;
    if (g_x < bound && F_grid_v_device[tid * 3 + 0] < 0.0f) {
        F_grid_v_device[tid * 3 + 0] = 0.0f;
    }
    if (g_x > n_grid - bound && F_grid_v_device[tid * 3 + 0] > 0.0f) {
        F_grid_v_device[tid * 3 + 0] = 0.0f;
    }
    if (g_y < bound && F_grid_v_device[tid * 3 + 1] < 0.0f) {
        F_grid_v_device[tid * 3 + 1] = 0.0f;
    }
    if (g_y > n_grid - bound && F_grid_v_device[tid * 3 + 1] > 0.0f) {
        F_grid_v_device[tid * 3 + 1] = 0.0f;
    }
    if (g_z < bound && F_grid_v_device[tid * 3 + 2] < 0.0f) {
        F_grid_v_device[tid * 3 + 2] = 0.0f;
    }
    if (g_z > n_grid - bound && F_grid_v_device[tid * 3 + 2] > 0.0f) {
        F_grid_v_device[tid * 3 + 2] = 0.0f;
    }
    if (g_y > 0.5f * inv_dx && g_z < 0.49 * inv_dx) {
        F_grid_v_device[tid * 3 + 0] = 0.0f;
        F_grid_v_device[tid * 3 + 1] = 0.0f;
        F_grid_v_device[tid * 3 + 2] = 0.0f;
    }
}

__device__ float compute_particle_distance(
    Eigen::Vector3f particle_point,
    float* tool_center_device
) {
    Eigen::Vector3f center;
    center << tool_center_device[0], tool_center_device[1], tool_center_device[2];
    float distance = (particle_point - center).norm() - 0.08;
    return distance;
}
__device__ float compute_distance(
    Eigen::Vector3f point,
    Eigen::Vector3f proj_point
) {
    return abs((point - proj_point).norm());
}
__device__ bool compute_grid_T(
    Eigen::Vector3f plane_normal,
    Eigen::Vector3f point,
    Eigen::Vector3f proj_point
) {
    return (plane_normal.dot(point - proj_point) > 0);
}
__device__ bool is_valid(
    int surface,
    float* tool_vertices_device,
    unsigned int* tool_indices_device,
    Eigen::Vector3f proj_point
) {
    int index = 3 * surface;
    unsigned int idx_a = tool_indices_device[index + 0];
    unsigned int idx_b = tool_indices_device[index + 1];
    unsigned int idx_c = tool_indices_device[index + 2];
    Eigen::Vector3f a, b, c, ab, bc, ca, ap, bp, cp, temp1, temp2, temp3;
    a << tool_vertices_device[idx_a * 6 + 0],
        tool_vertices_device[idx_a * 6 + 1],
        tool_vertices_device[idx_a * 6 + 2];
    b << tool_vertices_device[idx_b * 6 + 0],
        tool_vertices_device[idx_b * 6 + 1],
        tool_vertices_device[idx_b * 6 + 2];
    c << tool_vertices_device[idx_c * 6 + 0],
        tool_vertices_device[idx_c * 6 + 1],
        tool_vertices_device[idx_c * 6 + 2];
    ab = b - a;
    bc = c - b;
    ca = a - c;
    ap = proj_point - a;
    bp = proj_point - b;
    cp = proj_point - c;
    temp1 = cross(ab, ap);
    temp2 = cross(bc, bp);
    temp3 = cross(ca, cp);
    return (temp1.dot(temp2) > 0 && temp2.dot(temp3) > 0);
}
__global__ void kernel(
    float* tool_vertices_device,
    unsigned int* tool_indices_device,
    float* rigid_x_device
) {
    int surface = 0;
    Eigen::Vector3f proj_point;
    int idx_a, idx_b, idx_c;
    idx_a = tool_indices_device[surface * 3 + 0];
    idx_b = tool_indices_device[surface * 3 + 1];
    idx_c = tool_indices_device[surface * 3 + 2];
    Eigen::Vector3f a, b, c, r;
    a << tool_vertices_device[idx_a * 6 + 0],
        tool_vertices_device[idx_a * 6 + 1],
        tool_vertices_device[idx_a * 6 + 2];
    b << tool_vertices_device[idx_b * 6 + 0],
        tool_vertices_device[idx_b * 6 + 1],
        tool_vertices_device[idx_b * 6 + 2];
    c << tool_vertices_device[idx_c * 6 + 0],
        tool_vertices_device[idx_c * 6 + 1],
        tool_vertices_device[idx_c * 6 + 2];
    r << rigid_x_device[surface * 3 + 0], rigid_x_device[surface * 3 + 1], rigid_x_device[surface * 3 + 2];
    printf("a: (%f,%f,%f)\n", a(0), a(1), a(2));
    printf("b: (%f,%f,%f)\n", b(0), b(1), b(2));
    printf("c: (%f,%f,%f)\n", c(0), c(1), c(2));
    printf("r: (%f,%f,%f)\n", r(0), r(1), r(2));
}

__device__ Eigen::Vector3f compute_proj_point(
    int surface,
    float* tool_vertices_device,
    unsigned int* tool_indices_device,
    Eigen::Vector3f plane_normal,
    Eigen::Vector3f point
) {
    int index = tool_indices_device[3 * surface];
    Eigen::Vector3f plane_point;
    plane_point <<
        tool_vertices_device[index * 6 + 0],
        tool_vertices_device[index * 6 + 1],
        tool_vertices_device[index * 6 + 2];
    float A = plane_normal(0);
    float B = plane_normal(1);
    float C = plane_normal(2);
    float D = - A * plane_point(0) - B * plane_point(1) - C * plane_point(2);
    float temp = (A * A + B * B + C * C);
    float proj_x = ((B * B + C * C) * point(0) - A * (B * point(1) + C * point(2) + D)) / temp;
    float proj_y = ((A * A + C * C) * point(1) - B * (A * point(0) + C * point(2) + D)) / temp;
    float proj_z = ((A * A + B * B) * point(2) - C * (A * point(0) + B * point(1) + D)) / temp;
    Eigen::Vector3f proj_point;
    proj_point << proj_x, proj_y, proj_z;
    return proj_point;
}

__device__ Eigen::Vector3f compute_plane_normal(
    int surface,
    float* tool_vertices_device,
    unsigned int* tool_indices_device
) {
    int index = 3 * surface;
    unsigned int idx_a = tool_indices_device[index + 0];
    unsigned int idx_b = tool_indices_device[index + 1];
    unsigned int idx_c = tool_indices_device[index + 2];
    Eigen::Vector3f ab, bc, re;
    ab <<
        tool_vertices_device[idx_b * 6 + 0] - tool_vertices_device[idx_a * 6 + 0],
        tool_vertices_device[idx_b * 6 + 1] - tool_vertices_device[idx_a * 6 + 1],
        tool_vertices_device[idx_b * 6 + 2] - tool_vertices_device[idx_a * 6 + 2];
    bc <<
        tool_vertices_device[idx_c * 6 + 0] - tool_vertices_device[idx_b * 6 + 0],
        tool_vertices_device[idx_c * 6 + 1] - tool_vertices_device[idx_b * 6 + 1],
        tool_vertices_device[idx_c * 6 + 2] - tool_vertices_device[idx_b * 6 + 2];
    re = cross(ab, bc);
    return re.normalized();
}

__global__ void grid_CDF(
    int n_rigid_particles,
    float* tool_vertices_device,
    unsigned int* tool_indices_device,
    float* tool_center_device,
    float* rigid_x_device,
    float dx,
    float inv_dx,
    float* C_grid_d_device,
    int* C_grid_A_device,
    int* C_grid_T_device,
    int* C_grid_surface_device,
    int n_grid
) {
    int tid = blockDim.x * blockIdx.x + threadIdx.x;
    if (tid > n_rigid_particles)
        return;
    Eigen::Vector3f rp;
    rp << rigid_x_device[tid * 3 + 0], rigid_x_device[tid * 3 + 1], rigid_x_device[tid * 3 + 2];
    Eigen::Vector3i base = (rp.array() * inv_dx - 0.5f).cast<int>();
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            for (int k = 0; k < 3; k++) {
                Eigen::Vector3i offset;
                offset << i, j, k;
                Eigen::Vector3f grid_node;
                grid_node = (offset + base).cast<float>() * dx;
                Eigen::Vector3f plane_normal, proj_point;
                plane_normal = compute_plane_normal(tid, tool_vertices_device, tool_indices_device);
                proj_point = compute_proj_point(
                    tid,
                    tool_vertices_device,
                    tool_indices_device,
                    plane_normal,
                    grid_node
                );
                if (is_valid(tid, tool_vertices_device, tool_indices_device, proj_point) == true) {
                int temp_index = (base + offset)(0) * n_grid * n_grid + (base + offset)(1) * n_grid + (base + offset)(2);
                    C_grid_A_device[temp_index] = 1;
                    float distance = compute_distance(grid_node, proj_point);
                    if (C_grid_surface_device[temp_index] == -1 || C_grid_d_device[temp_index] > distance) {
                        C_grid_d_device[temp_index] = distance;
                        C_grid_surface_device[temp_index] = tid;
                        if (compute_grid_T(plane_normal, grid_node, proj_point)) {
                            C_grid_T_device[temp_index] = 1;
                        }
                        else {
                            C_grid_T_device[temp_index] = -1;
                        }
                    }
                }
            }
        }
    }
}



__global__ void p2g(
    float* F_x_device,
    float* F_v_device,
    float* F_C_device,
    float* F_dg_device,
    float* F_Jp_device,
    float* F_colors_device,
    int* F_materials_device,
    float* F_grid_v_device,
    float* F_grid_m_device,
    int* F_used_device,
    float dt,
    int n_particles,
    float dx,
    float p_rho,
    float p_vol,
    float p_mass,
    float nu,
    float mu_0,
    float lambda_0,
    int WATER,
    int JELLY,
    int SNOW,
    float* C_grid_d_device,
    int* C_grid_A_device,
    int* C_grid_T_device,
    int* C_grid_surface_device,
    float* C_p_d_device,
    int* C_p_A_device,
    int* C_p_T_device,
    float* C_p_n_device,
    float* tool_center_device,
    int n_grid
) {
    int i = blockDim.x * blockIdx.x + threadIdx.x;
    if (i > n_particles) {
        return;
    }
    F_colors_device[i * 3 + 0] = 0.93f;
    F_colors_device[i * 3 + 1] = 0.33f;
    F_colors_device[i * 3 + 2] = 0.13f;
    Eigen::Vector3f Xp;
    Eigen::Vector3f F_xp(F_x_device[i * 3 + 0], F_x_device[i * 3 + 1], F_x_device[i * 3 + 2]);
    Xp = F_xp / dx;
    Eigen::Vector3i base = (Xp.array() - 0.5f).cast<int>();
    Eigen::Vector3f fx = Xp - base.cast<float>();
    Eigen::Vector3f w[3]{
        0.5f * (1.5f - fx.array()) * (1.5f - fx.array()),
        0.75f - (fx.array() - 1.0f) * (fx.array() - 1.0f),
        0.5f * (fx.array() - 0.5f) * (fx.array() - 0.5f)
    };

    //CDF=============
    C_p_A_device[i] = 0;
    C_p_T_device[i] = 0;
    C_p_d_device[i] = 0.0f;
    float Tpr = 0.0f;
    for (int ii = 0; ii < 3; ii++) {
        for (int jj = 0; jj < 3; jj++) {
            for (int kk = 0; kk < 3; kk++) {
                Eigen::Vector3i offset;
                offset << ii, jj, kk;
                int temp_index = (base + offset)(0) * n_grid * n_grid + (base + offset)(1) * n_grid + (base + offset)(2);
                if (C_grid_A_device[temp_index] == 1) {
                    C_p_A_device[i] = 1;
                }
                float weight = w[ii](0) * w[jj](1) * w[kk](2);
                Tpr += weight * C_grid_d_device[temp_index] * C_grid_T_device[temp_index];
            }
        }
    }
    C_p_d_device[i] = abs(Tpr);
    if (C_p_A_device[i] == 1) {
        C_p_d_device[i] = compute_particle_distance(Xp, tool_center_device);
        if (Tpr > 0) {
            C_p_T_device[i] = 1;
            F_colors_device[i * 3 + 1] = 1.0f;
        }
        else {
            C_p_T_device[i] = -1;
            F_colors_device[i * 3 + 2] = 1.0f;
        }
    }
    //=============


    Eigen::Matrix3f F_dgp;
    F_dgp <<
        F_dg_device[i * 9 + 0], F_dg_device[i * 9 + 1], F_dg_device[i * 9 + 2],
        F_dg_device[i * 9 + 3], F_dg_device[i * 9 + 4], F_dg_device[i * 9 + 5],
        F_dg_device[i * 9 + 6], F_dg_device[i * 9 + 7], F_dg_device[i * 9 + 8];
    Eigen::Matrix3f F_Cp;
    F_Cp <<
        F_C_device[i * 9 + 0], F_C_device[i * 9 + 1], F_C_device[i * 9 + 2],
        F_C_device[i * 9 + 3], F_C_device[i * 9 + 4], F_C_device[i * 9 + 5],
        F_C_device[i * 9 + 6], F_C_device[i * 9 + 7], F_C_device[i * 9 + 8];
    F_dgp = (Eigen::Matrix3f::Identity() + dt * F_Cp) * F_dgp;
    float h = expf(10.0f * (1.0f - F_Jp_device[i]));
    if (F_materials_device[i] == JELLY) {
        h = 0.3f;
    }
    float mu = mu_0 * h;
    float la = lambda_0 * h;
    if (F_materials_device[i] == WATER) {
        mu = 0.0f;
    }

    /*todo: SVD*/
    Eigen::Matrix3f U, sig, V;
    float u11, u12, u13,
        u21, u22, u23,
        u31, u32, u33;
    float s11, s22, s33;
    float v11, v12, v13,
        v21, v22, v23,
        v31, v32, v33;
    svd_cu(F_dgp(0, 0), F_dgp(0, 1), F_dgp(0, 2),
        F_dgp(1, 0), F_dgp(1, 1), F_dgp(1, 2),
        F_dgp(2, 0), F_dgp(2, 1), F_dgp(2, 2),
        u11, u12, u13,
        u21, u22, u23,
        u31, u32, u33,
        s11, s22, s33,
        v11, v12, v13,
        v21, v22, v23,
        v31, v32, v33);
    U << u11, u12, u13,
        u21, u22, u23,
        u31, u32, u33;
    sig << s11, 0, 0,
        0, s22, 0,
        0, 0, s33;
    V << v11, v12, v13,
        v21, v22, v23,
        v31, v32, v33;

    float J = 1.0f;
    for (int d = 0; d < 3; d++) {
        float new_sig = sig(d, d);
        if (F_materials_device[i] == SNOW) {
            new_sig = min(max(sig(d, d), 1.0f - 2.5e-2f), 1.0f + 4.5e-3f);
        }
        F_Jp_device[i] *= sig(d, d) / new_sig;
        sig(d, d) = new_sig;
        J *= new_sig;
    }
    if (F_materials_device[i] == WATER) {
        Eigen::Matrix3f new_F = Eigen::Matrix3f::Identity();
        new_F(0, 0) = J;
        F_dgp = new_F;
    }
    else if (F_materials_device[i] == SNOW) {
        F_dgp = U * sig * V.transpose();
    }
    Eigen::Matrix3f stress;
    stress = 2 * mu * (F_dgp - U * V.transpose()) * F_dgp.transpose()
        + Eigen::Matrix3f::Identity() * la * J * (J - 1);
    stress = (-dt * p_vol * 4) * stress / (dx * dx);
    Eigen::Matrix3f affine;
    affine = stress + p_mass * F_Cp;

    for (int ii = 0; ii < 3; ii++) {
        for (int jj = 0; jj < 3; jj++) {
            for (int kk = 0; kk < 3; kk++) {
                Eigen::Vector3f dpos;
                Eigen::Vector3i offset;
                offset << ii, jj, kk;

                int temp_index = (base + offset)(0) * n_grid * n_grid + (base + offset)(1) * n_grid + (base + offset)(2);
                if (C_p_T_device[i] * C_grid_T_device[temp_index] == -1) {
                    continue;
                }

                dpos = (offset.cast<float>() - fx) * dx;
                float weight = w[ii](0) * w[jj](1) * w[kk](2);
                Eigen::Vector3f F_grid_v, F_vp;
                F_vp << F_v_device[i * 3 + 0], F_v_device[i * 3 + 1], F_v_device[i * 3 + 2];
                F_grid_v = weight * (p_mass * F_vp + affine * dpos);
                atomicAdd(&F_grid_v_device[temp_index * 3 + 0], F_grid_v(0));
                atomicAdd(&F_grid_v_device[temp_index * 3 + 1], F_grid_v(1));
                atomicAdd(&F_grid_v_device[temp_index * 3 + 2], F_grid_v(2));
                atomicAdd(&F_grid_m_device[temp_index], weight * p_mass);
            }
        }
    }
    F_dg_device[i * 9 + 0] = F_dgp(0, 0);
    F_dg_device[i * 9 + 1] = F_dgp(0, 1);
    F_dg_device[i * 9 + 2] = F_dgp(0, 2);
    F_dg_device[i * 9 + 3] = F_dgp(1, 0);
    F_dg_device[i * 9 + 4] = F_dgp(1, 1);
    F_dg_device[i * 9 + 5] = F_dgp(1, 2);
    F_dg_device[i * 9 + 6] = F_dgp(2, 0);
    F_dg_device[i * 9 + 7] = F_dgp(2, 1);
    F_dg_device[i * 9 + 8] = F_dgp(2, 2);
}
__global__ void g2p(
    float* F_x_device,
    float* F_C_device,
    float* F_v_device,
    float* F_colors_device,
    int* F_materials_device,
    int* F_used_device,
    float* F_grid_v_device,
    float* F_grid_m_device,
    int n_particles,
    int n_grid,
    float dx,
    float dt,
    float* C_grid_d_device,
    int* C_grid_A_device,
    int* C_grid_T_device,
    int* C_grid_surface_device,
    float* C_p_d_device,
    int* C_p_A_device,
    int* C_p_T_device,
    float* C_p_n_device,
    float* tool_center_device,
    float p_mass
) {
    int p = blockDim.x * blockIdx.x + threadIdx.x;
    if (p > n_particles) 
        return;

    Eigen::Vector3f Xp;
    Eigen::Vector3f F_xp(F_x_device[p * 3 + 0], F_x_device[p * 3 + 1], F_x_device[p * 3 + 2]);
    Xp = F_xp / dx;
    Eigen::Vector3i base = (Xp.array() - 0.5f).cast<int>();
    Eigen::Vector3f fx = Xp - base.cast<float>();
    Eigen::Vector3f w[3]{
        0.5f * (1.5f - fx.array()) * (1.5f - fx.array()),
        0.75f - (fx.array() - 1.0f) * (fx.array() - 1.0f),
        0.5f * (fx.array() - 0.5f) * (fx.array() - 0.5f)
    };

    Eigen::Vector3f new_v = Eigen::Vector3f::Zero();
    Eigen::Matrix3f new_C = Eigen::Matrix3f::Zero();

    Eigen::Vector3f cp, tool_center, np;
    tool_center << tool_center_device[0], tool_center_device[1], tool_center_device[2];
    cp = Xp - tool_center;
    np = cp.normalized() * C_p_T_device[p];

    Eigen::Vector3f vp;
    vp << F_v_device[p * 3 + 0], F_v_device[p * 3 + 1], F_v_device[p * 3 + 2];
    for (int ii = 0; ii < 3; ii++) {
        for (int jj = 0; jj < 3; jj++) {
            for (int kk = 0; kk < 3; kk++) {
                Eigen::Vector3f dpos;
                Eigen::Vector3i offset;
                offset << ii, jj, kk;
                dpos = (offset.cast<float>() - fx) * dx;
                float weight = 1.0f;
                weight = w[ii](0) * w[jj](1) * w[kk](2);
                int temp_index = (base + offset)(0) * n_grid * n_grid + (base + offset)(1) * n_grid + (base + offset)(2);
                Eigen::Vector3f g_v;
                g_v.setZero();

                if (C_p_T_device[p] * C_grid_T_device[temp_index] == -1) {
                    float sg;
                    sg = vp.dot(np);
                    if (sg > 0) {
                        g_v = vp;
                    }
                    else {
                        g_v = vp - vp.dot(np) * np;
                    }
                    if (C_p_T_device[p] * C_p_d_device[p] > 0) {
                        g_v += np * 5;
                    }
                }
                else {
                    g_v << F_grid_v_device[temp_index * 3 + 0], F_grid_v_device[temp_index * 3 + 1], F_grid_v_device[temp_index * 3 + 2];
                }
                //g_v << F_grid_v_device[temp_index * 3 + 0], F_grid_v_device[temp_index * 3 + 1], F_grid_v_device[temp_index * 3 + 2];
                new_v += weight * g_v;
                new_C += 4 * weight * g_v * dpos.transpose() / (dx * dx);
            }
        }
    }
    F_v_device[p * 3 + 0] = new_v(0);
    F_v_device[p * 3 + 1] = new_v(1);
    F_v_device[p * 3 + 2] = new_v(2);
    if (C_p_A_device[p] == 1 && C_p_T_device[p] * C_p_d_device[p] < 0) {
        float kh = 0.001f;
        Eigen::Vector3f f_penalty;
        f_penalty = -kh * np * C_p_d_device[p] * C_p_T_device[p];
        F_v_device[p * 3 + 0] += dt * f_penalty(0) / p_mass;
        F_v_device[p * 3 + 1] += dt * f_penalty(1) / p_mass;
        F_v_device[p * 3 + 2] += dt * f_penalty(2) / p_mass;

    }
    F_x_device[p * 3 + 0] += dt * F_v_device[p * 3 + 0];
    F_x_device[p * 3 + 1] += dt * F_v_device[p * 3 + 1];
    F_x_device[p * 3 + 2] += dt * F_v_device[p * 3 + 2];
    F_C_device[p * 9 + 0] = new_C(0, 0);
    F_C_device[p * 9 + 1] = new_C(0, 1);
    F_C_device[p * 9 + 2] = new_C(0, 2);
    F_C_device[p * 9 + 3] = new_C(1, 0);
    F_C_device[p * 9 + 4] = new_C(1, 1);
    F_C_device[p * 9 + 5] = new_C(1, 2);
    F_C_device[p * 9 + 6] = new_C(2, 0);
    F_C_device[p * 9 + 7] = new_C(2, 1);
    F_C_device[p * 9 + 8] = new_C(2, 2);
}




void read_MCTable() {
    std::ifstream file;
    file.open(MCTable_path);
    for (int i = 0; i < 256; i++) {
        int index = i * 12;
        for (int j = 0; j < 12; j++) {
            file >> MC_et_host[index + j];
        }
        float temp;
        file >> temp;
    }
    file.close();
}

__global__ void compute_implicit_face(
    int n_MC_grid,
    int n_MC_total_grid,
    int n_particles,
    float MC_dx,
    float* F_x_device,
    float* MC_SDF_device
) {
    int tid = blockDim.x * blockIdx.x + threadIdx.x;
    if (tid > n_MC_total_grid)
        return;
    float min_dis = 10.0f;
    Eigen::Vector3f node_pos;
    int g_x = tid / (n_MC_grid * n_MC_grid);
    int g_y = (tid - g_x * n_MC_grid * n_MC_grid) / n_MC_grid;
    int g_z = tid % n_MC_grid;
    node_pos << g_x * MC_dx, g_y* MC_dx, g_z* MC_dx;
    for (int p = 0; p < n_particles; p++) {
        Eigen::Vector3f xp;
        xp << F_x_device[p * 3 + 0], F_x_device[p * 3 + 1], F_x_device[p * 3 + 2];
        float distance = (xp - node_pos).norm() - 0.02;
        if(distance < min_dis){
            min_dis = distance;
        }
    }
    MC_SDF_device[tid] = min_dis;
}

__device__ Eigen::Vector3f compute_MC_vertice(
    int tid,
    int n_MC_grid,
    float MC_dx,
    float* MC_SDF_device,
    int edge
) {
    int g_x = tid / (n_MC_grid * n_MC_grid);
    int g_y = (tid - g_x * n_MC_grid * n_MC_grid) / n_MC_grid;
    int g_z = tid % n_MC_grid;
    float a = abs(MC_SDF_device[tid]);
    float b = abs(MC_SDF_device[tid + n_MC_grid * n_MC_grid]);
    float c = abs(MC_SDF_device[tid + n_MC_grid * n_MC_grid + 1]);
    float d = abs(MC_SDF_device[tid + 1]);
    float e = abs(MC_SDF_device[tid + n_MC_grid]);
    float f = abs(MC_SDF_device[tid + n_MC_grid * n_MC_grid + n_MC_grid]);
    float g = abs(MC_SDF_device[tid + n_MC_grid * n_MC_grid + n_MC_grid + 1]);
    float h = abs(MC_SDF_device[tid + n_MC_grid + 1]);
    Eigen::Vector3f res_pos, base;
    base << g_x * MC_dx, g_y* MC_dx, g_z* MC_dx;
    if (edge == 0) {
        float temp = a / (a + b);
        res_pos = base + Eigen::Vector3f(MC_dx * temp, 0, 0);
    }
    if (edge == 1) {
        float temp = b / (b + c);
        res_pos = base + Eigen::Vector3f(MC_dx, 0, MC_dx * temp);
    }
    if (edge == 2) {
        float temp = d / (c + d);
        res_pos = base + Eigen::Vector3f(MC_dx * temp, 0, MC_dx);
    }
    if (edge == 3) {
        float temp = a / (a + d);
        res_pos = base + Eigen::Vector3f(0, 0, MC_dx * temp);
    }
    if (edge == 4) {
        float temp = e / (e + f);
        res_pos = base + Eigen::Vector3f(MC_dx * temp, MC_dx, 0);
    }
    if (edge == 5) {
        float temp = f / (f + g);
        res_pos = base + Eigen::Vector3f(MC_dx, MC_dx, MC_dx * temp);
    }
    if (edge == 6) {
        float temp = h / (h + g);
        res_pos = base + Eigen::Vector3f(MC_dx * temp, MC_dx, MC_dx);
    }
    if (edge == 7) {
        float temp = e / (e + h);
        res_pos = base + Eigen::Vector3f(0, MC_dx, MC_dx * temp);
    }
    if (edge == 8) {
        float temp = a / (a + e);
        res_pos = base + Eigen::Vector3f(0, MC_dx * temp, 0);
    }
    if (edge == 9) {
        float temp = b / (b + f);
        res_pos = base + Eigen::Vector3f(MC_dx, MC_dx * temp, 0);
    }
    if (edge == 10) {
        float temp = c / (c + g);
        res_pos = base + Eigen::Vector3f(MC_dx, MC_dx * temp, MC_dx);
    }
    if (edge == 11) {
        float temp = d / (d + h);
        res_pos = base + Eigen::Vector3f(0, MC_dx * temp, MC_dx);
    }
    return res_pos;
}


__global__ void implicit_to_explicit(
    int n_MC_grid,
    int n_MC_total_grid,
    float* MC_SDF_device,
    int* n_MC_triangle_device,
    int* MC_et_device,
    float* MC_vertices_device,
    float MC_dx
) {
    int tid = blockDim.x * blockIdx.x + threadIdx.x;
    if (tid > n_MC_total_grid)
        return;
    int g_x = tid / (n_MC_grid * n_MC_grid);
    int g_y = (tid - g_x * n_MC_grid * n_MC_grid) / n_MC_grid;
    int g_z = tid % n_MC_grid;
    if (g_x == n_MC_grid - 1 || g_y == n_MC_grid - 1 || g_z == n_MC_grid - 1)
        return;
    int id = 0;
    if (MC_SDF_device[tid] > 0)
        id |= 1;
    if (MC_SDF_device[tid + n_MC_grid * n_MC_grid] > 0)
        id |= 2;
    if (MC_SDF_device[tid + n_MC_grid * n_MC_grid + 1] > 0)
        id |= 4;
    if (MC_SDF_device[tid + 1] > 0)
        id |= 8;
    if (MC_SDF_device[tid + n_MC_grid] > 0)
        id |= 16;
    if (MC_SDF_device[tid + n_MC_grid * n_MC_grid + n_MC_grid] > 0)
        id |= 32;
    if (MC_SDF_device[tid + n_MC_grid * n_MC_grid + n_MC_grid + 1] > 0)
        id |= 64;
    if (MC_SDF_device[tid + n_MC_grid + 1] > 0)
        id |= 128;
    for (int i = 0; i < 4; i++) {
        int temp = id * 4 + i;
        if (MC_et_device[temp * 3 + 0] > 0) {
            int index = atomicAdd(n_MC_triangle_device, 1);
            Eigen::Vector3f t1, t2, t3, normal;
            t1 = compute_MC_vertice(tid, n_MC_grid, MC_dx, MC_SDF_device, MC_et_device[temp * 3 + 0]);
            t2 = compute_MC_vertice(tid, n_MC_grid, MC_dx, MC_SDF_device, MC_et_device[temp * 3 + 1]);
            t3 = compute_MC_vertice(tid, n_MC_grid, MC_dx, MC_SDF_device, MC_et_device[temp * 3 + 2]);
            normal = cross(t3 - t1, t2 - t1).normalized();
            MC_vertices_device[index * 18 + 0] = t1(0);
            MC_vertices_device[index * 18 + 1] = t1(1);
            MC_vertices_device[index * 18 + 2] = t1(2);
            MC_vertices_device[index * 18 + 3] = normal(0);
            MC_vertices_device[index * 18 + 4] = normal(1);
            MC_vertices_device[index * 18 + 5] = normal(2);

            MC_vertices_device[index * 18 + 6] = t2(0);
            MC_vertices_device[index * 18 + 7] = t2(1);
            MC_vertices_device[index * 18 + 8] = t2(2);
            MC_vertices_device[index * 18 + 9] = normal(0);
            MC_vertices_device[index * 18 + 10] = normal(1);
            MC_vertices_device[index * 18 + 11] = normal(2);


            MC_vertices_device[index * 18 + 12] = t3(0);
            MC_vertices_device[index * 18 + 13] = t3(1);
            MC_vertices_device[index * 18 + 14] = t3(2);
            MC_vertices_device[index * 18 + 15] = normal(0);
            MC_vertices_device[index * 18 + 16] = normal(1);
            MC_vertices_device[index * 18 + 17] = normal(2);
            //printf("%f\n", t1(0));
        }

        //int temp = 12 * id + i * 3;

    }

}


std::map<std::vector<float>, int> vertice_map;
unsigned int MC_indices[n_MC_total_grid * 4 * 3] = { 0 };
void smooth_normal() {
    memset(MC_indices, 0, sizeof(MC_indices));
    for (int i = 0; i < *n_MC_triangle_host; i++) {
        for (int j = 0; j < 3; j++) {
            int index = i * 18 + j * 6;
            std::vector<float> vPos(3);
            vPos[0] = MC_vertices_host[index + 0];
            vPos[1] = MC_vertices_host[index + 1];
            vPos[2] = MC_vertices_host[index + 2];
            if (vertice_map.count(vPos) == 0) {
                int value = i * 3 + j;
                vertice_map[vPos] = value;
                MC_indices[value] = value;
            }
            else {
                int value = i * 3 + j;
                MC_indices[value] = vertice_map[vPos];
                MC_vertices_host[vertice_map[vPos] * 6 + 3] += MC_vertices_host[index + 3];
                MC_vertices_host[vertice_map[vPos] * 6 + 4] += MC_vertices_host[index + 4];
                MC_vertices_host[vertice_map[vPos] * 6 + 5] += MC_vertices_host[index + 5];
            }
        }
    }
}

int main() {
    read_MCTable();

    float* F_x_device;
    float* F_v_device;
    float* F_C_device;
    float* F_dg_device;
    float* F_Jp_device;
    float* F_colors_device;
    int* F_materials_device;
    float* F_grid_v_device;
    float* F_grid_m_device;
    int* F_used_device;

    float* C_grid_d_device;
    int* C_grid_A_device;
    int* C_grid_T_device;
    int* C_grid_surface_device;
    float* C_p_d_device;
    int* C_p_A_device;
    int* C_p_T_device;
    float* C_p_n_device;
    
    float* tool_vertices_device;
    unsigned int* tool_indices_device;
    float* tool_center_device;
    float* rigid_x_device;

    float* MC_SDF_device;
    int* n_MC_triangle_device;
    int* MC_et_device;
    float* MC_vertices_device;

    checkCudaErrors(cudaMalloc(&F_x_device, dim * n_particles * sizeof(float)));
    checkCudaErrors(cudaMalloc(&F_v_device, dim * n_particles * sizeof(float)));
    checkCudaErrors(cudaMalloc(&F_C_device, dim * dim * n_particles * sizeof(float)));
    checkCudaErrors(cudaMalloc(&F_dg_device, dim * dim * n_particles * sizeof(float)));
    checkCudaErrors(cudaMalloc(&F_Jp_device, n_particles * sizeof(float)));
    checkCudaErrors(cudaMalloc(&F_colors_device, 3 * n_particles * sizeof(float)));
    checkCudaErrors(cudaMalloc(&F_materials_device, n_particles * sizeof(int)));
    checkCudaErrors(cudaMalloc(&F_grid_v_device, dim * n_total_grid * sizeof(float)));
    checkCudaErrors(cudaMalloc(&F_grid_m_device, n_total_grid * sizeof(float)));
    checkCudaErrors(cudaMalloc(&F_used_device, n_particles * sizeof(int)));

    checkCudaErrors(cudaMalloc(&C_grid_d_device, n_total_grid * sizeof(float)));
    checkCudaErrors(cudaMalloc(&C_grid_A_device, n_total_grid * sizeof(int)));
    checkCudaErrors(cudaMalloc(&C_grid_T_device, n_total_grid * sizeof(int)));
    checkCudaErrors(cudaMalloc(&C_grid_surface_device, n_total_grid * sizeof(int)));
    checkCudaErrors(cudaMalloc(&C_p_d_device, n_particles * sizeof(float)));
    checkCudaErrors(cudaMalloc(&C_p_A_device, n_particles * sizeof(int)));
    checkCudaErrors(cudaMalloc(&C_p_T_device, n_particles * sizeof(int)));
    checkCudaErrors(cudaMalloc(&C_p_n_device, dim * n_particles * sizeof(float)));

    checkCudaErrors(cudaMalloc(&tool_vertices_device, 6 * tool_row * tool_col * sizeof(float)));
    checkCudaErrors(cudaMalloc(&tool_indices_device, 6 * (tool_row - 1) * tool_col * sizeof(unsigned int)));
    checkCudaErrors(cudaMalloc(&tool_center_device, dim * sizeof(float)));
    checkCudaErrors(cudaMalloc(&rigid_x_device, dim * n_rigid_particles * sizeof(float)));

    checkCudaErrors(cudaMalloc(&MC_SDF_device, n_MC_total_grid * sizeof(float)));
    checkCudaErrors(cudaMalloc(&n_MC_triangle_device, sizeof(int)));
    checkCudaErrors(cudaMalloc(&MC_et_device, 256 * 12 * sizeof(int)));
    checkCudaErrors(cudaMalloc(&MC_vertices_device, dim * n_MC_total_grid * 15 * sizeof(float)));
    
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "LearnOpenGL", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }
    glEnable(GL_DEPTH_TEST);

    create_sphere();
    Shader sphereShader("instancing.vs", "instancing.fs");
    unsigned int sphere_offset_buffer;
    glGenBuffers(1, &sphere_offset_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, sphere_offset_buffer);
    glBufferData(GL_ARRAY_BUFFER, n_particles * sizeof(glm::vec3), &F_x_host[0], GL_DYNAMIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    unsigned int sphere_color_buffer;
    glGenBuffers(1, &sphere_color_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, sphere_color_buffer);
    glBufferData(GL_ARRAY_BUFFER, n_particles * sizeof(glm::vec3), &F_colors_host[0], GL_DYNAMIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    unsigned int sphere_VAO, sphere_VBO;
    glGenVertexArrays(1, &sphere_VAO);
    glGenBuffers(1, &sphere_VBO);
    glBindVertexArray(sphere_VAO);
    glBindBuffer(GL_ARRAY_BUFFER, sphere_VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(sphere_vertices), sphere_vertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    
    glEnableVertexAttribArray(2);
    glBindBuffer(GL_ARRAY_BUFFER, sphere_offset_buffer);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glVertexAttribDivisor(2, 1);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glEnableVertexAttribArray(3);
    glBindBuffer(GL_ARRAY_BUFFER, sphere_color_buffer);
    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glVertexAttribDivisor(3, 1);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    unsigned int sphere_EBO;
    glGenBuffers(1, &sphere_EBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, sphere_EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(sphere_indices), sphere_indices, GL_STATIC_DRAW);

    sphereShader.use();
    glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
    sphereShader.setMat4("projection", projection);
    sphereShader.setVec3("lightPos", lightPos);
    sphereShader.setVec3("lightColor", lightColor);
    glm::mat4 model = glm::mat4(1.0f);
    sphereShader.setMat4("model", model);

    create_tool();
    Shader toolShader("tool.vs", "tool.fs");
    unsigned int tool_VAO, tool_VBO;
    glGenVertexArrays(1, &tool_VAO);
    glGenBuffers(1, &tool_VBO);
    glBindVertexArray(tool_VAO);
    glBindBuffer(GL_ARRAY_BUFFER, tool_VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(tool_vertices_host), tool_vertices_host, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    unsigned int tool_EBO;
    glGenBuffers(1, &tool_EBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, tool_EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(tool_indices_host), tool_indices_host, GL_STATIC_DRAW);
    toolShader.use();
    toolShader.setMat4("projection", projection);
    toolShader.setVec3("lightPos", lightPos);
    toolShader.setVec3("lightColor", lightColor);
    toolShader.setVec3("objectColor", 0.7f, 0.8f, 0.7f);
    toolShader.setMat4("model", model);


    Shader mcShader("MCShader.vs", "MCShader.fs");
    unsigned int mc_VAO, mc_VBO, mc_EBO;
    glGenVertexArrays(1, &mc_VAO);
    glGenBuffers(1, &mc_VBO);
    glBindVertexArray(mc_VAO);
    glBindBuffer(GL_ARRAY_BUFFER, mc_VBO);
    glBufferData(GL_ARRAY_BUFFER,sizeof(MC_vertices_host), MC_vertices_host, GL_DYNAMIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE,6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glGenBuffers(1, &mc_EBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mc_EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * n_MC_total_grid * 12, MC_indices, GL_DYNAMIC_DRAW);
    
    mcShader.setMat4("projection", projection);
    mcShader.setVec3("lightPos", lightPos);
    mcShader.setVec3("lightColor", lightColor);
    mcShader.setVec3("objectColor", 0.7f, 0.8f, 0.7f);
    mcShader.setMat4("model", model);

    int frame = 0;
    double render_time = 0.0;
    double grid_clear_time = 0.0;
    double p2g_time = 0.0;
    double boundary_condition_time = 0.0;
    double g2p_time = 0.0;

    checkCudaErrors(cudaMemcpy(tool_center_device, tool_center_host, dim * sizeof(float), cudaMemcpyHostToDevice));
    checkCudaErrors(cudaMemcpy(tool_vertices_device, tool_vertices_host, 6 * tool_row * tool_col * sizeof(float), cudaMemcpyHostToDevice));
    checkCudaErrors(cudaMemcpy(tool_indices_device, tool_indices_host, 6 * (tool_row - 1) * tool_col * sizeof(unsigned int), cudaMemcpyHostToDevice));
    checkCudaErrors(cudaMemcpy(MC_et_device, MC_et_host, 256 * 12 * sizeof(int), cudaMemcpyHostToDevice));
    init_particle << <n_particle_blocks, n_threads >> > (
        n_particles,
        F_x_device,
        F_v_device,
        F_C_device,
        F_dg_device,
        F_Jp_device,
        F_colors_device,
        F_materials_device,
        F_used_device
        );
    init_chuiti();
    checkCudaErrors(cudaMemcpy(F_x_device, F_x_host, dim* n_particles * sizeof(float), cudaMemcpyHostToDevice));

    init_rigid_particles << <n_rigid_blocks, n_threads >> > (
        n_rigid_particles,
        tool_vertices_device,
        tool_indices_device,
        tool_center_device,
        rigid_x_device
        );

    n_MC_triangle_host = new(int);
    while (!glfwWindowShouldClose(window)) {
        frame++;
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;
        processInput(window);
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        
        for (int s = 0; s < step; s++) {
            if (tool_center_host[1] > -0.1) {
                tool_center_host[1] -= dt;
                checkCudaErrors(cudaMemcpy(tool_center_device, tool_center_host, dim * sizeof(float), cudaMemcpyHostToDevice));
                rigid_move << <n_rigid_blocks, n_threads >> > (
                    n_rigid_particles,
                    rigid_x_device,
                    dt
                    );
                tool_vertices_move << <n_tool_vertices, n_threads >> > (
                    n_tool_vertices,
                    tool_vertices_device,
                    dt
                    );
            }
            
            grid_clear << <n_grid_blocks, n_threads >> > (
                n_total_grid,
                F_grid_v_device,
                F_grid_m_device,
                C_grid_d_device,
                C_grid_A_device,
                C_grid_T_device,
                C_grid_surface_device
                );

            grid_CDF << <n_rigid_blocks, n_threads >> > (
                n_rigid_particles,
                tool_vertices_device,
                tool_indices_device,
                tool_center_device,
                rigid_x_device,
                dx,
                inv_dx,
                C_grid_d_device,
                C_grid_A_device,
                C_grid_T_device,
                C_grid_surface_device,
                n_grid
                );

            p2g << <n_particle_blocks, n_threads >> > (
                F_x_device,
                F_v_device,
                F_C_device,
                F_dg_device,
                F_Jp_device,
                F_colors_device,
                F_materials_device,
                F_grid_v_device,
                F_grid_m_device,
                F_used_device,
                dt,
                n_particles,
                dx,
                p_rho,
                p_vol,
                p_mass,
                nu,
                mu_0,
                lambda_0,
                WATER,
                JELLY,
                SNOW,
                C_grid_d_device,
                C_grid_A_device,
                C_grid_T_device,
                C_grid_surface_device,
                C_p_d_device,
                C_p_A_device,
                C_p_T_device,
                C_p_n_device,
                tool_center_device,
                n_grid
                );

            boundary_condition << <n_grid_blocks, n_threads >> > (
                n_total_grid,
                F_grid_v_device,
                F_grid_m_device,
                dt,
                n_grid,
                bound,
                inv_dx
                );

            g2p << <n_particle_blocks, n_threads >> > (
                F_x_device,
                F_C_device,
                F_v_device,
                F_colors_device,
                F_materials_device,
                F_used_device,
                F_grid_v_device,
                F_grid_m_device,
                n_particles,
                n_grid,
                dx,
                dt,
                C_grid_d_device,
                C_grid_A_device,
                C_grid_T_device,
                C_grid_surface_device,
                C_p_d_device,
                C_p_A_device,
                C_p_T_device,
                C_p_n_device,
                tool_center_device,
                p_mass    
                );
        }
        
        checkCudaErrors(cudaMemcpy(F_x_host, F_x_device, dim * n_particles * sizeof(float), cudaMemcpyDeviceToHost));
        checkCudaErrors(cudaMemcpy(F_colors_host, F_colors_device, dim * n_particles * sizeof(float), cudaMemcpyDeviceToHost));
        //checkCudaErrors(cudaMemcpy(tool_center_host, tool_center_device, dim * sizeof(float), cudaMemcpyDeviceToHost));
        glm::mat4 view = camera.GetViewMatrix();
        sphereShader.use();
        sphereShader.setMat4("view", view);
        glBindVertexArray(sphere_VAO);
        glBindBuffer(GL_ARRAY_BUFFER, sphere_offset_buffer);
        glBufferSubData(GL_ARRAY_BUFFER, 0, n_particles * sizeof(glm::vec3), F_x_host);
        glBindBuffer(GL_ARRAY_BUFFER, sphere_color_buffer);
        glBufferSubData(GL_ARRAY_BUFFER, 0, n_particles * sizeof(glm::vec3), F_colors_host);
        //glDrawElementsInstanced(GL_TRIANGLES, 6 * (sphere_row - 1) * sphere_col, GL_UNSIGNED_INT, 0, n_particles);

        toolShader.use();
        toolShader.setMat4("view", view);
        toolShader.setVec3("aOffset", tool_center_host[0] - tool_center_init[0], tool_center_host[1] - tool_center_init[1], tool_center_host[2] - tool_center_init[2]);
        glBindVertexArray(tool_VAO);
       glDrawElements(GL_TRIANGLES, 6 * (tool_row - 1) * tool_col, GL_UNSIGNED_INT, 0);

        checkCudaErrors(cudaMemcpy(rigid_x_host, rigid_x_device, dim * n_rigid_particles * sizeof(float), cudaMemcpyDeviceToHost));
        sphereShader.use();
        sphereShader.setMat4("view", view);
        glBindVertexArray(sphere_VAO);
        glBindBuffer(GL_ARRAY_BUFFER, sphere_offset_buffer);
        glBufferSubData(GL_ARRAY_BUFFER, 0, n_rigid_particles * sizeof(glm::vec3), rigid_x_host);
        glBindBuffer(GL_ARRAY_BUFFER, sphere_color_buffer);
        glBufferSubData(GL_ARRAY_BUFFER, 0, n_particles * sizeof(glm::vec3), F_colors_host);
        glDrawElementsInstanced(GL_TRIANGLES, 6 * (sphere_row - 1) * sphere_col, GL_UNSIGNED_INT, 0, n_rigid_particles);


        
        *n_MC_triangle_host = 0;
        //std::cout << *n_MC_triangle_host << std::endl;
        checkCudaErrors(cudaMemcpy(n_MC_triangle_device, n_MC_triangle_host, sizeof(int), cudaMemcpyHostToDevice));
        compute_implicit_face << <n_MC_grid_blocks, n_threads >> > (
            n_MC_grid,
            n_MC_total_grid,
            n_particles,
            MC_dx,
            F_x_device,
            MC_SDF_device
            );
        implicit_to_explicit << <n_MC_grid_blocks, n_threads >> > (
            n_MC_grid,
            n_MC_total_grid,
            MC_SDF_device,
            n_MC_triangle_device,
            MC_et_device,
            MC_vertices_device,
            MC_dx
            );
        checkCudaErrors(cudaMemcpy(MC_vertices_host, MC_vertices_device, dim * 15 * n_MC_total_grid *sizeof(float), cudaMemcpyDeviceToHost));
        checkCudaErrors(cudaMemcpy(n_MC_triangle_host, n_MC_triangle_device, sizeof(int), cudaMemcpyDeviceToHost));
       /* sphereShader.use();
        sphereShader.setMat4("view", view);
        sphereShader.use();
        sphereShader.setMat4("view", view);
        glBindVertexArray(sphere_VAO);
        glBindBuffer(GL_ARRAY_BUFFER, sphere_offset_buffer);
        glBufferSubData(GL_ARRAY_BUFFER, 0, n_particles * sizeof(glm::vec3), MC_vertices_host);
        glBindBuffer(GL_ARRAY_BUFFER, sphere_color_buffer);
        glBufferSubData(GL_ARRAY_BUFFER, 0, n_particles * sizeof(glm::vec3), F_colors_host);
        glDrawElementsInstanced(GL_TRIANGLES, 6 * (sphere_row - 1) * sphere_col, GL_UNSIGNED_INT, 0, n_particles);*/

        mcShader.use();
        projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
        mcShader.setMat4("projection", projection);
        mcShader.setVec3("lightPos", lightPos);
        mcShader.setVec3("lightColor", lightColor);
        mcShader.setVec3("objectColor", 0.8f, 0.3f, 0.3f);
        model = glm::mat4(1.0f);
        mcShader.setMat4("model", model);
        view = camera.GetViewMatrix();
        mcShader.setMat4("view", view);
        
        smooth_normal();
        glBindVertexArray(mc_VAO);
        glBindBuffer(GL_ARRAY_BUFFER, mc_VBO);
        glBufferSubData(GL_ARRAY_BUFFER, 0, 3 * *n_MC_triangle_host * 6 *sizeof(float), MC_vertices_host);
        ////glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        //glDrawArrays(GL_TRIANGLES, 0, 3 * *n_MC_triangle_host);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mc_EBO);
        glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, sizeof(unsigned int)* 3 * *n_MC_triangle_host, MC_indices);
        glDrawElements(GL_TRIANGLES, 3 * *n_MC_triangle_host, GL_UNSIGNED_INT, 0);
            
        
        glfwSwapBuffers(window);
        glfwPollEvents();
        
    }
    printf("------------------------------------\n");
    printf("-grid_clear:                %.6f\n", grid_clear_time / frame);
    printf("-p2g:                       %.6f\n", p2g_time / frame);
    printf("-boundary_condition:        %.6f\n", boundary_condition_time / frame);
    printf("-g2p:                       %.6f\n", g2p_time / frame);
    printf("-render:                    %.6f\n", render_time / frame);
    printf("-total:                    %.6f\n", (grid_clear_time + p2g_time + boundary_condition_time + g2p_time + render_time) / frame);
    printf("------------------------------------\n");

    checkCudaErrors(cudaFree(F_x_device));
    checkCudaErrors(cudaFree(F_v_device));
    checkCudaErrors(cudaFree(F_C_device));
    checkCudaErrors(cudaFree(F_dg_device));
    checkCudaErrors(cudaFree(F_Jp_device));
    checkCudaErrors(cudaFree(F_colors_device));
    checkCudaErrors(cudaFree(F_materials_device));
    checkCudaErrors(cudaFree(F_grid_v_device));
    checkCudaErrors(cudaFree(F_grid_m_device));
    checkCudaErrors(cudaFree(F_used_device));

    checkCudaErrors(cudaFree(C_grid_d_device));
    checkCudaErrors(cudaFree(C_grid_A_device));
    checkCudaErrors(cudaFree(C_grid_T_device));
    checkCudaErrors(cudaFree(C_grid_surface_device));
    checkCudaErrors(cudaFree(C_p_d_device));
    checkCudaErrors(cudaFree(C_p_A_device));
    checkCudaErrors(cudaFree(C_p_T_device));
    checkCudaErrors(cudaFree(C_p_n_device));

    checkCudaErrors(cudaFree(tool_vertices_device));
    checkCudaErrors(cudaFree(tool_indices_device));
    checkCudaErrors(cudaFree(tool_center_device));
    checkCudaErrors(cudaFree(rigid_x_device));

    checkCudaErrors(cudaFree(MC_SDF_device));
    checkCudaErrors(cudaFree(n_MC_triangle_device));
    checkCudaErrors(cudaFree(MC_et_device));
    checkCudaErrors(cudaFree(MC_vertices_device));

    free(n_MC_triangle_host);
 
    glDeleteVertexArrays(1, &sphere_VAO);
    glDeleteBuffers(1, &sphere_VBO);
    glDeleteBuffers(1, &sphere_EBO);
    glDeleteBuffers(1, &sphere_offset_buffer);
    glDeleteBuffers(1, &sphere_color_buffer);
    glDeleteVertexArrays(1, &tool_VAO);
    glDeleteBuffers(1, &tool_VBO);
    glDeleteBuffers(1, &tool_EBO);
    glfwTerminate();

    return 0;
}


void create_sphere() {
    float r = 0.008;
    for (int i = 0; i < sphere_row; i++) {
        float theta = i * glm::pi<float>() / sphere_row;
        for (int j = 0; j < sphere_col; j++) {
            float phi = j * 2 * glm::pi<float>() / sphere_col;
            float x = r * sinf(theta) * cosf(phi);
            float y = r * sinf(theta) * sinf(phi);
            float z = r * cosf(theta);
            int index = (i * sphere_col + j) * 6;
            sphere_vertices[index + 0] = x;
            sphere_vertices[index + 1] = y;
            sphere_vertices[index + 2] = z;
            glm::vec3 norm = glm::normalize(glm::vec3(x, y, z));
            sphere_vertices[index + 3] = norm.x;
            sphere_vertices[index + 4] = norm.y;
            sphere_vertices[index + 5] = norm.z;
        }
    }
    for (int i = 0; i < sphere_row - 1; i++) {
        for (int j = 0; j < sphere_col; j++) {
            int index = (i * sphere_col + j) * 6;
            int temp = i * sphere_col + j;
            sphere_indices[index + 0] = temp;
            if ((temp + 1) % sphere_col == 0) {
                sphere_indices[index + 1] = i * sphere_col;
                sphere_indices[index + 2] = (i + 1) * sphere_col;
            }
            else {
                sphere_indices[index + 1] = temp + 1;
                sphere_indices[index + 2] = temp + 1 + sphere_col;
            }
            sphere_indices[index + 3] = temp;
            sphere_indices[index + 4] = sphere_indices[index + 2];
            sphere_indices[index + 5] = temp + sphere_col;
        }
    }
}

void create_tool() {
    float r = 0.1;
    //float center[3]{ 0.0f, 0.0f, 0.0f };
    for (int i = 0; i < tool_row; i++) {
        float phi = (8.0 - i) / 14.0 * glm::pi<float>();
        for (int j = 0; j < tool_col; j++) {
            int index = (i * tool_col + j) * 6;
            float theta = 2.0f * j / tool_col * glm::pi<float>();
            float x = r * sin(phi) * cos(theta);
            float z = r * sin(phi) * sin(theta);
            float y = r * cos(phi);
            tool_vertices_host[index + 0] = x + tool_center_host[0];
            tool_vertices_host[index + 1] = y + tool_center_host[1];
            tool_vertices_host[index + 2] = z + tool_center_host[2];
            glm::vec3 norm = glm::normalize(glm::vec3(x, y, z));
            tool_vertices_host[index + 3] = norm.x;
            tool_vertices_host[index + 4] = norm.y;
            tool_vertices_host[index + 5] = norm.z;
        }
    }
    for (int i = 0; i < tool_row - 1; i++) {
        for (int j = 0; j < tool_col; j++) {
            int index = (i * tool_col + j) * 6;
            int temp = i * tool_col + j;
            tool_indices_host[index + 0] = temp;
            if ((temp + 1) % tool_col == 0) {
                tool_indices_host[index + 1] = i * tool_col;
                tool_indices_host[index + 2] = (i + 1) * tool_col;
            }
            else {
                tool_indices_host[index + 1] = temp + 1;
                tool_indices_host[index + 2] = temp + 1 + tool_col;
            }
            tool_indices_host[index + 3] = temp;
            tool_indices_host[index + 4] = tool_indices_host[index + 2];
            tool_indices_host[index + 5] = temp + tool_col;
        }
    }
}

