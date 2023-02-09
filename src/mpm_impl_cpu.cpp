#include "mpm_impl.h" // implement this header

#include <Eigen/core>

#include <cstdio>
#include <random>


void initialize(float* x, float* v, float* F, float* Jp, int* material, float* color, unsigned int n_particles) {

	std::random_device rd;
	std::mt19937 gen(rd());
	std::uniform_real_distribution<> dis(0, 1);//uniform distribution between 0 and 1

	int group_size;
	group_size = n_particles / 3;
	for (int i = 0; i < n_particles; i++) {
		float px, py;
		if (i / group_size == 0) {
			px = 0.05f + 0 * 0.3f + float(dis(gen)) * 0.25f;
			py = 0.05f + 1 * 0.3f + float(dis(gen)) * 0.25f;
			material[i] = 2;
			color[i * 3 + 0] = 1.0f; // r
			color[i * 3 + 1] = 0.976f; // g
			color[i * 3 + 2] = 0.976f; // b
		}
		if (i / group_size == 1) {
			px = 0.05f + 1 * 0.3f + float(dis(gen)) * 0.25f;
			py = 0.05f + 2 * 0.3f + float(dis(gen)) * 0.25f;
			material[i] = 0;
			color[i * 3 + 0] = 0.52f; // r
			color[i * 3 + 1] = 0.80f; // g
			color[i * 3 + 2] = 0.976f; // b
		}
		if (i / group_size == 2) {
			px = 0.05f + 2 * 0.3f + float(dis(gen)) * 0.25f;
			py = 0.05f + 1 * 0.3f + float(dis(gen)) * 0.25f;
			material[i] = 1;
			color[i * 3 + 0] = 0.99f; // r
			color[i * 3 + 1] = 0.7f; // g
			color[i * 3 + 2] = 0.2f; // b
		}

		x[i * 2 + 0] = px;
		x[i * 2 + 1] = py;
		v[i * 2 + 0] = v[i * 2 + 1] = 0.0f;
		F[i * 4 + 0] = F[i * 4 + 3] = 1.0f;
		F[i * 4 + 1] = F[i * 4 + 2] = 0.0f;
		Jp[i] = 1.0f;
	}
}

void inigrid_substep(float* grid_v, float* grid_m, unsigned int n_grid) {
	for (auto i = 0; i < n_grid; i++) {
		for (auto j = 0; j < n_grid; j++) {
			auto index = i * n_grid + j;
			grid_v[index * 2 + 0] = 0;
			grid_v[index * 2 + 1] = 0;
			grid_m[index] = 0;
		}
	}
}

inline void svd22_raw(const float* a, float* u, float* s, float* v) {
	// const float a[4], float u[4], float s[2], float v[4]

	s[0] = (sqrtf(powf(a[0] - a[3], 2) + pow(a[1] + a[2], 2)) + sqrtf(powf(a[0] + a[3], 2) + powf(a[1] - a[2], 2))) / 2;
	s[1] = fabsf(s[0] - sqrtf(pow(a[0] - a[3], 2) + pow(a[1] + a[2], 2)));
	v[2] = (s[0] > s[1]) ? sinf((atan2f(2 * (a[0] * a[1] + a[2] * a[3]), a[0] * a[0] - a[1] * a[1] + a[2] * a[2] - a[3] * a[3])) / 2) : 0;
	v[0] = sqrtf(1 - v[2] * v[2]);
	v[1] = -v[2];
	v[3] = v[0];
	u[0] = (s[0] != 0) ? (a[0] * v[0] + a[1] * v[2]) / s[0] : 1;
	u[2] = (s[0] != 0) ? (a[2] * v[0] + a[3] * v[2]) / s[0] : 0;
	u[1] = (s[1] != 0) ? (a[0] * v[1] + a[1] * v[3]) / s[1] : -u[2];
	u[3] = (s[1] != 0) ? (a[2] * v[1] + a[3] * v[3]) / s[1] : u[0];
}

