#pragma once

#include "simulation_type.h"
#include "constraint_PD.h"
#include "constraint_PBD.h"
#include <vector>
#include <memory>

class PhyMesh;
typedef PhyMesh*                 PhyMesh_rptr;
typedef std::unique_ptr<PhyMesh> PhyMesh_uptr;
typedef std::shared_ptr<PhyMesh> PhyMesh_sptr;

class PhyMesh {
public:
	Size_type vert_size;
	Size_type ele_size;
	Size_type edge_size;
	Size_type constraint_PD_size;
	Size_type constraint_PBD_size;

	VectorX_type position{}; // 3*N
	Elements4_type elements{}; // 4*M_ele
	Edges_type edges{}; // 2*M_edge
	VectorX_type f_ext{}; // 3*N
	VectorX_type velocity{}; // 3*N
	VectorX_type mass{}; // N
	VectorX_type inv_mass{}; // N

	std::vector<std::unique_ptr<SimPD::StrainConstraint>> constraints_PD;
	std::vector<std::unique_ptr<SimPBD::Constraint>> constraints_PBD;

	PhyMesh(const std::vector<float>& position, Size_type N, 
		const std::vector<uint32_t>& edges, Size_type E,
		const std::vector<uint32_t>& elements, Size_type T,
		Scalar_type g, Scalar_type m) {
		this->mass.resize(N); this->mass.setZero();
		this->inv_mass.resize(N); this->inv_mass.setZero();
		this->f_ext.resize(N * 3); this->f_ext.setZero();
		for (Size_type i = 0; i < N; i++) {
			this->f_ext.block<3, 1>(i * 3, 0) = Vec3_type{ 0, -g * m, 0 };
			this->mass[i] = m;
			if (m != 0) this->inv_mass[i] = 1.0 / m;
		}
		this->velocity.resize(N * 3); this->velocity.setZero();
		setup_position(position, N);
		setup_edges(edges, E),
		setup_elements(elements, T);
	}

	void setup_position(const std::vector<float>& position, Size_type N) {
		this->position.resize(N * 3);
		Vec3_type pos;
		for (Size_type i = 0; i < N; i++) {
			pos = { position[i * 3 + 0],position[i * 3 + 1] , position[i * 3 + 2] };
			this->position.block<3, 1>(i * 3, 0) = pos; // can we make this one line?
		}
		this->vert_size = N;
	}

	void setup_elements(const std::vector<uint32_t>& elements, Size_type E) {
		for (Size_type i = 0; i < E; i++) {
			this->elements.push_back({
				elements[i * 4 + 0], elements[i * 4 + 1],
				elements[i * 4 + 2], elements[i * 4 + 3]});
		}
		this->ele_size = E;
	}

	void setup_edges(const std::vector<uint32_t>& edges, Size_type E) {
		for (Size_type i = 0; i < E; i++) {
			this->edges.push_back({
				edges[i * 2 + 0], edges[i * 2 + 1] });
		}
		this->edge_size = E;
	}

	void setup_constraint_PD(Scalar_type sigma_min, Scalar_type sigma_max, Scalar_type k) {
		for (size_t i = 0; i < ele_size; i++) {
			auto [v1, v2, v3, v4] = this->elements[i];
			auto constraint = std::make_unique<SimPD::StrainConstraint>(
				std::initializer_list<Index_type>{v1, v2, v3, v4}, 
				k, position, sigma_min, sigma_max);
			this->constraints_PD.push_back(std::move(constraint));
			this->constraint_PD_size++;
		}
	}

	void setup_constraint_Edge_PBD(Scalar_type stiff) {
		for (size_t i = 0; i < edge_size; i++) {
			auto [e1, e2] = this->edges[i];
			Scalar_type mass_1 = this->mass[e1];
			Scalar_type mass_2 = this->mass[e2];
			auto constraint = std::make_unique<SimPBD::EdgeConstraint>(
				std::initializer_list<Index_type>{e1, e2},
				stiff, this->position);
			this->constraints_PBD.push_back(std::move(constraint));
			this->constraint_PBD_size++;
		}
	}
	void setup_constraint_Volume_PBD(Scalar_type stiff) {
		for (size_t i = 0; i < ele_size; i++) {
			auto [v1, v2, v3, v4] = this->elements[i];
			Scalar_type mass_1 = this->mass[v1];
			Scalar_type mass_2 = this->mass[v2];
			Scalar_type mass_3 = this->mass[v3];
			Scalar_type mass_4 = this->mass[v4];
			auto constraint = std::make_unique<SimPBD::TetVolumeConstraint>(
				std::initializer_list<Index_type>{v1, v2, v3, v4},
				stiff, this->position);
			this->constraints_PBD.push_back(std::move(constraint));
			this->constraint_PBD_size++;
		}
	}
	void setup_constraint_Corotated_PBD(Scalar_type stiff) {
		for (size_t i = 0; i < ele_size; i++) {
			auto [v1, v2, v3, v4] = this->elements[i];
			Scalar_type mass_1 = this->mass[v1];
			Scalar_type mass_2 = this->mass[v2];
			Scalar_type mass_3 = this->mass[v3];
			Scalar_type mass_4 = this->mass[v4];
			auto constraint = std::make_unique<SimPBD::CorotatedConstraint>(
				std::initializer_list<Index_type>{v1, v2, v3, v4},
				stiff, this->position);
			this->constraints_PBD.push_back(std::move(constraint));
			this->constraint_PBD_size++;
		}
	}

};