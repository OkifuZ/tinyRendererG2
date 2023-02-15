#pragma once
#include "simulation_type.h"
#include "phymesh.h"
#include "constraint.h"
#include "util_physics.h"

#include <memory>

class PD_solver {
private:
	std::unique_ptr<Eigen::SimplicialCholesky<SparseMat_type>> cholesky_decomp_ptr;
	Scalar_type dt{}, dt2{}, dt_inv{}, dt2_inv{};

public:

	PhyMesh_rptr phymesh_ptr; // which type is better? 
	// I think it is raw pointer, PD_solver doesn't has phymesh_ptr, it "use" phymesh_ptr.
	// If type being unique_ptr, the user of PD_solver would has no elegant access to phymesh_ptr
	
	PD_solver(Scalar_type dt, PhyMesh_rptr phymesh_rptr) : phymesh_ptr(phymesh_rptr), 
		cholesky_decomp_ptr(std::move(std::make_unique <Eigen::SimplicialCholesky<SparseMat_type>>())) {
		this->set_dt(dt);
	}

	void reset(Scalar_type dt, PhyMesh_rptr phymesh_rptr) {
		this->set_dt(dt);
		this->phymesh_ptr = phymesh_rptr;
		cholesky_decomp_ptr = std::move(std::make_unique <Eigen::SimplicialCholesky<SparseMat_type>>());
		this->prefactor();
	}

	void set_dt(Scalar_type dt) {
		this->dt = dt;
		this->dt_inv = Scalar_type{ 1.0 } / dt;
		this->dt2 = dt * dt;
		this->dt2_inv = Scalar_type{ 1.0 } / (dt * dt);
	}

	void prefactor() {
		if (!phymesh_ptr || dt == 0) return;

		auto const& position = phymesh_ptr->position;
		auto const& mass = phymesh_ptr->mass;
		Size_type N = phymesh_ptr->vert_size;
		auto& constraints = phymesh_ptr->constraints;

		std::vector<Triplet_type> A_trip; // A = M/h^2 + L
		A_trip.reserve(3 * N);

		for (Index_type i = 0; i < N; i++) {
			// M/h^2
			A_trip.push_back({ 3 * i + 0, 3 * i + 0, mass(i) * dt2_inv });
			A_trip.push_back({ 3 * i + 1, 3 * i + 1, mass(i) * dt2_inv });
			A_trip.push_back({ 3 * i + 2, 3 * i + 2, mass(i) * dt2_inv });
		}

		for (const auto& constraint : constraints) {
			// L = \Sum (V * K * GG^T)_j
			std::vector<Triplet_type> V_K_GGT = constraint->get_v_k_G_GT(position);
			A_trip.insert(A_trip.end(), V_K_GGT.begin(), V_K_GGT.end());
		}

		SparseMat_type A(3 * N, 3 * N);
		A.setFromTriplets(A_trip.begin(), A_trip.end());

		// log_sparse_mat("../log/A_sparse.txt", A, "A sparse matrix");

		cholesky_decomp_ptr->compute(A);
	}

	void step(int iter_num) {
		if (!phymesh_ptr || dt == 0) return;

		const auto& constraints = phymesh_ptr->constraints;
		const auto& position = phymesh_ptr->position;
		const auto& velocity = phymesh_ptr->velocity;
		const auto& mass = phymesh_ptr->mass;
		const auto& fext = phymesh_ptr->f_ext;
		Size_type N = phymesh_ptr->vert_size;

		// y = q_t + dt*v_t + dt^2 * 1.0/M * fext_t
		Vector_type y_inertia(N * 3);
		for (size_t i = 0; i < N; i++) {
			auto x_ = i * 3 + 0;
			auto y_ = i * 3 + 1;
			auto z_ = i * 3 + 2;
			y_inertia[x_] = position[x_] + dt * velocity[x_] + dt2 * fext[x_] / mass[i];
			y_inertia[y_] = position[y_] + dt * velocity[y_] + dt2 * fext[y_] / mass[i];
			y_inertia[z_] = position[z_] + dt * velocity[z_] + dt2 * fext[z_] / mass[i];
		}

		// collision with groud
		for (size_t i = 0; i < N; i++) {
			auto y_ = i * 3 + 1;
			auto& y_pos = y_inertia[y_];
			if (y_pos < 0) y_pos = 0;
		}

		// f_inertia = M/dt^2 * y
		Vector_type f_inertia(N * 3);
		Mat3_type M;
		for (size_t i = 0; i < N; i++) {
			M.setZero();
			M(2, 2) = M(1, 1) = M(0, 0) = mass[i];
			auto const y_i = y_inertia.block(3 * i, 0, 3, 1);
			f_inertia.block(3 * i, 0, 3, 1) = dt2_inv * M * y_i;
		}

		// TODO: figure out why here needs an iterative-style solver
		Vector_type q = y_inertia; // x
		Vector_type b(N * 3); // b = M/dt^2 y + J * p = f_inertia + Jp
		for (int iter = 0; iter < iter_num; iter++) {
			b.setZero();
			for (const auto& constraint : constraints) {
				constraint->get_v_k_G_ST_p(q, b);
			}
			b += f_inertia;

			q = cholesky_decomp_ptr->solve(b); // Ax = b
		}
		
		phymesh_ptr->velocity = (q - phymesh_ptr->position) * dt_inv; // v_n+1 = q_n+1 - q_n
		phymesh_ptr->position = q;

	}

	// for testing
	void mock_step() {
		auto& position = phymesh_ptr->position;
		for (size_t i = 0; i < phymesh_ptr->vert_size; i++) {
			position.block<3, 1>(i * 3, 0) += Vec3_type{0.02f, 0, 0};
		}
	}

};