void p2g_substep(
	float* x, float* v, float* F, float* C, int* material, float* Jp, unsigned int n_particles,
	float* grid_v, float* grid_m, unsigned int n_grid,
	float dx, float inv_dx, float dt, float mu_0, float lambda_0, float p_vol, float p_mass)
{
	for (auto p = 0; p < n_particles; p++) {
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
		h = std::exp(10.0f * (1.0f - Jp[p]));
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
		svd22_raw(A, U_data, S_data, V_data);
		Eigen::Matrix2f U, V;
		U << U_data[0], U_data[1], U_data[2], U_data[3];
		V << V_data[0], V_data[1], V_data[2], V_data[3];

		float J = 1.0f;
		for (int i = 0; i < 2; i++) {
			float new_sig = S_data[i];
			if (material[p] == 2)
				new_sig = std::min(std::max(S_data[i], 1.0f - 2.5e-2f), 1.0f + 4.5e-3f);
			Jp[p] *= S_data[i] / new_sig;
			S_data[i] = new_sig;
			J *= new_sig;

		}
		if (material[p] == 0) {
			temp_F = identity * std::sqrt(J);
		}
		else if (material[p] == 2) {
			Eigen::Matrix2f sig22;
			sig22 << S_data[0], 0, 0, S_data[1];
			temp_F = U * sig22 * V.transpose();
		}

		// Eigen solution, what we used earlier
		/*Eigen::JacobiSVD<Eigen::MatrixXf> svd(temp_F, Eigen::ComputeFullU | Eigen::ComputeFullV);
		Eigen::Matrix2f U, V;
		U = svd.matrixU();
		Eigen::Matrix2Xf sig(2, 1);
		sig = svd.singularValues();
		V = svd.matrixV();

		float J = 1.0f;
		for (int i = 0; i < 2; i++) {
			float new_sig = sig(i, 0);
			if (material[p] == 2)
				new_sig = std::min(std::max(sig(i, 0), 1.0f - 2.5e-2f), 1.0f + 4.5e-3f);
			Jp[p] *= sig(i, 0) / new_sig;
			sig(i, 0) = new_sig;
			J *= new_sig;

		}
		if (material[p] == 0) {
			temp_F = identity * std::sqrt(J);
		}
		else if (material[p] == 2) {
			Eigen::Matrix2f sig22;
			sig22 << sig(0, 0), 0, 0, sig(1, 0);
			temp_F = U * sig22 * V.transpose();
		}*/

		Eigen::Matrix2f stress, affine;
		stress = 2 * mu * (temp_F - U * V.transpose()) * temp_F.transpose() + identity * la * J * (J - 1);
		stress = (-dt * p_vol * 4 * inv_dx * inv_dx) * stress;
		affine = stress + p_mass * temp_C;
		// SVD-related end


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
				grid_v[index * 2 + 0] += temp_gv(0);
				grid_v[index * 2 + 1] += temp_gv(1);
				grid_m[index] += weight * p_mass;
			}
		}

		std::copy(temp_F.data(), temp_F.data() + 4, F + 4 * p);
	}
}


void boundary_substep(float* grid_v, float* grid_m, unsigned int n_grid,
	float dt, float gravity)
{
	for (auto i = 0; i < n_grid; i++) {
		for (auto j = 0; j < n_grid; j++) {
			auto index = i * n_grid + j;
			if (grid_m[index] > 0) {
				grid_v[index * 2 + 0] = (1 / grid_m[index]) * grid_v[index * 2 + 0];
				grid_v[index * 2 + 1] = (1 / grid_m[index]) * grid_v[index * 2 + 1];
				//grid_v[index * 2 + 0] -= dt * gravity * 1.0f;
				grid_v[index * 2 + 1] -= dt * gravity * 1.0f;
				if (i < 3 && grid_v[index * 2] < 0) {
					grid_v[index * 2] = 0;
				}
				if (i > n_grid - 3 && grid_v[index * 2] > 0) {
					grid_v[index * 2] = 0;
				}
				if (j < 3 && grid_v[index * 2 + 1] < 0) {
					grid_v[index * 2 + 1] = 0;
				}
				if (j > n_grid - 3 && grid_v[index * 2 + 1] > 0) {
					grid_v[index * 2 + 1] = 0;
				}
			}
		}
	}
}


void g2p_substep(float* x, float* v, float* C, unsigned int n_particles,
	float* grid_v, float* grid_m, unsigned int n_grid,
	float dt, float inv_dx)
{
	for (auto p = 0; p < n_particles; p++) {

		Eigen::Vector2i base{ int(x[p * 2 + 0] * inv_dx - 0.5f),
							  int(x[p * 2 + 1] * inv_dx - 0.5f) };

		Eigen::Vector2f fx =
			Eigen::Vector2f(x[p * 2 + 0], x[p * 2 + 1]) * inv_dx - base.cast<float>();

		std::vector<Eigen::Vector2f> w{
			0.5f * (1.5f - fx.array()) * (1.5f - fx.array()),
			0.75f - (fx.array() - 1.0f) * (fx.array() - 1.0f),
			0.5f * (fx.array() - 0.5f) * (fx.array() - 0.5f)
		};

		Eigen::Vector2f new_v; new_v.setZero();
		Eigen::Matrix2f new_c; new_c.setZero();
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
		std::copy(new_v.data(), new_v.data() + 2, v + 2 * p);
		std::copy(new_c.data(), new_c.data() + 4, C + 4 * p);
		x[p * 2 + 0] += dt * v[p * 2 + 0];
		x[p * 2 + 1] += dt * v[p * 2 + 1];
	}
}


